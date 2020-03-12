################################################################################
#
# \file      src/CMakeLists.txt
# \copyright 2020 Charmworks, Inc.
#            All rights reserved See the LICENSE file for details.
# \brief     Build ExaM2M
#
################################################################################

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(EXAM2M C CXX)

set(CMAKE_VERBOSE_MAKEFILE 1)

# ExaM2M cmake modules directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

# Disallow in-source builds
include(DisallowInSourceBuilds)
disallow_in_source_builds()

# Set version number: major.minor
set(MAJOR_VER 0)
set(MINOR_VER 1)

# Sets CMAKE_INSTALL_{BIN,LIB}DIR needed for multilib support
include(GNUInstallDirs)

# Set CMAKE_BUILD_TYPE
include(BuildType)

# Set BUILD_SHARED_LIBS
include(BuildShared)

# Conditionally enforce static linking for executables added after this point.
# This must precede all add_executable() calls adding executables to link
# statically.
if (NOT BUILD_SHARED_LIBS)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif()

# Convert CMAKE_BUILD_TYPE
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

# Query target architecture
include(TargetArch)
target_architecture(ARCH)
message(STATUS "Target architecture: ${ARCH}")

# Echo install dir
message(STATUS "CMAKE_INSTALL_PREFIX: " ${CMAKE_INSTALL_PREFIX})

# Detect operating system type and version
include(DetectOS)
detect_os()
message(STATUS "Detected OS: ${HOST_OS}")

# If not specified on the command-line, set default third-party libraries
# directory based on cmake-supplied compiler ID
if (NOT TPL_DIR)
  string(TOLOWER "${CMAKE_CXX_COMPILER_ID}" COMP)
  set(REL_TPL_DIR ${PROJECT_SOURCE_DIR}/../external/install/${COMP}-${ARCH})
  get_filename_component(ABS_TPL_DIR ${REL_TPL_DIR} ABSOLUTE)
  set(TPL_DIR ${ABS_TPL_DIR} CACHE STRING "Third-party libraries directory.")
endif()

if (EXISTS ${TPL_DIR}/include)
  set(TPL_INCLUDE_DIR ${TPL_DIR}/include)
endif()
message(STATUS "Third-party library dir: " ${TPL_DIR})

# Detect compiler version
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  include(DetectCompilerVersion)
endif()

# Include third-party libraries configuration
include(TPLs)

# Compilers
# =========
# There are two sets of compilers, given by the following CMake variables:
#
# 1. The underlying compiler suite
#    UNDERLYING_C_COMPILER
#    UNDERLYING_CXX_COMPILER
#
# 2. The MPI wrappers (of the underlying compilers)
#    MPI_C_COMPILER
#    MPI_CXX_COMPILER

# Find MPI and get its underlying compilers. If a traditional MPI (e.g.,
# OpenMPI) is used, the get_mpi_compilers cmake function uses the built-in CMake
# functionality to find the MPI wrappers and based on the wrappers detects the
# underyling compilers If Charm++'s Adaptive MPI (AMPI) is used, the Charm++
# TPL build builds Charm++ as well as AMPI and we use the AMPI wrappers to build
# MPI libraries.
if(NOT ENABLE_AMPI)
  include(MPICompilers)
  get_mpi_compilers()
  set(LINKER_COMPILER "${CHARM_COMPILER}")
else()
  set(MPI_C_COMPILER "${AMPI_C_COMPILER}")
  set(MPI_CXX_COMPILER "${AMPI_CXX_COMPILER}")
  set(UNDERLYING_C_COMPILER "${CMAKE_C_COMPILER}")
  set(UNDERLYING_CXX_COMPILER "${CMAKE_CXX_COMPILER}")
  set(LINKER_COMPILER "${AMPI_CXX_COMPILER}")
  set(MPI_CXX_INCLUDE_DIRS "")
  MESSAGE(STATUS "AMPIrun: " ${AMPI_RUN})
endif()

MESSAGE(STATUS "MPI C compiler: " ${MPI_C_COMPILER})
MESSAGE(STATUS "MPI C++ compiler: " ${MPI_CXX_COMPILER})
MESSAGE(STATUS "Underlying C compiler: " ${UNDERLYING_C_COMPILER})
MESSAGE(STATUS "Underlying C++ compiler: " ${UNDERLYING_CXX_COMPILER})

MESSAGE(STATUS "Charmrun: " ${CHARM_RUN})
MESSAGE(STATUS "Linker compiler: " ${LINKER_COMPILER})

if (EXTRA_LINK_ARGS)
  MESSAGE(STATUS "Extra linker args (EXTRA_LINK_ARGS): ${EXTRA_LINK_ARGS}")
endif()

add_definitions(-DEXAM2M_CONFIG_MPI_ENABLED)

# Set compilers
set(COMPILER ${UNDERLYING_CXX_COMPILER})
set(MPI_COMPILER ${MPI_CXX_COMPILER})

# Mac OS X specifics
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  # On Mac OS X, set RPATH
  set(CMAKE_MACOSX_RPATH ON)

  # On Mac OS X, do not to resolve symbols when creating a shared library and
  # only resolve the symbols when linking executables. This behavior is
  # consistent with gcc, intel, and clang on linux.
  set(CMAKE_SHARED_LINKER_FLAGS
      "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")

  # On Mac OS X, due to changes in SnowLeopard (darwin10) linker changes, we add
  # "-no_compact_unwind" to the link line. This tells the linker to not produce
  # compact unwind information from the dwarf unwind info in .o files. Then at
  # runtime the darwin unwinder will fallback and use the slow dwarf unwind
  # info.
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no_compact_unwind" )

  # Set minimum OSX SDK target version
  #set(CMAKE_OSX_DEPLOYMENT_TARGET 107 CACHE STRING "Minimum OS X version to target for deployment (at runtime); newer APIs weak linked. Set to empty string for default value." FORCE)

endif()

# Set the requirement for the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "Required language standard: C++${CMAKE_CXX_STANDARD}")

# Macro to add C++ compiler flags
macro (add_compiler_flag flag)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endmacro (add_compiler_flag)

# Macro to add C compiler flags
macro (add_c_compiler_flag flag)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
endmacro (add_c_compiler_flag)

# Compiler-specific settings
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")  # clang-specific settings

  # Include function that allows to set standard C++ library
  include(libstdcxx)

  # Offer switch between libc++ and libstdc++, using libc++ as default
  if (LIBCXX_FOUND)
    set_libstdcpp_vs_libcpp(CMAKE_CXX_FLAGS "libc++")
  endif()

  # Compiler flags for clang
  add_compiler_flag("-fcolor-diagnostics")

  add_compiler_flag("-Weverything")
  add_compiler_flag("-Wno-c++98-compat")
  add_compiler_flag("-Wno-c++98-compat-pedantic")
  add_compiler_flag("-Wno-padded")
  add_compiler_flag("-Wno-exit-time-destructors")
  add_compiler_flag("-Wno-global-constructors")
  add_compiler_flag("-Wno-weak-vtables")
  add_compiler_flag("-Wno-unknown-pragmas")
  add_compiler_flag("-Wno-unknown-warning-option")
  add_compiler_flag("-Wno-unused-command-line-argument")
  add_compiler_flag("-Wno-disabled-macro-expansion")
  add_compiler_flag("-Wno-reserved-id-macro")
  add_compiler_flag("-Wno-unknown-warning-option")

  # Linker flags for clang
  set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)      # clear link flags
  set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -Wno-missing-prototypes -Wno-unused-parameter")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")  # gcc-specific settings

  # Gnu will use libstdc++ by default. We do not offer libc++.

  # Compiler flags for gcc
  add_compiler_flag("-fdiagnostics-color")

  add_compiler_flag("-Wall")
  add_compiler_flag("-Wextra")
  add_compiler_flag("-Wcast-align")
  add_compiler_flag("-Wcast-qual")
  add_compiler_flag("-Wdisabled-optimization")
  add_compiler_flag("-Wfloat-equal")
  add_compiler_flag("-Wformat=2")
  add_compiler_flag("-Wformat-nonliteral")
  add_compiler_flag("-Wformat-security")
  add_compiler_flag("-Wformat-y2k")
  add_compiler_flag("-Wimport")
  add_compiler_flag("-Winit-self")
  add_compiler_flag("-Winvalid-pch")
  add_compiler_flag("-Wmissing-field-initializers")
  add_compiler_flag("-Wmissing-format-attribute")
  add_compiler_flag("-Wmissing-noreturn")
  add_compiler_flag("-Wpacked")
  add_compiler_flag("-Wpointer-arith")
  add_compiler_flag("-Wredundant-decls")
  add_compiler_flag("-Wshadow")
  add_compiler_flag("-Wstack-protector")
  add_compiler_flag("-Wstrict-aliasing=2")
  add_compiler_flag("-Wunreachable-code")
  add_compiler_flag("-Wunused")
  add_compiler_flag("-Wunused-parameter")
  add_compiler_flag("-Wvariadic-macros")
  add_compiler_flag("-Wwrite-strings")
  add_compiler_flag("-Wno-sign-compare")
  add_compiler_flag("-Wno-unused-function")
  add_compiler_flag("-Wno-stack-protector")
  add_compiler_flag("-Wno-expansion-to-defined")
  add_compiler_flag("-Wno-int-in-bool-context")
  add_compiler_flag("-Wno-cast-function-type")
  add_compiler_flag("-Wno-format-overflow")
  add_compiler_flag("-Wno-pragmas")

  # Linker flags for gcc
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wno-unused-parameter")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")  # intel-specific settings

  # Include function that allows to set standard C++ library
  include(libstdcxx)

  # Offer switch between libc++ and libstdc++, using libstdc++ as default
  if (LIBCXX_FOUND)
    set_libstdcpp_vs_libcpp(CMAKE_CXX_FLAGS "libstdc++")
  endif()

  # Compiler flags for intel
  add_compiler_flag("-w3")       # enable diagnostics: remarks, warnings, errors
  add_compiler_flag("-Wremarks") # display remarks and comments

  # Globally suppress the following Intel compiler remarks:
  #     7: unrecognized token
  #    68: integer conversion resulted in a change of sign
  #    82: storage class is not first
  #   111: statement is unreachable
  #   186: pointless comparison of unsigned integer with zero
  #   177: function was declared but never referenced
  #   193: zero used for undefined preprocessing identifier
  #   280: selector expression is constant
  #   304: access control not specified ("public" by default)
  #   383: value copied to temporary, reference to temporary used
  #   327: NULL reference is not allowed
  #   411: defines no constructor for initializer_list<>
  #   444: destructor for base class is not virtual
  #   593: parameter was set but never used
  #   869: parameter was never referenced
  #   981: operands are evaluated in unspecified order
  #  1011: missing return statement at end of non-void function (constexpr)
  #  1292: unknown attribute "fallthrough"
  #  1418: external function definition with no prior declaration
  #  1419: external declaration in primary source file
  #  1572: floating-point equality and inequality comparisons are unreliable
  #  2259: non-pointer conversion from "int" to "char" may lose significant bits
  #  2407: the initialization of class will be done before that of member
  #  2415: variable of static storage duration was declared but never referenced
  #  2547: non-system include dir will be ignored when both a system and non-system include directory were specified
  #  2557: comparison between signed and unsigned operands
  #  3280: declaration hides member
  #  3346: dynamic exception specifications are deprecated
  #  3656: variable may be used before its value is set
  #  10120: overriding '-O3' with '-O2'
  add_compiler_flag("-diag-disable 7,68,82,111,177,186,193,280,304,327,383,411,444,593,869,981,1011,1292,1418,1419,1572,2259,2407,2415,2547,2557,3280,3346,3656,10120")

  # Compiler flags for optimization report
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qopt-report=0 -qopt-report-phase=loop,vec -qopt-report-file=stderr")

  # No limit on inlining. This can increase the build time.
  if(CMAKE_BUILD_TYPE MATCHES "REL") # Release | RelWithDebInfo | MinSizeRel
    #add_compiler_flag("-no-inline-max-size")
    #add_compiler_flag("-no-inline-max-total-size")
    #add_compiler_flag("-no-inline-max-per-routine")
    add_compiler_flag("-diag-disable 11074,11076")
  endif()

  # Linker flags for intel
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -diag-disable 869")

else()

  message(WARNING "Compiler '${CMAKE_CXX_COMPILER_ID}' could not be identified.")

endif()

# Echo compiler flags
message(STATUS "C compiler flags: '${CMAKE_C_FLAGS}'")
message(STATUS "C++ compiler flags: '${CMAKE_CXX_FLAGS}'")
message(STATUS "Executable linker flags: '${CMAKE_EXE_LINKER_FLAGS}'")

# Include cmake code to enable configuration for data layouts
include(ConfigureDataLayout)

# Configure cmake variables to pass to the build
configure_file( "${PROJECT_SOURCE_DIR}/Main/ExaM2MConfig.hpp.in"
                "${PROJECT_BINARY_DIR}/Main/ExaM2MConfig.hpp" )

if (BUILD_SHARED_LIBS)

  ### Set RPATH so that shared libs paths stay in the binary after install ###
  # Don't skip the full RPATH for the build tree
  set(CMAKE_SKIP_BUILD_RPATH false)

  # When building, don't use the install RPATH
  # (use later on when installing)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH false)

  # Add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)

  # the RPATH to be used when installing, but only if it's not a system directory
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
       "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
     set(CMAKE_INSTALL_RPATH
       "${CMAKE_INSTALL_RPATH};${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  endif("${isSystemDir}" STREQUAL "-1")

endif(BUILD_SHARED_LIBS)

# Set executable name
set(EXAM2M_EXECUTABLE exam2m)

# Components
if (CHARM_FOUND AND BRIGAND_FOUND AND SEACASExodus_FOUND AND EXODIFF_FOUND AND
    HDF5_FOUND AND BRIGAND_FOUND AND HIGHWAYHASH_FOUND)
  add_subdirectory(Base)
  add_subdirectory(IO)
  add_subdirectory(Mesh)
  add_subdirectory(Transfer)
  add_subdirectory(Main)
endif()

# Conditionally enforce static link for all executables. This must be after all
# add_executable() calls creating the targets for executables in EXECUTABLES.
if (NOT BUILD_SHARED_LIBS)
  foreach(exe IN LISTS EXECUTABLES)
    set_property(TARGET ${exe} PROPERTY LINK_SEARCH_START_STATIC 1)
    set_property(TARGET ${exe} PROPERTY LINK_SEARCH_END_STATIC 1)
  endforeach()
endif()
