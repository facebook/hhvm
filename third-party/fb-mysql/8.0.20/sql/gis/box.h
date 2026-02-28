#ifndef SQL_GIS_BOX_H_INCLUDED
#define SQL_GIS_BOX_H_INCLUDED

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
/// This file declares the Box class.
///
/// @see box_cs.h

#include "sql/gis/geometries.h"
#include "sql/gis/geometries_cs.h"

namespace gis {

/// A 2d box with sides parallel to the coordinate system grid.
///
/// Used by computations on minimum bounding boxes (MBRs).
class Box {
 public:
  Box() = default;
  Box(const Box &) = default;
  Box(Box &&) = default;
  Box &operator=(const Box &) = default;
  Box &operator=(Box &&) = default;

  virtual ~Box() {}

  /// Gets the coordinate system.
  ///
  /// @return The coordiante system type.
  virtual Coordinate_system coordinate_system() const = 0;

  /// Returns the minimum corner.
  ///
  /// @return The minimum corner.
  virtual Point const &min_corner() const = 0;
  virtual Point &min_corner() = 0;

  /// Returns the maximum corner.
  ///
  /// @return The maximum corner.
  virtual Point const &max_corner() const = 0;
  virtual Point &max_corner() = 0;
};

/// A Cartesian 2d box.
class Cartesian_box : public Box {
 private:
  /// The corner with minimum X and Y values.
  Cartesian_point m_min_corner;
  /// The corner with maximum X and Y values.
  Cartesian_point m_max_corner;

 public:
  Cartesian_box() = default;
  Cartesian_box(Cartesian_point &&min_corner, Cartesian_point &&max_corner)
      : m_min_corner(min_corner), m_max_corner(max_corner) {}
  Coordinate_system coordinate_system() const override {
    return Coordinate_system::kCartesian;
  }

  /// Returns the minimum corner.
  ///
  /// @return The minimum corner.
  Cartesian_point const &min_corner() const override { return m_min_corner; }
  Cartesian_point &min_corner() override { return m_min_corner; }

  /// Returns the maximum corner.
  ///
  /// @return The maximum corner.
  Cartesian_point const &max_corner() const override { return m_max_corner; }
  Cartesian_point &max_corner() override { return m_max_corner; }
};

/// A Geographic 2d box.
///
/// Unlike polygons, the sides of the box are not the shortest distance between
/// the endpoints. A box side will follow the latitude line, while a linestring
/// (or polygon segment) between the same points won't.
class Geographic_box : public Box {
 private:
  /// The corner with minimum X and Y values.
  Geographic_point m_min_corner;
  /// The corner with maximum X and Y values.
  Geographic_point m_max_corner;

 public:
  Geographic_box() = default;
  Geographic_box(Geographic_point &&min_corner, Geographic_point &&max_corner)
      : m_min_corner(min_corner), m_max_corner(max_corner) {}
  Coordinate_system coordinate_system() const override {
    return Coordinate_system::kGeographic;
  }

  /// Returns the minimum corner.
  ///
  /// @return The minimum corner.
  Geographic_point const &min_corner() const override { return m_min_corner; }
  Geographic_point &min_corner() override { return m_min_corner; }

  /// Returns the maximum corner.
  ///
  /// @return The maximum corner.
  Geographic_point const &max_corner() const override { return m_max_corner; }
  Geographic_point &max_corner() override { return m_max_corner; }
};

}  // namespace gis

#endif  // SQL_GIS_BOX_H_INCLUDED
