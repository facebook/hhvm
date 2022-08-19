#ifndef SQL_GIS_TRANSFORM_FUNCTOR_H_INCLUDED
#define SQL_GIS_TRANSFORM_FUNCTOR_H_INCLUDED

// Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
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
/// This file declares the transform functor interface.
///
/// The functor is not intended for use directly by MySQL code. It should be
/// used indirectly through the gis::transform() function.
///
/// @see gis::transform

#include <memory>  // std::unique_ptr
#include <string>

#include <boost/geometry.hpp>
#include <boost/geometry/srs/transformation.hpp>

#include "sql/gis/functor.h"
#include "sql/gis/geometries.h"
#include "sql/gis/geometries_traits.h"

namespace gis {

/// Transform functor that calls Boost.Geometry with the correct parameter
/// types.
///
/// The functor throws exceptions and is therefore only intended used to
/// implement transform or other geographic functions. It should not be used
/// directly by other MySQL code.
class Transform : public Unary_functor<std::unique_ptr<Geometry>> {
 private:
  /// The transformation object that holds information about input and output
  /// definitions.
  boost::geometry::srs::transformation<> m_transformation;
  /// Coordinate system of the output SRS.
  Coordinate_system m_output_cs;

 public:
  /// Create a new transform functor.
  ///
  /// Theoretically, we could deduce the coordinate system of the output SRS
  /// from the old_srs_param proj4 string, but it's easier to get it from the DD
  /// SRS object on the caller side.
  ///
  /// @param[in] old_srs_params The proj4 parameters of the input SRS.
  /// @param[in] new_srs_params The proj4 parameters of the output SRS.
  /// @param[in] output_cs The coordinate system of the output SRS.
  Transform(const std::string &old_srs_params,
            const std::string &new_srs_params, Coordinate_system output_cs);

  std::unique_ptr<Geometry> operator()(const Geometry &g) const override;
  std::unique_ptr<Geometry> eval(const Geometry &g) const;
  std::unique_ptr<Geometry> eval(const Geographic_point &g) const;
  std::unique_ptr<Geometry> eval(const Geographic_linestring &g) const;
  std::unique_ptr<Geometry> eval(const Geographic_polygon &g) const;
  std::unique_ptr<Geometry> eval(const Geographic_geometrycollection &g) const;
  std::unique_ptr<Geometry> eval(const Geographic_multipoint &g) const;
  std::unique_ptr<Geometry> eval(const Geographic_multilinestring &g) const;
  std::unique_ptr<Geometry> eval(const Geographic_multipolygon &g) const;
};

}  // namespace gis

#endif  // SQL_GIS_TRANSFORM_FUNCTOR_H_INCLUDED
