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

#### BLAS/LAPACK library with LAPACKE C-interface
find_package(LAPACKE)

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

#### Configure Brigand
set(BRIGAND_ROOT ${TPL_DIR}) # prefer ours
find_package(Brigand)

#### Zoltan2 library
find_package(Zoltan2)

#### Configure HighwayHash
set(HIGHWAYHASH_ROOT ${TPL_DIR}) # prefer ours
find_package(HighwayHash)

message(STATUS "------------------------------------------")
