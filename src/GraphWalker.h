#pragma once

#include <vector>
#include <iostream>
#include <string>
#include "log4cxx/logger.h"


//using VertexId = unsigned long; //uint32_t;
using VertexId = unsigned int; //uint32_t;
using EdgeId = unsigned int; //uint32_t;
using VertexPair = std::pair<VertexId, VertexId>;
using VertexPairCount = std::pair<VertexPair, unsigned int>;
using ValuedVertexPair = std::pair<std::pair<VertexId, VertexId>, float>;


/* indices, indptr, data
 * indices is array of column indices
 * data is array of corresponding nonzero values
 * indptr points to row starts in indices and data
 * length is n_row + 1, last item = number of values = length of both indices and data
 * nonzero values of the i-th row are data[indptr[i]:indptr[i+1]] with column indices indices[indptr[i]:indptr[i+1]]
 * item (i, j) can be accessed as data[indptr[i]+k], where k is position of j in indices[indptr[i]:indptr[i+1]]
 */

class GraphWalker {
public:
    static log4cxx::LoggerPtr logger;
    GraphWalker(const std::vector<VertexId>& indices_,
        const std::vector<VertexId>& indptr_,
        const std::vector<float>& degree_,
        int T);

    const std::vector<VertexId> indices;
    const std::vector<VertexId> indptr;
    const std::vector<float> degree;
    int T;

    std::vector<ValuedVertexPair> *sparsifier_upper, *sparsifier_lower;
    std::vector<ValuedVertexPair> *counter_merged;

    virtual void sampling(int round, int num_threads,
            const std::string& machine,
            int check_point) = 0;
    void transformation();
    void redsvd();

    static std::vector<ValuedVertexPair>* merge_counters(
        const std::vector<ValuedVertexPair>& counter,
        const std::vector<ValuedVertexPair>& counter_other);
};


