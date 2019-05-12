/* 
 *  Copyright (c) 2010 Daisuke Okanohara
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */

#include <iostream>
#include <sys/time.h>

#include "util.hpp"

using namespace std;
using namespace Eigen;

namespace REDSVD {

const float SVD_EPS = 0.0001f;

double Util::getSec(){
  timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void Util::sampleTwoGaussian(float& f1, float& f2){
  float v1 = (float)(rand() + 1.f) / ((float)RAND_MAX+2.f);
  float v2 = (float)(rand() + 1.f) / ((float)RAND_MAX+2.f);
  float len = sqrt(-2.f * log(v1));
  f1 = len * cos(2.f * M_PI * v2);
  f2 = len * sin(2.f * M_PI * v2);
}

void Util::sampleGaussianMat(MatrixXf& mat){
  for (int i = 0; i < mat.rows(); ++i){
    int j = 0;
    for ( ; j+1 < mat.cols(); j += 2){
      float f1, f2;
      sampleTwoGaussian(f1, f2);
      mat(i,j  ) = f1;
      mat(i,j+1) = f2;
    }
    for (; j < mat.cols(); j ++){
      float f1, f2;
      sampleTwoGaussian(f1, f2);
      mat(i, j)  = f1;
    }
  }
} 


void Util::processGramSchmidt(MatrixXf& mat){
  for (int i = 0; i < mat.cols(); ++i){
    for (int j = 0; j < i; ++j){
      float r = mat.col(i).dot(mat.col(j));
      mat.col(i) -= r * mat.col(j);
    }
    float norm = mat.col(i).norm();
    if (norm < SVD_EPS){
      for (int k = i; k < mat.cols(); ++k){
	mat.col(k).setZero();
      } 
      return;
    }
    mat.col(i) *= (1.f / norm);
  }
}

void Util::convertFV2Mat(const vector<fv_t>& fvs, REDSVD::SMatrixXf& A){
  int maxID = 0;
  size_t nonZeroNum = 0;
  for (size_t i = 0; i < fvs.size(); ++i){
    const fv_t& fv(fvs[i]);
    for (size_t j = 0; j < fv.size(); ++j){
      maxID = max(fv[j].first+1, maxID);
    }
    nonZeroNum += fv.size();
  }
  A.resize(fvs.size(), maxID);
  A.reserve(nonZeroNum);
  for (size_t i = 0; i < fvs.size(); ++i){
    A.startVec(i);
    const fv_t& fv(fvs[i]);
    for (size_t j = 0; j < fv.size(); ++j){
      A.insertBack(i, fv[j].first) = fv[j].second;
    }
  }
  A.finalize();
}


}
