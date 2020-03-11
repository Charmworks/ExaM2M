// *****************************************************************************
/*!
  \file      src/Main/ExaM2MDriver.hpp
  \copyright 2020 Charmworks, Inc.
             All rights reserved. See the LICENSE file for details.
  \brief     ExaM2M driver
  \details   ExaM2M driver.
*/
// *****************************************************************************
#ifndef ExaM2MDriver_h
#define ExaM2MDriver_h

#include <vector>
#include <string>

namespace exam2m {

//! ExaM2M driver
class ExaM2MDriver {

  public:
    //! Constructor
    explicit ExaM2MDriver( int argc, char** argv );

    //! Execute driver
    void execute() const;

  private:
    std::vector< std::string > m_argv;  //!< Command line arguments
};

} // exam2m::

#endif // ExaM2MDriver_h
