// *****************************************************************************
/*!
  \file      src/Inciter/Mapper.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Setup communication maps
  \details   Setup communication maps.
*/
// *****************************************************************************
#ifndef Mapper_h
#define Mapper_h

#include <vector>
#include <map>
#include <unordered_map>

#include "TaggedTuple.hpp"
#include "Tags.hpp"
#include "Callback.hpp"
#include "UnsMesh.hpp"
#include "CommMap.hpp"
#include "Callback.hpp"

#include "NoWarning/mapper.decl.h"

namespace exam2m {

//! Setup communication maps
class Mapper : public CBase_Mapper {

  public:
    //! Constructor
    explicit Mapper( const tk::CProxy_MeshWriter& meshwriter,
                     const CProxy_MeshArray& mesharray,
                     const tk::MapperCallback& cbm,
                     const tk::MeshCallback& cbw,
                     const std::vector< std::size_t >& ginpoel,
                     const tk::UnsMesh::CoordMap& coordmap,
                     const std::map< int, std::vector< std::size_t > >& bface,
                     const std::vector< std::size_t >& triinpoel,
                     const std::map< int, std::vector< std::size_t > >& bnode,
                     int nchare );

    #if defined(__clang__)
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wundefined-func-template"
    #endif
    //! Migrate constructor
    // cppcheck-suppress uninitMemberVarPrivate
    explicit Mapper( CkMigrateMessage* ) {}
    #if defined(__clang__)
      #pragma clang diagnostic pop
    #endif

    //! Setup chare mesh boundary node communication map
    void setup( std::size_t npoin );
    //! \brief Incoming query for a list mesh nodes for which this chare
    //!   compiles communication maps
    void query( int fromch, const tk::AllCommMaps& bnd );
    //! Report receipt of boundary node lists
    void recvquery();
    //! Respond to boundary node list queries
    void response();
    //! Receive boundary node communication maps for our mesh chunk
    void bnd( int fromch, const tk::CommMaps& commap );
    //! Receive receipt of boundary node communication map
    void recvbnd();

    //! Create worker chare array elements on this PE
    void create();

    /** @name Charm++ pack/unpack serializer member functions */
    ///@{
    //! \brief Pack/Unpack serialize member function
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    void pup( PUP::er &p ) override {
      p | m_meshwriter;
      p | m_cbm;
      p | m_cbw;
      p | m_ginpoel;
      p | m_coordmap;
      p | m_nbnd;
      p | m_bface;
      p | m_triinpoel;
      p | m_bnode;
      p | m_nchare;
      p | m_nodech;
      p | m_chnode;
      p | m_edgech;
      p | m_chedge;
      p | m_commap;
    }
    //! \brief Pack/Unpack serialize operator|
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \param[in,out] s Mapper object reference
    friend void operator|( PUP::er& p, Mapper& s ) { s.pup(p); }
    //@}

  private:
    //! MeshWriter proxy
    tk::CProxy_MeshWriter m_meshwriter;
    //! Worker proxy
    CProxy_MeshArray m_mesharray;
    //! Charm++ callbacks associated to compile-time tags for Mapper
    tk::MapperCallback m_cbm;
    //! Charm++ callbacks associated to compile-time tags for Worker
    tk::MeshCallback m_cbw;
    //! Tetrtahedron element connectivity of our chunk of the mesh (global ids)
    std::vector< std::size_t > m_ginpoel;
    //! Coordinates associated to global node IDs of our mesh chunk
    tk::UnsMesh::CoordMap m_coordmap;
    //! Counter for number of chares contributing to chare boundary nodes
    std::size_t m_nbnd;
    //! List of boundary faces associated to side-set IDs
    std::map< int, std::vector< std::size_t > > m_bface;
    //! Boundary face-node connectivity
    std::vector< std::size_t > m_triinpoel;
    //! List of boundary nodes associated to side-set IDs
    std::map< int, std::vector< std::size_t > > m_bnode;
    //! Total number of Mapperr chares
    int m_nchare;
    //! Node->chare map used to build boundary node communication maps
    std::unordered_map< std::size_t, std::vector< int > > m_nodech;
    //! Chare->node map used to build boundary node communication maps
    tk::NodeCommMap m_chnode;
    //! Edge->chare map used to build boundary edge communication maps
    std::unordered_map< tk::UnsMesh::Edge, std::vector< int >,
                        tk::UnsMesh::Hash<2>, tk::UnsMesh::Eq<2> > m_edgech;
    //! Chare->edge map used to build boundary edge communication maps
    tk::EdgeCommMap m_chedge;
    //! Communication maps associated to chare IDs
    tk::CommMaps m_commap;

    //! Create Discretization chare array elements on this PE
    void createDiscWorkers();
};

} // exam2m::

#endif // Mapper_h
