cmake_minimum_required(VERSION 2.8.5)

### ExaM2M executables build configuration #####################################

# Basic configuration
add_definitions()

# Include function for adding Charm++ modules
include(charm)

# Link executables with the charmc wrapper
STRING(REGEX REPLACE "<CMAKE_CXX_COMPILER>"
       "${LINKER_COMPILER} -module CommonLBs ${EXTRA_LINK_ARGS} -c++ <CMAKE_CXX_COMPILER>"
       CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE}")

include(ConfigExecutable)

# Configure executable targets

add_executable(${EXAM2M_EXECUTABLE}
               Mapper.cpp
               Partitioner.cpp
               Driver.cpp
               MeshArray.cpp
               ExaM2M.cpp)

config_executable(${EXAM2M_EXECUTABLE})

target_include_directories(${EXAM2M_EXECUTABLE} PUBLIC
                           ${PROJECT_SOURCE_DIR}
                           ${PROJECT_SOURCE_DIR}/IO
                           ${PROJECT_SOURCE_DIR}/Mesh
                           ${PROJECT_SOURCE_DIR}/Transfer
                           ${PROJECT_SOURCE_DIR}/Main
                           ${PROJECT_BINARY_DIR}/IO
                           ${PROJECT_BINARY_DIR}/Main
                           ${NETCDF_INCLUDES}
                           ${CHARM_INCLUDE_DIRS}
                           ${HIGHWAYHASH_INCLUDE_DIRS})

target_link_libraries(${EXAM2M_EXECUTABLE}
                      Base
                      Mesh
                      ExodusIIMeshIO
                      MeshWriter
                      Worker
                      ${SEACASExodus_LIBRARIES}
                      ${Zoltan2_LIBRARIES}
                      ${LAPACKE_LIBRARIES}    # only if MKL not found
                      ${MKL_INTERFACE_LIBRARY}
                      ${MKL_SEQUENTIAL_LAYER_LIBRARY}
                      ${MKL_CORE_LIBRARY}
                      ${MKL_INTERFACE_LIBRARY}
                      ${MKL_SEQUENTIAL_LAYER_LIBRARY}
                      ${LIBCXX_LIBRARIES}     # only for static link with libc++
                      ${LIBCXXABI_LIBRARIES}) # only for static link with libc++

addCharmModule( "mapper" "${EXAM2M_EXECUTABLE}" )
addCharmModule( "partitioner" "${EXAM2M_EXECUTABLE}" )
addCharmModule( "mesharray" "${EXAM2M_EXECUTABLE}" )
addCharmModule( "driver" "${EXAM2M_EXECUTABLE}" )
addCharmModule( "exam2m" "${EXAM2M_EXECUTABLE}" )
