#ifndef SQL_GIS_GC_UTILS_H_INCLUDED
#define SQL_GIS_GC_UTILS_H_INCLUDED

// Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
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
/// This file declares the interface of various utility functions for
/// geometrycollections. The functions may throw exceptions.

#include <memory>  // std::unique_ptr

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/difference_functor.h"
#include "sql/gis/geometries.h"
#include "sql/gis/union_functor.h"

namespace gis {

/// Invalid geometry exception.
///
/// Thrown when it is discovered that at an input parameter contains an invalid
/// geometry.
class invalid_geometry_exception : public std::exception {};

/// Too large polygon exception.
///
/// Thrown when it is discovered that at a polygon covers half the globe or
/// more. Boost Geometry doesn't handle such polygons.
class too_large_polygon_exception : public std::exception {};

/// Splits a geometrycollection into points, linestrings and polygons.
///
/// All output collections may contain overlapping geometries and
/// duplicates. This is not a problem for the multipoint and multilinestring
/// outputs, but the multipolygon may be geometrically invalid.
///
/// @param[in] gc Geometry collection.
/// @param[out] mpt All points in the geometrycollection.
/// @param[out] mls All linestrings in the geometrycollection.
/// @param[out] mpy All polygons in the geometrycollection.
void split_gc(const Geometrycollection *gc, std::unique_ptr<Multipoint> *mpt,
              std::unique_ptr<Multilinestring> *mls,
              std::unique_ptr<Multipolygon> *mpy);

/// Cleans up overlapping geometries so that the geometrycollection is broken
/// down into non-overlapping collections.
///
/// All input collections may contain overlapping geometries and
/// duplicates, including the multipolygon. This function requires that at least
/// one of the inputs must be non-empty.
///
/// May throw exceptions from BG operations.
///
/// @param[in] semi_major Semi-major axis of ellipsoid.
/// @param[in] semi_minor Semi-minor axis of ellipsoid.
/// @param[in,out] mpt All points in the geometrycollection.
/// @param[in,out] mls All linestrings in the geometrycollection.
/// @param[in,out] mpy All polygons in the geometrycollection.
void gc_union(double semi_major, double semi_minor,
              std::unique_ptr<Multipoint> *mpt,
              std::unique_ptr<Multilinestring> *mls,
              std::unique_ptr<Multipolygon> *mpy);

}  // namespace gis

#endif  // SQL_GIS_GC_UTILS_H_INCLUDED
