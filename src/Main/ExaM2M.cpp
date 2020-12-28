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

#include "ExaM2MDriver.hpp"
#include "ProcessException.hpp"

#include "NoWarning/exam2m.decl.h"

#include "collidecharm.h"

using exam2m::CProxy_Transporter;

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif

//! \brief Charm handle to the main proxy, facilitates call-back to finalize,
//!    etc., must be in global scope.
CProxy_Main mainProxy;

//! \brief Charm handle to the collision detection library instance
CollideHandle collideHandle;

CProxy_Transporter transporterProxy;

//! \brief boolean to collect and print stats
bool collectStats;

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

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-prototypes"
#endif

void printCollisionHandler( [[maybe_unused]] void *param,
                            int nColl,
                            Collision *colls )
{
  transporterProxy.processCollisions( nColl, colls );
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
  #pragma clang diagnostic ignored "-Wunused-private-field"
#endif

//! Charm++ main chare for the exam2m executable.
class Main : public CBase_Main {

  public:
    //! Constructor
    Main( CkArgMsg* msg )
    try :
      // Create driver
      m_signal( tk::setSignalHandlers() ),
      m_mesh_complete( 0 ),
      m_driver( msg->argc, msg->argv )
    {
      if (msg->argc > 3) {
        exam2m::g_virtualization = std::atof( msg->argv[3] );
      }

      collectStats = false;
      for(int i=0; i < msg->argc; i++) {
        if(strcmp(msg->argv[i], "+printTetStats") == 0)
          collectStats = true;
      }

      delete msg;
      mainProxy = thisProxy;
      transporterProxy = CProxy_Transporter::ckNew( 0 );
      transporterProxy.run();
      // Fire up an asynchronous execute object, which when created at some
      // future point in time will call back to this->execute(). This is
      // necessary so that this->execute() can access already migrated
      // global-scope data.
      CProxy_execute::ckNew();

      double gridX = 0.008, gridY = 0.008, gridZ = 0.008;

      CkPrintf("ExaM2M> Collision Detection Library gridMap: %lf X %lf X %lf\n", gridX, gridY, gridZ);
      CollideGrid3d gridMap(CkVector3d(0, 0, 0),CkVector3d(gridX, gridY, gridZ));
      collideHandle = CollideCreate(gridMap,
          CollideSerialClient(printCollisionHandler, 0));

    } catch (...) { tk::processExceptionCharm(); }

    //! Migrate constructor: returning from a checkpoint
    explicit Main( CkMigrateMessage* msg ) : CBase_Main( msg ),
      m_signal( tk::setSignalHandlers() ),
      m_driver( reinterpret_cast<CkArgMsg*>(msg)->argc,
                reinterpret_cast<CkArgMsg*>(msg)->argv )
    {
      mainProxy = thisProxy;
      CkStartQD( CkCallback( CkIndex_Main::quiescence(), thisProxy ) );
    }

    //! Execute driver created and initialized by constructor
    void execute() {
      try {
        m_driver.execute();
      } catch (...) { tk::processExceptionCharm(); }
    }

    //! Towards normal exit but collect chare state first (if any)
    void finalize() {
      try {
        CkExit();
      } catch (...) { tk::processExceptionCharm(); }
    }

    //! Entry method triggered when quiescence is detected
    void quiescence() {
      try {
        std::cout << "Quiescence detected\n";
        CkExit();
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
    int m_mesh_complete;                //!< Used to delay exit until all done
    exam2m::ExaM2MDriver m_driver;      //!< Driver
};

//! \brief Charm++ chare execute
//! \details By the time this object is constructed, the Charm++ runtime system
//!    has finished migrating all global-scoped read-only objects which happens
//!    after the main chare constructor has finished.
class execute : public CBase_execute {
  public:
    //! Constructor
    execute() { mainProxy.execute(); }
    //! Migrate constructor
    explicit execute( CkMigrateMessage* m ) : CBase_execute( m ) {}
};

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#include "NoWarning/exam2m.def.h"
