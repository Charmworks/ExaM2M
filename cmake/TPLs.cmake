################################################################################
#
# \file      TPLs.cmake
# \copyright 2020 Charmworks, Inc.
#            All rights reserved. See the LICENSE file for details.
# \brief     Find the third-party libraries required to build ExaM2M
#
################################################################################

# Add TPL_DIR to modules directory for TPLs that provide cmake FIND_PACKAGE
# code, such as Trilinos
SET(CMAKE_PREFIX_PATH ${TPL_DIR} ${CMAKE_PREFIX_PATH})

# Include support for multiarch path names
include(GNUInstallDirs)

#### TPLs we attempt to find on the system #####################################

message(STATUS "------------------------------------------")

#### Charm++
set(CHARM_ROOT ${TPL_DIR}/charm)
find_package(Charm)

### HDF5/NetCDF (NetCDF only for static link)
set(HDF5_PREFER_PARALLEL true)
if(NOT BUILD_SHARED_LIBS)
  set(HDF5_USE_STATIC_LIBRARIES true)
endif()
find_package(HDF5 COMPONENTS C HL)
find_package(NetCDF)

if (NOT HDF5_FOUND)
  set(HDF5_INCLUDE_DIRS "")
endif()

#### ExodusII library
find_package(SEACASExodus)
set(EXODUS_ROOT ${TPL_DIR}) # prefer ours
find_package(Exodiff)

message(STATUS "------------------------------------------")

# Function to print a list of missing library names
# Arguments:
#   'target' a string to use in the error message printed for which libraries
#            are not found
#   'reqlibs' list of cmake variables in the form of "CHARM_FOUND", etc.
# Details: For each variable in 'reqlibs' if evaluates to false, trim the
# ending "_FOUND", convert to lower case and print an error message with the
# list of missing variables names. Intended to use after multiple find_package
# calls, passing all cmake variables named '*_FOUND' for all required libraries
# for a target.
function(PrintMissing target reqlibs)
  foreach(lib ${reqlibs})
    if(NOT ${lib})
      string(REPLACE "_FOUND" "" lib ${lib})
      string(TOLOWER ${lib} lib)
      list(APPEND missing "${lib}")
    endif()
  endforeach()
  string(REPLACE ";" ", " missing "${missing}")
  message(STATUS "Target '${target}' will NOT be configured, missing: ${missing}")
endfunction(PrintMissing)
