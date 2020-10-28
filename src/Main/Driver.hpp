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

#include "NoWarning/driver.decl.h"
#include "NoWarning/mapper.decl.h"

namespace exam2m{

//! Driver drives time integration
class Driver : public CBase_Driver {
Driver_SDAG_CODE;
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
      //p | m_nchare;
      //p | m_partitioner;
      //p | m_meshwriter;
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

    //! Take collision list and distribute it to the destination mesh
    //! to start testing for actual collisions
    void distributeCollisions( int nColl, Collision* colls );

    struct MeshData {
      int m_nchare;                        //!< Number of worker chares
      int m_firstchunk;                    //!< First chunk ID (for collision)
      CProxy_Partitioner m_partitioner;    //!< Partitioner nodegroup proxy
      tk::CProxy_MeshWriter m_meshwriter;  //!< Mesh writer nodegroup proxy
      CProxy_Mapper m_mapper;              //!< Mapper array proxy
      CProxy_Worker m_worker;              //!< Worker array proxy
      std::size_t m_nelem;                 //!< Total number of elements in mesh
      std::size_t m_npoin;                 //!< Total number of nodes in mesh
    };
    //! Mesh Data for all meshes
    std::vector<MeshData> meshes;
    //! Chunk counter for ensuring unique chunk number for each mesh
    std::size_t m_currentchunk;
    //! ID of the source mesh
    std::size_t m_sourcemeshid;
    //! ID of the dest mesh
    std::size_t m_destmeshid;
};

} // exam2m::

#endif // Driver_h
