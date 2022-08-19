#ifndef SQL_GIS_ST_UNITS_OF_MEASURE_H_INCLUDED
#define SQL_GIS_ST_UNITS_OF_MEASURE_H_INCLUDED

/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA. */

#include <string.h>

#include "map_helpers.h"

namespace gis {
enum class Unit_Type { kLinear };

struct Unit {
  Unit_Type unit_type;
  double conversion_factor;
  std::string description;

  Unit() {}

  Unit(const Unit_Type unit_type, const double conversion_factor)
      : unit_type(unit_type), conversion_factor(conversion_factor) {}
  bool operator==(const gis::Unit &rhs) const {
    return unit_type == rhs.unit_type &&
           conversion_factor == rhs.conversion_factor &&
           description == rhs.description;
  }
};

/// A function to obtaint the supported units for the gis module.
///
/// @return Map of supported units for ST_DISTANCE
collation_unordered_map<std::string, Unit> units();

/// Retrieves the length of the unit in meters.
/// @param unit the name of the unit we want the conversion factor for.
/// @param[out] conversion_factor A pointer to where the result should be put,
/// not touched in case of error.
/// @retval True if unit is not found.
/// @retval False in case of success.
bool get_conversion_factor(const std::string &unit, double *conversion_factor);

}  // namespace gis

#endif  // SQL_GIS_ST_UNITS_OF_MEASURE_H_INCLUDED
