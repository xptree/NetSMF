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

#ifndef REDSVDFILE_HPP__
#define REDSVDFILE_HPP__

#include <string>
#include <iostream>
#include <fstream>
#include "util.hpp"

namespace REDSVD{

class RedSVD;
class RedPCA;
class RedSymEigen;
class RedSVDIncr;

void readMatrix(const std::string& fn, SMatrixXf& A);
void readMatrix(const std::string& fn, Eigen::MatrixXf& A);

void writeMatrix(const std::string& fn, const RedSVD& A);
void writeMatrix(const std::string& fn, const RedPCA& A);
void writeMatrix(const std::string& fn, const RedSymEigen& A);
void writeMatrix(const std::string& fn, const RedSVDIncr& A);

template<class Mat, class RetMat>
void fileProcess(const std::string& inputFileName,
		 const std::string& outputFileName,
		 int rank){
  double startSec = Util::getSec();
  std::cout << "read matrix from " << inputFileName << " ... " << std::flush;
  Mat A;
  readMatrix(inputFileName.c_str(), A);
  std::cout << Util::getSec() - startSec << " sec." <<std:: endl;
  std::cout << "rows:\t" << A.rows() << std::endl
	    << "cols:\t" << A.cols() << std::endl
	    << "rank:\t" << rank  << std::endl;

  std::cout << "compute ... " << std::flush;
  startSec = Util::getSec();
  RetMat retMat(A, rank);
  std::cout << Util::getSec() - startSec << " sec." << std::endl;
  
  startSec = Util::getSec();
  writeMatrix(outputFileName, retMat);
  std::cout << Util::getSec() - startSec << " sec." << std::endl
	    << "finished." << std::endl;
}

}

#endif // REDSVDFILE_HPP__
