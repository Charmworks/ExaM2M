// *****************************************************************************
/*!
  \file      src/NoWarning/exam2m.def.h
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include exam2m.def.h with turning off specific compiler
             warnings
*/
// *****************************************************************************
#ifndef nowarning_exam2m_def_h
#define nowarning_exam2m_def_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
//  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "../Main/exam2m.def.h"

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#endif

#endif // nowarning_exam2m_def_h
