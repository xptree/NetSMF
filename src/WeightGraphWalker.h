#pragma once

#include "GraphWalker.h"


class WeightGraphWalker : public GraphWalker {
public:
    static WeightGraphWalker* getWalker(const std::string& name, int T);
    WeightGraphWalker(const std::vector<VertexId>& indices,
            const std::vector<VertexId>& indptr, int T,
            const std::vector<float>& data,
            const std::vector<float>& prefix_sum,
            const std::vector<float>& degree);

    void samplePath(VertexId u, VertexId v, double w, int r, unsigned* seed,
            std::vector<ValuedVertexPair>& sampled_pairs) const;
    VertexId randomWalk(VertexId u, int step, double& Z, unsigned* seed) const;
    void sampling(int round, int num_threads,
            const std::string& machine,
            int check_point);

    static float merge(const std::vector<ValuedVertexPair>& counter,
            std::vector<ValuedVertexPair>& tmp,
            std::vector<ValuedVertexPair>& sampled_pairs);

    const std::vector<float> data;
    const std::vector<float> prefix_sum;
};
