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
#include "NoWarning/mapper.decl.h"

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

    //! Reduction target: all PEs have created their Mappers
    void mapinserted( int error );

    //! Reduction target: all Mapper chares have queried their boundary nodes
    void queried();
    //! \brief Reduction target: all Mapper chares have responded with their
    //!   boundary nodes
    void responded();

    ///! Reduction target: all Workers have been created
    void workinserted();

    //! Reduction target: all Worker constructors have been called
    void workcreated();

    //! Reduction target: all Workers have written out mesh/field data
    void written();

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
    CProxy_Mapper m_mapper;              //!< Mapper array proxy
    CProxy_Worker m_worker;              //!< Worker array proxy
    std::size_t m_nelem;                 //!< Total number of elements in mesh
    std::size_t m_npoin;                 //!< Total number of nodes in mesh

    //! Normal finish of time stepping
    void finish();
};

} // exam2m::

#endif // Transporter_h
