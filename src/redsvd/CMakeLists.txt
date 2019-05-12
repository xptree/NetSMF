FILE(
  GLOB
  redsvd_files
  *.cpp
  *.hpp
  *.h
)
# Add "redsvd" library
ADD_LIBRARY(redsvd ${redsvd_files})
IF(USE_BLAS)
    TARGET_LINK_LIBRARIES(redsvd ${BLAS_LIBRARIES} Eigen3::Eigen)
ENDIF(USE_BLAS)
IF(USE_LAPACK)
    TARGET_LINK_LIBRARIES(redsvd ${LAPACK_LIBRARIES} Eigen3::Eigen)
ENDIF(USE_LAPACK)
