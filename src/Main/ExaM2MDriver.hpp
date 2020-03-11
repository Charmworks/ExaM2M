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

namespace exam2m {

//! ExaM2M driver
class ExaM2MDriver {

  public:
    //! Constructor
    explicit ExaM2MDriver();

    //! Execute driver
    void execute() const;
};

} // exam2m::

#endif // ExaM2MDriver_h
