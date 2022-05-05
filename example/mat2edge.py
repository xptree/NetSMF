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
    print(f"mat2edgelist from {file} to {output}")
    A = load_adjacency_matrix(file)
    A.eliminate_zeros()
    min_v, max_v = min(A.data) , max(A.data)
    print(f"minimum non-zero value={min_v:.2f} maximum non-zero value={max_v:.2f}")
    unweighted = math.isclose(min_v, 1.0) and math.isclose(max_v, 1.0)
    print("unweighted graph" if unweighted else "weighted graph")
    A = A.tocoo()
    with open(output, "w") as f:
        for x, y, v in zip(A.row, A.col, A.data):
            print(f"{x}\t{y}" if unweighted else f"{x}\t{y}\t{v}", file=f)

if __name__ == "__main__":
    #mat2edge("youtube.mat", "youtube.edge")
    mat2edge(sys.argv[1], sys.argv[2])
