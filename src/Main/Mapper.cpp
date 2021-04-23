// *****************************************************************************
/*!
  \file      src/Main/Mapper.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Setup communication maps
  \details   Setup communication maps.
*/
// *****************************************************************************

#include <vector>
#include <algorithm>

#include "Mapper.hpp"
#include "Reorder.hpp"
#include "DerivedData.hpp"

using exam2m::Mapper;

Mapper::Mapper( const tk::CProxy_MeshWriter& meshwriter,
                const CProxy_MeshArray& mesharray,
                const tk::MapperCallback& cbm,
                const tk::MeshCallback& cbw,
                const std::vector< std::size_t >& ginpoel,
                const tk::UnsMesh::CoordMap& coordmap,
                const std::map< int, std::vector< std::size_t > >& bface,
                const std::vector< std::size_t >& triinpoel,
                const std::map< int, std::vector< std::size_t > >& bnode,
                int nchare ) :
  m_meshwriter( meshwriter ),
  m_mesharray( mesharray ),
  m_cbm( cbm ),
  m_cbw( cbw ),
  m_ginpoel( ginpoel ),
  m_coordmap( coordmap ),
  m_nbnd( 0 ),
  m_bface( bface ),
  m_triinpoel( triinpoel ),
  m_bnode( bnode ),
  m_nchare( nchare ),
  m_nodech(),
  m_chnode(),
  m_edgech(),
  m_chedge(),
  m_commap()
// *****************************************************************************
//  Constructor: prepare owned mesh node IDs for reordering
//! \param[in] meshwriter Mesh writer Charm++ proxy
//! \param[in] cbm Charm++ callbacks for Mapper
//! \param[in] cbw Charm++ callbacks for Worker
//! \param[in] ginpoel Mesh connectivity (this chare) using global node IDs
//! \param[in] coordmap Mesh node coordinates (this chare) for global node IDs
//! \param[in] bface Face lists mapped to side set ids
//! \param[in] triinpoel Interconnectivity of points and boundary-faces
//! \param[in] bnode Node ids mapped to side set ids
//! \param[in] nchare Total number of Charm++ worker chares
// *****************************************************************************
{
  // Ensure boundary face ids will not index out of face connectivity
  Assert( std::all_of( begin(m_bface), end(m_bface),
            [&](const auto& s)
            { return std::all_of( begin(s.second), end(s.second),
                       [&](auto f){ return f*3+2 < m_triinpoel.size(); } ); } ),
          "Boundary face data structures inconsistent" );
}

void
Mapper::setup( std::size_t npoin )
// *****************************************************************************
// Setup chare mesh boundary node communication map
//! \param[in] npoin Total number of mesh points in mesh
// *****************************************************************************
{
  // Compute the number of nodes (chunksize) a chare will build a node
  // communication map for. We compute the value of chunksize based on global
  // node id, bounded between [0...npoin-1], inclusive. To compute the bin id,
  // we use the chunksize which always gives a chare id that is (strictly)
  // lower than the number of chares.
  auto N = static_cast< std::size_t >( m_nchare );
  std::size_t chunksize = npoin / N;

  // Find chare-boundary nodes and edges of our mesh chunk. This algorithm
  // collects the global mesh node ids and edges on the chare boundary. A node
  // is on a chare boundary if it belongs to a face of a tetrahedron that has
  // no neighbor tet at a face. The edge is on the chare boundary if its first
  // edge-end point is on a chare boundary. The nodes are categorized to bins
  // that will be sent to different chares to build point-to-point
  // communication maps across all chares. The binning is determined by the
  // global node id divided by the chunksizes.
  tk::CommMaps chbnd;
  auto el = tk::global2local( m_ginpoel );      // generate local mesh data
  const auto& inpoel = std::get< 0 >( el );     // local connectivity
  auto esup = tk::genEsup( inpoel, 4 );         // elements surrounding points
  auto esuel = tk::genEsuelTet( inpoel, esup ); // elems surrounding elements
  for (std::size_t e=0; e<esuel.size()/4; ++e) {
    auto mark = e*4;
    for (std::size_t f=0; f<4; ++f)
      if (esuel[mark+f] == -1)
        for (std::size_t n=0; n<3; ++n) {
          auto g = m_ginpoel[ mark+tk::lpofa[f][n] ];
          auto bin = g / chunksize;
          if (bin >= N) bin = N - 1;
          Assert( bin < N, "Will index out of number of chares" );
          auto& b = chbnd[ static_cast< int >( bin ) ];
          b.get< tag::node >().insert( g );
          auto h = m_ginpoel[ mark + tk::lpofa[ f ][ tk::lpoet[n][1] ] ];
          b.get< tag::edge >().insert( { std::min(g,h), std::max(g,h) } );
        }
  }

  // Send boundary data in bins to chares that will compute communication maps
  // for the data in the bin. These bins form a distributed table.  Note that
  // we only send data to those chares that have data to work on. The receiving
  // sides do not know in advance if they receive messages or not.  Completion
  // is detected by having the receiver respond back and counting the responses
  // on the sender side, i.e., this chare.
  m_nbnd = chbnd.size();
  if (m_nbnd == 0)
    contribute( m_cbm.get< tag::queried >() );
  else
    for (const auto& [ targetchare, bnd ] : chbnd)
      thisProxy[ targetchare ].query( thisIndex, bnd );
}

void
Mapper::query( int fromch, const tk::AllCommMaps& bnd )
// *****************************************************************************
// Incoming query for a list of mesh nodes for which this chare compiles node
// communication maps
//! \param[in] fromch Sender chare ID
//! \param[in] bnd Chare-boundary data from another chare
// *****************************************************************************
{
  // Store incoming nodes in node->chare and its inverse, chare->node, maps
  const auto& nodes = bnd.get< tag::node >();
  for (auto n : nodes) m_nodech[ n ].push_back( fromch );
  m_chnode[ fromch ].insert( begin(nodes), end(nodes) );

  // Store incoming edges in edge->chare and its inverse, chare->edge, maps
  const auto& edges = bnd.get< tag::edge >();
  for (const auto& e : edges) m_edgech[ e ].push_back( fromch );
  m_chedge[ fromch ].insert( begin(edges), end(edges) );

  // Report back to chare message received from
  thisProxy[ fromch ].recvquery();
}

void
Mapper::recvquery()
// *****************************************************************************
// Receive receipt of boundary node lists to query
// *****************************************************************************
{
  if (--m_nbnd == 0)
    contribute( m_cbm.get< tag::queried >() );
}

void
Mapper::response()
// *****************************************************************************
//  Respond to boundary node list queries
// *****************************************************************************
{
  std::unordered_map< int, tk::CommMaps > exp;

  // Compute node communication map to be sent back to chares
  for (const auto& [ neighborchare, bndnodes ] : m_chnode) {
    auto& nc = exp[ neighborchare ];
    for (auto n : bndnodes)
      for (auto d : tk::cref_find(m_nodech,n))
        if (d != neighborchare)
          nc[d].get< tag::node >().insert( n );
  }

  // Compute edge communication map to be sent back to chares
  for (const auto& [ neighborchare, bndedges ] : m_chedge) {
    auto& ec = exp[ neighborchare ];
    for (const auto& e : bndedges)
      for (auto d : tk::cref_find(m_edgech,e))
        if (d != neighborchare)
          ec[d].get< tag::edge >().insert( e );
  }

  // Send communication maps to chares that issued a query to us. Communication
  // maps were computed above for those chares that queried this map from us.
  // This data form a distributed table and we only work on a chunk of it. Note
  // that we only send data back to those chares that have queried us. The
  // receiving sides do not know in advance if the receive messages or not.
  // Completion is detected by having the receiver respond back and counting
  // the responses on the sender side, i.e., this chare.
  m_nbnd = exp.size();
  if (m_nbnd == 0)
    contribute( m_cbm.get< tag::responded >() );
  else
    for (const auto& [ targetchare, maps ] : exp)
      thisProxy[ targetchare ].bnd( thisIndex, maps );
}

void
Mapper::bnd( int fromch, const tk::CommMaps& commap )
// *****************************************************************************
// Receive boundary node communication maps for our mesh chunk
//! \param[in] fromch Sender chare ID
//! \param[in] commap Communication map(s) assembled by chare fromch
// *****************************************************************************
{
  for (const auto& [ neighborchare, maps ] : commap) {
    auto& m = m_commap[ neighborchare ];
    const auto& nodemap = maps.get< tag::node >();
    m.get< tag::node >().insert( begin(nodemap), end(nodemap) );
    const auto& edgemap = maps.get< tag::edge >();
    m.get< tag::edge >().insert( begin(edgemap), end(edgemap) );
  }

  // Report back to chare message received from
  thisProxy[ fromch ].recvbnd();
}

void
Mapper::recvbnd()
// *****************************************************************************
// Receive receipt of boundary node communication map
// *****************************************************************************
{
  if (--m_nbnd == 0)
    contribute( m_cbm.get< tag::responded >() );
}

void
Mapper::create()
// *****************************************************************************
//  Create worker chare array elements
// *****************************************************************************
{
  m_mesharray[ thisIndex ].insert( m_meshwriter, m_cbw,
    m_ginpoel, m_coordmap, m_commap, m_bface, m_triinpoel, m_bnode, m_nchare );

    contribute( m_cbm.get< tag::workinserted >() );

  // Free up some memory
  tk::destroy( m_ginpoel );
  tk::destroy( m_coordmap );
  tk::destroy( m_bface );
  tk::destroy( m_triinpoel );
  tk::destroy( m_bnode );
  tk::destroy( m_nodech );
  tk::destroy( m_chnode );
  tk::destroy( m_commap );
}

#include "NoWarning/mapper.def.h"
