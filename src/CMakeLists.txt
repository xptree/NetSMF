# Add subdirectories
ADD_SUBDIRECTORY(redsvd)

# Find dependencies libraries for Math
IF(UNIX)
  SET(MATH_LIB m)
ELSE(UNIX)
  SET(MATH_LIB)
ENDIF(UNIX)

# executable declaration
ADD_EXECUTABLE(netsmf netsmf.cc BinaryGraphWalker.cc GraphWalker.cc WeightGraphWalker.cc)

# Linking
TARGET_LINK_LIBRARIES( netsmf
                       redsvd
                       ${MATH_LIB}
                       ${ZLIB_LIBRARIES} 
                       ${GFLAGS_LIBRARIES} 
                       ${LOG4CXX_LIBRARIES} )

# TARGET_LINK_LIBRARIES( GraphWalker )
# TARGET_LINK_LIBRARIES( BinaryGraphWalker 
#                        GraphWalker
#                        ${GFLAGS_LIBRARIES} 
#                        ${LOG4CXX_LIBRARIES} )

# Installing
INSTALL(TARGETS netsmf DESTINATION bin)
