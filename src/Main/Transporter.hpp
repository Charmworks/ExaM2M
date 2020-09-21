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
Transporter_SDAG_CODE;
  public:
    //! Constructor
    explicit Transporter();

    //! Migrate constructor: returning from a checkpoint
    explicit Transporter( CkMigrateMessage* m );

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
    //! \param[in,out] t Transporter object reference
    friend void operator|( PUP::er& p, Transporter& t ) { t.pup(p); }
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
    //! SDAG variable for mesh ID, used as a refnum in iteration
    std::size_t m_meshid;
};

} // exam2m::

#endif // Transporter_h
