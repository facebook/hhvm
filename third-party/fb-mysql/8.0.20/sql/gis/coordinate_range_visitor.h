#ifndef SQL_GIS_COORDINATE_RANGE_VISITOR_H_INCLUDED
#define SQL_GIS_COORDINATE_RANGE_VISITOR_H_INCLUDED

// Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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

#include <cmath>  // M_PI, M_PI_2

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometry_visitor.h"

namespace gis {

/// A visitor that checks if coordinates are within range for a spatial
/// reference system.
///
/// If a coordinate value is found to be out of range, the visitor returns
/// true. Otherwise, it retruns false.
///
/// Checking stops on the first value found to be out of range. Cartesian
/// coordinates are always within range.
class Coordinate_range_visitor : public Nop_visitor {
 private:
  /// Spatial reference system of the geometry
  const dd::Spatial_reference_system *m_srs;
  /// Whether an out of range longitude value has been encountered
  bool m_detected_longitude;
  /// Whether an out of range latitude value has been encountered
  bool m_detected_latitude;
  /// The coordinate value that is out of range, in SRS unit
  double m_coordinate;

 public:
  /// Construct a new coordinate range visitor.
  ///
  /// @param srs The spatial reference system of the geometry.
  Coordinate_range_visitor(const dd::Spatial_reference_system *srs)
      : m_srs(srs),
        m_detected_longitude(false),
        m_detected_latitude(false),
        m_coordinate(0.0) {}

  /// Check if the visitor has detected any out of range longitude values.
  ///
  /// @retval true At least one out of range longitude value.
  /// @retval false All longitude values are within range.
  bool longitude_out_of_range() const { return m_detected_longitude; }

  /// Check if the visitor has detected any out of range latitude values.
  ///
  /// @retval true At least one out of range latitude value.
  /// @retval false All latitude values are within range.
  bool latitude_out_of_range() const { return m_detected_latitude; }

  /// Get the coordinate value that is out of range.
  ///
  /// @return The coordinate value in the SRS unit.
  double coordinate_value() const { return m_coordinate; }

  using Nop_visitor::visit_enter;
  bool visit_enter(Geometry *) override {
    if (m_srs == nullptr || m_srs->is_cartesian())
      return true;  // Don't descend into each child.

    return false;
  }

  using Nop_visitor::visit;
  bool visit(Point *pt) override {
    if (m_srs == nullptr || m_srs->is_cartesian())
      return false;  // Everything OK.

    double lon = pt->x() - m_srs->prime_meridian() * m_srs->angular_unit();
    if (!m_srs->positive_east()) lon *= -1.0;
    if (lon <= -M_PI || lon > M_PI) {
      m_detected_longitude = true;
      m_coordinate = m_srs->from_radians(lon);
      return true;  // Out of range coordinate detected.
    }

    double lat = pt->y();
    if (!m_srs->positive_north()) lat *= -1.0;
    if (lat < -M_PI_2 || lat > M_PI_2) {
      m_detected_latitude = true;
      m_coordinate = m_srs->from_radians(lat);
      return true;  // Out of range coordinate detected.
    }

    return false;  // Everything OK.
  }
};

}  // namespace gis

#endif  // SQL_GIS_COORDINATE_RANGE_VISITOR_H_INCLUDED
