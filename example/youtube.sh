#!/bin/bash

set -x

NETSMF=../bin/Release/netsmf
if [ -z "$1" ]; then
    INPUT="youtube.edge"
else
    INPUT=$1
fi

if [ -z "$2" ]; then
#    mkdir -p youtube
    OUTPUT="youtube.netsmf"
else
    OUTPUT=$2
fi

if [ -z "$3" ]; then
	LABEL=youtube.mat
else
	LABEL=$3
fi

[ ! -f $INPUT ] && python mat2edge.py $LABEL $INPUT

(/usr/bin/time -p $NETSMF -T 10 \
    -filename $INPUT \
    -machine $HOSTNAME \
    -output_svd $OUTPUT \
    -rank 256 \
    -num_threads_sampling 10 \
    -num_threads_svd 32 \
    -rounds 2000 \
    -check_point 10 \
    -noweight \
    -nolog1p \
    -log4cxx log4cxx.config) |& tee -a youtube.log

python redsvd2emb.py --name $OUTPUT --dim 128
python predict.py --label $LABEL --embedding ${OUTPUT}_128.npy --seed 0 --C 1 --start-train-ratio 1 --stop-train-ratio 10 --num-train-ratio 10
python predict.py --label $LABEL --embedding ${OUTPUT}_128.npy --seed 0 --C 10 --start-train-ratio 1 --stop-train-ratio 10 --num-train-ratio 10
