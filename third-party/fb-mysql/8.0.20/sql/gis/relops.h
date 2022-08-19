#ifndef SQL_GIS_RELOPS_H_INCLUDED
#define SQL_GIS_RELOPS_H_INCLUDED

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

/// @file
///
/// This file declares the interface of relational GIS operations. These are
/// boolean operations that compute relations between geometries.

#include "sql/dd/types/spatial_reference_system.h"  // dd::Spatial_reference_system
#include "sql/gis/geometries.h"

namespace gis {

/// Computes the crosses relation between two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] crosses Whether g1 crosses g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool crosses(const dd::Spatial_reference_system *srs, const Geometry *g1,
             const Geometry *g2, const char *func_name, bool *crosses,
             bool *null) noexcept;

/// Computes the disjoint relation between two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] disjoint Whether g1 is disjoint from g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool disjoint(const dd::Spatial_reference_system *srs, const Geometry *g1,
              const Geometry *g2, const char *func_name, bool *disjoint,
              bool *null) noexcept;

/// Computes the equals relation between two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] equals Whether g1 equals g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool equals(const dd::Spatial_reference_system *srs, const Geometry *g1,
            const Geometry *g2, const char *func_name, bool *equals,
            bool *null) noexcept;

/// Computes the intersects relation between two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] intersects Whether g1 intersects g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool intersects(const dd::Spatial_reference_system *srs, const Geometry *g1,
                const Geometry *g2, const char *func_name, bool *intersects,
                bool *null) noexcept;

/// Computes the covered by relation between the minimum bounding rectangles of
/// two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] covered_by Whether the MBR of g1 is covered by the MBR of g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool mbr_covered_by(const dd::Spatial_reference_system *srs, const Geometry *g1,
                    const Geometry *g2, const char *func_name, bool *covered_by,
                    bool *null) noexcept;

/// Computes the disjoint relation between the minimum bounding rectangles of
/// two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] disjoint Whether the MBR of g1 is disjoint from the MBR of g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool mbr_disjoint(const dd::Spatial_reference_system *srs, const Geometry *g1,
                  const Geometry *g2, const char *func_name, bool *disjoint,
                  bool *null) noexcept;

/// Computes the equals relation between the minimum bounding rectangles of
/// two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] equals Whether the MBR of g1 equals the MBR of g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool mbr_equals(const dd::Spatial_reference_system *srs, const Geometry *g1,
                const Geometry *g2, const char *func_name, bool *equals,
                bool *null) noexcept;

/// Computes the intersects relation between the minimum bounding rectangles of
/// two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] intersects Whether the MBR of g1 intersects the MBR of g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool mbr_intersects(const dd::Spatial_reference_system *srs, const Geometry *g1,
                    const Geometry *g2, const char *func_name, bool *intersects,
                    bool *null) noexcept;

/// Computes the overlaps relation between the minimum bounding rectangles of
/// two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] overlaps Whether the MBR of g1 overlaps the MBR of g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool mbr_overlaps(const dd::Spatial_reference_system *srs, const Geometry *g1,
                  const Geometry *g2, const char *func_name, bool *overlaps,
                  bool *null) noexcept;

/// Computes the touches relation between the minimum bounding rectangles of
/// two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] touches Whether the MBR of g1 touches the MBR of g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool mbr_touches(const dd::Spatial_reference_system *srs, const Geometry *g1,
                 const Geometry *g2, const char *func_name, bool *touches,
                 bool *null) noexcept;

/// Computes the within relation between the minimum bounding rectangles of
/// two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] within Whether the MBR of g1 is within the MBR of g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool mbr_within(const dd::Spatial_reference_system *srs, const Geometry *g1,
                const Geometry *g2, const char *func_name, bool *within,
                bool *null) noexcept;

/// Computes the overlaps relation between two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] overlaps Whether g1 overlaps g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool overlaps(const dd::Spatial_reference_system *srs, const Geometry *g1,
              const Geometry *g2, const char *func_name, bool *overlaps,
              bool *null) noexcept;

/// Computes the touches relation between two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] touches Whether g1 touches g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool touches(const dd::Spatial_reference_system *srs, const Geometry *g1,
             const Geometry *g2, const char *func_name, bool *touches,
             bool *null) noexcept;

/// Computes the within relation between two geometries.
///
/// Both geometries must be in the same coordinate system (Cartesian or
/// geographic), and the coordinate system of the geometries must match
/// the coordinate system of the SRID. It is the caller's responsibility
/// to guarantee this.
///
/// @param[in] srs The spatial reference system, common to both geometries.
/// @param[in] g1 First geometry.
/// @param[in] g2 Second geometry.
/// @param[in] func_name Function name used in error reporting.
/// @param[out] within Whether g1 lies within g2.
/// @param[out] null True if the return value is NULL.
///
/// @retval false Success.
/// @retval true An error has occurred. The error has been reported with
/// my_error().
bool within(const dd::Spatial_reference_system *srs, const Geometry *g1,
            const Geometry *g2, const char *func_name, bool *within,
            bool *null) noexcept;

}  // namespace gis

#endif  // SQL_GIS_RELOPS_H_INCLUDED
