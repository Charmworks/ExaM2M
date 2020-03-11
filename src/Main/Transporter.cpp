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

Transporter::Transporter( const std::vector< std::string >& argv )
// *****************************************************************************
//  Constructor
//! \param[in] argv Command line arguments
// *****************************************************************************
{
  std::size_t nstep = 1;

  if ( nstep != 0 ) {    // start time stepping, exercising mesh-to-mesh transfer

    std::cout << "Args: ";
    for (const auto& arg : argv) std::cout << arg << ' ';
    std::cout << '\n';

    if (argv.empty()) Throw( "The first argument must be an exodus filename" );
    std::string inputfile( argv[0] );

    std::map< int, std::vector< std::size_t > > bface;
    std::map< int, std::vector< std::size_t > > faces;
    std::map< int, std::vector< std::size_t > > bnode;

    // Create ExodusII mesh file reader 
    tk::ExodusIIMeshReader mr( inputfile );

    // Read boundary-face connectivity on side sets
    mr.readSidesetFaces( bface, faces );
    // Read node lists on side sets
    bnode = mr.readSidesetNodes();

    // Create partitioner callbacks (order matters)
    tk::PartitionerCallback cbp {{
        CkCallback( CkReductionTarget(Transporter,load), thisProxy )
      , CkCallback( CkReductionTarget(Transporter,distributed), thisProxy )
    }};

    // Create MeshWriter chare group
    m_meshwriter = tk::CProxy_MeshWriter::ckNew();

    // Create mesh partitioner Charm++ chare nodegroup
    m_partitioner =
      CProxy_Partitioner::ckNew( inputfile, cbp, m_meshwriter,
                                 bface, faces, bnode );

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

  // Compute load distribution given total work (nelem) and virtualization
  uint64_t chunksize, remainder;
  m_nchare = static_cast< int >(
               tk::linearLoadDistributor( virtualization,
                 nelem, CkNumPes(), chunksize, remainder ) );

  // Print out info on load distribution
  std::cout << "Initial load distribution\n";
  std::cout << "Virtualization [0.0...1.0]: " << virtualization << '\n';
  std::cout << "Number of tetrahedra: " << nelem << '\n';
  //std::cout << "Number of points: " << npoin << '\n';
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
  mainProxy.finalize();
}

#include "NoWarning/transporter.def.h"
