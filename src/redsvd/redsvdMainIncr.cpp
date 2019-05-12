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

#include <string>
#include <fstream>
#include <sstream>

#include "cmdline.h"
#include "fileReader.hpp"
#include "redsvdFile.hpp"
#include "redsvd.hpp"
#include "redsvdIncr.hpp"
#include "util.hpp"

using namespace std;
using namespace REDSVD;

void IncrRun(const string& inputFileName,
             const string& outputFileName,
             int rank){
  FileReader fileReader;
  std::cout << "read matrix from " << inputFileName << " ... " << std::flush;
  double startSec = Util::getSec();
  fileReader.OpenFile(inputFileName.c_str());
  std::cout << Util::getSec() - startSec << " sec." <<std:: endl;

  fileReader.GetStat();
  
  std::cout << "rows:\t" << fileReader.rows() << std::endl
	    << "cols:\t" << fileReader.cols() << std::endl
	    << "rank:\t" << rank  << std::endl;

  std::cout << "compute ... " << std::flush;
  startSec = Util::getSec();
  RedSVDIncr redsvd_incr(fileReader, rank);
  std::cout << Util::getSec() - startSec << " sec." << std::endl;
  startSec = Util::getSec();
  writeMatrix(outputFileName, redsvd_incr);
  std::cout << Util::getSec() - startSec << " sec." << std::endl
	    << "finished." << std::endl;

  
}

int main(int argc, char* argv[]){
  cmdline::parser p;
  p.add<string>("input",  'i', "input file", true);
  p.add<string>("output", 'o', "output file's prefix", true);
  p.add<int>   ("rank",   'r', "rank      ", false, 10);
  p.set_program_name("redsvd_incr");

  if (argc == 1){
    cerr << p.usage() << endl;
    return 0;
  }

  if (p.parse(argc, argv) == 0){
    cerr << "Error:" << p.error() << endl
	 << p.usage() << endl;
    return -1;
  }

  string input  = p.get<string>("input");
  string output = p.get<string>("output");
  int    rank   = p.get<int>   ("rank");

  if (rank <= 0){
    cerr << "rank=" << rank << endl
	 << "rank should be positive integer" << endl;
    return -1;
  }

  try {
    IncrRun(input, output, rank);
  } catch (const string& error){
    cerr << "Error: " << error << endl;
    return -1;
  }
  return 0;
}


