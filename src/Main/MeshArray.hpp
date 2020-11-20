// *****************************************************************************
/*!
  \file      src/Main/MeshArray.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Chare class declaration for mesharrays holding part of a mesh
  \details   Chare class declaration for mesharrays holding part of a mesh.
*/
// *****************************************************************************
#ifndef MeshArray_h
#define MeshArray_h

#include "Types.hpp"
#include "PUPUtil.hpp"
#include "UnsMesh.hpp"
#include "CommMap.hpp"
#include "Fields.hpp"

#include "NoWarning/mesharray.decl.h"

namespace exam2m {

//! MeshArray chare array holding part of a mesh
class MeshArray : public CBase_MeshArray {

  public:
    //! Constructor
    explicit
      MeshArray(
        const tk::CProxy_MeshWriter& meshwriter,
        const tk::MeshCallback& cbw,
        const std::vector< std::size_t >& ginpoel,
        const tk::UnsMesh::CoordMap& coordmap,
        const tk::CommMaps& commaps,
        const std::map< int, std::vector< std::size_t > >& bface,
        const std::vector< std::size_t >& triinpoel,
        const std::map< int, std::vector< std::size_t > >& bnode,
        int nc );

    #if defined(__clang__)
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wundefined-func-template"
    #endif
    //! Migrate constructor
    // cppcheck-suppress uninitMemberVar
    explicit MeshArray( CkMigrateMessage* ) {}
    #if defined(__clang__)
      #pragma clang diagnostic pop
    #endif

    //! Output mesh and fields data (solution dump) to file(s)
    void write( int meshid,
                const std::vector< std::size_t >& inpoel,
                const tk::UnsMesh::Coords& coord,
                const std::map< int, std::vector< std::size_t > >& bface,
                const std::map< int, std::vector< std::size_t > >& bnode,
                const std::vector< std::size_t >& triinpoel,
                const std::vector< std::string>& elemfieldnames,
                const std::vector< std::string>& nodefieldnames,
                const std::vector< std::string>& nodesurfnames,
                const std::vector< std::vector< tk::real > >& elemfields,
                const std::vector< std::vector< tk::real > >& nodefields,
                const std::vector< std::vector< tk::real > >& nodesurfs,
                CkCallback c );

    //! Write out some field data to file(s)
    void out( int meshid );

    //! Mesh and field data written to file(s)
    void written();

    void solutionFound();
    void transferSource();
    void transferDest();

    /** @name Charm++ pack/unpack serializer member functions */
    ///@{
    //! \brief Pack/Unpack serialize member function
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    void pup( PUP::er &p ) override {
      p | m_cbw;
      p | m_nchare;
      p | m_it;
      p | m_itr;
      p | m_itf;
      p | m_t;
      p | m_lastDumpTime;
      p | m_meshwriter;
      p | m_el;
      if (p.isUnpacking()) {
        m_inpoel = std::get< 0 >( m_el );
        m_gid = std::get< 1 >( m_el );
        m_lid = std::get< 2 >( m_el );
      }
      p | m_coord;
      p | m_nodeCommMap;
      p | m_edgeCommMap;
      p | m_bface;
      p | m_triinpoel;
      p | m_bnode;
      p | m_u;
    }
    //! \brief Pack/Unpack serialize operator|
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    //! \param[in,out] i MeshArray object reference
    friend void operator|( PUP::er& p, MeshArray& i ) { i.pup(p); }
    //@}

  private:
    //! Charm++ callbacks associated to compile-time tags for MeshArray
    tk::MeshCallback m_cbw;
    //! Total number of MeshArray chares
    int m_nchare;
    //! Iteration count
    uint64_t m_it;
    //! Iteration count with mesh refinement
    //! \details Used as the restart sequence number {RS} in saving output in
    //!    an ExodusII sequence
    //! \see https://www.paraview.org/Wiki/Restarted_Simulation_Readers
    uint64_t m_itr;
    //! Field output iteration count without mesh refinement
    //! \details Counts the number of field outputs to file during two
    //!   time steps with mesh efinement
    uint64_t m_itf;
    //! Physical time
    tk::real m_t;
    //! Physical time at last field output
    tk::real m_lastDumpTime;
    //! Mesh writer proxy
    tk::CProxy_MeshWriter m_meshwriter;
    //! \brief Elements of the mesh chunk we operate on
    //! \details Initialized by the constructor. The first vector is the element
    //!   connectivity (local IDs), the second vector is the global node IDs of
    //!   owned elements, while the third one is a map of global->local node
    //!   IDs.
    tk::UnsMesh::Chunk m_el;
    //! Alias to element connectivity
    std::vector< std::size_t >& m_inpoel = std::get<0>( m_el );
    //! Alias to global node IDs of owned elements
    std::vector< std::size_t >& m_gid = std::get<1>( m_el );
    //! \brief Alias to local node ids associated to the global ones of owned
    //!    elements
    std::unordered_map< std::size_t, std::size_t >& m_lid = std::get<2>( m_el );
    //! Mesh point coordinates
    tk::UnsMesh::Coords m_coord;
    //! \brief Global mesh node IDs bordering the mesh chunk held by fellow
    //!   Discretization chares associated to their chare IDs
    tk::NodeCommMap m_nodeCommMap;
    //! \brief Edges with global node IDs bordering the mesh chunk held by
    //!   fellow Discretization chares associated to their chare IDs
    tk::EdgeCommMap m_edgeCommMap;
    //! Boundary face lists mapped to side set ids
    std::map< int, std::vector< std::size_t > > m_bface;
    //! Triangle face connectivity 
    std::vector< std::size_t > m_triinpoel;
    //! Boundary node lists mapped to side set ids
    std::map< int, std::vector< std::size_t > > m_bnode;
    //! Solution in mesh nodes
    tk::Fields m_u;

    //! Set mesh coordinates based on coordinates map
    tk::UnsMesh::Coords setCoord( const tk::UnsMesh::CoordMap& coordmap );
};

} // exam2m::

#endif // MeshArray_h
