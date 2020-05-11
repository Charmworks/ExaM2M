// *****************************************************************************
/*!
  \file      src/Transfer/Worker.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Chare class declaration for workers holding part of a mesh
  \details   Chare class declaration for workers holding part of a mesh.
*/
// *****************************************************************************

#include "Worker.hpp"
#include "Reorder.hpp"

#include "collidecharm.h"

extern CollideHandle collideHandle;

using exam2m::Worker;

Worker::Worker(
  const tk::CProxy_MeshWriter& meshwriter,
  const tk::WorkerCallback& cbw,
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
  m_bnode( bnode )
// *****************************************************************************
//  Constructor
//! \param[in] meshwriter Mesh writer proxy
//! \param[in] cbw Charm++ callbacks for Worker
//! \param[in] ginpoel Vector of mesh element connectivity owned (global IDs)
//! \param[in] coordmap Coordinates of mesh nodes and their global IDs
//! \param[in] commaps Communication maps associated to chare IDs bordering the
//!   mesh chunk we operate on
//! \param[in] bface Boundary face lists mapped to side set ids
//! \param[in] triinpoel Triangle element connectivity
//! \param[in] bnode Boundary node lists mapped to side set ids
//! \param[in] nc Total number of Worker chares
// *****************************************************************************
{
  Assert( !ginpoel.empty(), "No elements assigned to Worker chare" );
  Assert( tk::positiveJacobians( m_inpoel, m_coord ),
          "Jacobian in input mesh to Worker non-positive" );
  Assert( tk::conforming( m_inpoel, m_coord ),
          "Input mesh to Worker not conforming" );

  // Store communication maps
  for (const auto& [ c, maps ] : commaps) {
    m_nodeCommMap[c] = maps.get< tag::node >();
    m_edgeCommMap[c] = maps.get< tag::edge >();
  }

  CollideRegister(collideHandle, thisIndex);

  // Tell the RTS that the Worker chares have been created and compute
  // the total number of mesh points across whole problem
  contribute( m_cbw.get< tag::workcreated >() );
}

void
Worker::out()
// *****************************************************************************
// Write out some field data to file(s)
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
  const auto& x = m_coord[0];
  const auto& y = m_coord[1];
  const auto& z = m_coord[2];
  auto npoin = m_coord[0].size();
  std::vector< tk::real > s( npoin, 0.0 );
  for (std::size_t i=0; i<npoin; ++i) {
    s[i] = 1.0 * exp( -(x[i]*x[i] + y[i]*y[i] + z[i]*z[i])/(2.0 * 0.05) );
  }
  nodefields.push_back( s );

  // Surface field data in nodes
  std::vector< std::string > nodesurfnames;
  std::vector< std::vector< tk::real > > nodesurfs;

  // Surface field output should also work fine, but empty for now. This may be
  // used for outputing data along side sets, which is usually orders of
  // magnitude smaller then volume field data. Could also be useful for
  // debugging.

  // Send mesh and fields data for output to file
  write( m_inpoel, m_coord, m_bface, tk::remap( m_bnode, m_lid ),
         m_triinpoel, elemfieldnames, nodefieldnames, nodesurfnames,
         elemfields, nodefields, nodesurfs,
         CkCallback(CkIndex_Worker::written(), thisProxy[thisIndex]) );
}

void
Worker::written()
// *****************************************************************************
// Mesh and field data written to file(s)
// *****************************************************************************
{
  contribute( m_cbw.get< tag::written >() );
}

tk::UnsMesh::Coords
Worker::setCoord( const tk::UnsMesh::CoordMap& coordmap )
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
Worker::write(
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
           "out",       // output basefilename
           inpoel, coord, bface, bnode, triinpoel, elemfieldnames,
           nodefieldnames, nodesurfnames, elemfields, nodefields, nodesurfs,
           {},  // no surface output for now (even if passed in nodesurf)
           c );
}

#include "NoWarning/worker.def.h"
