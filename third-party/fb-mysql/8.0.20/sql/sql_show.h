/* Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_SHOW_H
#define SQL_SHOW_H

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_inttypes.h"
#include "mysql/status_var.h"
#include "typelib.h"

/* Forward declarations */
class JOIN;
class QEP_TAB;
class SELECT_LEX;
class String;
class THD;
class sp_name;
struct CHARSET_INFO;
struct HA_CREATE_INFO;
struct LEX;
struct ST_SCHEMA_TABLE;
struct System_status_var;
struct TABLE;
struct TABLE_LIST;
typedef enum enum_mysql_show_type SHOW_TYPE;
enum enum_schema_table_state : int;
enum enum_schema_tables : int;
enum enum_var_type : int;
enum enum_field_types : int;

bool store_create_info(THD *thd, TABLE_LIST *table_list, String *packet,
                       HA_CREATE_INFO *create_info_arg, bool show_database);

void append_identifier(const THD *thd, String *packet, const char *name,
                       size_t length, const CHARSET_INFO *from_cs,
                       const CHARSET_INFO *to_cs);

void append_identifier(const THD *thd, String *packet, const char *name,
                       size_t length);

void mysqld_list_fields(THD *thd, TABLE_LIST *table, const char *wild);
bool mysqld_show_create(THD *thd, TABLE_LIST *table_list);
bool mysqld_show_create_db(THD *thd, char *dbname, HA_CREATE_INFO *create);

void mysqld_list_processes(THD *thd, const char *user, bool verbose);
bool mysqld_show_privileges(THD *thd);
void calc_sum_of_all_status(System_status_var *to);
void append_definer(const THD *thd, String *buffer,
                    const LEX_CSTRING &definer_user,
                    const LEX_CSTRING &definer_host);
bool add_status_vars(const SHOW_VAR *list);
void remove_status_vars(SHOW_VAR *list);
void init_status_vars();
void free_status_vars();
bool get_status_var(THD *thd, SHOW_VAR *list, const char *name,
                    char *const buff, enum_var_type var_type, size_t *length);
void reset_status_vars();
ulonglong get_status_vars_version(void);
bool show_create_trigger(THD *thd, const sp_name *trg_name);
void view_store_options(const THD *thd, TABLE_LIST *table, String *buff);

bool schema_table_store_record(THD *thd, TABLE *table);

/**
  Store record to I_S table, convert HEAP table to InnoDB table if necessary.

  @param[in]  thd            thread handler
  @param[in]  table          Information schema table to be updated
  @param[in]  make_ondisk    if true, convert heap table to on disk table.
                             default value is true.
  @return 0 on success
  @return error code on failure.
*/
int schema_table_store_record2(THD *thd, TABLE *table, bool make_ondisk);

/**
  Convert HEAP table to InnoDB table if necessary

  @param[in] thd     thread handler
  @param[in] table   Information schema table to be converted.
  @param[in] error   the error code returned previously.
  @return false on success, true on error.
*/
bool convert_heap_table_to_ondisk(THD *thd, TABLE *table, int error);
void initialize_information_schema_acl();
bool make_table_list(THD *thd, SELECT_LEX *sel, const LEX_CSTRING &db_name,
                     const LEX_CSTRING &table_name);

ST_SCHEMA_TABLE *find_schema_table(THD *thd, const char *table_name);
ST_SCHEMA_TABLE *get_schema_table(enum enum_schema_tables schema_table_idx);
bool make_schema_select(THD *thd, SELECT_LEX *sel,
                        enum enum_schema_tables schema_table_idx);
bool mysql_schema_table(THD *thd, LEX *lex, TABLE_LIST *table_list);
bool get_schema_tables_result(JOIN *join,
                              enum enum_schema_table_state executed_place);
enum enum_schema_tables get_schema_table_idx(ST_SCHEMA_TABLE *schema_table);

const char *get_one_variable(THD *thd, const SHOW_VAR *variable,
                             enum_var_type value_type, SHOW_TYPE show_type,
                             System_status_var *status_var,
                             const CHARSET_INFO **charset, char *buff,
                             size_t *length, bool *is_null = nullptr);

const char *get_one_variable_ext(THD *running_thd, THD *target_thd,
                                 const SHOW_VAR *variable,
                                 enum_var_type value_type, SHOW_TYPE show_type,
                                 System_status_var *status_var,
                                 const CHARSET_INFO **charset, char *buff,
                                 size_t *length, bool *is_null = nullptr);

/* These functions were under INNODB_COMPATIBILITY_HOOKS */
int get_quote_char_for_identifier(const THD *thd, const char *name,
                                  size_t length);

void show_sql_type(enum_field_types type, bool is_array, uint metadata,
                   String *str, const CHARSET_INFO *field_cs = nullptr);

bool do_fill_information_schema_table(THD *thd, TABLE_LIST *table_list,
                                      QEP_TAB *qep_tab);

extern TYPELIB grant_types;
#endif /* SQL_SHOW_H */
