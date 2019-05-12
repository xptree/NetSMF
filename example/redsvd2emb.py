#!/usr/bin/env python
# encoding: utf-8
# File Name: redsvd2emb.py
# Author: Jiezhong Qiu
# Create Time: 2018/10/22 03:37
# TODO:


import scipy.sparse as sp
import numpy as np
import logging
import argparse

logger = logging.getLogger(__name__)

def redsvd2emb(u, s):
    return sp.diags(np.sqrt(s)).dot(u.T).T


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--name", type=str, required=True,
            help="file name")
    parser.add_argument("--dim", type=int, required=True,
            help="dimension")
    args = parser.parse_args()
    logging.basicConfig(level=logging.INFO,
            format='%(asctime)s %(message)s') # include timestamp
    u = np.loadtxt("%s.U" % args.name)[:, :args.dim]
    s = np.loadtxt("%s.S" % args.name)[:args.dim]
    embedding = redsvd2emb(u, s)
    logger.info("save embedding to %s_%d.npy", args.name, args.dim)
    np.save("%s_%d.npy" % (args.name, args.dim), embedding, allow_pickle=False)


