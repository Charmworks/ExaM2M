// *****************************************************************************
/*!
  \file      src/NoWarning/charm.hpp
  \copyright 20202 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include charm.hpp with turning off specific compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_charm_h
#define nowarning_charm_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
//  #pragma clang diagnostic ignored "-Wconversion"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wcast-qual"
#elif defined(__INTEL_COMPILER)
  #pragma warning( push )
//  #pragma warning( disable: 2282 )
#endif

#include <charm.h>

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#elif defined(__INTEL_COMPILER)
  #pragma warning( pop )
#endif

#endif // nowarning_charm_h
