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

#include "sql/gis/st_units_of_measure.h"
#include <m_ctype.h>
#include "mysqld_error.h"
namespace gis {

collation_unordered_map<std::string, Unit> units() {
  collation_unordered_map<std::string, Unit> units(
      &my_charset_utf8mb4_0900_ai_ci, PSI_INSTRUMENT_ME);

  units[std::string("millimetre")] = Unit(Unit_Type::kLinear, 0.001);
  units[std::string("centimetre")] = Unit(Unit_Type::kLinear, 0.01);
  units[std::string("metre")] = Unit(Unit_Type::kLinear, 1);
  units[std::string("foot")] = Unit(Unit_Type::kLinear, 0.3048);
  units[std::string("US survey foot")] =
      Unit(Unit_Type::kLinear, 0.30480060960121924);
  units[std::string("Clarke's foot")] = Unit(Unit_Type::kLinear, 0.3047972654);
  units[std::string("fathom")] = Unit(Unit_Type::kLinear, 1.8288);
  units[std::string("nautical mile")] = Unit(Unit_Type::kLinear, 1852);
  units[std::string("German legal metre")] =
      Unit(Unit_Type::kLinear, 1.0000135965);
  units[std::string("US survey chain")] =
      Unit(Unit_Type::kLinear, 20.11684023368047);
  units[std::string("US survey link")] =
      Unit(Unit_Type::kLinear, 0.2011684023368047);
  units[std::string("US survey mile")] =
      Unit(Unit_Type::kLinear, 1609.3472186944375);
  units[std::string("kilometre")] = Unit(Unit_Type::kLinear, 1000);
  units[std::string("Clarke's yard")] = Unit(Unit_Type::kLinear, 0.9143917962);
  units[std::string("Clarke's chain")] =
      Unit(Unit_Type::kLinear, 20.1166195164);
  units[std::string("Clarke's link")] =
      Unit(Unit_Type::kLinear, 0.201166195164);
  units[std::string("British yard (Sears 1922)")] =
      Unit(Unit_Type::kLinear, 0.9143984146160287);
  units[std::string("British foot (Sears 1922)")] =
      Unit(Unit_Type::kLinear, 0.3047994715386762);
  units[std::string("British chain (Sears 1922)")] =
      Unit(Unit_Type::kLinear, 20.116765121552632);
  units[std::string("British link (Sears 1922)")] =
      Unit(Unit_Type::kLinear, 0.2011676512155263);
  units[std::string("British yard (Benoit 1895 A)")] =
      Unit(Unit_Type::kLinear, 0.9143992);
  units[std::string("British foot (Benoit 1895 A)")] =
      Unit(Unit_Type::kLinear, 0.3047997333333333);
  units[std::string("British chain (Benoit 1895 A)")] =
      Unit(Unit_Type::kLinear, 20.1167824);
  units[std::string("British link (Benoit 1895 A)")] =
      Unit(Unit_Type::kLinear, 0.201167824);
  units[std::string("British yard (Benoit 1895 B)")] =
      Unit(Unit_Type::kLinear, 0.9143992042898124);
  units[std::string("British foot (Benoit 1895 B)")] =
      Unit(Unit_Type::kLinear, 0.30479973476327077);
  units[std::string("British chain (Benoit 1895 B)")] =
      Unit(Unit_Type::kLinear, 20.116782494375872);
  units[std::string("British link (Benoit 1895 B)")] =
      Unit(Unit_Type::kLinear, 0.2011678249437587);
  units[std::string("British foot (1865)")] =
      Unit(Unit_Type::kLinear, 0.30480083333333335);
  units[std::string("Indian foot")] =
      Unit(Unit_Type::kLinear, 0.30479951024814694);
  units[std::string("Indian foot (1937)")] =
      Unit(Unit_Type::kLinear, 0.30479841);
  units[std::string("Indian foot (1962)")] =
      Unit(Unit_Type::kLinear, 0.3047996);
  units[std::string("Indian foot (1975)")] =
      Unit(Unit_Type::kLinear, 0.3047995);
  units[std::string("Indian yard")] =
      Unit(Unit_Type::kLinear, 0.9143985307444408);
  units[std::string("Indian yard (1937)")] =
      Unit(Unit_Type::kLinear, 0.91439523);
  units[std::string("Indian yard (1962)")] =
      Unit(Unit_Type::kLinear, 0.9143988);
  units[std::string("Indian yard (1975)")] =
      Unit(Unit_Type::kLinear, 0.9143985);
  units[std::string("Statute mile")] = Unit(Unit_Type::kLinear, 1609.344);
  units[std::string("Gold Coast foot")] =
      Unit(Unit_Type::kLinear, 0.3047997101815088);
  units[std::string("British foot (1936)")] =
      Unit(Unit_Type::kLinear, 0.3048007491);
  units[std::string("yard")] = Unit(Unit_Type::kLinear, 0.9144);
  units[std::string("chain")] = Unit(Unit_Type::kLinear, 20.1168);
  units[std::string("link")] = Unit(Unit_Type::kLinear, 0.201168);
  units[std::string("British yard (Sears 1922 truncated)")] =
      Unit(Unit_Type::kLinear, 0.914398);
  units[std::string("British foot (Sears 1922 truncated)")] =
      Unit(Unit_Type::kLinear, 0.30479933333333337);
  units[std::string("British chain (Sears 1922 truncated)")] =
      Unit(Unit_Type::kLinear, 20.116756);
  units[std::string("British link (Sears 1922 truncated)")] =
      Unit(Unit_Type::kLinear, 0.20116756);

  return units;
}

bool get_conversion_factor(const std::string &unit, double *conversion_factor) {
  DBUG_ASSERT(conversion_factor);
  collation_unordered_map<std::string, Unit> unit_table = units();
  collation_unordered_map<std::string, Unit>::iterator unit_it =
      unit_table.find(unit);
  if (unit_it == unit_table.end()) {
    my_error(ER_UNIT_NOT_FOUND, myf(0), unit.c_str());
    return true;
  }
  *conversion_factor = unit_it->second.conversion_factor;
  return false;
}

}  // namespace gis
