// *****************************************************************************
/*!
  \file      src/NoWarning/transporter.decl.h
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include transporter.decl.h with turning off specific compiler
             warnings
*/
// *****************************************************************************
#ifndef nowarning_transporter_decl_h
#define nowarning_transporter_decl_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
//  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wcast-qual"
  #pragma GCC diagnostic ignored "-Wshadow"
  #pragma GCC diagnostic ignored "-Wredundant-decls"
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
  #pragma GCC diagnostic ignored "-Wfloat-equal"
  #pragma GCC diagnostic ignored "-Wextra"
#elif defined(__INTEL_COMPILER)
  #pragma warning( push )
//  #pragma warning( disable: 2282 )
#endif

#include "../Main/transporter.decl.h"

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#elif defined(__INTEL_COMPILER)
  #pragma warning( pop )
#endif

#endif // nowarning_transporter_decl_h
