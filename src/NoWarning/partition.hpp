// *****************************************************************************
/*!
  \file      src/NoWarning/partition.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Include brigand/algorithms/partition.hpp with turning off
             specific compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_partition_h
#define nowarning_partition_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wundef"
  #pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif

#include <brigand/algorithms/partition.hpp>

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#endif // nowarning_partition_h
