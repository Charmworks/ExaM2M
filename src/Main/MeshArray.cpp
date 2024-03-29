// *****************************************************************************
/*!
  \file      src/Main/MeshArray.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Chare class declaration for mesharrays holding part of a mesh
  \details   Chare class declaration for mesharrays holding part of a mesh.
*/
// *****************************************************************************

#include <iostream>     // NOT NEEDED WHEN DEBUGGED

#include "MeshArray.hpp"
#include "Reorder.hpp"
#include "DerivedData.hpp"

#include "Controller.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wold-style-cast"
#endif
PUPbytes(Collision);
#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

using exam2m::MeshArray;

MeshArray::MeshArray(
  const tk::CProxy_MeshWriter& meshwriter,
  const tk::MeshCallback& cbw,
  const std::vector< std::size_t >& ginpoel,
  const tk::UnsMesh::CoordMap& coordmap,
  const tk::CommMaps& commaps,
  const std::map< int, std::vector< std::size_t > >& bface,
  const std::vector< std::size_t >& triinpoel,
  const std::map< int, std::vector< std::size_t > >& bnode,
  int nc ) :
  m_cbw( cbw ),
  m_nchare( nc ),
  m_it( 0 ),
  m_itr( 0 ),
  m_itf( 0 ),
  m_t( 0.0 ),
  m_lastDumpTime( -std::numeric_limits< tk::real >::max() ),  
  m_meshwriter( meshwriter ),
  m_el( tk::global2local( ginpoel ) ),     // fills m_inpoel, m_gid, m_lid
  m_coord( setCoord( coordmap ) ),
  m_nodeCommMap(),
  m_bface( bface ),
  m_triinpoel( triinpoel ),
  m_bnode( bnode ),
  m_u( m_coord[0].size(), 1 )
// *****************************************************************************
//  Constructor
//! \param[in] meshwriter Mesh writer proxy
//! \param[in] cbw Charm++ callbacks for MeshArray
//! \param[in] ginpoel Vector of mesh element connectivity owned (global IDs)
//! \param[in] coordmap Coordinates of mesh nodes and their global IDs
//! \param[in] commaps Communication maps associated to chare IDs bordering the
//!   mesh chunk we operate on
//! \param[in] bface Boundary face lists mapped to side set ids
//! \param[in] triinpoel Triangle element connectivity
//! \param[in] bnode Boundary node lists mapped to side set ids
//! \param[in] nc Total number of MeshArray chares
// *****************************************************************************
{
  Assert( !ginpoel.empty(), "No elements assigned to MeshArray chare" );
  Assert( tk::positiveJacobians( m_inpoel, m_coord ),
          "Jacobian in input mesh to MeshArray non-positive" );
  Assert( tk::conforming( m_inpoel, m_coord ),
          "Input mesh to MeshArray not conforming" );

  // Store communication maps
  for (const auto& [ c, maps ] : commaps) {
    m_nodeCommMap[c] = maps.get< tag::node >();
    m_edgeCommMap[c] = maps.get< tag::edge >();
  }

  // Tell the RTS that the MeshArray chares have been created
  contribute( m_cbw.get< tag::workcreated >() );
}

void
MeshArray::setSolution(Solution& s, CkCallback cb) {
  for (std::size_t i = 0; i < m_coord[0].size(); i++) {
    m_u(i,0,0) = s.f(m_coord[0][i], m_coord[1][i], m_coord[2][i]);
  }
  contribute(cb);
}

void MeshArray::checkSolution(Solution& s, CkCallback cb) {
  for (std::size_t i = 0; i < m_coord[0].size(); i++) {
    tk::real expected = s.f(m_coord[0][i], m_coord[1][i], m_coord[2][i]);
    tk::real diff = abs(m_u(i,0,0) - expected);
    if (diff > std::numeric_limits<float>::epsilon() && m_u(i,0,0) > diff) {
      CkAbort("Elem %i/%i (%f %f %f) DIFF TOO BIG! %f - %f = %e\n",
          i, m_coord[0].size(), m_coord[0][i], m_coord[1][i], m_coord[2][i],
          expected, m_u(i,0,0), diff);
    }
  }
  contribute(cb);
}

void
MeshArray::out( int meshid )
// *****************************************************************************
// Write out some field data to file(s)
//! \param[in] meshid Mesh id
// *****************************************************************************
{
  // Volume elem field data
  std::vector< std::string > elemfieldnames;
  std::vector< std::vector< tk::real > > elemfields;

  // Empty elem field data for now (but the writer should be able write those
  // fine too, and may be useful for debugging later.

  // Volume node field data (this is what we care about mostly at first)
  std::vector< std::string > nodefieldnames;
  std::vector< std::vector< tk::real > > nodefields;

  nodefieldnames.push_back( "scalar" );
  nodefields.push_back( m_u.extract(0, 0) );

  // Surface field data in nodes
  std::vector< std::string > nodesurfnames;
  std::vector< std::vector< tk::real > > nodesurfs;

  // Surface field output should also work fine, but empty for now. This may be
  // used for outputing data along side sets, which is usually orders of
  // magnitude smaller then volume field data. Could also be useful for
  // debugging.

  // Send mesh and fields data for output to file
  write( meshid, m_inpoel, m_coord, m_bface, tk::remap( m_bnode, m_lid ),
         m_triinpoel, elemfieldnames, nodefieldnames, nodesurfnames,
         elemfields, nodefields, nodesurfs,
         CkCallback(CkIndex_MeshArray::written(), thisProxy[thisIndex]) );
}

void
MeshArray::written()
// *****************************************************************************
// Mesh and field data written to file(s)
// *****************************************************************************
{
  contribute( m_cbw.get< tag::written >() );
}

tk::UnsMesh::Coords
MeshArray::setCoord( const tk::UnsMesh::CoordMap& coordmap )
// *****************************************************************************
// Set mesh coordinates based on coordinates map
// *****************************************************************************
{
  Assert( coordmap.size() == m_gid.size(), "Size mismatch" );
  Assert( coordmap.size() == m_lid.size(), "Size mismatch" );

  tk::UnsMesh::Coords coord;
  coord[0].resize( coordmap.size() );
  coord[1].resize( coordmap.size() );
  coord[2].resize( coordmap.size() );

  for (const auto& [ gid, coords ] : coordmap) {
    auto i = tk::cref_find( m_lid, gid );
    coord[0][i] = coords[0];
    coord[1][i] = coords[1];
    coord[2][i] = coords[2];
  }

  return coord;
}

void
MeshArray::write(
  int meshid,
  const std::vector< std::size_t >& inpoel,
  const tk::UnsMesh::Coords& coord,
  const std::map< int, std::vector< std::size_t > >& bface,
  const std::map< int, std::vector< std::size_t > >& bnode,
  const std::vector< std::size_t >& triinpoel,
  const std::vector< std::string>& elemfieldnames,
  const std::vector< std::string>& nodefieldnames,
  const std::vector< std::string>& nodesurfnames,
  const std::vector< std::vector< tk::real > >& elemfields,
  const std::vector< std::vector< tk::real > >& nodefields,
  const std::vector< std::vector< tk::real > >& nodesurfs,
  CkCallback c )
// *****************************************************************************
//  Output mesh and fields data (solution dump) to file(s)
//! \param[in] meshid Mesh id
//! \param[in] inpoel Mesh connectivity for the mesh chunk to be written
//! \param[in] coord Node coordinates of the mesh chunk to be written
//! \param[in] bface Map of boundary-face lists mapped to corresponding side set
//!   ids for this mesh chunk
//! \param[in] bnode Map of boundary-node lists mapped to corresponding side set
//!   ids for this mesh chunk
//! \param[in] triinpoel Interconnectivity of points and boundary-face in this
//!   mesh chunk
//! \param[in] elemfieldnames Names of element fields to be output to file
//! \param[in] nodefieldnames Names of node fields to be output to file
//! \param[in] nodesurfnames Names of node surface fields to be output to file
//! \param[in] elemfields Field data in mesh elements to output to file
//! \param[in] nodefields Field data in mesh nodes to output to file
//! \param[in] nodesurfs Surface field data in mesh nodes to output to file
//! \param[in] c Function to continue with after the write
//! \details Since m_meshwriter is a Charm++ chare group, it never migrates and
//!   an instance is guaranteed on every PE. We index the first PE on every
//!   logical compute node. In Charm++'s non-SMP mode, a node is the same as a
//!   PE, so the index is the same as CkMyPe(). In SMP mode the index is the
//!   first PE on every logical node. In non-SMP mode this yields one or more
//!   output files per PE with zero or non-zero virtualization, respectively. If
//!   there are multiple chares on a PE, the writes are serialized per PE, since
//!   only a single entry method call can be executed at any given time. In SMP
//!   mode, still the same number of files are output (one per chare), but the
//!   output is serialized through the first PE of each compute node. In SMP
//!   mode, channeling multiple files via a single PE on each node is required
//!   by NetCDF and HDF5, as well as ExodusII, since none of these libraries are
//!   thread-safe.
// *****************************************************************************
{
  // If the previous iteration refined (or moved) the mesh or this is called
  // before the first time step, we also output the mesh.
  bool meshoutput = m_itf == 0 ? true : false;

  auto eps = std::numeric_limits< tk::real >::epsilon();
  bool fieldoutput = false;

  // Output field data only if there is no dump at this physical time yet
  if (std::abs(m_lastDumpTime - m_t) > eps ) {
    m_lastDumpTime = m_t;
    ++m_itf;
    fieldoutput = true;
  }

  m_meshwriter[ CkNodeFirst( CkMyNode() ) ].
    write( meshoutput, fieldoutput, m_itr, m_itf, m_t, thisIndex,
           "out." + std::to_string(meshid),       // output basefilename
           inpoel, coord, bface, bnode, triinpoel, elemfieldnames,
           nodefieldnames, nodesurfnames, elemfields, nodefields, nodesurfs,
           {},  // no surface output for now (even if passed in nodesurf)
           c );
}

void MeshArray::transferSource()
// *****************************************************************************
//  Pass Mesh Data to m2m transfer library
// *****************************************************************************
{
  exam2m::setSourceTets(thisProxy, thisIndex, &m_inpoel, &m_coord, m_u);
}

void MeshArray::transferDest()
// *****************************************************************************
//  Pass Mesh Data to m2m transfer library
// *****************************************************************************
{
  exam2m::setDestPoints(thisProxy, thisIndex, &m_coord, m_u, CkCallback(CkIndex_MeshArray::solutionFound(), thisProxy[thisIndex]));
}

void MeshArray::solutionFound() {
  contribute( m_cbw.get< tag::solutionfound >() );
}

#include "NoWarning/mesharray.def.h"
