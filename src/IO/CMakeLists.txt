cmake_minimum_required(VERSION 2.8.5)

# Include function for adding Charm++ modules
include(charm)

add_library(ExodusIIMeshIO
            ExodusIIMeshReader.cpp
            ExodusIIMeshWriter.cpp)

target_include_directories(ExodusIIMeshIO PUBLIC
                           ${PROJECT_SOURCE_DIR}
                           ${PROJECT_SOURCE_DIR}/Base
                           ${PROJECT_SOURCE_DIR}/Mesh
                           ${SEACASExodus_INCLUDE_DIRS}
                           ${NETCDF_INCLUDES}
                           ${HIGHWAYHASH_INCLUDE_DIRS}
                           ${MPI_CXX_INCLUDE_DIRS})

set_target_properties(ExodusIIMeshIO PROPERTIES LIBRARY_OUTPUT_NAME
                      exam2m_exodusiimeshio)

INSTALL(TARGETS ExodusIIMeshIO
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
)

add_library(MeshWriter
            MeshWriter.cpp)

target_include_directories(MeshWriter PUBLIC
                           ${PROJECT_SOURCE_DIR}
                           ${PROJECT_SOURCE_DIR}/Base
                           ${PROJECT_SOURCE_DIR}/Mesh
                           ${PROJECT_BINARY_DIR}/IO
                           ${NETCDF_INCLUDES}
                           ${CHARM_INCLUDE_DIRS}
                           ${HIGHWAYHASH_INCLUDE_DIRS}
)

set_target_properties(MeshWriter PROPERTIES LIBRARY_OUTPUT_NAME
                      exam2m_meshwriter)

INSTALL(TARGETS MeshWriter
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
)

addCharmModule( "meshwriter" "MeshWriter" )
