#include "BinaryGraphWalker.h"
#include "WeightGraphWalker.h"
#include <gflags/gflags.h>
#include <cstdio>
#include <cstdlib>
// include log4cxx header files.
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr logger(Logger::getLogger("main"));


DEFINE_int32(T, 10, "Window size.");
DEFINE_string(filename, "edgelist", "Filename for edgelist file.");
DEFINE_string(machine, "localhost", "machine name for generating random seed by hash.");
// DEFINE_string(output_samples, "sample", "Filename for sampled pairs.");
DEFINE_string(output_svd, "sample", "Filename for svd results.");
DEFINE_int32(rank, 256, "embedding dimension.");
DEFINE_int32(negative, 1, "number of negative sampling.");
DEFINE_int32(num_threads_sampling, 32, "Number of threads.");
DEFINE_int32(num_threads_svd, 32, "Number of threads for svd.");
DEFINE_int32(rounds, 1000, "Number of rounds.");
DEFINE_int32(check_point, 2, "Check point every ? rounds.");
// DEFINE_int32(max_mem_GB, 200, "Maximum cached data.");
DEFINE_bool(weight, false, "Weighted graph");
DEFINE_bool(log1p, false, "Using log1p instead of truncated logarithm");
DEFINE_string(log4cxx, "log4cxx.config", "Log4cxx config file");

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    //BasicConfigurator::configure();
    PropertyConfigurator::configure(FLAGS_log4cxx.c_str());
    LOG4CXX_INFO(logger, "Entering application.");

    GraphWalker *walker = FLAGS_weight ?
        (GraphWalker*)WeightGraphWalker::getWalker(FLAGS_filename.c_str(), FLAGS_T) :
        (GraphWalker*)BinaryGraphWalker::getWalker(FLAGS_filename.c_str(), FLAGS_T);

    walker->sampling(FLAGS_rounds, FLAGS_num_threads_sampling,  FLAGS_machine, FLAGS_check_point);
    walker->transformation();
    walker->redsvd();
    LOG4CXX_INFO(logger, "Exiting application.");
    return 0;
}
