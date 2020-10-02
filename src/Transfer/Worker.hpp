// *****************************************************************************
/*!
  \file      src/Transfer/Worker.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     Chare class declaration for workers holding part of a mesh
  \details   Chare class declaration for workers holding part of a mesh.
*/
// *****************************************************************************
#ifndef Worker_h
#define Worker_h

#include "Types.hpp"
#include "PUPUtil.hpp"
#include "UnsMesh.hpp"
#include "CommMap.hpp"

#include "NoWarning/worker.decl.h"

namespace exam2m {

struct PotentialCollision {
  std::size_t source_index, dest_index;
  CkVector3d point;
  void pup(PUP::er& p) { p | source_index; p | dest_index; p | point; }
};

struct SolutionData {
  std::size_t dest_index;
  tk::real solution;
  void pup(PUP::er& p) { p | dest_index; p | solution; }
};

//! Worker chare array holding part of a mesh
class Worker : public CBase_Worker {

  public:
    //! Constructor
    explicit
      Worker(
        std::size_t firstchunk,
        const tk::CProxy_MeshWriter& meshwriter,
        const tk::WorkerCallback& cbw,
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
    explicit Worker( CkMigrateMessage* ) {}
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

    //! Initialize dest mesh solution with background data
    void background();

    //! Contribute vertex information to the collsion detection library
    void collideVertices() const;

    //! Contribute tet information to the collision detection library
    void collideTets() const;

    //! Process potential collisions in the destination mesh
    void processCollisions( CProxy_Worker proxy,
                            std::size_t nchare,
                            std::size_t offset,
                            int nColls,
                            Collision* colls ) const;

    //! Identify actual collisions in the source mesh
    void determineActualCollisions( CProxy_Worker proxy,
                                    int index,
                                    int nColls,
                                    PotentialCollision* colls ) const;

    //! Transfer the interpolated solution data back to destination mesh
    void transferSolution( int nPoints, SolutionData* soln );

    /** @name Charm++ pack/unpack serializer member functions */
    ///@{
    //! \brief Pack/Unpack serialize member function
    //! \param[in,out] p Charm++'s PUP::er serializer object reference
    void pup( PUP::er &p ) override {
      p | m_firstchunk;
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
    //! \param[in,out] i Worker object reference
    friend void operator|( PUP::er& p, Worker& i ) { i.pup(p); }
    //@}

  private:
    //! The ID of my first chunk (used for collision detection library)
    std::size_t m_firstchunk;
    //! Charm++ callbacks associated to compile-time tags for Worker
    tk::WorkerCallback m_cbw;
    //! Total number of Worker chares
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
    std::vector< tk::real > m_u;

    //! Set mesh coordinates based on coordinates map
    tk::UnsMesh::Coords setCoord( const tk::UnsMesh::CoordMap& coordmap );

    //! Determine if a point is in a tet
    bool intet(const CkVector3d &point,
               std::size_t e,
               std::array< real, 4 >& N) const;

    //! Returns true if we are the owner of the vertex
    bool owner( std::size_t index ) const;
};

} // exam2m::

#endif // Worker_h
