cmake_minimum_required(VERSION 2.8.5)

# Include function for adding Charm++ modules
include(charm)

add_library(Worker
            Worker.cpp)

target_include_directories(Worker PUBLIC
                           ${PROJECT_SOURCE_DIR}
                           ${PROJECT_SOURCE_DIR}/Base
                           ${PROJECT_SOURCE_DIR}/Mesh
                           ${PROJECT_BINARY_DIR}/Transfer
                           ${PROJECT_BINARY_DIR}/Main
                           ${PROJECT_BINARY_DIR}/IO
                           ${CHARM_INCLUDE_DIRS}
                           ${HIGHWAYHASH_INCLUDE_DIRS}
)

set_target_properties(Worker PROPERTIES LIBRARY_OUTPUT_NAME exam2m_worker
                      PUBLIC_HEADER "Controller.hpp")

INSTALL(TARGETS Worker
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

addCharmModule( "worker" "Worker" )
