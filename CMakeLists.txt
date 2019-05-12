#
# CMAKE parameters file for 'configure'
#

SET(CMAKE_LEGACY_CYGWIN_WIN32 0) # Remove when CMake >= 2.8.4 is required

CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

#
# Misc parameters
#

# output parameters for 'make'
SET(CMAKE_COLOR_MAKEFILE "ON")

# project declaration
PROJECT(NETSMF)

# Release/Debug
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_DEFAULT_BUILD_TYPE "Release")
  SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)
MESSAGE(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})

# path for binary
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

#
# compiler flags
#
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fPIC")
IF(NOT APPLE)
  ADD_DEFINITIONS(-march=native)
ENDIF(NOT APPLE)

# c++ 11
set(CMAKE_CXX_STANDARD 11)

#
# Check for required libraries
# FIND_PACKAGE( xxx REQUIRED ) : REQUIRED removed so that MESSAGE is written
#

# gflags
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")
find_package(Gflags REQUIRED)
find_package(LOG4CXX REQUIRED)
#include_directories("${GFLAGS_INCLUDE_DIR}")
#include_directories("${LOG4CXX_INCLUDE_DIR}")

if (GFLAGS_FOUND)
 MESSAGE(STATUS "Compiling with gflags support")
 SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lgflags")
 SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lgflags")
 SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgflags")
ENDIF(GFLAGS_FOUND)

if (LOG4CXX_FOUND)
 MESSAGE(STATUS "Compiling with log4cxx support")
 SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -llog4cxx")
 SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -llog4cxx")
 SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -llog4cxx")
ENDIF(LOG4CXX_FOUND)


find_package (Eigen3 REQUIRED NO_MODULE)

# Threads
FIND_PACKAGE( Threads )
IF ( NOT THREADS_FOUND )
  MESSAGE(FATAL_ERROR "Package Threads required, but not found!")
ENDIF( NOT THREADS_FOUND )

# Zlib
FIND_PACKAGE( ZLIB )
IF ( NOT ZLIB_FOUND )
  MESSAGE(FATAL_ERROR "Package ZLIB required, but not found!")
ENDIF( NOT ZLIB_FOUND )

# Blas
FIND_PACKAGE(BLAS)
IF(BLAS_FOUND)
  SET(USE_BLAS 1)
ENDIF(BLAS_FOUND)

# Lapack
FIND_PACKAGE(LAPACK)
IF(LAPACK_FOUND)
  SET(USE_LAPACK 1)
ENDIF(LAPACK_FOUND)

# OpenMP
FIND_PACKAGE(OpenMP)
if (OPENMP_FOUND)
 MESSAGE(STATUS "Compiling with OpenMP support")
 SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
 SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
 SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
ENDIF(OPENMP_FOUND)

#
# Default Options
#

# should we use VERBOSE functions?
OPTION (VERBOSE  "Use VERBOSE option" OFF)

# should we use VERBOSE functions?
OPTION (EIGEN_USE_MKL_ALL  "Use MKL Library option" OFF)

#
# Management
#

# configure a header file to pass some of the CMake settings
# to the source code
CONFIGURE_FILE (
  ${CMAKE_SOURCE_DIR}/src/config.h.cmake
  ${CMAKE_SOURCE_DIR}/src/config.h
)

# specify the cross compiler
SET(CMAKE_CXX_COMPILER g++)


ADD_SUBDIRECTORY(src)
# INCLUDE_DIRECTORIES(data)
