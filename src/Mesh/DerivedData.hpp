// *****************************************************************************
/*!
  \file      src/Mesh/DerivedData.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Generate data structures derived from unstructured mesh
  \details   Generate data structures derived from the connectivity information
     of an unstructured mesh.
*/
// *****************************************************************************
#ifndef DerivedData_h
#define DerivedData_h

#include <vector>
#include <map>
#include <utility>
#include <cstddef>
#include "Types.hpp"
#include "Fields.hpp"
#include "UnsMesh.hpp"

namespace tk {

//! Const array defining the node ordering convention for tetrahedron faces
//! \details This two-dimensional array stores the naming/ordering convention of
//!   the node indices of a tetrahedron (tet) element. The dimensions are 4x3 as
//!   a tetrahedron has a total of 4 nodes and each (triangle) face has 3 nodes.
//!   Thus the array below associates tet node 0 with nodes {1,2,3}, tet node 1
//!   with {2,0,3}, tet node 2 with {3,0,1}, and tet node 3 with {0,2,1}. Note
//!   that not only these mappings are important, but also the order of the
//!   nodes within the triplets as this specific order also defines the outwards
//!   normal of each face.
const std::array< UnsMesh::Face, 4 >
  lpofa{{ {{1,2,3}}, {{2,0,3}}, {{3,0,1}}, {{0,2,1}} }};

//! Const array defining the node ordering convention for tetrahedron edges
const std::array< UnsMesh::Edge, 6 >
  lpoed{{ {{0,1}}, {{1,2}}, {{0,2}}, {{0,3}}, {{1,3}}, {{2,3}} }};

//! Const array defining the node ordering convention for triangle edges
const std::array< UnsMesh::Edge, 3 > lpoet{{ {{0,1}}, {{1,2}}, {{2,0}} }};

//! Determine edge orientation
int
orient( const UnsMesh::Edge& t, const UnsMesh::Edge& e );

//! Compute number of points (nodes) in mesh from connectivity
std::size_t
npoin_in_graph( const std::vector< std::size_t >& inpoel );

//! Compute the unit normal vector of a triangle
std::array< tk::real, 3 >
normal( const std::array< tk::real, 3 >& x,
        const std::array< tk::real, 3 >& y,
        const std::array< tk::real, 3 >& z );

//! Compute the area of a triangle
tk::real
area( const std::array< tk::real, 3 >& x,
      const std::array< tk::real, 3 >& y,
      const std::array< tk::real, 3 >& z );

//! Generate derived data structure, elements surrounding points
std::pair< std::vector< std::size_t >, std::vector< std::size_t > >
genEsup( const std::vector< std::size_t >& inpoel, std::size_t nnpe );

//! Generate derived data structure, points surrounding points
std::pair< std::vector< std::size_t >, std::vector< std::size_t > >
genPsup( const std::vector< std::size_t >& inpoel,
         std::size_t nnpe,
         const std::pair< std::vector< std::size_t >,
                          std::vector< std::size_t > >& esup );

//! Generate derived data structure, edges surrounding points
std::pair< std::vector< std::size_t >, std::vector< std::size_t > >
genEdsup( const std::vector< std::size_t >& inpoel,
          std::size_t nnpe,
          const std::pair< std::vector< std::size_t >,
                           std::vector< std::size_t > >& esup );

//! Generate derived data structure, edge connectivity
std::vector< std::size_t >
genInpoed( const std::vector< std::size_t >& inpoel,
           std::size_t nnpe,
           const std::pair< std::vector< std::size_t >,
                            std::vector< std::size_t > >& esup );

//! Generate derived data structure, elements surrounding points of elements
std::pair< std::vector< std::size_t >, std::vector< std::size_t > >
genEsupel( const std::vector< std::size_t >& inpoel,
           std::size_t nnpe,
           const std::pair< std::vector< std::size_t >,
                            std::vector< std::size_t > >& esup );

//! Generate derived data structure, elements surrounding elements
std::pair< std::vector< std::size_t >, std::vector< std::size_t > >
genEsuel( const std::vector< std::size_t >& inpoel,
          std::size_t nnpe,
          const std::pair< std::vector< std::size_t >,
                           std::vector< std::size_t > >& esup );

//! \brief Generate derived data structure, elements surrounding elements
//!   as a fixed length data structure as a full vector, including boundary
//!   elements as -1.
std::vector< int >
genEsuelTet( const std::vector< std::size_t >& inpoel,
             const std::pair< std::vector< std::size_t >,
                              std::vector< std::size_t > >& esup );

//! Generate derived data structure, edges of elements
std::vector< std::size_t >
genInedel( const std::vector< std::size_t >& inpoel,
           std::size_t nnpe,
           const std::vector< std::size_t >& inpoed );

//! Generate derived data structure, elements surrounding edges
std::unordered_map< UnsMesh::Edge, std::vector< std::size_t >,
                    UnsMesh::Hash<2>, UnsMesh::Eq<2> >
genEsued( const std::vector< std::size_t >& inpoel,
          std::size_t nnpe,
          const std::pair< std::vector< std::size_t >,
                           std::vector< std::size_t > >& esup );

//! Generate total number of boundary faces in this chunk
std::size_t
genNbfacTet( std::size_t tnbfac,
             const std::vector< std::size_t >& inpoel,
             const std::vector< std::size_t >& triinpoel_complete,
             const std::map< int, std::vector< std::size_t > >& bface_complete,
             const std::unordered_map< std::size_t, std::size_t >& lid,
             std::vector< std::size_t >& triinpoel,
             std::map< int, std::vector< std::size_t > >& bface );

//! Generate number of internal and physical-boundary faces
std::size_t
genNipfac( std::size_t nfpe,
           std::size_t nbfac,
           const std::vector< int >& esuelTet );

//! Generate derived data structure, elements surrounding faces
std::vector< int >
genEsuf( std::size_t nfpe,
         std::size_t nipfac,
         std::size_t nbfac,
         const std::vector< std::size_t >& belem,
         const std::vector< int >& esuelTet );

//! Generate derived data structure, node-face connectivity
std::vector< std::size_t >
genInpofaTet( std::size_t nipfac,
              std::size_t nbfac,
              const std::vector< std::size_t >& inpoel,
              const std::vector< std::size_t >& triinpoel,
              const std::vector< int >& esuelTet );

//! Generate derived data structure, host/boundary element
std::vector< std::size_t >
genBelemTet( std::size_t nbfac,
              const std::vector< std::size_t >& inpofa,
              const std::pair< std::vector< std::size_t >,
                               std::vector< std::size_t > >& esup );

//! Generate derived data structure, face geometry
tk::Fields
genGeoFaceTri( std::size_t nipfac,
               const std::vector< std::size_t >& inpofa,
               const tk::UnsMesh::Coords& coord );

//! Compute geometry of the face given by three vertices
tk::Fields
geoFaceTri( const std::array< tk::real, 3 >& x,
            const std::array< tk::real, 3 >& y,
            const std::array< tk::real, 3 >& z );

//! Generate derived data structure, element geometry
tk::Fields
genGeoElemTet( const std::vector< std::size_t >& inpoel,
               const tk::UnsMesh::Coords& coord );

//! Perform leak-test on mesh (partition)
bool
leakyPartition( const std::vector< int >& esueltet,
                const std::vector< std::size_t >& inpoel,
                const tk::UnsMesh::Coords& coord );

//! Check if mesh (partition) is conforming
bool
conforming( const std::vector< std::size_t >& inpoel,
            const tk::UnsMesh::Coords& coord,
            bool cerr = true );

} // tk::

#endif // DerivedData_h
