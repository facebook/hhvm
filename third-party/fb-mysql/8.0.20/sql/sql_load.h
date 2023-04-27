/* Copyright (c) 2006, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_LOAD_INCLUDED
#define SQL_LOAD_INCLUDED

#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "sql/sql_cmd.h"         /* Sql_cmd */
#include "sql/sql_data_change.h" /* enum_duplicates */
#include "sql/sql_exchange.h"    /* sql_exchange */
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql_string.h"

class Item;
class READ_INFO;
class THD;
struct TABLE_LIST;

class Sql_cmd_load_table final : public Sql_cmd {
 public:
  Sql_cmd_load_table(enum_filetype filetype, bool is_local_file,
                     const LEX_STRING &filename, On_duplicate on_duplicate,
                     Table_ident *table, bool opt_compressed,
                     List<String> *opt_partitions,
                     const CHARSET_INFO *opt_charset,
                     String *opt_xml_rows_identified_by,
                     const Field_separators &field_separators,
                     const Line_separators &line_separators, ulong skip_lines,
                     List<Item> *opt_fields_or_vars, List<Item> *opt_set_fields,
                     List<Item> *opt_set_exprs,
                     List<String> *opt_set_expr_strings)
      : m_exchange(filename.str, false, filetype),
        m_is_local_file(is_local_file),
        m_on_duplicate(on_duplicate),
        m_table(table),
        m_opt_partitions(opt_partitions),
        m_opt_set_expr_strings(opt_set_expr_strings) {
    if (opt_fields_or_vars) m_opt_fields_or_vars = *opt_fields_or_vars;
    DBUG_ASSERT((opt_set_fields == nullptr) ^ (opt_set_exprs != nullptr));
    if (opt_set_fields) {
      DBUG_ASSERT(opt_set_fields->elements == opt_set_exprs->elements);
      m_opt_set_fields = *opt_set_fields;
      m_opt_set_exprs = *opt_set_exprs;
    }

    m_exchange.load_compressed = opt_compressed;
    m_exchange.cs = opt_charset;

    if (opt_xml_rows_identified_by != nullptr)
      m_exchange.line.line_term = opt_xml_rows_identified_by;

    m_exchange.field.merge_field_separators(field_separators);
    m_exchange.line.merge_line_separators(line_separators);
    m_exchange.skip_lines = skip_lines;
  }

  enum_sql_command sql_command_code() const override { return SQLCOM_LOAD; }

  bool execute(THD *thd) override;

 public:
  sql_exchange m_exchange;
  const bool m_is_local_file;
  const On_duplicate m_on_duplicate;
  Table_ident *const m_table;
  List<String> *const m_opt_partitions;
  List<Item> m_opt_fields_or_vars;
  List<Item> m_opt_set_fields;
  List<Item> m_opt_set_exprs;

  /**
    A list of strings is maintained to store the SET clause command user strings
    which are specified in load data operation.  This list will be used
    during the reconstruction of "load data" statement at the time of writing
    to binary log.
  */
  List<String> *const m_opt_set_expr_strings;

 private:
  bool execute_inner(THD *thd, enum enum_duplicates handle_duplicates);

  bool read_fixed_length(THD *thd, COPY_INFO &info, TABLE_LIST *table_list,
                         READ_INFO &read_info, ulong skip_lines);

  bool read_sep_field(THD *thd, COPY_INFO &info, TABLE_LIST *table_list,
                      READ_INFO &read_info, const String &enclosed,
                      ulong skip_lines);

  bool read_xml_field(THD *thd, COPY_INFO &info, TABLE_LIST *table_list,
                      READ_INFO &read_info, ulong skip_lines);

  bool write_execute_load_query_log_event(THD *thd, const char *db,
                                          const char *table_name,
                                          bool is_concurrent,
                                          enum enum_duplicates duplicates,
                                          bool transactional_table,
                                          int errocode, bool compressed);
};

#endif /* SQL_LOAD_INCLUDED */
