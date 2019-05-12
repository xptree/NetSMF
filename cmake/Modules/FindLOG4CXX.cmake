# - Try to find LOG4CXX
#
# The following variables are optionally searched for defaults
#  LOG4CXX_ROOT_DIR:            Base directory where all LOG4CXX components are found
#
# The following are set after configuration is done:
#  LOG4CXX_FOUND
#  LOG4CXX_INCLUDE_DIRS
#  LOG4CXX_LIBRARIES
#  LOG4CXX_LIBRARYRARY_DIRS

include(FindPackageHandleStandardArgs)

set(LOG4CXX_ROOT_DIR "" CACHE PATH "Folder contains LOG4CXX")

# We are testing only a couple of files in the include directories
if(WIN32)
    find_path(LOG4CXX_INCLUDE_DIR log4cxx/logger.h
        PATHS ${LOG4CXX_ROOT_DIR}/src/windows)
else()
    find_path(LOG4CXX_INCLUDE_DIR log4cxx/logger.h
        PATHS ${LOG4CXX_ROOT_DIR})
endif()

if(MSVC)
    find_library(LOG4CXX_LIBRARY_RELEASE
        NAMES liblog4cxx
        PATHS ${LOG4CXX_ROOT_DIR}
        PATH_SUFFIXES Release)

    find_library(LOG4CXX_LIBRARY_DEBUG
        NAMES liblog4cxx-debug
        PATHS ${LOG4CXX_ROOT_DIR}
        PATH_SUFFIXES Debug)

    set(LOG4CXX_LIBRARY optimized ${LOG4CXX_LIBRARY_RELEASE} debug ${LOG4CXX_LIBRARY_DEBUG})
else()
    find_library(LOG4CXX_LIBRARY log4cxx)
endif()

find_package_handle_standard_args(LOG4CXX DEFAULT_MSG LOG4CXX_INCLUDE_DIR LOG4CXX_LIBRARY)


if(LOG4CXX_FOUND)
    set(LOG4CXX_INCLUDE_DIRS ${LOG4CXX_INCLUDE_DIR})
    set(LOG4CXX_LIBRARIES ${LOG4CXX_LIBRARY})
    message(STATUS "Found LOG4CXX  (include: ${LOG4CXX_INCLUDE_DIR}, library: ${LOG4CXX_LIBRARY})")
    mark_as_advanced(LOG4CXX_LIBRARY_DEBUG LOG4CXX_LIBRARY_RELEASE
                     LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR LOG4CXX_ROOT_DIR)
endif()
