// *****************************************************************************
/*!
  \file      src/Main/Driver.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Driver drives time stepping
  \details   Driver drives time stepping.
*/
// *****************************************************************************
#ifndef Driver_h
#define Driver_h

#include <vector>
#include <string>

#include "Partitioner.hpp"
#include "Timer.hpp"

#include "NoWarning/driver.decl.h"
#include "NoWarning/mapper.decl.h"

namespace exam2m{

//! Driver drives time integration
class Driver : public CBase_Driver {
  #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-parameter"
  #elif defined(STRICT_GNUC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
  #endif
  Driver_SDAG_CODE
  #if defined(__clang__)
    #pragma clang diagnostic pop
  #elif defined(STRICT_GNUC)
    #pragma GCC diagnostic pop
  #endif

  public:
    //! Constructor
    explicit Driver();

    //! Migrate constructor: returning from a checkpoint
    explicit Driver( CkMigrateMessage* m );

    /** @name Charm++ pack/unpack serializer member functions */
    ///@{
    //! \brief Pack/Unpack serialize member function
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \note This is a Charm++ mainchare, pup() is thus only for
    //!    checkpoint/restart.
    void pup( PUP::er& p ) override {
      p | m_meshes;
      p | m_sourcemeshid;
      p | m_destmeshid;
      p | m_curriter;
      p | m_totaliter;
    }
    //! \brief Pack/Unpack serialize operator|
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \param[in,out] t Driver object reference
    friend void operator|( PUP::er& p, Driver& t ) { t.pup(p); }
    //@}

  private:

    //! Add mesh by filename
    void initMeshData( const std::string& file );

    //! Once partitioner returns initial partitioning info, update the
    //! number of elements and number of chares in the associated mesh
    void updatenelems( std::size_t meshid, std::size_t nelems );

    struct MeshData {
      int m_nchare;                        //!< Number of worker chares
      CProxy_Partitioner m_partitioner;    //!< Partitioner nodegroup proxy
      tk::CProxy_MeshWriter m_meshwriter;  //!< Mesh writer nodegroup proxy
      CProxy_Mapper m_mapper;              //!< Mapper array proxy
      CProxy_MeshArray m_mesharray;        //!< Mesh array proxy
      std::size_t m_nelem;                 //!< Total number of elements in mesh
      std::size_t m_npoin;                 //!< Total number of nodes in mesh
      void pup( PUP::er& p ) {
        p | m_nchare;
        p | m_partitioner;
        p | m_meshwriter;
        p | m_mapper;
        p | m_mesharray;
        p | m_nelem;
        p | m_npoin;
      }
      friend void operator|( PUP::er& p, MeshData& t ) { t.pup(p); }
    };

    //! Mesh Data for all meshes
    std::vector<MeshData> m_meshes;
    //! ID of the source mesh
    int m_sourcemeshid;
    //! ID of the dest mesh
    int m_destmeshid;
    //! Timers
    std::vector< tk::Timer > m_timer;
    //! SDAG variable for iteration
    int m_curriter;
    //! Total number of iterations to run
    int m_totaliter;
};

} // exam2m::

#endif // Driver_h
