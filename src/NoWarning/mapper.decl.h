// *****************************************************************************
/*!
  \file      src/NoWarning/mapper.decl.h
  \copyright 2012-2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include mapper.decl.h with turning off specific compiler warnings.
*/
// *****************************************************************************
#ifndef nowarning_mapper_decl_h
#define nowarning_mapper_decl_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
  #pragma clang diagnostic ignored "-Wold-style-cast"
  #pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wshadow"
#endif

#include "../Main/mapper.decl.h"

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#endif

#endif // nowarning_mapper_decl_h
