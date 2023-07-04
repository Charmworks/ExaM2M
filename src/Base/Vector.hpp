// *****************************************************************************
/*!
  \file      src/Base/Vector.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2023 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Vector algebra
  \details   Vector algebra.
*/
// *****************************************************************************
#ifndef Vector_h
#define Vector_h

#include <array>
#include <cmath>
#include <vector>

#include "Types.hpp"
#include "Exception.hpp"

namespace tk {

//! Flip sign of vector components
//! \param[in] v Vector whose components to multiply by -1.0
inline void
flip( std::array< real, 3 >& v )
{
  v[0] = -v[0];
  v[1] = -v[1];
  v[2] = -v[2];
}

//! Compute the cross-product of two vectors
//! \param[in] v1x x coordinate of the 1st vector
//! \param[in] v1y y coordinate of the 1st vector
//! \param[in] v1z z coordinate of the 1st vector
//! \param[in] v2x x coordinate of the 2nd vector
//! \param[in] v2y y coordinate of the 2nd vector
//! \param[in] v2z z coordinate of the 2nd vector
//! \param[out] rx x coordinate of the product vector
//! \param[out] ry y coordinate of the product vector
//! \param[out] rz z coordinate of the product vector
inline void
cross( real v1x, real v1y, real v1z,
       real v2x, real v2y, real v2z,
       real& rx, real& ry, real& rz )
{
  rx = v1y*v2z - v2y*v1z;
  ry = v1z*v2x - v2z*v1x;
  rz = v1x*v2y - v2x*v1y;
}

//! Compute the cross-product of two vectors
//! \param[in] v1 1st vector
//! \param[in] v2 2nd vector
//! \return Cross-product
inline std::array< real, 3 >
cross( const std::array< real, 3 >& v1, const std::array< real, 3 >& v2 )
{
  real rx, ry, rz;
  cross( v1[0], v1[1], v1[2], v2[0], v2[1], v2[2], rx, ry, rz );
  return { std::move(rx), std::move(ry), std::move(rz) };
}

//! Compute the cross-product of two vectors divided by a scalar
//! \param[in] v1x x coordinate of the 1st vector
//! \param[in] v1y y coordinate of the 1st vector
//! \param[in] v1z z coordinate of the 1st vector
//! \param[in] v2x x coordinate of the 2nd vector
//! \param[in] v2y y coordinate of the 2nd vector
//! \param[in] v2z z coordinate of the 2nd vector
//! \param[in] j The scalar to divide the product with
//! \param[out] rx x coordinate of the product vector
//! \param[out] ry y coordinate of the product vector
//! \param[out] rz z coordinate of the product vector
inline void
crossdiv( real v1x, real v1y, real v1z,
          real v2x, real v2y, real v2z,
          real j,
          real& rx, real& ry, real& rz )
{
  cross( v1x, v1y, v1z, v2x, v2y, v2z, rx, ry, rz );
  rx /= j;
  ry /= j;
  rz /= j;
}

//! Compute the cross-product of two vectors divided by a scalar
//! \param[in] v1 1st vector
//! \param[in] v2 2nd vector
//! \param[in] j Scalar to divide each component by
//! \return Cross-product divided by scalar
inline std::array< real, 3 >
crossdiv( const std::array< real, 3 >& v1,
          const std::array< real, 3 >& v2,
          real j )
{
  return {{ (v1[1]*v2[2] - v2[1]*v1[2]) / j,
            (v1[2]*v2[0] - v2[2]*v1[0]) / j,
            (v1[0]*v2[1] - v2[0]*v1[1]) / j }};
}

//! Compute the dot-product of two vectors
//! \param[in] v1 1st vector
//! \param[in] v2 2nd vector
//! \return Dot-product
inline real
dot( const std::array< real, 3 >& v1, const std::array< real, 3 >& v2 )
{
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

//! Compute length of a vector
//! \param[in] x X coordinate of vector
//! \param[in] y Y coordinate of vector
//! \param[in] z Z coordinate of vector
//! \return length
inline real
length( real x, real y, real z )
{
  return std::sqrt( x*x + y*y + z*z );
}

//! Compute length of a vector
//! \param[in] v vector
//! \return length
inline real
length( const std::array< real, 3 >& v )
{
  return std::sqrt( dot(v,v) );
}

//! Scale vector to unit length
//! \param[in,out] v Vector to normalize
inline void
unit( std::array< real, 3 >& v ) noexcept(ndebug)
{
  auto len = length( v );
  Assert( len > std::numeric_limits< tk::real >::epsilon(), "div by zero" );
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}

//! Compute the triple-product of three vectors
//! \param[in] v1x x coordinate of the 1st vector
//! \param[in] v1y y coordinate of the 1st vector
//! \param[in] v1z z coordinate of the 1st vector
//! \param[in] v2x x coordinate of the 2nd vector
//! \param[in] v2y y coordinate of the 2nd vector
//! \param[in] v2z z coordinate of the 2nd vector
//! \param[in] v3x x coordinate of the 3rd vector
//! \param[in] v3y y coordinate of the 3rd vector
//! \param[in] v3z z coordinate of the 3rd vector
//! \return Scalar value of the triple product
inline tk::real
triple( real v1x, real v1y, real v1z,
        real v2x, real v2y, real v2z,
        real v3x, real v3y, real v3z )
{
  real cx, cy, cz;
  cross( v2x, v2y, v2z, v3x, v3y, v3z, cx, cy, cz );
  return v1x*cx + v1y*cy + v1z*cz;
}

//! Compute the triple-product of three vectors
//! \param[in] v1 1st vector
//! \param[in] v2 2nd vector
//! \param[in] v3 3rd vector
//! \return Triple-product
inline real
triple( const std::array< real, 3 >& v1,
        const std::array< real, 3 >& v2,
        const std::array< real, 3 >& v3 )
{
  return dot( v1, cross(v2,v3) );
}

//! Rotate vector about X axis
//! \param[in] v Vector to rotate
//! \param[in] angle Angle to use to rotate with
//! \return Rotated vector
inline std::array< real, 3 >
rotateX( const std::array< real, 3 >& v, real angle )
{
  using std::cos;  using std::sin;

  std::array< std::array< real, 3 >, 3 >
    R{{ {{ 1.0,         0.0,          0.0 }},
        {{ 0.0,   cos(angle), -sin(angle) }},
        {{ 0.0,   sin(angle),  cos(angle) }} }};

  return {{ dot(R[0],v), dot(R[1],v), dot(R[2],v) }};
}

//! Rotate vector about Y axis
//! \param[in] v Vector to rotate
//! \param[in] angle Angle to use to rotate with
//! \return Rotated vector
inline std::array< real, 3 >
rotateY( const std::array< real, 3 >& v, real angle )
{
  using std::cos;  using std::sin;

  std::array< std::array< real, 3 >, 3 >
    R{{ {{ cos(angle),  0.0, sin(angle) }},
        {{ 0.0,         1.0,        0.0 }},
        {{ -sin(angle), 0.0, cos(angle) }} }};

  return {{ dot(R[0],v), dot(R[1],v), dot(R[2],v) }};
}

//! Rotate vector about Z axis
//! \param[in] v Vector to rotate
//! \param[in] angle Angle to use to rotate with
//! \return Rotated vector
inline std::array< real, 3 >
rotateZ( const std::array< real, 3 >& v, real angle )
{
  using std::cos;  using std::sin;

  std::array< std::array< real, 3 >, 3 >
    R{{ {{ cos(angle), -sin(angle), 0.0 }},
        {{ sin(angle),  cos(angle), 0.0 }},
        {{ 0.0,         0.0,        1.0 }} }};

  return {{ dot(R[0],v), dot(R[1],v), dot(R[2],v) }};
}

//! \brief Compute the determinant of the Jacobian of a coordinate
//!  transformation over a tetrahedron
//! \param[in] v1 (x,y,z) coordinates of 1st vertex of the tetrahedron
//! \param[in] v2 (x,y,z) coordinates of 2nd vertex of the tetrahedron
//! \param[in] v3 (x,y,z) coordinates of 3rd vertex of the tetrahedron
//! \param[in] v4 (x,y,z) coordinates of 4th vertex of the tetrahedron
//! \return Determinant of the Jacobian of transformation of physical
//!   tetrahedron to reference (xi, eta, zeta) space
inline real
Jacobian( const std::array< real, 3 >& v1,
          const std::array< real, 3 >& v2,
          const std::array< real, 3 >& v3,
          const std::array< real, 3 >& v4 )
{
  std::array< real, 3 > ba{{ v2[0]-v1[0], v2[1]-v1[1], v2[2]-v1[2] }},
                        ca{{ v3[0]-v1[0], v3[1]-v1[1], v3[2]-v1[2] }},
                        da{{ v4[0]-v1[0], v4[1]-v1[1], v4[2]-v1[2] }};
  return triple( ba, ca, da );
}

//! \brief Compute the inverse of the Jacobian of a coordinate transformation
//!   over a tetrahedron
//! \param[in] v1 (x,y,z) coordinates of 1st vertex of the tetrahedron
//! \param[in] v2 (x,y,z) coordinates of 2nd vertex of the tetrahedron
//! \param[in] v3 (x,y,z) coordinates of 3rd vertex of the tetrahedron
//! \param[in] v4 (x,y,z) coordinates of 4th vertex of the tetrahedron
//! \return Inverse of the Jacobian of transformation of physical
//!   tetrahedron to reference (xi, eta, zeta) space
inline std::array< std::array< real, 3 >, 3 >
inverseJacobian( const std::array< real, 3 >& v1,
                 const std::array< real, 3 >& v2,
                 const std::array< real, 3 >& v3,
                 const std::array< real, 3 >& v4 )
{
  std::array< std::array< real, 3 >, 3 > jacInv;

  auto detJ = Jacobian( v1, v2, v3, v4 );

  jacInv[0][0] =  (  (v3[1]-v1[1])*(v4[2]-v1[2])
                   - (v4[1]-v1[1])*(v3[2]-v1[2])) / detJ;
  jacInv[1][0] = -(  (v2[1]-v1[1])*(v4[2]-v1[2])
                   - (v4[1]-v1[1])*(v2[2]-v1[2])) / detJ;
  jacInv[2][0] =  (  (v2[1]-v1[1])*(v3[2]-v1[2])
                   - (v3[1]-v1[1])*(v2[2]-v1[2])) / detJ;

  jacInv[0][1] = -(  (v3[0]-v1[0])*(v4[2]-v1[2])
                   - (v4[0]-v1[0])*(v3[2]-v1[2])) / detJ;
  jacInv[1][1] =  (  (v2[0]-v1[0])*(v4[2]-v1[2])
                   - (v4[0]-v1[0])*(v2[2]-v1[2])) / detJ;
  jacInv[2][1] = -(  (v2[0]-v1[0])*(v3[2]-v1[2])
                   - (v3[0]-v1[0])*(v2[2]-v1[2])) / detJ;

  jacInv[0][2] =  (  (v3[0]-v1[0])*(v4[1]-v1[1])
                   - (v4[0]-v1[0])*(v3[1]-v1[1])) / detJ;
  jacInv[1][2] = -(  (v2[0]-v1[0])*(v4[1]-v1[1])
                   - (v4[0]-v1[0])*(v2[1]-v1[1])) / detJ;
  jacInv[2][2] =  (  (v2[0]-v1[0])*(v3[1]-v1[1])
                   - (v3[0]-v1[0])*(v2[1]-v1[1])) / detJ;

  return jacInv;
}

//! Compute the determinant of 3x3 matrix
//!  \param[in] a 3x3 matrix
//!  \return Determinant of the 3x3 matrix
inline tk::real
determinant( const std::array< std::array< tk::real, 3 >, 3 >& a )
{
  return ( a[0][0] * (a[1][1]*a[2][2]-a[1][2]*a[2][1])
         - a[0][1] * (a[1][0]*a[2][2]-a[1][2]*a[2][0])
         + a[0][2] * (a[1][0]*a[2][1]-a[1][1]*a[2][0]) );
}

//! Solve a 3x3 system of equations using Cramer's rule
//!  \param[in] a 3x3 lhs matrix
//!  \param[in] b 3x1 rhs matrix
//!  \return Array of solutions of the 3x3 system
inline std::array < tk::real, 3 >
cramer( const std::array< std::array< tk::real, 3 >, 3>& a,
        const std::array< tk::real, 3 >& b )
{
  auto de = determinant( a );

  auto nu(0.0);
  std::array < real, 3 > x;

  nu = determinant( {{{{b[0], a[0][1], a[0][2]}},
                      {{b[1], a[1][1], a[1][2]}},
                      {{b[2], a[2][1], a[2][2]}}}} );
  x[0] = nu/de;

  nu = determinant( {{{{a[0][0], b[0], a[0][2]}},
                      {{a[1][0], b[1], a[1][2]}},
                      {{a[2][0], b[2], a[2][2]}}}} );
  x[1] = nu/de;

  nu = determinant( {{{{a[0][0], a[0][1], b[0]}},
                      {{a[1][0], a[1][1], b[1]}},
                      {{a[2][0], a[2][1], b[2]}}}} );
  x[2] = nu/de;

  return x;
}

//! Move a point to a reference space given coordinates of origin of that space
//!  \param[in] origin Origin of reference frame to which point is to be moved
//!  \param[in,out] point Point that needs to be moved to reference frame
inline void
movePoint( const std::array< tk::real, 3 >& origin,
  std::array< tk::real, 3 >& point )
{
  for (std::size_t i=0; i<3; ++i)
    point[i] -= origin[i];
}

//! Rotate a point in 3D space by specifying rotation angles in degrees
//!  \param[in] angles Angles in 3D space by which point is to be rotated
//!  \param[in,out] point Point that needs to be rotated
inline void
rotatePoint( const std::array< tk::real, 3 >& angles,
  std::array< tk::real, 3 >& point )
{
  // Convert angles to radian
  tk::real pi = 4.0*std::atan(1.0);
  auto a = angles[0] * pi/180.0;
  auto b = angles[1] * pi/180.0;
  auto c = angles[2] * pi/180.0;

  // Rotation matrix
  std::array< std::array< tk::real, 3 >, 3 > rotMat;
  {
    using namespace std;
    rotMat[0][0] = cos(b)*cos(c);
    rotMat[0][1] = - cos(b)*sin(c);
    rotMat[0][2] = sin(b);

    rotMat[1][0] = sin(a)*sin(b)*cos(c) + cos(a)*sin(c);
    rotMat[1][1] = - sin(a)*sin(b)*sin(c) + cos(a)*cos(c);
    rotMat[1][2] = - sin(a)*cos(b);

    rotMat[2][0] = - cos(a)*sin(b)*cos(c) + sin(a)*sin(c);
    rotMat[2][1] = cos(a)*sin(b)*sin(c) + sin(a)*cos(c);
    rotMat[2][2] = cos(a)*cos(b);
  }

  // Apply rotation
  std::array< tk::real, 3 > x{{0.0, 0.0, 0.0}};
  for (std::size_t i=0; i<3; ++i) {
    for (std::size_t j=0; j<3; ++j) {
      x[i] += rotMat[i][j]*point[j];
    }
  }
  point = x;
}

} // tk::

#endif // Vector_h
