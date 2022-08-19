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
/// Interface for determining the aproximate distance between two geometries by
/// assuming the world is a sphere.

#ifndef SQL_GIS_DISTANCE_SPHERE_H_INCLUDED
#define SQL_GIS_DISTANCE_SPHERE_H_INCLUDED

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"                     // gis::Geometry

namespace gis {

/// Compute the approximate distance between two geometries by assuming the
/// world is a sphere.
///
/// The coordinate system of the geometries must match the coordinate system of
/// the SRID. It is the caller's responsibility to guarantee this.
///
/// @param[in] srs The spatial reference system.
/// @param[in] g1 Geometry 1.
/// @param[in] g2 Geometry 2.
/// @param[in] func_name Function name used in error reporting.
/// @param[in] sphere_radius Radius of sphere.
/// @param[out] result Distance between g1 and g2. Invalid if `result_null`.
/// @param[out] result_null Whether return value is `NULL` instead of `result`.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool distance_sphere(const dd::Spatial_reference_system *srs,
                     const Geometry *g1, const Geometry *g2,
                     const char *func_name, double sphere_radius,
                     double *result, bool *result_null) noexcept;

}  // namespace gis

#endif  // include guard
