/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_IMPORT_INCLUDED
#define SQL_IMPORT_INCLUDED

#include "lex_string.h"
#include "my_sqlcommand.h"
#include "sql/mem_root_array.h"
#include "sql/sql_cmd.h"  // Sql_cmd

class THD;

/**
  @file sql/sql_import.h Declaration of command class for the IMPORT TABLES
  command.
 */

/**
  Command class for the IMPORT command.
 */
class Sql_cmd_import_table : public Sql_cmd {
  typedef Mem_root_array_YY<LEX_STRING> Sdi_patterns_type;
  const Sdi_patterns_type m_sdi_patterns;

 public:
  /**
    Called by sql_yacc.yy.

    @param patterns - Mem_root_array_YY of all the sdi file patterns
    provided as arguments.
   */
  Sql_cmd_import_table(const Sdi_patterns_type &patterns);

  /**
    Import tables from SDI files or patterns provided to constructor.
    @param thd - thread handle
    @retval true on error
    @retval false otherwise
 */
  virtual bool execute(THD *thd);

  /**
    Provide access to the command code enum value.
    @return command code enum value
   */
  virtual enum_sql_command sql_command_code() const;
};
#endif /* !SQL_IMPORT_INCLUDED */
