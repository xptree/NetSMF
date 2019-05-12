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

#ifndef FILEREADER_HPP_
#define FILEREADER_HPP_

#include <fstream>
#include <sstream>
#include "util.hpp"

namespace REDSVD{

class FileReader {
public:
  FileReader() : rows_(0), cols_(0) {}
  ~FileReader() {}

  void OpenFile(const char* inputFileName){
    inputFileName_ = inputFileName;
    ifs_.close();
    ifs_.clear();
    ifs_.open(inputFileName_.c_str(), std::ifstream::in);
    if (!ifs_){
      throw std::string("open error ") + inputFileName_;
    }
  }

  void Rewind(){
    ifs_.clear();
    ifs_.seekg(0);
  }

  void GetStat(){
    rows_ = 0;
    cols_ = 0;
    for (fv_t fv; ReadRow(fv) != -1; ++rows_){
      if (fv.size() == 0) continue;
      cols_ = std::max(fv.back().first+1, cols_);
    }
    ifs_.clear();
    ifs_.seekg(0);
  }

  int ReadRow(fv_t& fv){
    std::string line;
    if (!getline(ifs_, line)){
      return -1;
    }
    std::istringstream is(line);
    
    int id;
    char sep;
    float val;
    while (is >> id >> sep >> val){
      fv.push_back(std::make_pair(id, val));
    }
    sort(fv.begin(), fv.end());
    fv.erase(unique(fv.begin(), fv.end()), fv.end());
    
    return 0;
  }

  int rows() const {
    return rows_;
  }

  int cols() const {
    return cols_;
  }

private:
  std::ifstream ifs_;
  std::string inputFileName_;
  int rows_;
  int cols_;
};

}

#endif // FILEREADER_HPP_
