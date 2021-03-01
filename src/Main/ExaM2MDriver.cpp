// *****************************************************************************
/*!
  \file      src/Main/ExaM2MDriver.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     ExaM2M driver
  \details   ExaM2M driver.
*/
// *****************************************************************************

#include <iostream>

#include "ExaM2MDriver.hpp"

#include "MeshData.hpp"
#include "NoWarning/transporter.decl.h"

using exam2m::ExaM2MDriver;
using exam2m::CProxy_Transporter;

extern CProxy_Transporter transporterProxy;

ExaM2MDriver::ExaM2MDriver( int argc, char** argv )
// *****************************************************************************
//  Constructor
//! \param[in] argc Number of C-style character arrays in argv
//! \param[in] argv C-style character array of character arrays
// *****************************************************************************
{
  // Save command line args
  for (int i=1; i<argc; ++i) m_argv.push_back( std::string(argv[i]) );

  // All global-scope data to be migrated to all PEs initialized here (if any)
}

void
ExaM2MDriver::execute() const
// *****************************************************************************
//  Run ExaM2M
// *****************************************************************************
{
  // Instantiate Transporter chare on PE 0 which drives time-integration,
  // during which there are multiple mesh-to-mesh transfers may be required as
  // a problem evolves in time.

  CkPrintf("ExaM2M> Args:");
  for (const auto& arg : m_argv) CkPrintf("%s ", arg.c_str());
  CkPrintf("\n");

  if ( m_argv.size() < 2 ) Throw( "The first two arguments must be exodus filenames");

  CkPrintf("ExaM2M> Preparing meshes (read, partition, distribute)\n");

  // TODO: This is not synchronized in the Transporter
  transporterProxy.addMesh(m_argv[0]);
  transporterProxy.addMesh(m_argv[1]);
}
