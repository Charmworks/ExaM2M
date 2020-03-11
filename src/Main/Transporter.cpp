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
#include "NoWarning/exam2m.decl.h"

extern CProxy_Main mainProxy;

using exam2m::Transporter;

Transporter::Transporter()
// *****************************************************************************
//  Constructor
// *****************************************************************************
{
  std::size_t nstep = 0;

  if ( nstep != 0) {

    // start time stepping, exercising mesh-to-mesh transfer

  } else finish();      // stop if no time stepping requested
}

Transporter::Transporter( CkMigrateMessage* m ) :
  CBase_Transporter( m )
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
