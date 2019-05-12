#!/usr/bin/env python
# encoding: utf-8
# File Name: mat2edge.py
# Author: Jiezhong Qiu
# Create Time: 2019/03/18 12:01
# TODO:

import scipy.io
import math
import sys

def load_adjacency_matrix(file, variable_name="network"):
    data = scipy.io.loadmat(file)
    return data[variable_name]

def mat2edge(file, output):
    print("mat2edgelist from %s to %s" % (file, output))
    A = load_adjacency_matrix(file)
    A.eliminate_zeros()
    min_v, max_v = min(A.data) , max(A.data)
    print("minimum non-zero value=%.2f maximum non-zero value=%.2f" \
            % (min_v, max_v))
    unweighted = math.isclose(min_v, 1.0) and math.isclose(max_v, 1.0)
    print("unweighted graph" if unweighted else "weighted graph")
    A = A.todok()
    with open(output, "w") as f:
        for (x, y), v in A.items():
            assert(math.isclose(A[y, x], v))
            print("%d\t%d" % (x, y) if unweighted else "%d\t%d\t%f" % (x, y, v),end="\n", file=f)

if __name__ == "__main__":
    #mat2edge("youtube.mat", "youtube.edge")
    mat2edge(sys.argv[1], sys.argv[2])
