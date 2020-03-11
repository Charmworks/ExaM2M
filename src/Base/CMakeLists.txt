cmake_minimum_required(VERSION 2.8.5)

add_library(Base
            Exception.cpp
            ProcessException.cpp)

target_include_directories(Base PUBLIC
                           ${PROJECT_SOURCE_DIR}
                           ${PROJECT_SOURCE_DIR}/Base
                           ${MPI_CXX_INCLUDE_DIRS}
                           ${CHARM_INCLUDE_DIRS})

set_target_properties(Base PROPERTIES LIBRARY_OUTPUT_NAME exam2m_base)

INSTALL(TARGETS Base
       RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
       LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
       ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development
)
