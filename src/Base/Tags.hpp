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

//! Tags used as unique-type labels for compile-time code-generation
namespace tag {

struct distributed {};
struct mapinserted {};
struct workinserted {};
struct workcreated {};
struct load {};
struct node {};
struct edge {};
struct queried {};
struct responded {};
struct written {};

} // tag::

#endif // Tags_h
