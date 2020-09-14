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

PUPbytes(Collision);

extern CProxy_Main mainProxy;

using exam2m::Transporter;

Transporter::Transporter() : completed(0), m_currentchunk(0)
// *****************************************************************************
//  Constructor
// *****************************************************************************
{}

void Transporter::addMesh( const std::string& file )
// *****************************************************************************
//  addMesh
//! \param[in] file Name of the file to read the mesh data from
// *****************************************************************************
{
  std::map< int, std::vector< std::size_t > > bface;
  std::map< int, std::vector< std::size_t > > faces;
  std::map< int, std::vector< std::size_t > > bnode;

  MeshData mesh;
  int meshid = meshes.size();

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
      CkCallback( CkReductionTarget(Transporter,load), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,distributed), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,mapinserted), thisProxy )
  }};

  // Create Mapper callbacks (order matters)
  tk::MapperCallback cbm {{
      CkCallback( CkReductionTarget(Transporter,queried), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,responded), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,workinserted), thisProxy )
  }};

  // Create Worker callbacks (order matters)
  tk::WorkerCallback cbw {{
      CkCallback( CkReductionTarget(Transporter,workcreated), thisProxy )
    , CkCallback( CkReductionTarget(Transporter,written), thisProxy )
  }};

  // Create MeshWriter chare group
  mesh.m_meshwriter = tk::CProxy_MeshWriter::ckNew();

  // Create empty Mappers, will setup communication maps
  mesh.m_mapper = CProxy_Mapper::ckNew();

  // Create empty Workers, will hold chunk of the mesh
  mesh.m_worker = CProxy_Worker::ckNew();

  // Create Partitioner nodegroup
  mesh.m_partitioner =
    CProxy_Partitioner::ckNew( meshid, file, cbp, cbm, cbw,
       mesh.m_meshwriter, mesh.m_mapper, mesh.m_worker, bface, faces, bnode );

  meshes.push_back(mesh);
}

void
Transporter::load( std::size_t meshid, std::size_t nelem )
// *****************************************************************************
// Reduction target: the mesh has been read from file on all PEs
//! \param[in] nelem Total number of mesh elements (summed across all nodes)
// *****************************************************************************
{
  tk::real virtualization = 0.0;
  meshes[meshid].m_nelem = nelem;

  // Compute load distribution given total work (nelem) and virtualization
  uint64_t chunksize, remainder;
  // TODO: This needs to be used to ensure unique chunk IDs at some point?
  meshes[meshid].m_nchare = static_cast< int >(
               tk::linearLoadDistributor( virtualization,
                 nelem, CkNumPes(), chunksize, remainder ) );
  meshes[meshid].m_firstchunk = m_currentchunk;
  m_currentchunk += meshes[meshid].m_nchare;

  // Print out info on load distribution
  std::cout << "Initial load distribution\n";
  std::cout << "Virtualization [0.0...1.0]: " << virtualization << '\n';
  std::cout << "Number of work units: " << meshes[meshid].m_nchare << '\n';

  // Tell the meshwriter the total number of chares
  meshes[meshid].m_meshwriter.nchare( meshes[meshid].m_nchare );

  // Partition the mesh
  meshes[meshid].m_partitioner.partition( meshes[meshid].m_nchare );
}

void
Transporter::distributed( std::size_t meshid )
// *****************************************************************************
// Reduction target: all PEs have distrbuted their mesh after partitioning
// *****************************************************************************
{
  meshes[meshid].m_partitioner.map();
}

void
Transporter::mapinserted( std::size_t meshid, std::size_t error )
// *****************************************************************************
// Reduction target: all PEs have created their Mappers
//! \param[in] error aggregated across all PEs with operator max
// *****************************************************************************
{
  if (error) {

    std::cout << "\n>>> ERROR: A Mapper chare was not assigned any mesh "
              "elements. This can happen in SMP-mode with a large +ppn "
              "parameter (number of worker threads per logical node) and is "
              "most likely the fault of the mesh partitioning algorithm not "
              "tolerating the case when it is asked to divide the "
              "computational domain into a number of partitions different "
              "than the number of ranks it is called on, i.e., in case of "
              "overdecomposition and/or calling the partitioner in SMP mode "
              "with +ppn larger than 1. Solution 1: Try a different "
              "partitioning algorithm (e.g., rcb instead of mj). Solution 2: "
              "Decrease +ppn.";

    finish();

  } else {

     meshes[meshid].m_mapper.doneInserting();
     meshes[meshid].m_mapper.setup( meshes[meshid].m_npoin, meshes[meshid].m_firstchunk );

  }
}

void
Transporter::queried( std::size_t meshid )
// *****************************************************************************
// Reduction target: all Mapper chares have queried their boundary nodes
// *****************************************************************************
{
  meshes[meshid].m_mapper.response();
}

void
Transporter::responded( std::size_t meshid )
// *****************************************************************************
// Reduction target: all Mapper chares have responded with their boundary nodes
// *****************************************************************************
{
  meshes[meshid].m_mapper.create();
}

void
Transporter::workinserted( std::size_t meshid )
// *****************************************************************************
// Reduction target: all Workers have been created
// *****************************************************************************
{
  meshes[meshid].m_worker.doneInserting();
} 

void
Transporter::workcreated( std::size_t meshid )
// *****************************************************************************
// Reduction target: all Worker constructors have been called
// *****************************************************************************
{
  std::cout << "Number of tetrahedra for mesh " << meshid << ": "
      << meshes[meshid].m_nelem << '\n';;
  std::cout << "Number of points for mesh " << meshid << ": "
      << meshes[meshid].m_npoin << '\n';

  meshes[meshid].m_worker.out();

  completed++;
  m_sourcemeshid = 0;
  m_destmeshid = 1;
  if (completed == meshes.size()) {
    meshes[m_sourcemeshid].m_worker.collideTets();
    meshes[m_destmeshid].m_worker.collideVertices();
  }
}

void
Transporter::written()
// *****************************************************************************
// Reduction target: all Workers have written out mesh/field data
// *****************************************************************************
{
  // FIXME: No guarantee the write finishes.
  //finish();
}

Transporter::Transporter( CkMigrateMessage* m ) : CBase_Transporter( m )
// *****************************************************************************
//  Migrate constructor: returning from a checkpoint
//! \param[in] m Charm++ migrate message
// *****************************************************************************
{
  std::cout << "Restarted from checkpoint\n";
}

void
Transporter::finish()
// *****************************************************************************
// Normal finish of time stepping
// *****************************************************************************
{
  std::cout << "Normal finish\n";
  mainProxy.finalize();
}

void
Transporter::processCollisions(int nColl, Collision* colls)
// *****************************************************************************
// Normal finish of time stepping
// *****************************************************************************
{
  std::cout << "Collisions found: " << nColl << std::endl;
  std::size_t first = meshes[m_destmeshid].m_firstchunk;
  std::size_t nchare = meshes[m_destmeshid].m_nchare;
  std::vector<Collision>* separated = new std::vector<Collision>[nchare];
  for (int i = 0; i < nColl; i++) {
    if (colls[i].A.chunk >= first && colls[i].A.chunk < first + nchare) {
      separated[colls[i].A.chunk - first].push_back(colls[i]);
    } else {
      separated[colls[i].B.chunk - first].push_back(colls[i]);
    }
  }

  for (int i = 0; i < nchare; i++) {
    CkPrintf("Dest mesh chunk %i has %i\n", i, separated[i].size());
    meshes[m_destmeshid].m_worker[i].processCollisions(separated[i].size(), separated[i].data(), meshes[m_sourcemeshid].m_nchare, meshes[m_sourcemeshid].m_firstchunk, meshes[m_sourcemeshid].m_worker);
  }

  delete[] separated;
}

#include "NoWarning/transporter.def.h"
