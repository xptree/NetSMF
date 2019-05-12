#include "BinaryGraphWalker.h"

#include <cassert>
#include <numeric>
#include <fstream>
#include <cassert>
#include <functional>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <omp.h>

// include gflags
#include <gflags/gflags.h>
DECLARE_int32(num_threads_svd);
DECLARE_int32(rank);
DECLARE_int32(negative);
DECLARE_string(output_svd);

using namespace log4cxx;

BinaryGraphWalker::BinaryGraphWalker(const std::vector<VertexId>& indices,
        const std::vector<VertexId>& indptr, int T, const std::vector<float>& degree)
    : GraphWalker(indices, indptr, degree, T) {
    LOG4CXX_INFO(logger, "unweighted network");
}

BinaryGraphWalker* BinaryGraphWalker::getWalker(const std::string& fname, int T) {
    std::vector<VertexId> edges;
	std::vector<VertexId> out_degree;
	std::vector<VertexId> edge_pair;

    VertexId max_vertex_id = 0;
	std::ifstream fin(fname);
	assert(fin.is_open());
    VertexId src, dst;
	while (fin >> src >> dst) {
		// vertexid overflow
		if (src >= max_vertex_id || dst >= max_vertex_id) {
			max_vertex_id = std::max(src, dst);
			out_degree.resize(max_vertex_id + 1, 0);
		}
        if (src == dst) {
            continue;
        }
		++out_degree[src];
        edge_pair.push_back(src);
        edge_pair.push_back(dst);
	}
    int num_vertex = max_vertex_id + 1;

    std::vector<VertexId> indptr(num_vertex + 1, 0);
    std::partial_sum(out_degree.begin(), out_degree.end(), indptr.begin() + 1);
    std::vector<float> degree;
    for (auto const& val : out_degree) {
        degree.push_back(float(val));
    }

    EdgeId edge_cnt = edge_pair.size() >> 1;
    std::vector<VertexId> indices(edge_cnt, 0);

    for (EdgeId e = 0; e < edge_cnt; ++e) {
        VertexId src = edge_pair[e << 1];
        VertexId dst = edge_pair[(e << 1) + 1];

        EdgeId idx = indptr[src] + (--out_degree[src]);
        indices[idx] = dst;
    }
    return new BinaryGraphWalker(indices, indptr, T, degree);
}


VertexId BinaryGraphWalker::randomWalk(VertexId u, int step,
        unsigned* seed) const {
    for (;step--;) {
        // u's neighbors are indices[indptr[i]:indptr[i+1]]
        int offset = rand_r(seed) % (indptr[u+1] - indptr[u]);
        u = indices[indptr[u] + offset];
    }
    return u;
}

void BinaryGraphWalker::samplePath(const VertexId u, const VertexId v, int r, unsigned* seed,
        std::vector<VertexPair>& sampled_pairs) const {
    int k = rand_r(seed) % r + 1;
    VertexId u_ = randomWalk(u, k - 1, seed);
    VertexId v_ = randomWalk(v, r - k, seed);
    // add record (u_, v_, 1)

    if (u_ > v_) {
        std::swap(u_, v_);
    }

    sampled_pairs.push_back(std::make_pair(u_, v_));
}

void BinaryGraphWalker::sampling(int round, int num_threads,
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
            + std::string("_thread_") + std::to_string(this_thread); // + std::string("_time_") + std::to_string(time(0));

        LOG4CXX_INFO(logger, "[thread " << this_thread << "]" << " thread name is " << thread_name );
        unsigned seed = std::hash<std::string>{}(thread_name);

        std::vector<VertexPair> sampled_pairs;
        std::vector<ValuedVertexPair> *&counter = counters[this_thread];
        std::vector<ValuedVertexPair> *counter_tmp = new std::vector<ValuedVertexPair>;

        LOG4CXX_INFO(logger, "[thread " << this_thread << "]" << " set seed " << seed);
        int my_round= ceil((double)round / num_threads);

        for (int i=0; i<my_round; ++i) {
            for (VertexId u=0; u+1 < indptr.size(); ++u) {
                for (size_t j=indptr[u]; j<indptr[u+1]; ++j) {
                    VertexId v = indices[j];
                    for (int r=1; r<=T; ++r) {
                        // printf("%d %d %d\n", u, v, r);
                        samplePath(u, v, r, &seed, sampled_pairs);
                    }
                }
            }
            if ((i + 1) % check_point == 0 || i + 1 == my_round) {
                float max_val = merge(*counter, *counter_tmp, sampled_pairs);
                std::swap(counter, counter_tmp);
                sampled_pairs.clear();
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

float BinaryGraphWalker::merge(const std::vector<ValuedVertexPair>& counter,
        std::vector<ValuedVertexPair>& tmp,
        std::vector<VertexPair>& sampled_pairs) {
    float max_val = 0;
    std::sort(sampled_pairs.begin(), sampled_pairs.end());

    std::vector<ValuedVertexPair>::const_iterator iter = counter.cbegin();
    for (size_t i = 0, j = 0; i < sampled_pairs.size(); i = j) {
        for (j = i + 1; j < sampled_pairs.size() && sampled_pairs[j] == sampled_pairs[i]; ++j);
        for (;iter != counter.end() && iter->first < sampled_pairs[i]; ++iter) {
            max_val = std::max(max_val, iter->second);
            tmp.push_back(*iter);
        }
        if (iter != counter.end() && iter->first == sampled_pairs[i]) {
            max_val = std::max(max_val, j - i + iter->second);
            tmp.push_back(
                    std::make_pair(iter->first, j - i + iter->second));
            ++iter;
        } else {
            max_val = std::max(max_val, float(j - i));
            tmp.push_back(std::make_pair(sampled_pairs[i], float(j - i)));
        }
    }
    for (;iter != counter.end(); ++iter) {
        max_val = std::max(max_val, iter->second);
        tmp.push_back(*iter);
    }
    return max_val;
}




