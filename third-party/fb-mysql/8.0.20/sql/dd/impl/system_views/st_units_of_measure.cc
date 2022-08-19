/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/dd/impl/system_views/st_units_of_measure.h"

#include <m_string.h>
#include <algorithm>
#include <iomanip>
#include <string>

#include "sql/gis/st_units_of_measure.h"
namespace dd {
namespace system_views {

/// Escapes (only) apostrophes
/// @return string with apostrophes escaped with backslash
static std::string escape(const std::string &str) {
  std::string res = "";
  for (auto letter : str) {
    if (letter == '\'') {
      res += "\\'";
    } else {
      res += letter;
    }
  }
  return res;
}

/// Returns a string represention of the Unit_type
/// As there is currently only one Unit_Type it will return "Linear".
/// @return "Linear" if unit type is linear
static std::string to_string(const gis::Unit_Type unit_type) {
  if (unit_type == gis::Unit_Type::kLinear) {
    return "LINEAR";
  } else {
    DBUG_ASSERT(false);
    return "";
  }
}

const St_units_of_measure &St_units_of_measure::instance() {
  static St_units_of_measure *m_instance = new St_units_of_measure();
  return *m_instance;
}

St_units_of_measure::St_units_of_measure() {
  Stringstream_type ss;
  ss << "JSON_TABLE('[";
  collation_unordered_map<std::string, gis::Unit> units = gis::units();
  char buffer[FLOATING_POINT_BUFFER];
  for (std::pair<const std::string, gis::Unit> &unit_conversion : units) {
    if (unit_conversion != *units.begin()) {
      ss << ",";
    }
    my_fcvt_compact(unit_conversion.second.conversion_factor, buffer, nullptr);
    ss << "[\"" << dd::system_views::escape(unit_conversion.first) << "\","
       << '\"' << dd::system_views::to_string(unit_conversion.second.unit_type)
       << "\"," << '\"' << unit_conversion.second.description << "\"," << buffer
       << "]";
  }

  ss << "]', '$[*]' COLUMNS(UNIT_NAME VARCHAR(255) CHARSET utf8mb4 PATH '$[0]'"
     << ", "
     << "UNIT_TYPE VARCHAR(7) CHARSET utf8mb4 PATH '$[1]'"
     << ", "
     << "DESCRIPTION VARCHAR(255) CHARSET utf8mb4 PATH '$[2]'"
     << ", "
     << "CONVERSION_FACTOR DOUBLE PRECISION PATH '$[3]'"
     << ")) AS ST_UNITS_OF_MEASURE";
  m_target_def.set_view_name(view_name());
  m_target_def.add_field(FIELD_UNIT_NAME, "UNIT_NAME", "UNIT_NAME");
  m_target_def.add_field(FIELD_UNIT_TYPE, "UNIT_TYPE", "UNIT_TYPE");
  m_target_def.add_field(FIELD_CONVERSION_FACTOR, "CONVERSION_FACTOR",
                         "CONVERSION_FACTOR");
  m_target_def.add_field(FIELD_DESCRIPTION, "DESCRIPTION", "DESCRIPTION");
  m_target_def.add_from(ss.str());
}
}  // namespace system_views
}  // namespace dd
