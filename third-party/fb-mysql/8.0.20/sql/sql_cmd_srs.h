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
/// This file declares the interface of classes Sql_cmd_create_srs and
/// Sql_cmd_drop_srs, which handles the CREATE/DROP SPATIAL REFERENCE
/// SYSTEM statements, respectively.

#ifndef SQL_SQL_CMD_SRS_H_INCLUDED
#define SQL_SQL_CMD_SRS_H_INCLUDED

#include "my_sqlcommand.h"           // SQLCOM_CREATE_SRS, SQLCOM_DROP_SRS
#include "mysql/mysql_lex_string.h"  // MYSQL_LEX_STRING
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/gis/srid.h"  // gis::srid_t
#include "sql/sql_cmd.h"

class THD;

struct Sql_cmd_srs_attributes {
  MYSQL_LEX_STRING srs_name;
  MYSQL_LEX_STRING definition;
  MYSQL_LEX_STRING organization;
  unsigned long long organization_coordsys_id;
  MYSQL_LEX_STRING description;

  Sql_cmd_srs_attributes()
      : srs_name({nullptr, 0}),
        definition({nullptr, 0}),
        organization({nullptr, 0}),
        organization_coordsys_id(0),
        description({nullptr, 0}) {}
};

class Sql_cmd_create_srs final : public Sql_cmd {
 public:
  Sql_cmd_create_srs() {}
  void init(bool or_replace, bool if_not_exists, gis::srid_t srid,
            MYSQL_LEX_STRING srs_name, MYSQL_LEX_STRING definition,
            MYSQL_LEX_STRING organization, gis::srid_t organization_coordsys_id,
            MYSQL_LEX_STRING description) {
    m_or_replace = or_replace;
    m_if_not_exists = if_not_exists;
    m_srid = srid;
    m_srs_name = srs_name;
    m_definition = definition;
    m_organization = organization;
    m_organization_coordsys_id = organization_coordsys_id;
    m_description = description;
  }
  enum_sql_command sql_command_code() const override {
    return SQLCOM_CREATE_SRS;
  }
  bool execute(THD *thd) override;

  /// Fill an SRS with information from this CREATE statement (except the ID).
  ///
  /// @param[in,out] srs The SRS.
  ///
  /// @retval false Success.
  /// @retval true An error occurred (i.e., invalid SRS definition). The error
  /// has been reported with my_error.
  bool fill_srs(dd::Spatial_reference_system *srs);

 private:
  /// Whether OR REPLACE was specified.
  bool m_or_replace = false;
  /// Whether IF NOT EXISTS was specified
  bool m_if_not_exists = false;
  /// The SRID of the new SRS.
  gis::srid_t m_srid = 0;
  /// The name of the new SRS.
  ///
  /// The value is always a valid name (verified by PT_create_srs), but it may
  /// be a duplicate of an existing one.
  MYSQL_LEX_STRING m_srs_name;
  /// The definition of the new SRS.
  ///
  /// The definition is not parsed and validated until the SRS is created.
  MYSQL_LEX_STRING m_definition;
  /// Organization that is the source of the SRS definition.
  MYSQL_LEX_STRING m_organization;
  /// Source organization's SRS ID.
  gis::srid_t m_organization_coordsys_id = 0;
  /// Description of the new SRS.
  MYSQL_LEX_STRING m_description;
};

class Sql_cmd_drop_srs final : public Sql_cmd {
 public:
  Sql_cmd_drop_srs(gis::srid_t srid, bool if_exists)
      : m_srid(srid), m_if_exists(if_exists) {}
  enum_sql_command sql_command_code() const override { return SQLCOM_DROP_SRS; }
  bool execute(THD *thd) override;

 private:
  /// SRID of the SRS to drop.
  gis::srid_t m_srid;
  /// Whether IF EXISTS was specified.
  bool m_if_exists;
};

#endif  // SQL_SQL_CMD_SRS_H_INCLUDED
