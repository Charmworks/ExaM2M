// *****************************************************************************
/*!
  \file      src/NoWarning/index_of.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include brigand/algorithms/index_of.hpp with turning off
             specific compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_index_of_h
#define nowarning_index_of_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif

#include <brigand/algorithms/index_of.hpp>

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#endif // nowarning_index_of_h
