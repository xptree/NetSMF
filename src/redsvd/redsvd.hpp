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

#ifndef REDSVD_HPP__
#define REDSVD_HPP__

#define EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET

#include <vector>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include <gflags/gflags.h>
DECLARE_bool(sparse_proj);
DECLARE_int32(density_multiplier);
#include "util.hpp"
#include "log4cxx/logger.h"

using namespace log4cxx;

namespace REDSVD {

class RedSVD {
public:
  RedSVD(){}

  template <class Mat>
  RedSVD(Mat& A){
    int r = (A.rows() < A.cols()) ? A.rows() : A.cols();
    run(A, r);
  }

  template <class Mat>
  RedSVD(Mat& A, const int rank){
    run(A, rank);
  }

  template <class Mat>
  void run(Mat& A, const int rank){
    LoggerPtr logger(Logger::getLogger("redsvd"));
    if (A.cols() == 0 || A.rows() == 0) return;
    int r = (rank < A.cols()) ? rank : A.cols();
    r = (r < A.rows()) ? r : A.rows();

    // Gaussian Random Matrix for A^T
    Eigen::MatrixXf O(A.rows(), r);

    if (FLAGS_sparse_proj) {
        LOG4CXX_INFO(logger, "sampling sparse random projection matrix O for A^T ...");
        Util::sampleSparseProjMat(O, FLAGS_density_multiplier);
    } else {
        LOG4CXX_INFO(logger, "sampling gaussian random matrix O for A^T ...");
        Util::sampleGaussianMat(O);
    }
    LOG4CXX_INFO(logger, "sampling random matrix O done.");

    // Compute Sample Matrix of A^T
    // Eigen::MatrixXf Y = A.transpose() * O;
    Eigen::MatrixXf Y = A * O;
    LOG4CXX_INFO(logger, "compute sample matrix of Y = A^T * O  = A * O done (because A^T = A)");

    // Orthonormalize Y
    Util::processGramSchmidt(Y);
    LOG4CXX_INFO(logger, "orthonormalize Y done.");

    // Range(B) = Range(A^T)
    Eigen::MatrixXf B = A * Y;
    LOG4CXX_INFO(logger, "B = A * Y done, Range(B) = Range(A^T).");

    // Gaussian Random Matrix
    Eigen::MatrixXf P(B.cols(), r);
    if (FLAGS_sparse_proj) {
        LOG4CXX_INFO(logger, "sampling another sparse random projection matrix P ...");
        Util::sampleSparseProjMat(P, FLAGS_density_multiplier);
    } else {
        LOG4CXX_INFO(logger, "sampling another gaussian random projection matrix P ...");
        Util::sampleGaussianMat(P);
    }
    LOG4CXX_INFO(logger, "sampling random matrix P done.");

    // Compute Sample Matrix of B
    Eigen::MatrixXf Z = B * P;
    LOG4CXX_INFO(logger, "compute sample matrix of Z = B * P done.")

    // Orthonormalize Z
    Util::processGramSchmidt(Z);
    LOG4CXX_INFO(logger, "orthonormalize Z done.");

    // Range(C) = Range(B)
    Eigen::MatrixXf C = Z.transpose() * B;
    LOG4CXX_INFO(logger, "C = Z^T * B done, Range(C) = Range(B).");

    Eigen::JacobiSVD<Eigen::MatrixXf> svdOfC(C, Eigen::ComputeThinU | Eigen::ComputeThinV);
    LOG4CXX_INFO(logger, "JacabiSVD for C done.");


    // C = USV^T
    // A = Z * U * S * V^T * Y^T()
    matU_ = Z * svdOfC.matrixU();
    matS_ = svdOfC.singularValues();
    matV_ = Y * svdOfC.matrixV();
    LOG4CXX_INFO(logger, "compute U S V done.");
  }

  const Eigen::MatrixXf& matrixU() const {
    return matU_;
  }

  const Eigen::VectorXf& singularValues() const {
    return matS_;
  }

  const Eigen::MatrixXf& matrixV() const {
    return matV_;
  }

private:
  Eigen::MatrixXf matU_;
  Eigen::VectorXf matS_;
  Eigen::MatrixXf matV_;
};

class RedSymEigen {
public:
  RedSymEigen(){}

  template <class Mat>
  RedSymEigen(Mat& A, const int rank){
    run(A, rank);
  }

  template <class Mat>
  void run(Mat& A, const int rank){
    if (A.cols() == 0 || A.rows() == 0) return;
    int r = (rank < A.cols()) ? rank : A.cols();
    r = (r < A.rows()) ? r : A.rows();

    // Gaussian Random Matrix
    Eigen::MatrixXf O(A.rows(), r);
    Util::sampleGaussianMat(O);

    // Compute Sample Matrix of A
    Eigen::MatrixXf Y = A.transpose() * O;

    // Orthonormalize Y
    Util::processGramSchmidt(Y);

    Eigen::MatrixXf B = Y.transpose() * A * Y;
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigenOfB(B);

    eigenValues_ = eigenOfB.eigenvalues();
    eigenVectors_ = Y * eigenOfB.eigenvectors();
  }

  const Eigen::MatrixXf& eigenVectors() const {
    return eigenVectors_;
  }

  const Eigen::VectorXf& eigenValues() const {
    return eigenValues_;
  }

private:
  Eigen::VectorXf eigenValues_;
  Eigen::MatrixXf eigenVectors_;
};

class RedPCA {
public:
  RedPCA(){}

  template <class Mat>
  RedPCA(const Mat& A, const int rank) {
    run(A, rank);
  }

  template <class Mat>
  void run(const Mat& A, const int rank) {
    RedSVD redsvd;
    redsvd.run(A, rank);
    const Eigen::VectorXf& S = redsvd.singularValues();
    principalComponents_ = redsvd.matrixV();
    scores_              = redsvd.matrixU() * S.asDiagonal();
  }

  const Eigen::MatrixXf& principalComponents() const {
    return principalComponents_;
  }

  const Eigen::MatrixXf& scores() const {
    return scores_;
  }

 private:
  Eigen::MatrixXf principalComponents_;
  Eigen::MatrixXf scores_;
};

}

#endif // REDSVD_HPP__
