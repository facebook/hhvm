/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_PARSE_INCLUDED
#define SQL_PARSE_INCLUDED

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_command.h"
#include "my_sqlcommand.h"
#include "mysql_com.h"    // enum_server_command
#include "sql/handler.h"  // enum_schema_tables

struct mysql_rwlock_t;
template <typename T>
class SQL_I_List;

/**
  @addtogroup GROUP_PARSER
  @{
*/

class Comp_creator;
class Item;
class Object_creation_ctx;
class Parser_state;
class THD;
class Table_ident;
struct LEX;
struct LEX_USER;
struct ORDER;
struct Parse_context;
struct TABLE_LIST;
union COM_DATA;

const std::unordered_map<int, std::string> slow_log_ddls = {
    {SQLCOM_CREATE_TABLE, "CREATE_TABLE"},
    {SQLCOM_ALTER_TABLE, "ALTER_TABLE"},
    {SQLCOM_DROP_TABLE, "DROP_TABLE"},
    {SQLCOM_CREATE_INDEX, "CREATE_INDEX"},
    {SQLCOM_DROP_INDEX, "DROP_INDEX"},
    {SQLCOM_RENAME_TABLE, "RENAME_TABLE"},
    {SQLCOM_TRUNCATE, "TRUNCATE_TABLE"}};

extern "C" int test_if_data_home_dir(const char *dir);

bool stmt_causes_implicit_commit(const THD *thd, uint mask);

#ifndef DBUG_OFF
extern void turn_parser_debug_on();
#endif

bool parse_sql(THD *thd, Parser_state *parser_state,
               Object_creation_ctx *creation_ctx);

void free_items(Item *item);
void cleanup_items(Item *item);

Comp_creator *comp_eq_creator(bool invert);
Comp_creator *comp_equal_creator(bool invert);
Comp_creator *comp_ge_creator(bool invert);
Comp_creator *comp_gt_creator(bool invert);
Comp_creator *comp_le_creator(bool invert);
Comp_creator *comp_lt_creator(bool invert);
Comp_creator *comp_ne_creator(bool invert);

int prepare_schema_table(THD *thd, LEX *lex, Table_ident *table_ident,
                         enum enum_schema_tables schema_table_idx);
void get_default_definer(THD *thd, LEX_USER *definer);
LEX_USER *create_default_definer(THD *thd);
LEX_USER *get_current_user(THD *thd, LEX_USER *user);
bool check_string_char_length(const LEX_CSTRING &str, const char *err_msg,
                              size_t max_char_length, const CHARSET_INFO *cs,
                              bool no_error);
bool merge_charset_and_collation(const CHARSET_INFO *charset,
                                 const CHARSET_INFO *collation,
                                 const CHARSET_INFO **to);
bool merge_sp_var_charset_and_collation(const CHARSET_INFO *charset,
                                        const CHARSET_INFO *collation,
                                        const CHARSET_INFO **to);
bool check_host_name(const LEX_CSTRING &str);
bool mysql_test_parse_for_slave(THD *thd);
bool is_update_query(enum enum_sql_command command);
bool is_acl_ddl_query(enum enum_sql_command command);
bool support_high_priority(enum enum_sql_command command) noexcept;
bool is_explainable_query(enum enum_sql_command command);
bool is_log_table_write_query(enum enum_sql_command command);
bool alloc_query(THD *thd, const char *packet, size_t packet_length);
void mysql_parse(THD *thd, Parser_state *parser_state,
                 ulonglong *last_timer = nullptr);
void mysql_reset_thd_for_next_command(THD *thd);
bool create_select_for_variable(Parse_context *pc, const char *var_name);
void create_table_set_open_action_and_adjust_tables(LEX *lex);
int mysql_execute_command(THD *thd, bool first_level = false,
                          ulonglong *last_timer = nullptr);
bool do_command(THD *thd);
bool dispatch_command(THD *thd, const COM_DATA *com_data,
                      enum enum_server_command command);
bool prepare_index_and_data_dir_path(THD *thd, const char **data_file_name,
                                     const char **index_file_name,
                                     const char *table_name);
int append_file_to_dir(THD *thd, const char **filename_ptr,
                       const char *table_name);
void execute_init_command(THD *thd, LEX_STRING *init_command,
                          mysql_rwlock_t *var_lock);
void add_to_list(SQL_I_List<ORDER> &list, ORDER *order);
void add_join_on(TABLE_LIST *b, Item *expr);
bool push_new_name_resolution_context(Parse_context *pc, TABLE_LIST *left_op,
                                      TABLE_LIST *right_op);
void init_sql_command_flags(void);
const CHARSET_INFO *get_bin_collation(const CHARSET_INFO *cs);
void killall_non_super_threads(THD *thd);
bool shutdown(THD *thd, enum mysql_enum_shutdown_level level, uchar exit_code,
              bool ro_instance_only);
bool show_precheck(THD *thd, LEX *lex, bool lock);

/* Variables */

extern uint sql_command_flags[];
extern const LEX_CSTRING command_name[];

inline bool is_supported_parser_charset(const CHARSET_INFO *cs) {
  return (cs->mbminlen == 1);
}

bool sqlcom_can_generate_row_events(enum enum_sql_command command);

bool all_tables_not_ok(THD *thd, TABLE_LIST *tables);
bool some_non_temp_table_to_be_updated(THD *thd, TABLE_LIST *tables);

bool execute_show(THD *thd, TABLE_LIST *all_tables);

// TODO: remove after refactoring of ALTER DATABASE:
bool set_default_charset(HA_CREATE_INFO *create_info,
                         const CHARSET_INFO *value);
// TODO: remove after refactoring of ALTER DATABASE:
bool set_default_collation(HA_CREATE_INFO *create_info,
                           const CHARSET_INFO *value);
bool set_db_read_only(HA_CREATE_INFO *create_info, int super_read_only, int on);

/* Bits in sql_command_flags */

#define CF_CHANGES_DATA (1U << 0)
/* The 2nd bit is unused -- it used to be CF_HAS_ROW_COUNT. */
#define CF_STATUS_COMMAND (1U << 2)
#define CF_SHOW_TABLE_COMMAND (1U << 3)
#define CF_WRITE_LOGS_COMMAND (1U << 4)
/**
  Must be set for SQL statements that may contain
  Item expressions and/or use joins and tables.
  Indicates that the parse tree of such statement may
  contain rule-based optimizations that depend on metadata
  (i.e. number of columns in a table), and consequently
  that the statement must be re-prepared whenever
  referenced metadata changes. Must not be set for
  statements that themselves change metadata, e.g. RENAME,
  ALTER and other DDL, since otherwise will trigger constant
  reprepare. Consequently, complex item expressions and
  joins are currently prohibited in these statements.
*/
#define CF_REEXECUTION_FRAGILE (1U << 5)
/**
  Implicitly commit before the SQL statement is executed.

  Statements marked with this flag will cause any active
  transaction to end (commit) before proceeding with the
  command execution.

  This flag should be set for statements that probably can't
  be rolled back or that do not expect any previously metadata
  locked tables.
*/
#define CF_IMPLICIT_COMMIT_BEGIN (1U << 6)
/**
  Implicitly commit after the SQL statement.

  Statements marked with this flag are automatically committed
  at the end of the statement.

  This flag should be set for statements that will implicitly
  open and take metadata locks on system tables that should not
  be carried for the whole duration of a active transaction.
*/
#define CF_IMPLICIT_COMMIT_END (1U << 7)
/**
  CF_IMPLICIT_COMMIT_BEGIN and CF_IMPLICIT_COMMIT_END are used
  to ensure that the active transaction is implicitly committed
  before and after every DDL statement and any statement that
  modifies our currently non-transactional system tables.
*/
#define CF_AUTO_COMMIT_TRANS (CF_IMPLICIT_COMMIT_BEGIN | CF_IMPLICIT_COMMIT_END)

/**
  Diagnostic statement.
  Diagnostic statements:
  - SHOW WARNING
  - SHOW ERROR
  - GET DIAGNOSTICS (WL#2111)
  do not modify the Diagnostics Area during execution.
*/
#define CF_DIAGNOSTIC_STMT (1U << 8)

/**
  Identifies statements that may generate row events
  and that may end up in the binary log.
*/
#define CF_CAN_GENERATE_ROW_EVENTS (1U << 9)

/**
  Identifies statements which may deal with temporary tables and for which
  temporary tables should be pre-opened to simplify privilege checks.
*/
#define CF_PREOPEN_TMP_TABLES (1U << 10)

/**
  Identifies statements for which open handlers should be closed in the
  beginning of the statement.
*/
#define CF_HA_CLOSE (1U << 11)

/**
  Identifies statements that can be explained with EXPLAIN.
*/
#define CF_CAN_BE_EXPLAINED (1U << 12)

/** Identifies statements which may generate an optimizer trace */
#define CF_OPTIMIZER_TRACE (1U << 14)

/**
   Identifies statements that should always be disallowed in
   read only transactions.
*/
#define CF_DISALLOW_IN_RO_TRANS (1U << 15)

/**
  Identifies statements and commands that can be used with Protocol Plugin
*/
#define CF_ALLOW_PROTOCOL_PLUGIN (1U << 16)

/**
  Identifies statements (typically DDL) which needs auto-commit mode
  temporarily turned off.

  @note This is necessary to prevent InnoDB from automatically committing
        InnoDB transaction each time data-dictionary tables are closed
        after being updated.

  @note This is also necessary for ACL DDL, so the code which
        saves GTID state or slave state in the system tables at the
        commit time works correctly. This code does statement commit
        on low-level (see System_table_access:: close_table()) and
        thus can pre-maturely commit DDL if @@autocommit=1.
*/
#define CF_NEEDS_AUTOCOMMIT_OFF (1U << 17)

/**
  Identifies statements which can return rows of data columns (SELECT, SHOW ...)
*/
#define CF_HAS_RESULT_SET (1U << 18)

/**
  Identifies DDL statements which can be atomic.
  Having the bit ON does not yet define an atomic.
  The property is used both on the master and slave.
  On the master atomicity infers the binlog and gtid_executed system table.
  On the slave it more involves the slave info table.

  @note At the momemnt of declaration the covered DDL subset coincides
        with the of CF_NEEDS_AUTOCOMMIT_OFF.
*/
#define CF_POTENTIAL_ATOMIC_DDL (1U << 19)

/**
  Statement is depending on the ACL cache, which can be disabled by the
  --skip-grant-tables server option.
*/
#define CF_REQUIRE_ACL_CACHE (1U << 20)

/* Bits in server_command_flags */

/**
  Skip the increase of the global query id counter. Commonly set for
  commands that are stateless (won't cause any change on the server
  internal states). This is made obsolete as query id is incremented
  for ping and statistics commands as well because of race condition
  (Bug#58785).
*/
#define CF_SKIP_QUERY_ID (1U << 0)

/**
  Skip the increase of the number of statements that clients have
  sent to the server. Commonly used for commands that will cause
  a statement to be executed but the statement might have not been
  sent by the user (ie: stored procedure).
*/
#define CF_SKIP_QUESTIONS (1U << 1)

/**
  1U << 16 is reserved for Protocol Plugin statements and commands
*/

/**
  @} (end of group GROUP_PARSER)
*/

std::string get_user_query_info_from_thd(THD *thd);
#endif /* SQL_PARSE_INCLUDED */
