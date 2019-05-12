#include "GraphWalker.h"
#include <numeric> // std::partial_sum
#include <omp.h>

// include gflags
#include <gflags/gflags.h>
DECLARE_int32(num_threads_svd);
DECLARE_int32(rank);
DECLARE_int32(negative);
DECLARE_string(output_svd);
DECLARE_bool(log1p);

// include redsvd headers
#include "redsvd/util.hpp"
#include "redsvd/redsvd.hpp"
#include "redsvd/redsvdFile.hpp"

using namespace log4cxx;
LoggerPtr GraphWalker::logger(Logger::getLogger("GraphWalker"));

GraphWalker::GraphWalker(const std::vector<VertexId>& indices_,
        const std::vector<VertexId>& indptr_,
        const std::vector<float>& degree_,
        int T_)
    : indices(indices_), indptr(indptr_), degree(degree_), T(T_) {
    assert(indptr.size() == degree.size() + 1);
    sparsifier_lower = new std::vector<ValuedVertexPair>();
    sparsifier_upper = new std::vector<ValuedVertexPair>();
    counter_merged = NULL;
}

void GraphWalker::transformation() {

    LOG4CXX_INFO(logger, "transformation ...");
    double M = 0;
    for (auto iter = counter_merged->cbegin(); iter != counter_merged->cend(); ++iter) {
        M += iter->second * 2;
    }
    LOG4CXX_INFO(logger, "total number of samples=" << M);
    double num_edges = (double)indices.size();
    double vol = 0.0;
    for (auto const& val : degree) {
        vol += val;
    }
    LOG4CXX_INFO(logger, "vol(G)=" << vol);
    double factor = vol * num_edges / M / FLAGS_negative;
    VertexId src, dst;
    double val;
    std::vector<VertexId> nnz_lower_row(degree.size(), 0);

    size_t nnz_lower = 0;
    sparsifier_upper->clear();
    sparsifier_lower->clear();
    if (FLAGS_log1p) {
        LOG4CXX_INFO(logger, "using log1p...");
    } else {
        LOG4CXX_INFO(logger, "using truncated logarithm...");
    }
    auto mylog = FLAGS_log1p ? [&](double x) -> double {return log1p(x);} : [&](double x) -> double {return log(x);};
    for (auto iter = counter_merged->cbegin(); iter != counter_merged->cend(); ++iter) {
        src = iter->first.first;
        dst = iter->first.second;
        val = src != dst ? iter->second : iter->second * 2;
        val = mylog(val * factor / degree[src] / degree[dst]);
        if (val > 0) {
            sparsifier_upper->push_back(std::make_pair(iter->first, (float)val));
            if (src != dst) {
                ++nnz_lower_row[dst];
                ++nnz_lower;
            }
        }
    }
    LOG4CXX_INFO(logger, "after log, #nnz in upper triangle and diagonal reduces to " << sparsifier_upper->size() << " (from " << counter_merged->size() << ")");
    counter_merged->clear();
    delete counter_merged;


    LOG4CXX_INFO(logger, "constructing lower triangle ...");
    // now, sparsifier stores upper triangle + diagonal
    // we will re-use sparsifier_lower to store lower triangle
    std::vector<VertexId> lower_indptr(degree.size() + 1, 0);
    std::partial_sum(nnz_lower_row.begin(), nnz_lower_row.end(), lower_indptr.begin() + 1);

    sparsifier_lower->resize(nnz_lower);
    LOG4CXX_INFO(logger, "lower triangle has " << nnz_lower << " nnz.");
    for (auto riter = sparsifier_upper->crbegin(); riter != sparsifier_upper->crend(); ++riter) {
        src = riter->first.first;
        dst = riter->first.second;
        if (src == dst) {
            continue;
        }
        auto iter = sparsifier_lower->begin() + lower_indptr[dst] + (--nnz_lower_row[dst]);
        iter->first.first = dst;
        iter->first.second = src;
        iter->second = riter->second;
    }
    LOG4CXX_INFO(logger, "lower triangle constructed.");
}

void GraphWalker::redsvd() {
    Eigen::setNbThreads(FLAGS_num_threads_svd);
    LOG4CXX_INFO(logger, "prepare svd ...");
    REDSVD::SMatrixXf A;
    // matrix size
    A.resize(degree.size(), degree.size());
    // number of nnz
    A.reserve(sparsifier_upper->size() + sparsifier_lower->size());
    auto iter_lower = sparsifier_lower->cbegin();
    auto iter_upper = sparsifier_upper->cbegin();
    for (size_t i = 0; i < degree.size(); ++i) {
        A.startVec(i);
        for (;iter_lower != sparsifier_lower->cend() && iter_lower->first.first == i; ++iter_lower) {
            A.insertBack(i, iter_lower->first.second) = iter_lower->second;
        }
        for (;iter_upper != sparsifier_upper->cend() && iter_upper->first.first == i; ++iter_upper) {
            A.insertBack(i, iter_upper->first.second) = iter_upper->second;
        }
    }
    A.finalize();
    sparsifier_upper->clear();
    sparsifier_lower->clear();
    delete sparsifier_upper;
    delete sparsifier_lower;

    LOG4CXX_INFO(logger, "running randomized SVD...");
    const double start = REDSVD::Util::getSec();
    REDSVD::RedSVD svdOfA(A, FLAGS_rank < degree.size() ? FLAGS_rank : degree.size());
    LOG4CXX_INFO(logger, "done in " << REDSVD::Util::getSec() - start);

    // set output name
	REDSVD::writeMatrix(FLAGS_output_svd, svdOfA);
}

std::vector<ValuedVertexPair>* GraphWalker::merge_counters(const std::vector<ValuedVertexPair>& counter,
        const std::vector<ValuedVertexPair>& counter_other) {
    std::vector<ValuedVertexPair>::const_iterator iter1 = counter.cbegin();
    std::vector<ValuedVertexPair>::const_iterator iter2 = counter_other.cbegin();

    std::vector<ValuedVertexPair> *counter_tmp = new std::vector<ValuedVertexPair>;

    while (iter1 != counter.cend() && iter2 != counter_other.cend()) {
        if (iter1->first < iter2->first) {
            counter_tmp->push_back(*(iter1++));
        } else if (iter1->first > iter2->first) {
            counter_tmp->push_back(*(iter2++));
        } else {
            counter_tmp->push_back(
                    std::make_pair(iter1->first, iter1->second + iter2->second));
            ++iter1;
            ++iter2;
        }
    }

    for (;iter1 != counter.cend(); ++iter1) {
        counter_tmp->push_back(*iter1);
    }

    for (;iter2 != counter_other.cend(); ++iter2) {
        counter_tmp->push_back(*iter2);
    }
    return counter_tmp;
}

