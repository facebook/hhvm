#ifndef SQL_GIS_SIMPLIFY_H_INCLUDED
#define SQL_GIS_SIMPLIFY_H_INCLUDED

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
/// This file declares the interface to simplify a geometry.

#include <memory>  // std::unique_ptr

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"

namespace gis {

/// Simplifies a geometry using the Douglas-Peucker algorithm.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g The geometry.
/// @param[in] max_distance Cut-off distance (in coordinate units) of the
/// Douglas-Peucker algorithm.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] result The simplified geometry.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool simplify(const dd::Spatial_reference_system *srs, const Geometry &g,
              double max_distance, const char *func_name,
              std::unique_ptr<Geometry> *result) noexcept;

}  // namespace gis

#endif  // SQL_GIS_SIMPLIFY_H_INCLUDED
