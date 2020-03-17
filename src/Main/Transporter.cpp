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

#include "NoWarning/exam2m.decl.h"

extern CProxy_Main mainProxy;

using exam2m::Transporter;

Transporter::Transporter( const std::string& meshfilename ) :
  m_nchare( 0 ),
  m_partitioner(),
  m_meshwriter(),
  m_worker(),
  m_nelem( 0 ),
  m_npoin( 0 )
// *****************************************************************************
//  Constructor
//! \param[in] argv Command line arguments
// *****************************************************************************
{
  std::size_t nstep = 1;

  if ( nstep != 0 ) {    // start time stepping, exercising mesh-to-mesh transfer

    std::map< int, std::vector< std::size_t > > bface;
    std::map< int, std::vector< std::size_t > > faces;
    std::map< int, std::vector< std::size_t > > bnode;

    // Create ExodusII mesh file reader 
    tk::ExodusIIMeshReader mr( meshfilename );

    // Read out total number of mesh points from mesh file
    m_npoin = mr.npoin();

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
    m_meshwriter = tk::CProxy_MeshWriter::ckNew();

    // Create empty Mappers, will setup communication maps
    m_mapper = CProxy_Mapper::ckNew();

    // Create empty Workers, will hold chunk of the mesh
    m_worker = CProxy_Worker::ckNew();

    // Create Partitioner nodegroup
    m_partitioner =
      CProxy_Partitioner::ckNew( meshfilename, cbp, cbm, cbw,
         m_meshwriter, m_mapper, m_worker, bface, faces, bnode );

  } else finish();      // stop if no time stepping requested
}

void
Transporter::load( std::size_t nelem )
// *****************************************************************************
// Reduction target: the mesh has been read from file on all PEs
//! \param[in] nelem Total number of mesh elements (summed across all nodes)
// *****************************************************************************
{
  tk::real virtualization = 0.0;
  m_nelem = nelem;

  // Compute load distribution given total work (nelem) and virtualization
  uint64_t chunksize, remainder;
  m_nchare = static_cast< int >(
               tk::linearLoadDistributor( virtualization,
                 nelem, CkNumPes(), chunksize, remainder ) );

  // Print out info on load distribution
  std::cout << "Initial load distribution\n";
  std::cout << "Virtualization [0.0...1.0]: " << virtualization << '\n';
  std::cout << "Number of work units: " << m_nchare << '\n';

  // Tell the meshwriter the total number of chares
  m_meshwriter.nchare( m_nchare );

  // Partition the mesh
  m_partitioner.partition( m_nchare );
}

void
Transporter::distributed()
// *****************************************************************************
// Reduction target: all PEs have distrbuted their mesh after partitioning
// *****************************************************************************
{
  m_partitioner.map();
}

void
Transporter::mapinserted( int error )
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

     m_mapper.doneInserting();
     m_mapper.setup( m_npoin );

  }
}

void
Transporter::queried()
// *****************************************************************************
// Reduction target: all Mapper chares have queried their boundary nodes
// *****************************************************************************
{
  m_mapper.response();
}

void
Transporter::responded()
// *****************************************************************************
// Reduction target: all Mapper chares have responded with their boundary nodes
// *****************************************************************************
{
  m_mapper.create();
}

void
Transporter::workinserted()
// *****************************************************************************
// Reduction target: all Workers have been created
// *****************************************************************************
{
  m_worker.doneInserting();
} 

void
Transporter::workcreated()
// *****************************************************************************
// Reduction target: all Worker constructors have been called
// *****************************************************************************
{
  std::cout << "Number of tetrahedra: " << m_nelem << '\n';;
  std::cout << "Number of points: " << m_npoin << '\n';

  m_worker.out();
}

void
Transporter::written()
// *****************************************************************************
// Reduction target: all Workers have written out mesh/field data
// *****************************************************************************
{
  finish();
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

#include "NoWarning/transporter.def.h"
