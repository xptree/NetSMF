#include "WeightGraphWalker.h"

#include <cassert>
#include <numeric>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <omp.h>

// include gflags
#include <gflags/gflags.h>
DECLARE_int32(num_threads_svd); DECLARE_int32(rank);
DECLARE_int32(negative);
DECLARE_string(output_svd);

using namespace log4cxx;

WeightGraphWalker::WeightGraphWalker(const std::vector<VertexId>& indices,
        const std::vector<VertexId>& indptr, int T,
        const std::vector<float>& data_,
        const std::vector<float>& prefix_sum_,
        const std::vector<float>& degree)
    : GraphWalker(indices, indptr, degree, T), data(data_), prefix_sum(prefix_sum_) {
    LOG4CXX_INFO(logger, "weighted network");
}

WeightGraphWalker* WeightGraphWalker::getWalker(const std::string& fname, int T) {
    std::vector<VertexId> edges;
	std::vector<VertexId> out_degree;
	std::vector<VertexId> edge_pair;
    std::vector<float> weight;
    std::vector<float> generalized_out_degree;

    VertexId max_vertex_id = 0;
	std::ifstream fin(fname);
	assert(fin.is_open());
    VertexId src, dst;
    double w;
	while (fin >> src >> dst >> w) {
		// vertexid overflow
		if (src >= max_vertex_id || dst >= max_vertex_id) {
			max_vertex_id = std::max(src, dst);
			out_degree.resize(max_vertex_id + 1, 0);
            generalized_out_degree.resize(max_vertex_id + 1, 0);
		}
        if (src == dst) {
            continue;
        }
		++out_degree[src];
        generalized_out_degree[src] += w;
        edge_pair.push_back(src);
        edge_pair.push_back(dst);
        weight.push_back(w);
	}


    int num_vertex = max_vertex_id + 1;

    std::vector<VertexId> indptr(num_vertex + 1, 0);
    std::partial_sum(out_degree.begin(), out_degree.end(), indptr.begin() + 1);

    EdgeId edge_cnt = edge_pair.size() >> 1;
    std::vector<VertexId> indices(edge_cnt, 0);

    std::vector<float> data(edge_cnt, 0.0);

    for (EdgeId e = 0; e < edge_cnt; ++e) {
        VertexId src = edge_pair[e << 1];
        VertexId dst = edge_pair[(e << 1) + 1];

        EdgeId idx = indptr[src] + (--out_degree[src]);
        indices[idx] = dst;
        data[idx] = weight[e];
    }


    std::vector<float> prefix_sum(edge_cnt, 0.0);
    for (VertexId v = 0; v < max_vertex_id; ++v) {
        std::partial_sum(data.begin() + indptr[v], data.begin() + indptr[v + 1], prefix_sum.begin() + indptr[v]);
    }

    return new WeightGraphWalker(indices, indptr, T, data, prefix_sum, generalized_out_degree);
}

VertexId WeightGraphWalker::randomWalk(VertexId u, int step, double& Z,
        unsigned* seed) const {
    for (;step--;) {
        // u's neighbors are indices[indptr[i]:indptr[i+1]]
        double ratio = (double)rand_r(seed) / RAND_MAX;
        int head = indptr[u], tail = indptr[u+1] - 1, pos = tail;
        double generalized_out_degree = prefix_sum[tail];
        for (;head < tail;) {
            int mid = (head + tail) >> 1;
            if (prefix_sum[mid] >= ratio * generalized_out_degree) {
                tail= mid - 1;
                pos = mid;
            } else {
                head = mid + 1;
            }
        }

        u = indices[pos];
        Z += 1. / data[pos];
    }
    return u;
}

void WeightGraphWalker::samplePath(VertexId u, VertexId v, double w, int r, unsigned* seed,
        std::vector<ValuedVertexPair>& sampled_pair) const {
    int k = rand_r(seed) % r + 1;
    double Z_half = 1. / w;
    VertexId u_ = randomWalk(u, k - 1, Z_half, seed);
    VertexId v_ = randomWalk(v, r - k, Z_half, seed);
    if (u_ > v_) {
        std::swap(u_, v_);
    }

    // add record (u_, v_, r / Z_half)
    sampled_pair.push_back(std::make_pair(std::make_pair(u_, v_), float(r / Z_half)));
}

void WeightGraphWalker::sampling(int round, int num_threads,
        const std::string& machine,
        int check_point) {
    omp_set_num_threads(num_threads);

    std::vector<std::vector<ValuedVertexPair>*> counters;
    for (int i = 0; i < num_threads; ++i) {
        counters.push_back(new std::vector<ValuedVertexPair>);
    }

    #pragma omp parallel default(shared)
    {
        int this_thread = omp_get_thread_num();
        std::string thread_name = std::string("machine_") + machine
            + std::string("_thread_") + std::to_string(this_thread);

        LOG4CXX_INFO(logger, "[thread " << this_thread << "]" << " thread name is " << thread_name);
        unsigned seed = std::hash<std::string>{}(thread_name);

        std::vector<ValuedVertexPair> sampled_pairs;
        std::vector<ValuedVertexPair> *&counter = counters[this_thread];
        std::vector<ValuedVertexPair> *counter_tmp = new std::vector<ValuedVertexPair>;

        LOG4CXX_INFO(logger, "[thread " << this_thread << "]" << " set seed " << seed);
        int my_round= ceil((double)round / num_threads);

        for (int i=0; i<my_round; ++i) {
            for (VertexId u=0; u+1 < indptr.size(); ++u) {
                for (size_t j=indptr[u]; j<indptr[u+1]; ++j) {
                    VertexId v = indices[j];
                    for (int r=1; r<T; ++r) {
                        samplePath(u, v, data[j], r, &seed, sampled_pairs);
                    }
                }
            }
            if ((i + 1) % check_point == 0 || i + 1 == my_round) {
                float max_val = merge(*counter, *counter_tmp, sampled_pairs);
                std::swap(counter, counter_tmp);
                counter_tmp->clear();
                LOG4CXX_INFO(logger, "[thread " << this_thread << "]" << " complete " << i + 1 << " rounds, size of counter=" << counter->size() << " counter.max_val=" << max_val);
            }
        }

        LOG4CXX_INFO(logger, "[thread " << this_thread << "] finish job");
        delete counter_tmp;
    }

    // now we have a list of counters, we want to merge them in a binary tree way --- from leaf to root
    while (counters.size() > 1) {
        LOG4CXX_INFO(logger, counters.size() << " counters to merge.");
        size_t n_half = (counters.size() + 1) >> 1;
        omp_set_num_threads(counters.size() >> 1);

        #pragma omp parallel default(shared)
        {
            int this_thread = omp_get_thread_num();
            LOG4CXX_INFO(logger, "merge counter " << this_thread << " and " << n_half + this_thread);
            std::vector<ValuedVertexPair> *counter_tmp = merge_counters(*counters[this_thread], *counters[n_half + this_thread]);

            delete counters[this_thread];
            delete counters[n_half + this_thread];
            counters[this_thread] = counter_tmp;
        }

        counters.resize(n_half);
    }
    counter_merged = counters[0];

}

float WeightGraphWalker::merge(const std::vector<ValuedVertexPair>& counter,
        std::vector<ValuedVertexPair>& tmp,
        std::vector<ValuedVertexPair>& sampled_pairs) {
    float max_val = 0;
    float w;
    std::sort(sampled_pairs.begin(), sampled_pairs.end());

    std::vector<ValuedVertexPair>::const_iterator iter = counter.cbegin();
    for (size_t i = 0, j = 0; i < sampled_pairs.size(); i = j) {
        w = sampled_pairs[i].second;
        for (j = i + 1; j < sampled_pairs.size()
                && sampled_pairs[j].first == sampled_pairs[i].first; ++j) {
            w += sampled_pairs[j].second;
        }
        for (;iter != counter.end() && iter->first < sampled_pairs[i].first; ++iter) {
            max_val = std::max(max_val, iter->second);
            tmp.push_back(*iter);
        }
        if (iter != counter.end() && iter->first == sampled_pairs[i].first) {
            max_val = std::max(max_val, w + iter->second);
            tmp.push_back(
                    std::make_pair(iter->first, w + iter->second));
            ++iter;
        } else {
            max_val = std::max(max_val, w);
            tmp.push_back(std::make_pair(sampled_pairs[i].first, w));
        }
    }
    for (;iter != counter.end(); ++iter) {
        max_val = std::max(max_val, iter->second);
        tmp.push_back(*iter);
    }
    return max_val;
}





