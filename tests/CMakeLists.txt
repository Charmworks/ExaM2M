################################################################################
#
# \file      tests/CMakeLists.txt
# \copyright 2020 Charmworks, Inc.
#            All rights reserved See the LICENSE file for details.
# \brief     Cmake code common to all regression testing
#
################################################################################

# Set paths required for running tests
set(TEST_RUNNER ${CMAKE_SOURCE_DIR}/../cmake/test_runner.cmake)

# Include function used to add regression tests
include(add_regression_test)

add_regression_test(self_sphere ${EXAM2M_EXECUTABLE}
                    NUMPES 1
                    INPUTFILES meshes/sphere_tetra.0.2.exo
                    ARGS sphere_tetra.0.2.exo sphere_tetra.0.2.exo 3
                    BIN_BASELINE self_sphere.src.std.exo
                                 self_sphere.dst.std.exo
                    BIN_RESULT out.0.e-s.0.1.0
                               out.1.e-s.0.1.0
                    BIN_DIFF_PROG_CONF exodiff.cfg)

add_regression_test(sphere2box ${EXAM2M_EXECUTABLE}
                    NUMPES 1
                    INPUTFILES meshes/sphere_full.exo meshes/unitcube_94K.exo
                    ARGS sphere_full.exo unitcube_94K.exo 3
                    BIN_BASELINE sphere2box.src.std.exo
                                 sphere2box.dst.std.exo
                    BIN_RESULT out.0.e-s.0.1.0
                               out.1.e-s.0.1.0
                    BIN_DIFF_PROG_CONF exodiff.cfg)

add_regression_test(sphere2box ${EXAM2M_EXECUTABLE}
                    NUMPES 2
                    PPN 1
                    INPUTFILES meshes/sphere_full.exo meshes/unitcube_94K.exo
                    ARGS sphere_full.exo unitcube_94K.exo 3
                    BIN_BASELINE sphere2box_pe2.src.std.exo.0
                                 sphere2box_pe2.src.std.exo.1
                                 sphere2box_pe2.dst.std.exo.0
                                 sphere2box_pe2.dst.std.exo.1
                    BIN_RESULT out.0.e-s.0.2.0
                               out.0.e-s.0.2.1
                               out.1.e-s.0.2.0
                               out.1.e-s.0.2.1
                    BIN_DIFF_PROG_CONF exodiff.cfg)

#add_regression_test(sphere2box_u0.8 ${EXAM2M_EXECUTABLE}
#                    NUMPES 2
#                    PPN 1
#                    INPUTFILES meshes/sphere_full.exo meshes/unitcube_94K.exo
#                    ARGS sphere_full.exo unitcube_94K.exo 1 0.8
#                    BIN_BASELINE sphere2box_u0.8_pe2.src.std.exo.0
#                                 sphere2box_u0.8_pe2.src.std.exo.1
#                                 sphere2box_u0.8_pe2.src.std.exo.2
#                                 sphere2box_u0.8_pe2.src.std.exo.3
#                                 sphere2box_u0.8_pe2.src.std.exo.4
#                                 sphere2box_u0.8_pe2.src.std.exo.5
#                                 sphere2box_u0.8_pe2.src.std.exo.6
#                                 sphere2box_u0.8_pe2.src.std.exo.7
#                                 sphere2box_u0.8_pe2.src.std.exo.8
#                                 sphere2box_u0.8_pe2.src.std.exo.9
#                                 sphere2box_u0.8_pe2.dst.std.exo.0
#                                 sphere2box_u0.8_pe2.dst.std.exo.1
#                                 sphere2box_u0.8_pe2.dst.std.exo.2
#                                 sphere2box_u0.8_pe2.dst.std.exo.3
#                                 sphere2box_u0.8_pe2.dst.std.exo.4
#                                 sphere2box_u0.8_pe2.dst.std.exo.5
#                                 sphere2box_u0.8_pe2.dst.std.exo.6
#                                 sphere2box_u0.8_pe2.dst.std.exo.7
#                                 sphere2box_u0.8_pe2.dst.std.exo.8
#                                 sphere2box_u0.8_pe2.dst.std.exo.9
#                    BIN_RESULT out.0.e-s.0.10.0
#                               out.0.e-s.0.10.1
#                               out.0.e-s.0.10.2
#                               out.0.e-s.0.10.3
#                               out.0.e-s.0.10.4
#                               out.0.e-s.0.10.5
#                               out.0.e-s.0.10.6
#                               out.0.e-s.0.10.7
#                               out.0.e-s.0.10.8
#                               out.0.e-s.0.10.9
#                               out.1.e-s.0.10.0
#                               out.1.e-s.0.10.1
#                               out.1.e-s.0.10.2
#                               out.1.e-s.0.10.3
#                               out.1.e-s.0.10.4
#                               out.1.e-s.0.10.5
#                               out.1.e-s.0.10.6
#                               out.1.e-s.0.10.7
#                               out.1.e-s.0.10.8
#                               out.1.e-s.0.10.9
#                    BIN_DIFF_PROG_CONF exodiff.cfg)
