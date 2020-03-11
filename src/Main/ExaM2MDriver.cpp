// *****************************************************************************
/*!
  \file      src/Main/ExaM2MDriver.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     ExaM2M driver
  \details   ExaM2M driver.
*/
// *****************************************************************************

#include "ExaM2MDriver.hpp"

#include "NoWarning/transporter.decl.h"

using exam2m::ExaM2MDriver;

ExaM2MDriver::ExaM2MDriver()
// *****************************************************************************
//  Constructor
// *****************************************************************************
{
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
  CProxy_Transporter::ckNew( 0 );
}
