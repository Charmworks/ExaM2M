################################################################################
#
# \file      DetectCompilerVersion.cmake
# \copyright 2020 Charmworks, Inc.
#            All rights reserved. See the LICENSE file for details.
# \brief     Detect C, C++, Fortran compiler major, minor, and patch version
#
################################################################################

set(COMPILERS C CXX)

if (DEFINED CMAKE_Fortran_COMPILER)
  list(APPEND COMPILERS "Fortran")
endif()

foreach(comp  ${COMPILERS})
  string(REGEX MATCH "([0-9]*)\\.([0-9]*)\\.([0-9]*)"
         major ${CMAKE_${comp}_COMPILER_VERSION})
  set(CMAKE_${comp}_COMPILER_MAJOR ${CMAKE_MATCH_1})
  set(CMAKE_${comp}_COMPILER_MINOR ${CMAKE_MATCH_2})
  set(CMAKE_${comp}_COMPILER_PATCH ${CMAKE_MATCH_3})
endforeach()
