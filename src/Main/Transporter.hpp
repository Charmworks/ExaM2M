// *****************************************************************************
/*!
  \file      src/Main/Transporter.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Transporter drives time stepping
  \details   Transporter drives time stepping.
*/
// *****************************************************************************
#ifndef Transporter_h
#define Transporter_h

#include <vector>
#include <string>

#include "Partitioner.hpp"

#include "NoWarning/transporter.decl.h"

namespace exam2m{

//! Transporter drives time integration
class Transporter : public CBase_Transporter {

  public:
    //! Constructor
    explicit Transporter( const std::vector< std::string >& argv );

    //! Migrate constructor: returning from a checkpoint
    explicit Transporter( CkMigrateMessage* m );

    //! Reduction target: the mesh has been read from file on all PEs
    void load( std::size_t nelem );

    //! Reduction target: all PEs have distrbuted their mesh after partitioning
    void distributed();

    /** @name Charm++ pack/unpack serializer member functions */
    ///@{
    //! \brief Pack/Unpack serialize member function
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \note This is a Charm++ mainchare, pup() is thus only for
    //!    checkpoint/restart.
    void pup( PUP::er& p ) override {
      p | m_nchare;
      p | m_partitioner;
      p | m_meshwriter;
    }
    //! \brief Pack/Unpack serialize operator|
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \param[in,out] t Transporter object reference
    friend void operator|( PUP::er& p, Transporter& t ) { t.pup(p); }
    //@}

  private:
    int m_nchare;                        //!< Number of worker chares
    CProxy_Partitioner m_partitioner;    //!< Partitioner nodegroup proxy    
    tk::CProxy_MeshWriter m_meshwriter;  //!< Mesh writer nodegroup proxy

    //! Normal finish of time stepping
    void finish();
};

} // exam2m::

#endif // Transporter_h
