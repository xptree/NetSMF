#pragma once

#include "GraphWalker.h"

class BinaryGraphWalker : public GraphWalker {
public:
    static BinaryGraphWalker* getWalker(const std::string& name, int T);
    BinaryGraphWalker(const std::vector<VertexId>& indices,
            const std::vector<VertexId>& indptr, int T,
            const std::vector<float>& degree);

    void samplePath(VertexId u, VertexId v, int r, unsigned* seed,
            std::vector<VertexPair>& sampled_pair) const;
    VertexId randomWalk(VertexId u, int step, unsigned* seed) const;
    void sampling(int round, int num_threads,
            const std::string& machine,
            int check_point);
    // void transformation();
    // void redsvd();
    // void merge_to_sparsifier(const std::vector<VertexPairCount>& counter);

    // static void dump(const std::string& filename,
    //         const std::vector<VertexPairCount>& counter);
    static float merge(const std::vector<ValuedVertexPair>& counter,
            std::vector<ValuedVertexPair>& tmp,
            std::vector<VertexPair>& sampled_pairs);
};

