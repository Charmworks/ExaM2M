// *****************************************************************************
/*!
  \file      src/Main/Macro.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Macro definitions
  \details   Macro definitions for various utility functionality.
*/
// *****************************************************************************
#ifndef Macro_h
#define Macro_h

namespace tk {

//! Macro to detect strictly gcc.
//! \details __GNUC__ and __GNUG__ were intended to indicate the GNU compilers.
//! However, they're also defined by Clang/LLVM and Intel compilers to indicate
//! compatibility. This macro can be used to detect strictly gcc and not clang
//! or icc.
#if (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
  #define STRICT_GNUC
#endif

} // tk::

#endif // Macro_h
