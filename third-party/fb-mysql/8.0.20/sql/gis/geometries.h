#ifndef SQL_GIS_GEOMETRIES_H_INCLUDED
#define SQL_GIS_GEOMETRIES_H_INCLUDED

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
/// This file declares the geometry class hierarchy used by the server as the
/// internal representation of geometries.
///
/// The hierarchy is closely modelled after the hierarchy in SFA-CA
/// (OGC 06-103r4), but since Boost.Geometry depends on type traits to
/// know if a geometry is in a Cartesian or geographic SRS, there are
/// Cartesian and geographic specializations of each instantiable type
/// in the SFA-CA. These are defined in geometries_cs.h.
///
/// Because of Boost.Geometry, iterators have to be of coordinate system
/// specific types. It is therefore not possible to have the standard begin()
/// and end() iterators in the common superclass. Instead, operator[] is
/// provided as a coordinate system agnostic option.
///
/// @see geometries_cs.h

#include <cmath>
#include <cstdint>

namespace gis {

/// Types of geometry objects. Not all are instantiable.
///
/// The values and the storage type are the same as used in WKB.
enum class Geometry_type : std::uint32_t {
  kGeometry = 0,
  kPoint = 1,
  kLinestring = 2,
  kPolygon = 3,
  kMultipoint = 4,
  kMultilinestring = 5,
  kMultipolygon = 6,
  kGeometrycollection = 7
  // kMulticurve = 11,
  // kMultisurface = 12,
  // kCurve = 13,
  // kSurface = 14,
};

/// Types of coordinate systems.
enum class Coordinate_system {
  /// A Cartesian plane with the same unit in both directions.
  kCartesian = 0,
  /// An ellipsoidal system with longitude and latitude coordinates,
  /// both in the same unit.
  kGeographic = 1
};

/// Direction of a ring.
enum class Ring_direction {
  /// Clockwise.
  kCW = 0,
  /// Counterclockwise.
  kCCW = 1,
  /// Unknown.
  ///
  /// Used only as an output value when the function is unable to determine the
  /// ring direction, e.g., if the ring contains only repetitions of the same
  /// point, or if there is a spike in the ring.
  kUnknown = 2
};

class Geometry_visitor;

/// Abstract superclass for all geometric objects.
///
/// Geometry is a non-instantiable type in SQL.
///
/// The hierarchy is first divided into abstract classes according to the SFA-CA
/// geometry type hierarchy, and then divided into concrete subclasses for each
/// coordinate system.
class Geometry {
 public:
  Geometry() = default;
  virtual ~Geometry() = default;
  Geometry(const Geometry &) = default;
  Geometry &operator=(const Geometry &) = default;

  /// Gets the geometry type of the object.
  ///
  /// @return The type of this object
  virtual Geometry_type type() const = 0;

  /// Gets the coordinate system.
  ///
  /// @return The coordiante system type.
  virtual Coordinate_system coordinate_system() const = 0;

  /// Applies a hierarchical visitor to this geometry.
  ///
  /// @see gis::Geometry_visitor
  ///
  /// @param v A hierarchical visitor.
  ///
  /// @retval true The execution was aborted by the visitor.
  /// @retval false The execution completed.
  virtual bool accept(Geometry_visitor *v) = 0;

  /// Check if this is an empty geometry.
  ///
  /// The definition of empty geometries is the one in SFA-CA (OGC 06-103r4,
  /// Sect. 6.1.2.2), i.e., an empty point set.
  ///
  /// @note This is different from the function "empty", which returns true if a
  /// geometry contains no subgeometries. E.g., a geometry collection may
  /// contain another geometry collection which is empty. In this case, the
  /// "empty" function would return false on the outermost geometry collection,
  /// while "is_empty" would return true.
  ///
  /// @retval true The geometry represents the empty point set.
  /// @retval false The geometry represent a non-empty point set.
  virtual bool is_empty() const = 0;
};

/// A 2d point.
///
/// Point is an instantiable type in SQL.
class Point : public Geometry {
 public:
  Point() : m_x(std::nan("")), m_y(std::nan("")) {}
  Point(double x, double y) : m_x(x), m_y(y) {}
  Geometry_type type() const override { return Geometry_type::kPoint; }
  bool accept(Geometry_visitor *v) override;
  bool is_empty() const override {
    return (std::isnan(m_x) || std::isnan(m_y));
  }

  /// Gets a coordinate value.
  ///
  /// This function signature must match the expectations of Boost.Geometry.
  ///
  /// @tparam K Coordinate number, zero indexed.
  /// @return The coordinate value.
  template <std::size_t K>
  double get() const;

  /// Gets the first coordinate value.
  ///
  /// For geographic points, the first coordinate is longitude.
  ///
  /// @return The first coordinate value.
  double x() const;

  /// Gets the second coordinate value.
  ///
  /// For geographic points, the second coordinate is latitude.
  ///
  /// @return The second coordinate value.
  double y() const;

  /// Sets a coordinate.
  ///
  /// This function signature must match the expectations of Boost.Geometry.
  ///
  /// @tparam K Coordinate number, zero indexed.
  /// @param d The coordinate value.
  template <std::size_t K>
  void set(double d);

  /// Sets the first coordinate value.
  ///
  /// For geographic points, the first coordinate is longitude.
  ///
  /// @param d The coordinate value.
  void x(double d);

  /// Sets the second coordinate value.
  ///
  /// For geographic points, the second coordinate is latitude.
  ///
  /// @param d The coordinate value.
  void y(double d);

 private:
  /// First coordinate (X or longitude).
  ///
  /// Geographic coordinates are in radians, positive to the East of
  /// Greenwich. Cartesian coordinates are in the SRSs length unit.
  double m_x;

  /// Second coordinate (Y or latitude).
  ///
  /// Geographic coordinates are in radians, positive to the North of the
  /// Equator. Cartesian coordinates are in the SRSs length unit.
  double m_y;
};

/// Compares two points.
///
/// The point with the lowest X coordinate is the smaller point. If X
/// coordinates are equal, the point with the lowest Y coordinate is the
/// smaller.
///
/// @param lhs Left hand side.
/// @param rhs Right hand side.
///
/// @retval true Left hand side sorts before right hand side.
/// @retval false Left hand side does not sort before right hand side.
inline bool operator<(const Point &lhs, const Point &rhs) {
  return (lhs.x() < rhs.x()) || (lhs.x() == rhs.x() && lhs.y() < rhs.y());
}

/// An abstract 2d curve.
///
/// Curve is a non-instantiable type in SQL.
class Curve : public Geometry {
 public:
  Geometry_type type() const override = 0;
  bool accept(Geometry_visitor *v) override = 0;
};

/// A string of connected line segments.
///
/// Linestring is an instantiable type in SQL.
///
/// According to the SFA-CA, linestrings have a linear interpolation between
/// points. In MySQL, a linestring represents the geodesic. On a plane, this is
/// linear interpolation, but on an ellipsoid, it's the shortest path along the
/// ellipsoid surface.
class Linestring : public Curve {
 public:
  Geometry_type type() const override { return Geometry_type::kLinestring; }
  bool accept(Geometry_visitor *v) override = 0;
  bool is_empty() const override { return empty(); }

  /// Adds a point to the end of the linestring.
  ///
  /// @param pt The point to add.
  virtual void push_back(const Point &pt) = 0;
  virtual void push_back(Point &&pt) = 0;

  /// Checks if the linestring is empty.
  ///
  /// Here, the definition of empty is that the linestring does not contain any
  /// points. An invalid linestring with only one coordinate is not empty.
  ///
  /// @retval true The linestring is empty.
  /// @retval false The linestring is not empty.
  virtual bool empty() const = 0;

  /// Returns the size of (number of points in) the linestring.
  ///
  /// @return Number of points in the linestring.
  virtual std::size_t size() const = 0;

  /// Removes all points from the linestring.
  virtual void clear() noexcept = 0;

  virtual Point &operator[](std::size_t i) = 0;
  virtual const Point &operator[](std::size_t i) const = 0;
};

/// A ring-shaped linestring.
///
/// Linearring is a non-instantiable type in SQL. It is used to represent
/// polygon rings.
///
/// The start and end point of the linestring must be the same (assumed, but not
/// enforced, by the implementation).
class Linearring : public Linestring {
 public:
  bool accept(Geometry_visitor *v) override = 0;
};

/// An abstract 2d surface.
///
/// Surface is a non-instantiable type in SQL.
class Surface : public Geometry {
 public:
  bool accept(Geometry_visitor *v) override = 0;

  Geometry_type type() const override = 0;
};

/// A polygon consisting of an outer ring and zero or more interior rings
/// defining holes in the polygon.
///
/// Polygon is an instantiable type in SQL.
///
/// The interior rings must be inside the exterior ring (not enforced by the
/// implementation).
class Polygon : public Surface {
 public:
  Geometry_type type() const override { return Geometry_type::kPolygon; }
  bool accept(Geometry_visitor *v) override = 0;
  bool is_empty() const override { return empty(); }

  /// Adds a linear ring to the polygon.
  ///
  /// The first ring will become the exterior ring. All following rings will be
  /// interior rings (holes).
  ///
  /// @param lr The linear ring to add.
  virtual void push_back(const Linearring &lr) = 0;
  virtual void push_back(Linearring &&lr) = 0;

  /// Checks if the polygon is empty.
  ///
  /// The polygon is considered empty if it has no rings.
  ///
  /// @retval true The polygon is empty.
  /// @retval false The polygon is not empty.
  virtual bool empty() const = 0;

  /// Returns the size of the polygon.
  ///
  /// @return Number of rings in the polygon (exterior + interior).
  virtual std::size_t size() const = 0;

  /// Returns the exterior ring of the polygon.
  ///
  /// @note If the polygon currently has no exterior ring, an empty one is
  /// added.
  ///
  /// @return The exterior ring.
  virtual Linearring &exterior_ring() = 0;

  /// Returns an interior ring of the polygon.
  ///
  /// @param n Ring number, zero indexed.
  ///
  /// @return The interior ring.
  virtual Linearring &interior_ring(std::size_t n) = 0;
};

/// A collection of geometries.
///
/// Geometrycollection is an instantiable type in SQL. It is the only
/// instantiable non-leaf geometry type.
///
/// The Geometrycollection class places no restrictions on type, overlapping,
/// etc. Subclasses do.
class Geometrycollection : public Geometry {
 public:
  Geometry_type type() const override {
    return Geometry_type::kGeometrycollection;
  }
  bool accept(Geometry_visitor *v) override = 0;

  /// Adds a geometry to the collection.
  ///
  /// @param g The geometry to add.
  virtual void push_back(const Geometry &g) = 0;
  virtual void push_back(Geometry &&g) = 0;

  /// Checks if the collection is empty.
  ///
  /// @retval true The polygon is empty.
  /// @retval false The polygon is not empty.
  virtual bool empty() const = 0;

  /// Returns the size of the geometrycollection.
  ///
  /// @return Number of geometries in the geometrycollection.
  virtual std::size_t size() const = 0;

  /// Resizes the geometrycollection to contain a given number of elements.
  ///
  /// If the new size is smaller than the current size, the remaining geometries
  /// are discarded.
  ///
  /// @param[in] count The new number of geometries.
  virtual void resize(std::size_t count) = 0;

  /// Removes all geometries from the geometrycollection.
  virtual void clear() noexcept = 0;

  virtual Geometry &operator[](std::size_t i) = 0;
  virtual const Geometry &operator[](std::size_t i) const = 0;
};

/// A collection of points.
///
/// Multipoint is an instantiable type in SQL.
class Multipoint : public Geometrycollection {
 public:
  Geometry_type type() const override { return Geometry_type::kMultipoint; }

  bool accept(Geometry_visitor *v) override = 0;
};

/// An abstract collection of curves.
///
/// Multicurve is a non-instantiable type in SQL.
class Multicurve : public Geometrycollection {
 public:
  Geometry_type type() const override = 0;
  bool accept(Geometry_visitor *v) override = 0;
};

/// A colletion of linestrings.
///
/// Multilinestring is an instantiable type in SQL.
class Multilinestring : public Multicurve {
 public:
  Geometry_type type() const override {
    return Geometry_type::kMultilinestring;
  }
  bool accept(Geometry_visitor *v) override = 0;
};

/// An abstract collection of surfaces.
///
/// Multisurface is a non-instantiable type in SQL.
class Multisurface : public Geometrycollection {
 public:
  Geometry_type type() const override = 0;
  bool accept(Geometry_visitor *v) override = 0;
};

/// A collection of polygons.
///
/// Multipolygon is an instantiable type in SQL.
class Multipolygon : public Multisurface {
 public:
  Geometry_type type() const override { return Geometry_type::kMultipolygon; }
  bool accept(Geometry_visitor *v) override = 0;
};

/// Get the type name string corresponding to a geometry type.
///
/// @param type The geometry type.
///
/// @return A string containing the type name (in uppercase).
const char *type_to_name(Geometry_type type);

}  // namespace gis

#endif  // SQL_GIS_GEOMETRIES_H_INCLUDED
