// *****************************************************************************
/*!
  \file      src/Main/Transporter.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Transporter drives time integration
  \details   Transporter drives time integration.
*/
// *****************************************************************************

#include <iostream>

#include "Transporter.hpp"
#include "ExodusIIMeshReader.hpp"
#include "LoadDistributor.hpp"

#include "collidecharm.h"

#include "NoWarning/exam2m.decl.h"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wold-style-cast"
#endif

PUPbytes(Collision);

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

extern CProxy_Main mainProxy;

namespace exam2m {

extern tk::real g_virtualization;

}

using exam2m::Transporter;

Transporter::Transporter(CProxy_collisionMgr cProxy)
// *****************************************************************************
//  Constructor
// *****************************************************************************
{
  m_currentchunk = 0;
  collisionMgrProxy = cProxy;
}

void Transporter::initMeshData( const std::string& file )
// *****************************************************************************
//  addMesh
//! \param[in] file Name of the file to read the mesh data from
// *****************************************************************************
{
  std::map< int, std::vector< std::size_t > > bface;
  std::map< int, std::vector< std::size_t > > faces;
  std::map< int, std::vector< std::size_t > > bnode;

  MeshData mesh;
  auto meshid = static_cast< unsigned short >( m_meshes.size() );

  // Create ExodusII mesh file reader
  tk::ExodusIIMeshReader mr( file );

  // Read out total number of mesh points from mesh file
  mesh.m_npoin = mr.npoin();

  // Read boundary-face connectivity on side sets
  mr.readSidesetFaces( bface, faces );
  // Read node lists on side sets
  bnode = mr.readSidesetNodes();

  // Create Partitioner callbacks (order matters)
  tk::PartitionerCallback cbp {{
      CkCallback( CkReductionTarget(Transporter,loaded), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,distributed), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,mapinserted), thisProxy )
  }};
  cbp.get<tag::load>().setRefnum(meshid);
  cbp.get<tag::distributed>().setRefnum(meshid);
  cbp.get<tag::mapinserted>().setRefnum(meshid);

  // Create Mapper callbacks (order matters)
  tk::MapperCallback cbm {{
      CkCallback( CkReductionTarget(Transporter,queried), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,responded), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,workinserted), thisProxy )
  }};
  cbm.get<tag::queried>().setRefnum(meshid);
  cbm.get<tag::responded>().setRefnum(meshid);
  cbm.get<tag::workinserted>().setRefnum(meshid);

  // Create Worker callbacks (order matters)
  tk::WorkerCallback cbw {{
      CkCallback( CkReductionTarget(Transporter,workcreated), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,written), thisProxy )
  }};
  cbw.get<tag::workcreated>().setRefnum(meshid);
  cbw.get<tag::written>().setRefnum(meshid);

  // Create MeshWriter chare group
  mesh.m_meshwriter = tk::CProxy_MeshWriter::ckNew();

  // Create empty Mappers, will setup communication maps
  mesh.m_mapper = CProxy_Mapper::ckNew();

  // Create empty Workers, will hold chunk of the mesh
  mesh.m_worker = CProxy_Worker::ckNew();

  // Create WorkerStats proxy
  mesh.m_workerStats = CProxy_WorkerStats::ckNew(mesh.m_worker);

  // Create Partitioner nodegroup
  mesh.m_partitioner =
    CProxy_Partitioner::ckNew( file, cbp, cbm, cbw,
       mesh.m_meshwriter, mesh.m_mapper, mesh.m_worker, mesh.m_workerStats,
       bface, faces, bnode );

  m_meshes.push_back(mesh);
}

void
Transporter::updatenelems( std::size_t meshid, std::size_t nelem )
// *****************************************************************************
// Reduction target: the mesh has been read from file on all PEs
//! \param[in] meshid The mesh ID of the mesh that has been read
//! \param[in] nelem Total number of mesh elements (summed across all nodes)
// *****************************************************************************
{
  MeshData& mesh = m_meshes[meshid];
  mesh.m_nelem = nelem;

  // Compute load distribution given total work (nelem) and virtualization
  uint64_t chunksize, remainder;
  mesh.m_nchare = static_cast< int >(
               tk::linearLoadDistributor( g_virtualization,
                 nelem, CkNumPes(), chunksize, remainder ) );
  mesh.m_firstchunk = m_currentchunk;
  m_currentchunk += static_cast< std::size_t >( mesh.m_nchare );

  // Print out info on load distribution
  std::cout << "ExaM2M> Virtualization [0.0...1.0]: "
            << g_virtualization << '\n';
  std::cout << "ExaM2M> Number of work units: " << mesh.m_nchare << '\n';

  // Tell the meshwriter the total number of chares
  mesh.m_meshwriter.nchare( mesh.m_nchare );
}

Transporter::Transporter( CkMigrateMessage* m ) : CBase_Transporter( m )
// *****************************************************************************
//  Migrate constructor: returning from a checkpoint
//! \param[in] m Charm++ migrate message
// *****************************************************************************
{
  std::cout << "Restarted from checkpoint\n";
}

using exam2m::collisionMgr;

void
collisionMgr::distributeCollisions(int nColl, Collision* colls)
// *****************************************************************************
//  Called when all potential collisions have been found, and now need to be
//  destributed to the chares in the destination mesh to determine actual
//  collisions.
//! \param[in] nColl Number of potential collisions found
//! \param[in] colls The list of potential collisions
// *****************************************************************************
{
  //std::cout << "ExaM2M> Collisions found: " << nColl << std::endl;
  auto first = m_meshes[m_destmeshid].m_firstchunk;
  auto nchare = static_cast< std::size_t >( m_meshes[m_destmeshid].m_nchare );
  std::vector< std::vector< Collision > > separated( nchare );

  // Separate collisions based on the destination mesh chare they belong to
  for (int i = 0; i < nColl; i++) {
    auto a = static_cast< std::size_t >( colls[i].A.chunk );
    if (a >= first && a < first + nchare) {
      separated[a-first].push_back( colls[i] );
    } else {
      auto b = static_cast< std::size_t >( colls[i].B.chunk );
      separated[b-first].push_back( colls[i] );
    }
  }

  //auto nonZeroCount = 0;

  // Send out each list to the destination chares for further processing
  for (int i=0; i<static_cast<int>(nchare); ++i) {
    auto I = static_cast< std::size_t >( i );

    if(separated[I].size() != 0 ) {
      //nonZeroCount++;
      //CkPrintf("Dest mesh chunk %i has %lu\n", i, separated[I].size());
      m_meshes[m_destmeshid].m_worker[i].processCollisions(
          m_meshes[m_sourcemeshid].m_worker,
          m_meshes[m_sourcemeshid].m_nchare,
          static_cast<int>(m_meshes[m_sourcemeshid].m_firstchunk),
          static_cast<int>(separated[I].size()),
          separated[I].data() );
    }
  }
  //CkPrintf("[%d] %d destPEs are receiving messages out of %d\n", CmiMyPe(), nonZeroCount, nchare);
}




#include "NoWarning/transporter.def.h"
