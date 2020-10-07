// *****************************************************************************
/*!
  \file      src/NoWarning/partitioner.decl.h
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include partitioner.decl.h with turning off specific compiler
             warnings
*/
// *****************************************************************************
#ifndef nowarning_partitioner_decl_h
#define nowarning_partitioner_decl_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wunused-parameter"
  #pragma clang diagnostic ignored "-Wold-style-cast"
  #pragma clang diagnostic ignored "-Wextra-semi"
  #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
  #pragma clang diagnostic ignored "-Wdeprecated"
  #pragma clang diagnostic ignored "-Wsign-conversion"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wshadow"
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "../Main/partitioner.decl.h"

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#endif

#endif // nowarning_partitioner_decl_h
