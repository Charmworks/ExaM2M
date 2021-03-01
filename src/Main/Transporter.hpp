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
#include "Timer.hpp"
#include "MeshData.hpp"

#include "NoWarning/transporter.decl.h"
#include "NoWarning/mapper.decl.h"


namespace exam2m{

class collisionResultMgr : public CBase_collisionResultMgr {
    std::vector <MeshData> m_meshes;
  public:
    collisionResultMgr() {}
    void recvCollResults(CkDataMsg *msg) {
      Collision *colls = (Collision *)msg->getData();
      int nColl = msg->getSize()/sizeof(Collision);
      CmiPrintf("[%d] Received %d collision results\n", CmiMyPe(), nColl);
    }
    void recv_meshData(std::vector< MeshData> meshes) {
      m_meshes = meshes;


    }
};

//! Transporter drives time integration
class Transporter : public CBase_Transporter {
  #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-parameter"
  #elif defined(STRICT_GNUC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
  #endif
  Transporter_SDAG_CODE
  #if defined(__clang__)
    #pragma clang diagnostic pop
  #elif defined(STRICT_GNUC)
    #pragma GCC diagnostic pop
  #endif

  public:
    //! Constructor
    explicit Transporter(CProxy_collisionResultMgr cProxy);

    //! Migrate constructor: returning from a checkpoint
    explicit Transporter( CkMigrateMessage* m );

    /** @name Charm++ pack/unpack serializer member functions */
    ///@{
    //! \brief Pack/Unpack serialize member function
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \note This is a Charm++ mainchare, pup() is thus only for
    //!    checkpoint/restart.
    void pup( PUP::er& p ) override {
      p | m_meshes;
      p | m_currentchunk;
      p | m_sourcemeshid;
      p | m_destmeshid;
      p | m_timer;
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

    //! Mesh Data for all meshes
    std::vector< MeshData > m_meshes;
    //! Chunk counter for ensuring unique chunk number for each mesh
    std::size_t m_currentchunk;
    //! ID of the source mesh
    std::size_t m_sourcemeshid;
    //! ID of the dest mesh
    std::size_t m_destmeshid;
    //! Timers
    std::vector< tk::Timer > m_timer;

    CProxy_collisionResultMgr collisionMgrProxy;
};

} // exam2m::

#endif // Transporter_h
