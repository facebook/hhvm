#ifndef SQL_GIS_COVERED_BY_FUNCTOR_H_INCLUDED
#define SQL_GIS_COVERED_BY_FUNCTOR_H_INCLUDED

// Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

/// @file
///
/// This file declares the covered_by functor interface.
///
/// The functor is not intended for use directly by MySQL code.

#include <memory>  // std::unique_ptr

#include <boost/geometry.hpp>

#include "sql/gis/box.h"
#include "sql/gis/functor.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"

namespace gis {

/// Covered_by functor that calls Boost.Geometry with the correct parameter
/// types.
///
/// The functor throws exceptions and is therefore only intended used to
/// implement covered_by or other geographic functions. It should not be used
/// directly by other MySQL code.
class Covered_by : public Functor<bool> {
 private:
  /// Semi-major axis of ellipsoid.
  double m_semi_major;
  /// Semi-minor axis of ellipsoid.
  double m_semi_minor;

 public:
  /// Creates a new Covered_by functor.
  ///
  /// @param semi_major Semi-major axis of ellipsoid.
  /// @param semi_minor Semi-minor axis of ellipsoid.
  Covered_by(double semi_major, double semi_minor);
  double semi_major() const { return m_semi_major; }
  double semi_minor() const { return m_semi_minor; }
  bool operator()(const Geometry *g1, const Geometry *g2) const override;
  bool operator()(const Box *b1, const Box *b2) const;
  bool eval(const Geometry *g1, const Geometry *g2) const;

  //////////////////////////////////////////////////////////////////////////////

  // covered_by(Box, Box)

  bool eval(const Cartesian_box *b1, const Cartesian_box *b2) const;
  bool eval(const Geographic_box *b1, const Geographic_box *b2) const;
};

}  // namespace gis

#endif  // SQL_GIS_COVERED_BY_FUNCTOR_H_INCLUDED
