/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/upgrade/server.h"
#include "sql/dd/upgrade/server.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <vector>

#include "lex_string.h"
#include "my_dbug.h"
#include "mysql/components/services/log_builtins.h"
#include "scripts/mysql_fix_privilege_tables_sql.h"
#include "scripts/sql_commands_system_tables_data_fix.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_schema.h"                // dd::Schema_MDL_locker
#include "sql/dd/dd_tablespace.h"            // dd::fill_table_and_parts...
#include "sql/dd/dd_trigger.h"               // dd::create_trigger
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // dd::DD_bootstrap_ctx
#include "sql/dd/impl/bootstrap/bootstrapper.h"
#include "sql/dd/impl/tables/dd_properties.h"  // dd::tables::DD_properties
#include "sql/dd/impl/utils.h"                 // dd::end_transaction
#include "sql/dd/types/routine.h"              // dd::Table
#include "sql/dd/types/table.h"                // dd::Table
#include "sql/dd/types/tablespace.h"
#include "sql/dd_sp.h"      // prepare_sp_chistics_from_dd_routine
#include "sql/sd_notify.h"  // sysd::notify
#include "sql/sp.h"         // Stored_routine_creation_ctx
#include "sql/sp_head.h"    // sp_head
#include "sql/sql_base.h"
#include "sql/sql_prepare.h"
#include "sql/strfunc.h"
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/thd_raii.h"
#include "sql/trigger.h"  // Trigger
#include "sql/trigger_def.h"

typedef ulonglong sql_mode_t;
extern const char *mysql_sys_schema[];
extern const char *fill_help_tables[];

const char *upgrade_modes[] = {
    "NONE", "MINIMAL", "AUTO", "FORCE", "FORCE_AND_SHUTDOWN", NullS};
TYPELIB upgrade_mode_typelib = {array_elements(upgrade_modes) - 1, "",
                                upgrade_modes, nullptr};

namespace dd {
namespace upgrade {

/***************************************************************************
 * Bootstrap_error_handler implementation
 ***************************************************************************/

void Bootstrap_error_handler::my_message_bootstrap(uint error, const char *str,
                                                   myf MyFlags) {
  set_abort_on_error(error);
  my_message_sql(error, str, MyFlags);
  if (m_log_error)
    LogEvent()
        .type(LOG_TYPE_ERROR)
        .subsys(LOG_SUBSYSTEM_TAG)
        .prio(ERROR_LEVEL)
        .errcode(ER_ERROR_INFO_FROM_DA)
        .verbatim(str);
}

void Bootstrap_error_handler::set_abort_on_error(uint error) {
  switch (error) {
    case ER_WRONG_COLUMN_NAME: {
      abort_on_error = true;
      m_log_error = true;
      break;
    }
    default:
      break;
  }
}

Bootstrap_error_handler::Bootstrap_error_handler() {
  m_old_error_handler_hook = error_handler_hook;
  error_handler_hook = my_message_bootstrap;
}

void Bootstrap_error_handler::set_log_error(bool log_error) {
  m_log_error = log_error;
}

Bootstrap_error_handler::~Bootstrap_error_handler() {
  error_handler_hook = m_old_error_handler_hook;
}

bool Bootstrap_error_handler::m_log_error = true;
bool Bootstrap_error_handler::abort_on_error = false;

/***************************************************************************
 * Routine_event_context_guard implementation
 ***************************************************************************/

Routine_event_context_guard::Routine_event_context_guard(THD *thd)
    : m_thd(thd) {
  m_thd = thd;
  m_sql_mode = m_thd->variables.sql_mode;
  m_client_cs = m_thd->variables.character_set_client;
  m_connection_cl = m_thd->variables.collation_connection;
  m_saved_time_zone = m_thd->variables.time_zone;
}
Routine_event_context_guard::~Routine_event_context_guard() {
  m_thd->variables.sql_mode = m_sql_mode;
  m_thd->variables.character_set_client = m_client_cs;
  m_thd->variables.collation_connection = m_connection_cl;
  m_thd->variables.time_zone = m_saved_time_zone;
}

/***************************************************************************
 * Syntax_error_handler implementation
 ***************************************************************************/

uint Syntax_error_handler::parse_error_count = 0;
bool Syntax_error_handler::is_parse_error = false;
dd::String_type Syntax_error_handler::reason = "";
const uint Syntax_error_handler::MAX_SERVER_CHECK_FAILS = 50;

bool Syntax_error_handler::handle_condition(
    THD *, uint sql_errno, const char *, Sql_condition::enum_severity_level *,
    const char *msg) {
  if (sql_errno == ER_PARSE_ERROR) {
    parse_error_count++;
    if (m_global_counter) (*m_global_counter)++;
    is_parse_error = true;
    reason = msg;
  } else {
    is_parse_error = false;
    reason = "";
  }
  return false;
}

bool Syntax_error_handler::has_too_many_errors() {
  return parse_error_count > MAX_SERVER_CHECK_FAILS;
}

bool Syntax_error_handler::has_errors() { return parse_error_count > 0; }

const char *Syntax_error_handler::error_message() { return reason.c_str(); }

/***************************************************************************
 * Upgrade_error_handler implementation
 ***************************************************************************/

bool Upgrade_error_counter::has_errors() { return (m_error_count > 0); }
bool Upgrade_error_counter::has_too_many_errors() {
  return (m_error_count > ERROR_LIMIT);
}
Upgrade_error_counter Upgrade_error_counter::operator++(int) {
  m_error_count++;
  return *this;
}

namespace {

static std::vector<uint> ignored_errors{
    ER_DUP_FIELDNAME, ER_DUP_KEYNAME, ER_BAD_FIELD_ERROR,
    ER_COL_COUNT_DOESNT_MATCH_PLEASE_UPDATE_V2, ER_DUP_ENTRY};

template <typename T, typename CLOS>
bool examine_each(Upgrade_error_counter *error_count,
                  std::vector<const T *> *list, CLOS &&clos) {
  for (const T *item : *list) {
    DBUG_ASSERT(item != nullptr);
    clos(item);
    if (error_count->has_too_many_errors()) return true;
  }
  return false;
}

template <typename T>
class Server_option_guard {
  T *server_opt;
  T old_value;

 public:
  Server_option_guard(T *opt, T new_value) : server_opt(opt), old_value(*opt) {
    *server_opt = new_value;
  }

  ~Server_option_guard() { *server_opt = old_value; }
};

class MySQL_check {
 private:
  std::vector<dd::String_type> alter_cmds, repairs;
  bool needs_repair;

  static dd::String_type escape_str(const dd::String_type &src) {
    dd::String_type res = "`";
    for (size_t i = 0; i < src.size(); i++) {
      if (src[i] == '`') res += '`';
      res += src[i];
    }
    res += "`";
    return res;
  }

  void comma_seperated_join(std::vector<dd::String_type> &list,
                            dd::String_type &dest) {
    dest = list[0];
    for (auto it = list.begin() + 1; it != list.end(); it++) dest += "," + *it;
  }

  bool get_schema_tables(THD *thd, const char *schema,
                         dd::String_type &tables_list) {
    Schema_MDL_locker mdl_handler(thd);
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    const dd::Schema *sch = nullptr;
    std::vector<const dd::Table *> tables;
    dd::Stringstream_type t_list;

    if (mdl_handler.ensure_locked(schema) ||
        thd->dd_client()->acquire(schema, &sch) ||
        thd->dd_client()->fetch_schema_components(sch, &tables)) {
      LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_TO_FETCH_TABLES);
      return (true);
    }

    bool first = true;
    std::for_each(tables.begin(), tables.end(), [&](const dd::Table *table) {
      if (table->type() != dd::enum_table_type::BASE_TABLE ||
          table->hidden() != dd::Abstract_table::HT_VISIBLE)
        return;
      if (!first) t_list << ", ";
      first = false;
      t_list << escape_str(sch->name()) << "." << escape_str(table->name());
    });

    tables_list = t_list.str();
    return false;
  }

  bool check_table(List<Ed_row>::iterator &it,
                   const List<Ed_row>::iterator &end, bool repair) {
    Ed_row &row = *it;
    const char *table = row[0].str, *alter_txt = nullptr;
    bool found_error = false;
    it++;

    while (strcmp(row[2].str, "status")) {
      if (strcmp(row[2].str, "note")) {
        found_error = true;
        alter_txt = strstr(row[3].str, "ALTER TABLE");
      }
      if (it == end || strcmp((*it)[0].str, table)) break;
      row = *it;
      ++it;
    }

    if (found_error && strcmp(row[3].str, "OK")) {
      if (repair) {
        LogErr(ERROR_LEVEL, ER_SERVER_UPGRADE_REPAIR_STATUS, table, "failed");
        return true;
      } else
        LogErr(WARNING_LEVEL, ER_SERVER_UPGRADE_REPAIR_REQUIRED, table);
      if (alter_txt)
        alter_cmds.push_back(dd::String_type(alter_txt));
      else
        repairs.push_back(dd::String_type(table));
    } else if (repair) {
      LogErr(INFORMATION_LEVEL, ER_SERVER_UPGRADE_REPAIR_STATUS, table,
             "successful");
    } else
      found_error = false;

    return found_error;
  }

  bool verify_response(List<Ed_row> &rset, bool repair) {
    auto it = rset.begin();
    bool error = false;
    while (it != rset.end()) error |= check_table(it, rset.end(), repair);
    return error;
  }

  /**
    Returns true if something went wrong while retreving the table list or
    executing CHECK TABLE statements.
  */
  bool check_tables(THD *thd, const char *schema) {
    Ed_connection con(thd);
    dd::String_type tables;
    LEX_STRING str;

    LogErr(INFORMATION_LEVEL, ER_SERVER_UPGRADE_CHECKING_DB, schema);
    if (get_schema_tables(thd, schema, tables)) return true;
    if (tables.size() == 0) return false;

    dd::String_type query = "CHECK TABLE " + tables + " FOR UPGRADE";
    lex_string_strmake(thd->mem_root, &str, query.c_str(), query.size());
    if (con.execute_direct(str)) return true;

    needs_repair |= verify_response(*con.get_result_sets(), false);
    return false;
  }

 public:
  MySQL_check() : needs_repair(false) {}

  bool check_all_schemas(THD *thd) {
    std::vector<dd::String_type> schemas;
    if (thd->dd_client()->fetch_global_component_names<dd::Schema>(&schemas))
      return true;
    for (dd::String_type &schema : schemas) {
      if (schema.compare("information_schema") == 0 ||
          schema.compare("performance_schema") == 0)
        continue;
      if (check_tables(thd, schema.c_str())) return true;
    }
    return false;
  }

  bool check_system_schemas(THD *thd) {
    return check_tables(thd, "mysql") || check_tables(thd, "sys");
  }

  bool repair_tables(THD *thd) {
    if (!needs_repair) return false;

    for (auto &alter : alter_cmds)
      if (dd::execute_query(thd, alter)) return true;
    alter_cmds.clear();

    if (repairs.size() == 0) return false;
    dd::String_type tables;
    comma_seperated_join(repairs, tables);

    Ed_connection con(thd);
    LEX_STRING str;
    dd::String_type query = "REPAIR TABLE " + tables;
    lex_string_strmake(thd->mem_root, &str, query.c_str(), query.size());
    if (con.execute_direct(str)) return true;
    repairs.clear();
    needs_repair = false;
    (void)verify_response(*con.get_result_sets(), true);
    return false;
  }
};

bool ignore_error_and_execute(THD *thd, const char *query_ptr) {
  Ed_connection con(thd);
  LEX_STRING str;
  lex_string_strmake(thd->mem_root, &str, query_ptr, strlen(query_ptr));

  // These are the same errors ignored in the mysql_upgrade client
  if (con.execute_direct(str) &&
      std::find(ignored_errors.begin(), ignored_errors.end(),
                con.get_last_errno()) == ignored_errors.end()) {
    LogErr(ERROR_LEVEL, ER_DD_INITIALIZE_SQL_ERROR, query_ptr,
           con.get_last_errno(), con.get_last_error());
    return true;
  }
  return false;
}

bool fix_sys_schema(THD *thd) {
  /*
    Re-create SYS schema if:

    - There is a server upgrade going on.
    - Or the SYS schema does not exist.

    With the SYS schema versioning removed, we make sure there is indeed
    a server upgrade going on before we re-create the SYS schema. This has
    the consequence that upgrade=FORCE will not re-create the SYS schema,
    unless it does not exist. This is in line with the old behavior of the
    SYS schema versioning and upgrade.
  */
  Schema_MDL_locker mdl_handler(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *sch = nullptr;
  if (mdl_handler.ensure_locked("sys") ||
      thd->dd_client()->acquire("sys", &sch))
    return true;

  if (sch != nullptr &&
      !dd::bootstrap::DD_bootstrap_ctx::instance().is_server_upgrade() &&
      (!is_force_upgrade()))
    return false;

  const char **query_ptr;
  LogErr(INFORMATION_LEVEL, ER_SERVER_UPGRADE_SYS_SCHEMA);
  thd->user_var_events_alloc = thd->mem_root;
  for (query_ptr = &mysql_sys_schema[0]; *query_ptr != nullptr; query_ptr++)
    if (ignore_error_and_execute(thd, *query_ptr)) return true;
  thd->mem_root->Clear();
  return false;
}

bool fix_mysql_tables(THD *thd) {
  const char **query_ptr;

  if (ignore_error_and_execute(thd, "USE mysql")) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_FIND_VALID_DATA_DIR);
    return true;
  }

  LogErr(INFORMATION_LEVEL, ER_SERVER_UPGRADE_MYSQL_TABLES);
  for (query_ptr = &mysql_fix_privilege_tables[0]; *query_ptr != nullptr;
       query_ptr++)
    if (ignore_error_and_execute(thd, *query_ptr)) return true;

  LogErr(INFORMATION_LEVEL, ER_SERVER_UPGRADE_SYSTEM_TABLES);
  for (query_ptr = &mysql_system_tables_data_fix[0]; *query_ptr != nullptr;
       query_ptr++)
    if (ignore_error_and_execute(thd, *query_ptr)) return true;

  return false;
}

bool upgrade_help_tables(THD *thd) {
  if (dd::execute_query(thd, "USE mysql")) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_FIND_VALID_DATA_DIR);
    return true;
  }
  LogErr(INFORMATION_LEVEL, ER_SERVER_UPGRADE_HELP_TABLE_STATUS, "started");
  for (const char **query_ptr = &fill_help_tables[0]; *query_ptr != nullptr;
       query_ptr++)
    if (dd::execute_query(thd, *query_ptr)) {
      LogErr(ERROR_LEVEL, ER_SERVER_UPGRADE_HELP_TABLE_STATUS, "failed");
      return true;
    }
  LogErr(INFORMATION_LEVEL, ER_SERVER_UPGRADE_HELP_TABLE_STATUS, "completed");
  return false;
}

static void create_upgrade_file() {
  FILE *out;
  char upgrade_info_file[FN_REFLEN] = {0};
  fn_format(upgrade_info_file, "mysql_upgrade_info", mysql_real_data_home_ptr,
            "", MYF(0));

  if ((out = my_fopen(upgrade_info_file, O_TRUNC | O_WRONLY, MYF(0)))) {
    /* Write new version to file */
    fputs(MYSQL_SERVER_VERSION, out);
    my_fclose(out, MYF(0));
    return;
  }
  LogErr(WARNING_LEVEL, ER_SERVER_UPGRADE_INFO_FILE, upgrade_info_file);
}

}  // namespace

/*
  This function runs checks on the database before running the upgrade to make
  sure that the database is ready to be upgraded to a newer version. New checks
  can be added as required. Returns false if the database can be upgraded.
*/
bool do_server_upgrade_checks(THD *thd) {
  if (!dd::bootstrap::DD_bootstrap_ctx::instance().is_server_upgrade_from_after(
          bootstrap::SERVER_VERSION_50700))
    return false;

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  Upgrade_error_counter error_count;

  /*
    For any server upgrade, we will analyze events, routines, views and
    triggers and reject upgrade if we find invalid syntax that would not
    have been accepted in a CREATE statement.
  */
  std::vector<const dd::Schema *> schema_vector;
  if (thd->dd_client()->fetch_global_components(&schema_vector))
    return dd::end_transaction(thd, true);

  Syntax_error_handler error_handler(&error_count);
  thd->push_internal_handler(&error_handler);

  for (const dd::Schema *schema : schema_vector) {
    std::vector<const dd::Table *> tables;
    if (thd->dd_client()->fetch_schema_components(schema, &tables))
      return dd::end_transaction(thd, true);

    if (examine_each(&error_count, &tables, [&](const dd::Table *table) {
          (void)invalid_triggers(thd, schema->name().c_str(), *table);
        }))
      break;

    std::vector<const dd::Event *> events;
    if (thd->dd_client()->fetch_schema_components(schema, &events))
      return dd::end_transaction(thd, true);

    if (examine_each(&error_count, &events, [&](const dd::Event *event) {
          dd::String_type sql;
          if (build_event_sp(thd, event->name().c_str(), event->name().size(),
                             event->definition().c_str(),
                             event->definition().size(), &sql) ||
              invalid_sql(thd, schema->name().c_str(), sql))
            LogErr(ERROR_LEVEL, ER_UPGRADE_PARSE_ERROR, "Event",
                   schema->name().c_str(), event->name().c_str(),
                   Syntax_error_handler::error_message());
          return false;
        }))
      break;

    std::vector<const dd::Routine *> routines;
    if (thd->dd_client()->fetch_schema_components(schema, &routines))
      return dd::end_transaction(thd, true);

    if (examine_each(&error_count, &routines, [&](const dd::Routine *routine) {
          if (invalid_routine(thd, *schema, *routine))
            LogErr(ERROR_LEVEL, ER_UPGRADE_PARSE_ERROR, "Routine",
                   schema->name().c_str(), routine->name().c_str(),
                   Syntax_error_handler::error_message());
          return false;
        }))
      break;

    std::vector<const dd::View *> views;
    if (thd->dd_client()->fetch_schema_components(schema, &views))
      return dd::end_transaction(thd, true);

    if (examine_each(&error_count, &views, [&](const dd::View *view) {
          if (invalid_sql(thd, schema->name().c_str(), view->definition()))
            LogErr(ERROR_LEVEL, ER_UPGRADE_PARSE_ERROR, "View",
                   schema->name().c_str(), view->name().c_str(),
                   Syntax_error_handler::error_message());
          return false;
        }))
      break;
  }
  thd->pop_internal_handler();

  /*
    If upgrade is crossing 8.0.13, we need to look out for partitioned
    tables having partitions in shared tablespaces, and err out
    if this is found. We reuse the schema vector that was retrieved above.
    We do this only if the number of soft errors found so far is below the
    defined limit.
  */
  if (!error_count.has_too_many_errors() &&
      dd::bootstrap::DD_bootstrap_ctx::instance().is_server_upgrade_from_before(
          bootstrap::SERVER_VERSION_80013)) {
    /*
      Get hold of the InnoDB handlerton. The check for partitioned tables
      using shared tablespaces is only relevant for InnoDB.
    */
    plugin_ref pr =
        ha_resolve_by_name_raw(thd, LEX_CSTRING{STRING_WITH_LEN("InnoDB")});
    handlerton *hton =
        (pr != nullptr ? plugin_data<handlerton *>(pr) : nullptr);
    DBUG_ASSERT(hton != nullptr && hton->get_tablespace_type);

    /*
      Get hold of all tablespaces, keep the non-implicit InnoDB spaces
      in a map.
    */
    std::vector<const dd::Tablespace *> tablespaces;
    if (thd->dd_client()->fetch_global_components(&tablespaces))
      return dd::end_transaction(thd, true);

    std::map<const String_type, const dd::Tablespace *> invalid_spaces;
    for (const dd::Tablespace *space : tablespaces) {
      if (my_strcasecmp(system_charset_info, space->engine().c_str(),
                        "InnoDB") != 0)
        continue;

      Tablespace_type space_type;
      if (hton->get_tablespace_type(*space, &space_type)) {
        LogErr(ERROR_LEVEL, ER_UNKNOWN_TABLESPACE_TYPE, space->name().c_str());
        return dd::end_transaction(thd, true);
      }

      if (space_type != Tablespace_type::SPACE_TYPE_IMPLICIT) {
        invalid_spaces.insert(
            std::pair<const String_type, const dd::Tablespace *>(space->name(),
                                                                 space));
      }
    }

    /*
      For each schema, get all tables, check if the partitioned InnoDB tables
      are using a shared tablespace. If so, print an error in the error log,
      but continue to analyze additional tables.
    */
    for (const dd::Schema *schema : schema_vector) {
      /*
        If we got to the error limit, exit. We check this only here, since if
        we get hold of all tables in a schema (i.e., complete the expensive
        part), we may as well analyze them all before checking if we exceeded
        the error limit.
      */
      if (error_count.has_too_many_errors()) break;

      std::vector<const dd::Table *> tables;
      /* Cannot continue if we have a DD error. */
      if (thd->dd_client()->fetch_schema_components(schema, &tables))
        return dd::end_transaction(thd, true);

      for (const dd::Table *table : tables) {
        /* Only consider partitioned InnoDB tables. */
        if (table->partition_type() == dd::Table::PT_NONE ||
            my_strcasecmp(system_charset_info, table->engine().c_str(),
                          "InnoDB") != 0)
          continue;

        Tablespace_hash_set space_names(PSI_INSTRUMENT_ME);
        if (fill_table_and_parts_tablespace_names(thd, schema->name().c_str(),
                                                  table->name().c_str(),
                                                  &space_names))
          return dd::end_transaction(thd, true);

        for (const std::string &name : space_names) {
          if (invalid_spaces.find(String_type(name.c_str())) !=
              invalid_spaces.end()) {
            error_count++;
            LogErr(ERROR_LEVEL, ER_SHARED_TABLESPACE_USED_BY_PARTITIONED_TABLE,
                   table->name().c_str(), name.c_str());
          }
        }
      }
    }
  }

  /*
    If there are errors from any of the checks, we abort upgrade.
  */
  if (error_count.has_errors()) return dd::end_transaction(thd, true);

  return false;
}

/**
  Validate a dd::Routine object.
*/
bool invalid_routine(THD *thd, const dd::Schema &schema,
                     const dd::Routine &routine) {
  Routine_event_context_guard guard(thd);
  sp_head *sp = nullptr;
  st_sp_chistics chistics;
  prepare_sp_chistics_from_dd_routine(&routine, &chistics);

  dd::String_type return_type_str;
  prepare_return_type_string_from_dd_routine(thd, &routine, &return_type_str);

  // Create SP creation context to be used in db_load_routine()
  Stored_program_creation_ctx *creation_ctx =
      Stored_routine_creation_ctx::create_routine_creation_ctx(&routine);

  thd->variables.character_set_client = creation_ctx->get_client_cs();
  thd->variables.collation_connection = creation_ctx->get_connection_cl();
  thd->update_charset();

  enum_sp_return_code error = db_load_routine(
      thd,
      routine.type() == dd::Routine::RT_FUNCTION ? enum_sp_type::FUNCTION
                                                 : enum_sp_type::PROCEDURE,
      schema.name().c_str(), schema.name().size(), routine.name().c_str(),
      routine.name().size(), &sp, routine.sql_mode(),
      routine.parameter_str().c_str(), return_type_str.c_str(),
      routine.definition().c_str(), &chistics, routine.definer_user().c_str(),
      routine.definer_host().c_str(), routine.created(true),
      routine.last_altered(true), creation_ctx);

  if (sp != nullptr)  // To be safe
    sp_head::destroy(sp);

  if (error) return (thd->get_stmt_da()->mysql_errno() == ER_PARSE_ERROR);
  thd->clear_error();
  return false;
}

/**
  Validate all the triggers of the given table.
*/
bool invalid_triggers(THD *thd, const char *schema_name,
                      const dd::Table &table) {
  if (!table.has_trigger()) return false;
  List<::Trigger> triggers;
  if (dd::load_triggers(thd, thd->mem_root, schema_name, table.name().c_str(),
                        table, &triggers))
    return true;
  for (::Trigger &t : triggers) {
    if (t.parse(thd, false) || t.has_parse_error()) {
      LogEvent()
          .type(LOG_TYPE_ERROR)
          .subsys(LOG_SUBSYSTEM_TAG)
          .prio(ERROR_LEVEL)
          .errcode(ER_UPGRADE_PARSE_ERROR)
          .verbatim(t.get_parse_error_message());
      thd->clear_error();
    }
    sp_head::destroy(t.get_sp());
    if (Syntax_error_handler::has_too_many_errors()) return true;
  }
  return Syntax_error_handler::has_errors();
}

bool invalid_sql(THD *thd, const char *dbname, const dd::String_type &sql) {
  bool error = false;
  Parser_state *old = thd->m_parser_state;
  Parser_state parser_state;

  if (parser_state.init(thd, sql.c_str(), sql.size())) return true;

  LEX_CSTRING old_db = thd->db();
  LEX lex, *lex_saved = thd->lex;

  thd->reset_db(to_lex_cstring(dbname));
  thd->lex = &lex;
  lex_start(thd);

  thd->m_parser_state = &parser_state;
  parser_state.m_lip.m_digest = nullptr;

  if (thd->sql_parser())
    error = (thd->get_stmt_da()->mysql_errno() == ER_PARSE_ERROR);

  lex_end(thd->lex);
  thd->lex = lex_saved;
  thd->reset_db(old_db);
  thd->m_parser_state = old;
  thd->clear_error();

  return error;
}

/**
  Helper function to create a stored procedure from an event body.
*/
bool build_event_sp(const THD *thd, const char *name, size_t name_len,
                    const char *body, size_t body_len,
                    dd::String_type *sp_sql) {
  const uint STATIC_SQL_LENGTH = 44;
  String temp(STATIC_SQL_LENGTH + name_len + body_len);

  temp.append(STRING_WITH_LEN("CREATE "));
  temp.append(STRING_WITH_LEN("PROCEDURE "));

  append_identifier(thd, &temp, name, name_len);

  temp.append(STRING_WITH_LEN("() SQL SECURITY INVOKER "));
  temp.append(body, body_len);

  *sp_sql = temp.ptr();
  return false;
}

bool upgrade_system_schemas(THD *thd) {
  Disable_autocommit_guard autocommit_guard(thd);
  Bootstrap_error_handler bootstrap_error_handler;

  Server_option_guard<bool> acl_guard(&opt_noacl, true);
  Server_option_guard<bool> general_log_guard(&opt_general_log, false);
  Server_option_guard<bool> slow_log_guard(&opt_slow_log, false);
  Server_option_guard<bool> bin_log_guard(&thd->variables.sql_log_bin, false);

  uint server_version = MYSQL_VERSION_ID;
  bool exists_version = false;

  if (dd::tables::DD_properties::instance().get(
          thd, "MYSQLD_VERSION_UPGRADED", &server_version, &exists_version) ||
      !exists_version)
    if (dd::tables::DD_properties::instance().get(
            thd, "MYSQLD_VERSION", &server_version, &exists_version) ||
        !exists_version)
      return true;

  MySQL_check check;

  LogErr(SYSTEM_LEVEL, ER_SERVER_UPGRADE_STATUS, server_version,
         MYSQL_VERSION_ID, "started");
  log_sink_buffer_check_timeout();
  sysd::notify("STATUS=Server upgrade in progress\n");

  bootstrap_error_handler.set_log_error(false);
  bool err =
      fix_mysql_tables(thd) || fix_sys_schema(thd) ||
      upgrade_help_tables(thd) ||
      (DBUG_EVALUATE_IF(
           "force_fix_user_schemas", true,
           dd::bootstrap::DD_bootstrap_ctx::instance()
               .is_server_upgrade_from_before(bootstrap::SERVER_VERSION_80011))
           ? check.check_all_schemas(thd)
           : check.check_system_schemas(thd)) ||
      check.repair_tables(thd) ||
      dd::tables::DD_properties::instance().set(thd, "MYSQLD_VERSION_UPGRADED",
                                                MYSQL_VERSION_ID);

  create_upgrade_file();
  bootstrap_error_handler.set_log_error(true);

  if (!err)
    LogErr(SYSTEM_LEVEL, ER_SERVER_UPGRADE_STATUS, server_version,
           MYSQL_VERSION_ID, "completed");
  log_sink_buffer_check_timeout();
  sysd::notify("STATUS=Server upgrade complete\n");

  /*
   * During server startup, dd::reset_tables_and_tablespaces is called, which
   * calls innobase_dict_cache_reset_tables_and_tablespaces. This tries to clear
   * the open tables cache. But not able to, which causes an assert. So we force
   * close everything.
   */
  close_thread_tables(thd);
  close_cached_tables_nsec(nullptr, nullptr, false, LONG_TIMEOUT_NSEC);

  return dd::end_transaction(thd, err);
}

bool no_server_upgrade_required() {
  return !(dd::bootstrap::DD_bootstrap_ctx::instance().is_server_upgrade() ||
           is_force_upgrade());
}

bool I_S_upgrade_required() {
  return dd::bootstrap::DD_bootstrap_ctx::instance().is_server_upgrade() ||
         dd::bootstrap::DD_bootstrap_ctx::instance().I_S_upgrade_done() ||
         is_force_upgrade();
}

bool is_force_upgrade() {
  return opt_upgrade_mode == UPGRADE_FORCE ||
         opt_upgrade_mode == UPGRADE_FORCE_AND_SHUTDOWN;
}

}  // namespace upgrade
}  // namespace dd
