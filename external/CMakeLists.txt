################################################################################
#
# \file      CMakeLists.txt
# \copyright 2020 Charmworks, Inc.
#            All rights reserved. See the LICENSE file for details.
# \brief     Build ExaM2M third-party Libraries
#
################################################################################

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(TPL C CXX)
include(ExternalProject)

# ExaM2M cmake modules directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

# Disallow in-source builds
include(DisallowInSourceBuilds)
disallow_in_source_builds()

# Set CMAKE_BUILD_TYPE
include(BuildType)

# Detect operating system type and version
include(DetectOS)
detect_os()
message(STATUS "Detected OS: ${HOST_OS}")

# Set BUILD_SHARED_LIBS
include(BuildShared)

# Conditionally enforce static linking for exeuctables added after this point.
# This must precede all add_executable() calls adding executables to link
# statically.
if (NOT BUILD_SHARED_LIBS)
  set(CMAKE_EXE_LINKER_FLAGS "-static")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

# Include support for multiarch path names
include(GNUInstallDirs)

# Set compiler id
string(TOLOWER "${CMAKE_CXX_COMPILER_ID}" COMP)

# Query target architecture
include(TargetArch)
target_architecture(ARCH)
message(STATUS "Detected target architecture: ${ARCH}")

# Set install directory based on compiler ID if was not given on command line
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install/${COMP}-${ARCH}"
      CACHE STRING "Third-party install prefix." FORCE)
  message(STATUS "CMAKE_INSTALL_PREFIX not specified, setting to '${PROJECT_SOURCE_DIR}/install/${COMP}-${ARCH}'")
else()
  message(STATUS "CMAKE_INSTALL_PREFIX: " ${CMAKE_INSTALL_PREFIX})
endif()

include(ProcessorCount)
ProcessorCount(PROCESSOR_COUNT)
if(PROCESSOR_COUNT EQUAL 0)     # if ncpus could not be determined, assign 1
  set(PROCESSOR_COUNT "1")
endif()
message(STATUS "Found ${PROCESSOR_COUNT} PEs")

# Convert system name to lower case
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME)

# Set which libraries are built/searched by default
set(ENABLE_CHARM ON CACHE BOOL "Enable Charm++")
set(ENABLE_HDF5 ON CACHE BOOL "Enable HDF5")
set(ENABLE_NETCDF ON CACHE BOOL "Enable NetCDF")
set(ENABLE_TRILINOS ON CACHE BOOL "Enable Trilinos")

# Charm++
if (ENABLE_CHARM)
  find_package(Charm)
  if(CHARM_FOUND)
    set(charm "")
  else()
    set(charm "charm")
  endif()
endif()

# If we are to build our own Charm++, setup prerequisites for its build command
if (charm)

  # Use mpi wrappers by default
  set(CHARM_COMPILER "")

  # Match Charm++ build architecture for system architecture detected
  if(ARCH MATCHES "x86_64")

    set(CHARM_ARCH "-${ARCH}")

  elseif(ARCH MATCHES "ppc64")

    set(SYSTEM_NAME "")
    set(CHARM_ARCH "bluegeneq")

  elseif(ARCH MATCHES "i386")

    if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      set(CHARM_ARCH "")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(CHARM_ARCH "-x86")
    endif()

  else()

    message(FATAL "Selecting a Charm++ architecture for ${ARCH} is not supported")

  endif()

  if(ENABLE_AMPI)
    message(STATUS "Charm++'s Adaptive MPI (AMPI) enabled")
  endif()

  # Set Charm++ architecture
  if (NOT DEFINED ENV{CRAY_PRGENVPGI} AND  # not Cray
      NOT DEFINED ENV{CRAY_PRGENVGNU} AND
      NOT DEFINED ENV{CRAY_PRGENVCRAY} AND
      NOT DEFINED ENV{CRAY_PRGENVINTEL})

    if(NOT ENABLE_AMPI)
      set(CHARM_COMM_SUBSYSTEM "mpi")
    else()

      # AMPI status: (add opts via CHARM_EXTRA_ARGS)
      # ------------
      #set(CHARM_COMM_SUBSYSTEM "netlrts") # opts tried: udp, tcp, to try: smp, pxshm
      # - udp: unittest works but slow; inciter, walker, rngtest immaturely abort
      # - tcp: unittest works; inciter, walker, rngtest immaturely abort
      #set(CHARM_COMM_SUBSYSTEM "ofi")
      # - hdf5-ampi: mpiexec: Error: unknown option "-pmi_args"
      set(CHARM_COMM_SUBSYSTEM "multicore")
      # - works, unittest runs MPI suite, but fails randomlym others immaturely abort
      #set(CHARM_COMM_SUBSYSTEM "verbs")
      #  hdf5-ampi: charmun: error attaching to node '127.0.0.1'
      #set(CHARM_COMM_SUBSYSTEM "sim")  # not attempted
      #set(CHARM_COMM_SUBSYSTEM "uth")  # not attempted

    endif()

    set(CHARM_ARCHITECTURE "${CHARM_COMM_SUBSYSTEM}-${SYSTEM_NAME}${CHARM_ARCH}")

  else()  # Cray

    if(NOT ENABLE_AMPI)
      set(CHARM_COMM_SUBSYSTEM "mpi")
    else()
      set(CHARM_COMM_SUBSYSTEM "gni")   # never tested
    endif()

    set(CHARM_ARCHITECTURE "${CHARM_COMM_SUBSYSTEM}-crayxc")

  endif()

  message(STATUS "Charm++ architecture/version: ${CHARM_ARCHITECTURE}")

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Cray")  # cray-specific settings
    set(CHARM_COMPILER "craycc")
  elseif (ENABLE_AMPI AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CHARM_COMPILER "clang")
  endif()

  if (CHARM_COMPILER)
    message(STATUS "Charm++ compiler: ${CHARM_COMPILER}")
  endif()

endif(charm)

# Compilers
# =========
# There are two sets of compilers, given by the following CMake variables:
#
# 1. The underlying compiler suite
#    UNDERLYING_C_COMPILER
#    UNDERLYING_CXX_COMPILER
#    UNDERLYING_Fortran_COMPILER
#
# 2. The MPI wrappers (of the underlying compilers)
#    MPI_C_COMPILER
#    MPI_CXX_COMPILER
#    MPI_Fortran_COMPILER

# Find MPI and get its underlying compilers. If a traditional MPI (e.g.,
# OpenMPI) is used, the get_mpi_compilers cmake function uses the built-in
# CMake functionality to find the MPI wrappers and based on the wrappers
# detects the underyling compilers. If Charm++'s Adaptive MPI (AMPI) is used,
# the Charm++ TPL build builds Charm++ as well as AMPI and we use the AMPI
# wrappers to build MPI libraries.
set(ampi "${charm}")  # used as a dependency for MPI libs
set(ENABLE_AMPI OFF CACHE BOOL "Enable AMPI")
if(NOT ENABLE_AMPI)
  include(MPICompilers)
  get_mpi_compilers()
  set(CHARM_BUILD_TARGET "charm++")
  set(ampi "")
  set(CHARM_RUN "${CMAKE_INSTALL_PREFIX}/charm/bin/charmrun")
else()
  set(MPI_C_COMPILER "${CMAKE_INSTALL_PREFIX}/charm/bin/ampicc")
  set(MPI_CXX_COMPILER "${CMAKE_INSTALL_PREFIX}/charm/bin/ampicxx")
  set(MPI_Fortran_COMPILER "${CMAKE_INSTALL_PREFIX}/charm/bin/ampif90")
  set(AMPI_RUN "${CMAKE_INSTALL_PREFIX}/charm/bin/ampirun")
  set(CHARM_RUN "${CMAKE_INSTALL_PREFIX}/charm/bin/charmrun")
  set(UNDERLYING_C_COMPILER "${CMAKE_C_COMPILER}")
  set(UNDERLYING_CXX_COMPILER "${CMAKE_CXX_COMPILER}")
  set(UNDERLYING_Fortran_COMPILER "${CMAKE_Fortran_COMPILER}")
  set(CHARM_BUILD_TARGET "AMPI")
  MESSAGE(STATUS "AMPIrun: ${AMPI_RUN}")
endif()

MESSAGE(STATUS "MPI C compiler: " ${MPI_C_COMPILER})
MESSAGE(STATUS "MPI C++ compiler: " ${MPI_CXX_COMPILER})
MESSAGE(STATUS "MPI Fortran compiler: " ${MPI_Fortran_COMPILER})
MESSAGE(STATUS "Underlying C compiler: " ${UNDERLYING_C_COMPILER})
MESSAGE(STATUS "Underlying C++ compiler: " ${UNDERLYING_CXX_COMPILER})
MESSAGE(STATUS "Underlying Fortran compiler: " ${UNDERLYING_Fortran_COMPILER})

MESSAGE(STATUS "Charmrun: ${CHARM_RUN}")

if (CHARM_EXTRA_ARGS)
  MESSAGE(STATUS "Charm++ extra build args (CHARM_EXTRA_ARGS): ${CHARM_EXTRA_ARGS}")
endif()

# Compiler-specific settings
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")  # clang-specific settings

  # Include function that allows to set standard C++ library
  include(libstdcxx)

  # Offer switch between libc++ and libstdc++, using libc++ as default
  if (LIBCXX_FOUND)
    set_libstdcpp_vs_libcpp(CXXFLAGS "libc++")
  endif()

  # Don't error out on functions not returning anything even when supposed to,
  # required only by TestU01
  set(CFLAGS "${CFLAGS} -Wno-error=return-type")

  # Detect compiler version
  include(DetectCompilerVersion)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")  # gcc-specific settings

  # Gnu will use libstdc++ by default. We do not offer libc++.

  # Detect compiler version
  include(DetectCompilerVersion)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")  # intel-specific settings

  # Include function that allows to set standard C++ library
  include(libstdcxx)

  # Offer switch between libc++ and libstdc++, using libstdc++ as default
  if (LIBCXX_FOUND)
    set_libstdcpp_vs_libcpp(CXXFLAGS "libstdc++")
  endif()

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "PGI")  # pgi-specific settings

  # Some configure scripts don't like pgi's preprocessor so use gnu's
  set(PGCPP "CPP=cpp")

  # PGI C compiler flags specific to netcdf
  set(CFLAGS "${CFLAGS} -fPIC")
  set(NETCDF_UTILS "--disable-utilities")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Cray")  # cray-specific settings

else()

  message(WARNING "Compiler '${CMAKE_CXX_COMPILER_ID}' could not be identified.")

endif()

# Mac OS X specifics
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOSX_RPATH "-DCMAKE_MACOSX_RPATH:BOOL=ON")
endif()

# Attempt to find some TPLs on the system

# HDF5
if (ENABLE_HDF5)
  set(HDF5_PREFER_PARALLEL true)
  if(NOT BUILD_SHARED_LIBS)
    set(HDF5_USE_STATIC_LIBRARIES true)
  endif()
  if(ENABLE_AMPI)
    set(HDF5_ROOT ${CMAKE_INSTALL_PREFIX})
  endif()
  find_package(HDF5 COMPONENTS C HL)
  if ((ENABLE_AMPI AND HDF5_FOUND AND HDF5_IS_PARALLEL AND
       HDF5_INCLUDE_DIRS MATCHES ${CMAKE_INSTALL_PREFIX}) OR
      (NOT ENABLE_AMPI AND HDF5_FOUND AND HDF5_IS_PARALLEL))
    set(hdf5 "")
    set(hdf5_include_dirs_flags "${HDF5_INCLUDE_DIRS}")
    string(REGEX REPLACE "([^;]+)" "CPPFLAGS=-I\\1" hdf5_include_dirs_flags
           "${hdf5_include_dirs_flags}")
    set(hdf5_library_dirs "${HDF5_LIBRARIES}")
    list(GET HDF5_LIBRARIES 0 hdf5_root)  # get the first item of cmake list
    get_filename_component(hdf5_root_last ${hdf5_root} NAME)
    get_filename_component(hdf5_root ${hdf5_root} DIRECTORY )
    set(hdf5_library_dirs_flags "${hdf5_root}")
    string(REGEX REPLACE "([^;]+)" "LDFLAGS=-L\\1" hdf5_library_dirs_flags
          "${hdf5_library_dirs_flags}")
    set(hdf5_library_dirs_flags "${hdf5_library_dirs_flags}/lib")
    # strip "/lib" and "/lib64"
    if ("${hdf5_root}" MATCHES "lib(64)?$")
      get_filename_component(hdf5_root ${hdf5_root} DIRECTORY)
    endif()
  else()
    message(STATUS "HDF5 either not found, not parallel, disabled, or AMPI is enabled: will build")
    set(hdf5 "hdf5")
    set(hdf5_ampi "-ampi")
    set(hdf5_include_dirs_flags "CPPFLAGS=-I${CMAKE_INSTALL_PREFIX}/include")
    set(hdf5_library_dirs_flags "LDFLAGS=-L${CMAKE_INSTALL_PREFIX}/lib")
    set(hdf5_library_dirs "${CMAKE_INSTALL_PREFIX}/lib")
    set(hdf5_root "${CMAKE_INSTALL_PREFIX}")
  endif()
endif()

# NetCDF
if (ENABLE_NETCDF)
  if(ENABLE_AMPI)
    set(NETCDF_ROOT ${CMAKE_INSTALL_PREFIX})
  endif()
  find_package(NetCDF)
  if ((ENABLE_AMPI AND NETCDF_FOUND AND
       NETCDF_INCLUDES MATCHES ${CMAKE_INSTALL_PREFIX}) OR
      (NOT ENABLE_AMPI AND NETCDF_FOUND))
    set(netcdf "")
    set(netcdf_include_dirs "${NETCDF_INCLUDES}")
    set(netcdf_library_dirs "${NETCDF_LIBRARY_DIRS}")
  else()
    set(netcdf "netcdf")
    message(STATUS "NetCDF either not found or AMPI is enabled: will build")
    set(netcdf_include_dirs "${CMAKE_INSTALL_PREFIX}/include")
    set(netcdf_library_dirs "${CMAKE_INSTALL_PREFIX}/lib")
  endif()
endif()

# Trilinos
if (ENABLE_TRILINOS)
  find_package(Zoltan2 QUIET)
  find_package(SEACASExodus QUIET)
  find_package(Exodiff QUIET)

  if ((ENABLE_AMPI AND Zoltan2_FOUND AND SEACASExodus_FOUND AND Exodiff_FOUND AND
       Zoltan2_INCLUDE_DIRS MATCHES ${CMAKE_INSTALL_PREFIX}) OR
      (NOT ENABLE_AMPI AND Zoltan2_FOUND AND SEACASExodus_FOUND AND Exodiff_FOUND))
    set(trilinos "")
  else()
    set(trilinos "trilinos")
    message(STATUS "Zoltan2, SEACASExodus, and Exodiff either not found or AMPI is enabled: will build")
  endif()
endif()

# Get compiler flags (CFLAGS, CXXFLAGS, FFLAGS) independent of CMAKE_BUILD_TYPE
# and echo flags that will be passed to all TPL builds
include(get_compiler_flags)
get_compiler_flags()

set(tpls2build ${charm} ${hdf5} ${netcdf} ${trilinos})

list(LENGTH tpls2build ntpl)
list(SORT tpls2build)

# Output TPLs to be built
if (tpls2build)
  message(STATUS "Will build the following ${ntpl} TPLs: ${tpls2build}")
else()
  message(STATUS "All TPLs found. No need to build.")
endif()

# Define fine-grained targets
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS
             download patch configure build install test)

#### Charm++ ###################################################################
# http://charm.cs.illinois.edu
if(BUILD_SHARED_LIBS)
  set(CHARM_SHARED "--build-shared")
endif()
# Enable "--with-production" for Charm++ build for Release builds
if(CMAKE_BUILD_TYPE MATCHES "REL")      # Release | RelWithDebInfo | MinSizeRel
  set(CHARM_RELEASE_BUILD "--with-production")
endif()
if (HOST_OS MATCHES "alpine")
  # To avoid requiring rpc.h on Alpine Linux using musl-libc, see also
  # https://lists.cs.illinois.edu/lists/arc/charm/2016-04/msg00002.html
  set(CHARM_ALPINE "--disable-charmdebug")
endif()
if (charm)
  set(CHARM_CXXFLAGS "${CXXFLAGS}")
  string(REPLACE " " ";" CHARM_CXXFLAGS "${CHARM_CXXFLAGS}")
  ExternalProject_Add(
    charm
    PREFIX charm
    # Move unpacked tarball to install dir and build there
    DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy_directory
                      ${PROJECT_SOURCE_DIR}/src/charm
                      ${CMAKE_INSTALL_PREFIX}/charm
    # Will apply patch (if any) in SOURCE_DIR which is the same as install dir
    SOURCE_DIR ${CMAKE_INSTALL_PREFIX}/charm
    # Build in install dir and leave stuff there - no install step
    BINARY_DIR ${CMAKE_INSTALL_PREFIX}/charm
    # Build Charm++
    BUILD_COMMAND ${CMAKE_INSTALL_PREFIX}/charm/build
                  ${CHARM_BUILD_TARGET}
                  ${CHARM_ARCHITECTURE}
                  ${CHARM_EXTRA_ARGS}
                  ${CHARM_COMPILER}
                  ${CHARM_PTHREADS}
                  ${CHARM_SHARED}
                  ${CHARM_RELEASE_BUILD}
                  ${CHARM_ALPINE}
                  -j${PROCESSOR_COUNT}
                  ${CHARM_CXXFLAGS}
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    LOG_CONFIGURE 1
    LOG_BUILD 1
  )
endif()

#### HDF5 ######################################################################
# http://www.hdfgroup.org/HDF5
if (hdf5)
  if(NOT BUILD_SHARED_LIBS)
    set(HDF5_PATCH "patch;-p1;-t;-i;${PROJECT_SOURCE_DIR}/../cmake/hdf5_remove_get_time.patch")
  endif()
  if(BUILD_SHARED_LIBS AND NOT ENABLE_AMPI)
    set(HDF5_SHARED "--enable-shared")
  else()
    set(HDF5_SHARED "--disable-shared;--enable-static;--with-pic")
  endif()
  if (ENABLE_AMPI)
    set(AMPI_RUNSERIAL "RUNSERIAL=\'\'${AMPI_RUN} ++quiet\'\'" )
    set(AMPI_RUNPARALLEL "RUNPARALLEL=\'\'${AMPI_RUN} ++quiet +p4\'\'" )
  endif()
  ExternalProject_Add(
    hdf5
    PREFIX hdf5
    DEPENDS ${ampi}
    DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${PROJECT_SOURCE_DIR}/src/hdf5${hdf5_ampi}
                     ${PROJECT_BINARY_DIR}/hdf5
    SOURCE_DIR ${PROJECT_BINARY_DIR}/hdf5
    PATCH_COMMAND ${HDF5_PATCH}
    CONFIGURE_COMMAND ${PROJECT_BINARY_DIR}/hdf5/configure
                      CC=${MPI_C_COMPILER}
                      CFLAGS=${CFLAGS}
                      ${PGCPP}
                      ${AMPI_RUNSERIAL}
                      ${AMPI_RUNPARALLEL}
                      --enable-parallel
                      --enable-symbols=yes
                      ${HDF5_SHARED}
                      --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
  )
endif()

#### NETCDF ####################################################################
## http://www.unidata.ucar.edu/downloads/netcdf/index.jsp
if (netcdf)
  if(BUILD_SHARED_LIBS AND NOT ENABLE_AMPI)
    set(NETCDF_SHARED "--enable-shared")
  else()
    set(NETCDF_SHARED "--enable-static;--disable-shared;--with-pic")
    if(ARCH MATCHES "ppc64")
      set(NETCDF_ZLIB "LIBS=${ZLIB_LIBRARIES}")
      set(NETCDF_DISABLE_TESTS "--disable-testsets")
    endif()
  endif()
  ExternalProject_Add(
    netcdf
    DEPENDS ${hdf5} ${ampi}
    PREFIX netcdf
    DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${PROJECT_SOURCE_DIR}/src/netcdf
                     ${PROJECT_BINARY_DIR}/netcdf
    CONFIGURE_COMMAND ${PROJECT_BINARY_DIR}/netcdf/configure
                      CC=${MPI_C_COMPILER}
                      CFLAGS=${CFLAGS}
                      ${NETCDF_UTILS}
                      ${PGCPP}
                      ${hdf5_include_dirs_flags}
                      ${hdf5_library_dirs_flags}
                      --enable-netcdf4
                      --disable-examples
                      --disable-fsync
                      --disable-dap
                      ${NETCDF_SHARED}
                      ${NETCDF_DISABLE_TESTS}
                      ${NETCDF_ZLIB}
                      --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
  )
endif()

##### Trilinos #################################################################
## http://trilinos.sandia.gov
if(BUILD_SHARED_LIBS AND NOT ENABLE_AMPI)
  set(TRILINOS_SHARED "-DBUILD_SHARED_LIBS=ON")
else()
  set(TRILINOS_SHARED "-DBUILD_SHARED_LIBS=OFF;-DTPL_FIND_SHARED_LIBS:BOOL=OFF;-DTrilinos_LINK_SEARCH_START_STATIC:BOOL=ON;-DCMAKE_SKIP_INSTALL_RPATH=TRUE")
  if(ARCH MATCHES "ppc64")
    set(tr_hdf5 "-DTrilinos_EXTRA_LINK_FLAGS=-L${hdf5_library_dirs} -lhdf5_hl -lhdf5 ${ZLIB_LIBRARIES} -lm -ldl")
  else()
    set(tr_hdf5 "-DTrilinos_EXTRA_LINK_FLAGS=-L${hdf5_library_dirs} -lhdf5_hl -lhdf5 -lz -lm -ldl")
  endif()
endif()
if (HOST_OS MATCHES "alpine")
  set(TRILINOS_ALPINE "-DBLAS_LIBRARY_DIRS=${CMAKE_INSTALL_PREFIX}/lib;-DLAPACK_LIBRARY_DIRS=${CMAKE_INSTALL_PREFIX}/lib;-DTrilinos_CXX11_FLAGS=-std=c++11")
endif()
if (MKL_FOUND)
  set(TRILINOS_MKL "-DTPL_ENABLE_BLAS=ON;-DTPL_BLAS_LIBRARIES=${MKL_LIBRARIES};-DTPL_LAPACK_LIBRARIES=${MKL_LIBRARIES}")
  if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(TRILINOS_MKL "-DTPL_ENABLE_MKL=ON;-DMKL_LIBRARY_DIRS=$ENV{MKLROOT}/lib;-DMKL_INCLUDE_DIRS=$ENV{MKLROOT}/include;-DTPL_BLAS_LIBRARIES=${MKL_LIBRARIES};-DTPL_LAPACK_LIBRARIES=${MKL_LIBRARIES}")
  endif()
else()
  if(BLAS_FOUND)
    set(TRILINOS_BLAS -DTPL_BLAS_LIBRARIES=${BLAS_LIBRARIES})
  else()
    set(TRILINOS_BLAS -DBLAS_LIBRARY_DIRS=${CMAKE_INSTALL_PREFIX}/lib)
  endif()
  if(LAPACKE_FOUND)
    set(TRILINOS_LAPACK -DTPL_LAPACK_LIBRARIES=${LAPACKE_LIBRARIES})
  else()
    set(TRILINOS_LAPACK -DLAPACK_LIBRARY_DIRS=${CMAKE_INSTALL_PREFIX}/lib)
  endif()
endif()
if(trilinos)
  ExternalProject_Add(
    trilinos
    DEPENDS ${hdf5} ${netcdf} ${lapack} ${ampi}
    PREFIX trilinos
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/trilinos
    PATCH_COMMAND patch -p1 -t -i ${PROJECT_SOURCE_DIR}/../cmake/exodiff_cstdint.patch
    CMAKE_ARGS -D Trilinos_ENABLE_ALL_PACKAGES:BOOL=OFF
               -D TPL_ENABLE_MPI:BOOL=ON
               -D Trilinos_ENABLE_ALL_OPTIONAL_PACKAGES=OFF
               -D Trilinos_ENABLE_Zoltan2:BOOL=ON
               -D Zoltan2_ENABLE_Experimental:BOOL=ON
               -D Trilinos_ENABLE_SECONDARY_STABLE_CODE:BOOL=ON
               -D Trilinos_ENABLE_SEACASExodus:BOOL=ON
               -D Trilinos_ENABLE_SEACASExodiff:BOOL=ON
               -D Trilinos_ENABLE_Fortran=OFF
               -D Trilinos_ENABLE_DEBUG=OFF
               ${tr_hdf5}
               ${TRILINOS_SHARED}
               ${TRILINOS_ALPINE}
               ${TRILINOS_MKL}
               ${TRILINOS_BLAS}
               ${TRILINOS_LAPACK}
               ${MACOSX_RPATH}
               -D TPL_ENABLE_Netcdf:BOOL=ON
               -D Netcdf_LIBRARY_DIRS:PATH=${netcdf_library_dirs}
               -D TPL_Netcdf_INCLUDE_DIRS:PATH=${netcdf_include_dirs}
               -D CMAKE_INSTALL_RPATH:PATH=${CMAKE_INSTALL_PREFIX}/lib
               -D CMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
               -D CMAKE_C_COMPILER:PATH=${MPI_C_COMPILER}
               -D CMAKE_CXX_COMPILER:PATH=${MPI_CXX_COMPILER}
               -D CMAKE_C_FLAGS:STRING=${CFLAGS}
               -D CMAKE_CXX_FLAGS:STRING=${CXXFLAGS}
               -D CMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    DOWNLOAD_COMMAND ""
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
  )
endif()
