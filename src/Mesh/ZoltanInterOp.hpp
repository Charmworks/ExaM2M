// *****************************************************************************
/*!
  \file      src/Mesh/ZoltanInterOp.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Interoperation with the Zoltan library
  \details   Interoperation with the Zoltan library, used for static mesh
    partitioning.
*/
// *****************************************************************************
#ifndef ZoltanInterOp_h
#define ZoltanInterOp_h

#include <vector>
#include <array>
#include <string>

#include "Types.hpp"

namespace tk {

class UnsMesh;

//! Interoperation with the Zoltan library, used for static mesh partitioning
namespace zoltan {

//! Partition mesh using Zoltan2 with a geometric partitioner, such as RCB, RIB
std::vector< std::size_t >
geomPartMesh( const std::string& algorithm,
              const std::array< std::vector< real >, 3 >& elemcoord,
              const std::vector< long long >& elemid,
              int npart );

} // zoltan::
} // tk::

#endif // ZoltanInterOp_h
