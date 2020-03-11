cmake_minimum_required(VERSION 2.8.5)

add_library(Mesh
            ZoltanInterOp.cpp
            Reorder.cpp)

target_include_directories(Mesh PUBLIC
                           ${PROJECT_SOURCE_DIR}
                           ${PROJECT_SOURCE_DIR}/Base
                           ${Zoltan2_INCLUDE_DIRS}
                           ${HIGHWAYHASH_INCLUDE_DIRS}
                           ${MPI_CXX_INCLUDE_DIRS})

set_target_properties(Mesh PROPERTIES LIBRARY_OUTPUT_NAME exam2m_mesh)

INSTALL(TARGETS Mesh
  	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
  	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
 	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
)
