#!/usr/bin/env python
# encoding: utf-8
# File Name: predict.py
# Author: Jiezhong Qiu
# Create Time: 2017/07/17 21:57
# TODO:

import warnings
warnings.filterwarnings("ignore")

import os
import pickle as pkl
import numpy as np
import scipy.sparse as sp
import scipy.io
import argparse
import logging
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import ShuffleSplit
from sklearn.multiclass import OneVsRestClassifier
from sklearn.metrics import f1_score
#from sklearn.exceptions import UndefinedMetricWarning
#warnings.filterwarnings("ignore", category=UserWarning)
#warnings.filterwarnings("ignore", category=UndefinedMetricWarning)

logger = logging.getLogger(__name__)

def construct_indicator(y_score, y):
    # rank the labels by the scores directly
    num_label = y.sum(axis=1, dtype=np.int32)
    # num_label = np.sum(y, axis=1, dtype=np.int)
    y_sort = np.fliplr(np.argsort(y_score, axis=1))
    #y_pred = np.zeros_like(y_score, dtype=np.int32)
    row, col = [], []
    for i in range(y_score.shape[0]):
        row += [i] * num_label[i, 0]
        col += y_sort[i, :num_label[i, 0]].tolist()
        #for j in range(num_label[i, 0]):
        #    y_pred[i, y_sort[i, j]] = 1
    y_pred = sp.csr_matrix(
            ([1] * len(row), (row, col)),
            shape=y.shape, dtype=np.bool_)
    return y_pred

def load_w2v_feature(file):
    with open(file, "rb") as f:
        nu = 0
        for line in f:
            content = line.strip().split()
            nu += 1
            if nu == 1:
                n, d = int(content[0]), int(content[1])
                feature = [[] for i in range(n)]
                continue
            index = int(content[0])
            for x in content[1:]:
                feature[index].append(float(x))
            if nu % 10000000 == 0:
                logger.info("read %d line from w2v feature file", nu)

#    for item in feature:
#        assert len(item) == d
    return np.array(feature, dtype=np.float32)


def load_label(file, variable_name="group"):
    if file.endswith(".tsv") or file.endswith(".txt"):
        data = np.loadtxt(file).astype(np.int32)
        label = sp.csr_matrix(([1] * data.shape[0], (data[:, 0], data[:, 1])), dtype=np.bool_)
        sp.save_npz("label.npz", label)
        return label
    elif file.endswith(".npz"):
        return sp.load_npz(file)
    else:
        data = scipy.io.loadmat(file)
        logger.info("loading mat file %s", file)

        label = data[variable_name].tocsr().astype(np.bool_)
        print(label.shape, label.dtype)
        return label

    label = data[variable_name].todense().astype(np.int32)
    label = np.array(label)
    return label

def predict_cv(X, y, train_ratio=0.2, n_splits=10, random_state=0, C=1., num_workers=1):
    micro, macro = [], []
    shuffle = ShuffleSplit(n_splits=n_splits, test_size=1-train_ratio,
            random_state=random_state)
    for train_index, test_index in shuffle.split(X):
        #print(train_index.shape, test_index.shape)
        #assert len(set(train_index) & set(test_index)) == 0
        #assert len(train_index) + len(test_index) == X.shape[0]
        X_train, X_test = X[train_index], X[test_index]
        y_train, y_test = y[train_index], y[test_index]
        clf = OneVsRestClassifier(
                LogisticRegression(
                    C=C,
                    solver="liblinear",
                    multi_class="ovr"),
                n_jobs=num_workers)
        clf.fit(X_train, y_train)
        y_score = clf.predict_proba(X_test)
        y_pred = construct_indicator(y_score, y_test)
        mi = f1_score(y_test, y_pred, average="micro")
        ma = f1_score(y_test, y_pred, average="macro")
        logger.info("micro f1 %f macro f1 %f", mi, ma)
        micro.append(mi)
        macro.append(ma)
    logger.info("%d fold validation, training ratio %f", len(micro), train_ratio)
    logger.info("Average micro %.2f, Average macro %.2f",
            np.mean(micro) * 100,
            np.mean(macro) * 100)
    return np.mean(micro)*100, np.mean(macro)*100


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--label", type=str, required=True,
            help="input file path for labels (.mat)")
    parser.add_argument("--embedding", type=str, required=True,
            help="input file path for embedding (.npy)")
    parser.add_argument("--matfile-variable-name", type=str, default='group',
            help='variable name of adjacency matrix inside a .mat file.')
    parser.add_argument("--seed", type=int, required=True,
            help="seed used for random number generator when randomly split data into training/test set.")
    parser.add_argument("--start-train-ratio", type=float, default=10,
            help="the start value of the train ratio (inclusive).")
    parser.add_argument("--stop-train-ratio", type=float, default=90,
            help="the end value of the train ratio (inclusive).")
    parser.add_argument("--num-train-ratio", type=int, default=9,
            help="the number of train ratio choosed from [train-ratio-start, train-ratio-end].")
    parser.add_argument("--C", type=float, default=1.0,
            help="inverse of regularization strength used in logistic regression.")
    parser.add_argument("--num-split", type=int, default=10,
            help="The number of re-shuffling & splitting for each train ratio.")
    parser.add_argument("--num-workers", type=int, default=60,
            help="Number of process")
    args = parser.parse_args()
    logging.basicConfig(
            filename="%s.log" % args.embedding, filemode="a", # uncomment this to log to file
            level=logging.INFO,
            format='%(asctime)s %(message)s') # include timestamp
    logger.info("C=%f", args.C)
    logger.info("Loading label from %s...", args.label)
    label = load_label(file=args.label, variable_name=args.matfile_variable_name)
    logger.info("Label loaded!")

    logger.info("Loading network embedding from %s...", args.embedding)
    ext = os.path.splitext(args.embedding)[1]
    if ext == ".npy":
        embedding = np.load(args.embedding)
    elif ext == ".pkl":
        with open(args.embedding, "rb") as f:
            embedding = pkl.load(f)
    else:
        # Load word2vec format
        embedding = load_w2v_feature(args.embedding)
        np.save("%s.npy" % args.embedding, embedding, allow_pickle=False)
    logger.info("Network embedding loaded!")

    logger.info("Embedding has shape %d, %d", embedding.shape[0], embedding.shape[1])
    logger.info("Label has shape %d, %d", label.shape[0], label.shape[1])

    if label.shape[0] != embedding.shape[0]:
        logger.info("Different shape ....")
        num_instance = min(label.shape[0], embedding.shape[0])
        label, embedding = label[:num_instance], embedding[:num_instance]

    num_label = label.sum(axis=1, dtype=np.int32)
    idx = np.argwhere(num_label == 0)
    logger.info("%d instances with no label" % len(idx))
    #  if len(idx):
    #      embedding = embedding[label.getnnz(1)>0]
    #      label = label[label.getnnz(1)>0]
    #  logger.info("After deleting ...")
    logger.info("Embedding has shape %d, %d", embedding.shape[0], embedding.shape[1])
    logger.info("Label has shape %d, %d", label.shape[0], label.shape[1])

    train_ratios = np.linspace(args.start_train_ratio, args.stop_train_ratio,
            args.num_train_ratio)


    f1 = list()
    for tr in train_ratios:
        res = predict_cv(embedding, label, train_ratio=tr/100.,
                n_splits=args.num_split, C=args.C, random_state=args.seed,
                num_workers=args.num_workers)
        f1.append(res)
    micro, macro = zip(*f1)
    print(" ".join([str(x) for x in micro]))
    logger.info(" ".join([str(x) for x in micro]))
    print(" ".join([str(x) for x in macro]))
    logger.info(" ".join([str(x) for x in macro]))
