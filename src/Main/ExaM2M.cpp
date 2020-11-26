// *****************************************************************************
/*!
  \file      src/Main/ExaM2M.cpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     ExaM2M, mesh-to-mesh transfer library, Charm++ main chare.
  \details   ExaM2M, mesh-to-mesh transfer library, Charm++ main chare. This
    file contains the definition of the Charm++ main chare, equivalent to main()
    in Charm++-land.
*/
// *****************************************************************************

#include <iostream>
#include <cstdlib>

#include "ProcessException.hpp"

#include "NoWarning/exam2m.decl.h"

using exam2m::CProxy_Driver;

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif

//! \brief Charm handle to the main proxy, facilitates call-back to finalize,
//!    etc., must be in global scope.
CProxy_Main mainProxy;
CProxy_Driver driverProxy;

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

//! ExaM2M declarations and definitions
namespace exam2m {

//! Global-scope data. Initialized by the main chare and distibuted to all PEs
//! by the Charm++ runtime system. Though semantically not const, all these
//! global data should be considered read-only. See also
//! http://charm.cs.illinois.edu/manuals/html/charm++/manual.html. The data
//! below is global-scope because they must be available to all PEs which could
//! be on different machines.

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif

// Global scope data
tk::real g_virtualization = 0.0;

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

} // exam2m::

//! Charm++ main chare for the exam2m executable.
class Main : public CBase_Main {

  public:
    //! Constructor
    Main( CkArgMsg* msg )
    try :
      // Create driver
      m_signal( tk::setSignalHandlers() )
    {
      CkPrintf("ExaM2M> Args:");
      for (int i = 1; i < msg->argc; i++) CkPrintf("%s ", msg->argv[i]);
      CkPrintf("\n");

      if ( msg->argc < 3 ) {
        Throw( "The first two arguments must be exodus filenames");
      }

      if (msg->argc > 3) {
        exam2m::g_virtualization = std::atof( msg->argv[3] );
      }

      mainProxy = thisProxy;

      // Create the driver, add the two meshes, and tell it to run
      CProxy_Driver driverProxy = CProxy_Driver::ckNew( 0 );
      driverProxy.addMesh(msg->argv[1]);
      driverProxy.addMesh(msg->argv[2]);
      driverProxy.run();

      delete msg;
    } catch (...) { tk::processExceptionCharm(); }

    //! Migrate constructor: returning from a checkpoint
    explicit Main( CkMigrateMessage* msg ) : CBase_Main( msg ),
      m_signal( tk::setSignalHandlers() )
    {
      CkStartQD( CkCallback( CkIndex_Main::quiescence(), thisProxy ) );
    }

    //! Towards normal exit but collect chare state first (if any)
    void finalize() {
      try {
        CkPrintf("ExaM2M> Transfer complete, exiting cleanly\n");
        CkExit();
      } catch (...) { tk::processExceptionCharm(); }
    }

    //! Entry method triggered when quiescence is detected
    void quiescence() {
      try {
        CkAbort("Queiscence detected before tranfser completed\n");
      } catch (...) { tk::processExceptionCharm(); }
    }

    /** @name Charm++ pack/unpack serializer member functions */
    ///@{
    //! \brief Pack/Unpack serialize member function
    // //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \note This is a Charm++ mainchare, pup() is thus only for
    //!    checkpoint/restart.
    void pup( PUP::er& ) override {
    }
    //! \brief Pack/Unpack serialize operator|
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \param[in,out] m Mainchare object reference
    friend void operator|( PUP::er& p, Main& m ) { m.pup(p); }
    //@}

  private:
    int m_signal;                       //!< Used to set signal handlers
};

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#include "NoWarning/exam2m.def.h"
