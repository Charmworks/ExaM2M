// *****************************************************************************
/*!
  \file      src/NoWarning/mpi.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include mpi.h with turning off specific compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_mpi_h
#define nowarning_mpi_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
//  #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
//  #pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

#include <mpi.h>

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#endif


#endif // nowarning_mpi_h
