#ifndef SQL_GIS_IS_VALID_H_INCLUDED
#define SQL_GIS_IS_VALID_H_INCLUDED

// Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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
/// This file declares the interface to calculate if a geometry is valid

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"

namespace gis {

/// Decides if a geometry is valid.
///
/// The SRS must match the SRS referenced by the geometry, this is the caller's
/// responsibility.
///
/// @param[in] srs The spatial reference system.
/// @param[in] g The geometry
/// @param[in] func_name Function name used in error reporting.
/// @param[out] is_valid The validity of the geometry
///
/// @retval false No error occurred
/// @retval true An error has occurred, the error has been reported with
/// my_error().
bool is_valid(const dd::Spatial_reference_system *srs, const Geometry *g,
              const char *func_name, bool *is_valid) noexcept;

}  // namespace gis

#endif  // SQL_GIS_IS_VALID_H_INCLUDED
