// *****************************************************************************
/*!
  \file      src/Base/Callback.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Tagged tuple types used for passing Charm++ callbacks
  \details   Tagged tuple types used for passing Charm++ callbacks.
*/
// *****************************************************************************
#ifndef Callbacks_h
#define Callbacks_h

#include "NoWarning/charm++.hpp"

#include "Tags.hpp"
#include "TaggedTuple.hpp"

namespace tk {

using PartitionerCallback =
  tk::TaggedTuple< brigand::list<
      tag::load,           CkCallback
    , tag::distributed,    CkCallback
    , tag::mapinserted,    CkCallback
  > >;

using MapperCallback =
  tk::TaggedTuple< brigand::list<
      tag::queried,        CkCallback
    , tag::responded,      CkCallback
    , tag::workinserted,   CkCallback
  > >;

using MeshCallback =
  tk::TaggedTuple< brigand::list<
      tag::workcreated,   CkCallback
    , tag::solutionfound, CkCallback
    , tag::written,       CkCallback
  > >;

} // tk::

#endif // Callbacks_h
