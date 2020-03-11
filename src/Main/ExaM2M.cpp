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

#include "ExaM2MDriver.hpp"
#include "ProcessException.hpp"

#include "NoWarning/exam2m.decl.h"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#endif

//! \brief Charm handle to the main proxy, facilitates call-back to finalize,
//!    etc., must be in global scope.
CProxy_Main mainProxy;

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
// ..

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
      m_signal( tk::setSignalHandlers() ),
      m_driver()
    {
      delete msg;
      mainProxy = thisProxy;
      CkStartQD( CkCallback( CkIndex_Main::quiescence(), thisProxy ) );
      // Fire up an asynchronous execute object, which when created at some
      // future point in time will call back to this->execute(). This is
      // necessary so that this->execute() can access already migrated
      // global-scope data.
      CProxy_execute::ckNew();
    } catch (...) { tk::processExceptionCharm(); }

    //! Migrate constructor: returning from a checkpoint
    explicit Main( CkMigrateMessage* msg ) : CBase_Main( msg ),
      m_signal( tk::setSignalHandlers() ),
      m_driver()
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
      } catch (...) { tk::processExceptionCharm(); }
    }

    /** @name Charm++ pack/unpack serializer member functions */
    ///@{
    //! \brief Pack/Unpack serialize member function
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
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

#include "NoWarning/exam2m.def.h"
