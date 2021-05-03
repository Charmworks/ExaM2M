// *****************************************************************************
/*!
  \file      src/Main/Driver.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Driver drives time integration
  \details   Driver drives time integration.
*/
// *****************************************************************************

#include <iostream>

#include "Driver.hpp"
#include "ExodusIIMeshReader.hpp"
#include "LoadDistributor.hpp"

#include "NoWarning/exam2m.decl.h"

extern CProxy_Main mainProxy;

namespace exam2m {

extern tk::real g_virtualization;

}

using exam2m::Driver;

Driver::Driver() : m_totaliter(1)
// *****************************************************************************
//  Constructor
// *****************************************************************************
{}

Driver::Driver( CkMigrateMessage* m ) : CBase_Driver( m )
// *****************************************************************************
//  Migrate constructor: returning from a checkpoint
//! \param[in] m Charm++ migrate message
// *****************************************************************************
{
  std::cout << "Restarted from checkpoint\n";
}

void Driver::initMeshData( const std::string& file )
// *****************************************************************************
//  addMesh
//! \param[in] file Name of the file to read the mesh data from
// *****************************************************************************
{
  std::map< int, std::vector< std::size_t > > bface;
  std::map< int, std::vector< std::size_t > > faces;
  std::map< int, std::vector< std::size_t > > bnode;

  MeshData mesh;
  auto meshid = static_cast< unsigned short > ( m_meshes.size() );

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
      CkCallback( CkReductionTarget(Driver,loaded), thisProxy )
    , CkCallback( CkReductionTarget(Driver,distributed), thisProxy )
    , CkCallback( CkReductionTarget(Driver,mapinserted), thisProxy )
  }};
  cbp.get<tag::load>().setRefnum(meshid);
  cbp.get<tag::distributed>().setRefnum(meshid);
  cbp.get<tag::mapinserted>().setRefnum(meshid);

  // Create Mapper callbacks (order matters)
  tk::MapperCallback cbm {{
      CkCallback( CkReductionTarget(Driver,queried), thisProxy )
    , CkCallback( CkReductionTarget(Driver,responded), thisProxy )
    , CkCallback( CkReductionTarget(Driver,workinserted), thisProxy )
  }};
  cbm.get<tag::queried>().setRefnum(meshid);
  cbm.get<tag::responded>().setRefnum(meshid);
  cbm.get<tag::workinserted>().setRefnum(meshid);

  // Create MeshArray callbacks (order matters)
  tk::MeshCallback cbw {{
      CkCallback( CkReductionTarget(Driver,workcreated), thisProxy )
    , CkCallback( CkReductionTarget(Driver,solutionfound), thisProxy )
    , CkCallback( CkReductionTarget(Driver,written), thisProxy )
  }};
  cbw.get<tag::workcreated>().setRefnum(meshid);
  cbw.get<tag::solutionfound>().setRefnum(meshid);
  cbw.get<tag::written>().setRefnum(meshid);

  // Create MeshWriter chare group
  mesh.m_meshwriter = tk::CProxy_MeshWriter::ckNew();

  // Create empty Mappers, will setup communication maps
  mesh.m_mapper = CProxy_Mapper::ckNew();

  // Create the empty MeshArray, will hold chunk of the mesh
  mesh.m_mesharray = CProxy_MeshArray::ckNew();

  // Create Partitioner nodegroup
  mesh.m_partitioner =
    CProxy_Partitioner::ckNew( file, cbp, cbm, cbw,
       mesh.m_meshwriter, mesh.m_mapper, mesh.m_mesharray, bface, faces, bnode );

  m_meshes.push_back(mesh);
}

void
Driver::updatenelems( std::size_t meshid, std::size_t nelem )
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

  // Print out info on load distribution
  std::cout << "Initial load distribution for mesh " << meshid << "\n";
  std::cout << "Virtualization [0.0...1.0]: " << g_virtualization << '\n';
  std::cout << "Number of work units: " << mesh.m_nchare << '\n';

  // Tell the meshwriter the total number of chares
  mesh.m_meshwriter.nchare( mesh.m_nchare );
}

#include "NoWarning/driver.def.h"
