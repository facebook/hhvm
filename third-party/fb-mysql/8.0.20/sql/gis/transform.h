#ifndef SQL_GIS_TRANSFORM_H_INCLUDED
#define SQL_GIS_TRANSFORM_H_INCLUDED

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
/// This file declares the interface of gis::transform, a function that converts
/// geometries from one spatial reference system to another.

#include <memory>  // std::unique_ptr

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"

namespace gis {

/// Transforms a geometry from one SRS to another.
///
/// @param[in] source_srs The SRS of the input geometry.
/// @param[in] in The geometry to transform.
/// @param[in] target_srs The SRS of the output geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] out The output geometry.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool transform(const dd::Spatial_reference_system *source_srs,
               const Geometry &in,
               const dd::Spatial_reference_system *target_srs,
               const char *func_name, std::unique_ptr<Geometry> *out) noexcept;

}  // namespace gis

#endif  // SQL_GIS_TRANSFORM_H_INCLUDED
