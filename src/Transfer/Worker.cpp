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

PUPbytes(Collision);

extern CollideHandle collideHandle;

using exam2m::Worker;

Worker::Worker(
  std::size_t meshid,
  std::size_t firstchunk,
  const tk::CProxy_MeshWriter& meshwriter,
  const tk::WorkerCallback& cbw,
  const std::vector< std::size_t >& ginpoel,
  const tk::UnsMesh::CoordMap& coordmap,
  const tk::CommMaps& commaps,
  const std::map< int, std::vector< std::size_t > >& bface,
  const std::vector< std::size_t >& triinpoel,
  const std::map< int, std::vector< std::size_t > >& bnode,
  int nc ) :
  m_meshid( meshid ),
  m_firstchunk( firstchunk ),
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

  CollideRegister(collideHandle, m_firstchunk + thisIndex);

  // Tell the RTS that the Worker chares have been created and compute
  // the total number of mesh points across whole problem
  contribute( sizeof( std::size_t ), &meshid, CkReduction::nop,
      m_cbw.get< tag::workcreated >() );
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

void
Worker::collideVertices() const
// *****************************************************************************
// Pass vertex information to the collision detection library
// *****************************************************************************
{
  int nBoxes = m_coord[0].size();
  bbox3d* boxes = new bbox3d[nBoxes];
  int* prio = new int[nBoxes];
  for (int i = 0; i < nBoxes; i++) {
    boxes[i].empty();
    boxes[i].add(CkVector3d(m_coord[0][i], m_coord[1][i], m_coord[2][i]));
    prio[i] = m_firstchunk;
  }
  CollideBoxesPrio(collideHandle, m_firstchunk + thisIndex, nBoxes, boxes, prio);
  delete[] boxes;
  delete[] prio;
}

void
Worker::collideTets() const
// *****************************************************************************
// Pass tet information to the collision detection library
// *****************************************************************************
{
  int nBoxes = m_inpoel.size() / 4;
  bbox3d* boxes = new bbox3d[nBoxes];
  int* prio = new int[nBoxes];
  for (int i = 0; i < nBoxes; i++) {
    boxes[i].empty();
    prio[i] = m_firstchunk;
    for (int j = 0; j < 4; j++) {
      // Get index of the jth point of the ith tet
      int p = m_inpoel[i * 4 + j];
      // Add that point to the tets bounding box
      boxes[i].add(CkVector3d(m_coord[0][p], m_coord[1][p], m_coord[2][p]));
    }
  }
  CollideBoxesPrio(collideHandle, m_firstchunk + thisIndex, nBoxes, boxes, prio);
  delete[] boxes;
  delete[] prio;
}

void
Worker::processCollisions(
    int nColl,
    Collision* colls,
    std::size_t numchares,
    std::size_t chunkoffset,
    CProxy_Worker proxy )
// *****************************************************************************
//
// *****************************************************************************
{
  int mychunk = thisIndex + m_firstchunk;
  CkPrintf("Worker %i received data for %i collisions\n", mychunk, nColl);

  std::vector<std::pair<CkVector3d, int>>* separated
      = new std::vector<std::pair<CkVector3d, int>>[numchares];
  for (int i = 0; i < nColl; i++) {
    int theirindex;
    int theirpoint;
    int mypoint;
    if (colls[i].A.chunk == mychunk) {
      theirindex = colls[i].B.chunk - chunkoffset;
      theirpoint = colls[i].B.number;
      mypoint = colls[i].A.number;
    } else {
      theirindex = colls[i].A.chunk - chunkoffset;
      theirpoint = colls[i].A.number;
      mypoint = colls[i].B.number;
    }
    separated[theirindex].push_back(std::make_pair(
        CkVector3d(m_coord[0][mypoint], m_coord[1][mypoint], m_coord[2][mypoint]),
        theirpoint));
  }

  for (int i = 0; i < numchares; i++) {
    proxy[i].determineActualCollisions(separated[i].size(), separated[i].data());
  }
  delete[] separated;
}

void
Worker::determineActualCollisions(
    int nPoints,
    std::pair<CkVector3d, int>* points )
// *****************************************************************************
//  Identify actual collisions by calling intet function on all possible collisions
//! \param[in] nPoints Number of points to be checked
//! \param[in] points Pairs of point and tet indices
// *****************************************************************************
{
  CkPrintf("[%i]: Received data for %i potential collisions with my tets\n", CkMyPe(), nPoints);

  std::array< real, 4 > N;
  int numInTet = 0;

  // Iterate over my potential collisions and determine if it is intet and the shapefunction
  for(int i = 0; i < nPoints; i++) {
    bool val = intet(points[i].first,
                     points[i].second,
                     N);

    if(val)
      numInTet++;

    // TODO: I'm guessing we need the point(points[i]), the tet index(points[i].second), and the shapefunction(N) to be stored from this function for future use?
  }
  CkPrintf("[%i]: %i collisions are actually in my tets out of %i potential collisions\n", CkMyPe(), numInTet, nPoints);
}

bool
Worker::intet(const CkVector3d &point,
      std::size_t e,
      std::array< real, 4 >& N)
  // *****************************************************************************
  //  Determine if a point is in a tetrahedron and evaluate the shapefunction
  //! \param[in] point Point coordinates
  //! \param[in] e Mesh cell index
  //! \param[in,out] N Shapefunctions evaluated at the point
  //! \return True if ppoint is in mesh cell
  //! \see Lohner, An Introduction to Applied CFD Techniques, Wiley, 2008
  // *****************************************************************************
{
  // Tetrahedron node indices
  const auto A = m_inpoel[e*4+0];
  const auto B = m_inpoel[e*4+1];
  const auto C = m_inpoel[e*4+2];
  const auto D = m_inpoel[e*4+3];

  // Tetrahedron node coordinates
  const auto& x = m_coord[0];
  const auto& y = m_coord[1];
  const auto& z = m_coord[2];

  // Point coordinates
  const auto& xp = point.x;
  const auto& yp = point.y;
  const auto& zp = point.z;

  // Evaluate linear shapefunctions at point locations using Cramer's Rule
  //    | xp |   | x1 x2 x3 x4 |   | N1 |
  //    | yp | = | y1 y2 y3 y4 | • | N2 |
  //    | zp |   | z1 z2 z3 z4 |   | N3 |
  //    | 1  |   | 1  1  1  1  |   | N4 |

  real DetX = (y[B]*z[C] - y[C]*z[B] - y[B]*z[D] + y[D]*z[B] +
      y[C]*z[D] - y[D]*z[C])*x[A] + x[B]*y[C]*z[A] - x[B]*y[A]*z[C] +
    x[C]*y[A]*z[B] - x[C]*y[B]*z[A] + x[B]*y[A]*z[D] - x[B]*y[D]*z[A] -
    x[D]*y[A]*z[B] + x[D]*y[B]*z[A] - x[C]*y[A]*z[D] + x[C]*y[D]*z[A] +
    x[D]*y[A]*z[C] - x[D]*y[C]*z[A] - x[B]*y[C]*z[D] + x[B]*y[D]*z[C] +
    x[C]*y[B]*z[D] - x[C]*y[D]*z[B] - x[D]*y[B]*z[C] + x[D]*y[C]*z[B];

  real DetX1 = (y[D]*z[C] - y[C]*z[D] + y[C]*zp - yp*z[C] -
      y[D]*zp + yp*z[D])*x[B] + x[C]*y[B]*z[D] - x[C]*y[D]*z[B] -
    x[D]*y[B]*z[C] + x[D]*y[C]*z[B] - x[C]*y[B]*zp + x[C]*yp*z[B] +
    xp*y[B]*z[C] - xp*y[C]*z[B] + x[D]*y[B]*zp - x[D]*yp*z[B] -
    xp*y[B]*z[D] + xp*y[D]*z[B] + x[C]*y[D]*zp - x[C]*yp*z[D] -
    x[D]*y[C]*zp + x[D]*yp*z[C] + xp*y[C]*z[D] - xp*y[D]*z[C];

  real DetX2 = (y[C]*z[D] - y[D]*z[C] - y[C]*zp + yp*z[C] +
      y[D]*zp - yp*z[D])*x[A] + x[C]*y[D]*z[A] - x[C]*y[A]*z[D] +
    x[D]*y[A]*z[C] - x[D]*y[C]*z[A] + x[C]*y[A]*zp - x[C]*yp*z[A] -
    xp*y[A]*z[C] + xp*y[C]*z[A] - x[D]*y[A]*zp + x[D]*yp*z[A] +
    xp*y[A]*z[D] - xp*y[D]*z[A] - x[C]*y[D]*zp + x[C]*yp*z[D] +
    x[D]*y[C]*zp - x[D]*yp*z[C] - xp*y[C]*z[D] + xp*y[D]*z[C];

  real DetX3 = (y[D]*z[B] - y[B]*z[D] + y[B]*zp - yp*z[B] -
      y[D]*zp + yp*z[D])*x[A] + x[B]*y[A]*z[D] - x[B]*y[D]*z[A] -
    x[D]*y[A]*z[B] + x[D]*y[B]*z[A] - x[B]*y[A]*zp + x[B]*yp*z[A] +
    xp*y[A]*z[B] - xp*y[B]*z[A] + x[D]*y[A]*zp - x[D]*yp*z[A] -
    xp*y[A]*z[D] + xp*y[D]*z[A] + x[B]*y[D]*zp - x[B]*yp*z[D] -
    x[D]*y[B]*zp + x[D]*yp*z[B] + xp*y[B]*z[D] - xp*y[D]*z[B];

  real DetX4 = (y[B]*z[C] - y[C]*z[B] - y[B]*zp + yp*z[B] +
      y[C]*zp - yp*z[C])*x[A] + x[B]*y[C]*z[A] - x[B]*y[A]*z[C] +
    x[C]*y[A]*z[B] - x[C]*y[B]*z[A] + x[B]*y[A]*zp - x[B]*yp*z[A] -
    xp*y[A]*z[B] + xp*y[B]*z[A] - x[C]*y[A]*zp + x[C]*yp*z[A] +
    xp*y[A]*z[C] - xp*y[C]*z[A] - x[B]*y[C]*zp + x[B]*yp*z[C] +
    x[C]*y[B]*zp - x[C]*yp*z[B] - xp*y[B]*z[C] + xp*y[C]*z[B];

  // Shape functions evaluated at point
  N[0] = DetX1/DetX;
  N[1] = DetX2/DetX;
  N[2] = DetX3/DetX;
  N[3] = DetX4/DetX;

  // if min( N^i, 1-N^i ) > 0 for all i, point is in cell
  if ( std::min(N[0],1.0-N[0]) > 0 && std::min(N[1],1.0-N[1]) > 0 &&
      std::min(N[2],1.0-N[2]) > 0 && std::min(N[3],1.0-N[3]) > 0 )
  {
    return true;
  } else {
    return false;
  }
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
