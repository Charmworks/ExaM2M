// *****************************************************************************
/*!
  \file      src/Control/Tags.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Tags
  \details   Tags are unique types, used for metaprogramming.
*/
// *****************************************************************************
#ifndef Tags_h
#define Tags_h

#include <string>

//! Tags used as unique-type labels for compile-time code-generation
namespace tag {

struct distributed {};
struct load {};

} // tag::

#endif // Tags_h
