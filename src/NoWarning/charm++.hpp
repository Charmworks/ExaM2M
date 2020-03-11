// *****************************************************************************
/*!
  \file      src/NoWarning/charm++.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include charm++.h with turning off specific compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_charmpp_h
#define nowarning_charmpp_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
//  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
//  #pragma GCC diagnostic ignored "-Warray-bounds"
#elif defined(__INTEL_COMPILER)
  #pragma warning( push )
//  #pragma warning( disable: 2282 )
#endif

#include <charm++.h>

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#elif defined(__INTEL_COMPILER)
  #pragma warning( pop )
#endif

#endif // nowarning_charmpp_h
