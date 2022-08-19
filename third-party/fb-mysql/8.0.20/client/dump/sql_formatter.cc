/*
  Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "client/dump/sql_formatter.h"

#include "my_config.h"

#include <sys/types.h>
#include <chrono>
#include <ctime>
#include <functional>
#include <sstream>
#include <string>

#include "client/dump/column_statistic.h"
#include "client/dump/mysql_function.h"
#include "client/dump/privilege.h"
#include "client/dump/stored_procedure.h"
#include "client/dump/view.h"
#include "m_ctype.h"

using namespace Mysql::Tools::Dump;

void Sql_formatter::format_row_group(Row_group_dump_task *row_group) {
  std::size_t row_data_length = 0;
  // Calculate total length of data to be formatted.
  for (std::vector<Row *>::iterator row_iterator = row_group->m_rows.begin();
       row_iterator != row_group->m_rows.end(); ++row_iterator) {
    row_data_length += 3;  // Space for enclosing parentheses and comma.

    Row *row = *row_iterator;
    for (size_t column = row->m_row_data.size(); column-- > 0;) {
      // Space for escaped string, enclosing " and comma.
      row_data_length += row->m_row_data.size_of_element(column) * 2 + 3;
    }
  }
  if (m_options->m_dump_column_names || row_group->m_has_generated_columns) {
    row_data_length += 3;  // Space for enclosing parentheses and space.
    const std::vector<Mysql_field> &fields = row_group->m_fields;
    for (std::vector<Mysql_field>::const_iterator field_iterator =
             fields.begin();
         field_iterator != fields.end(); ++field_iterator) {
      row_data_length += field_iterator->get_name().size() * 2 + 3;
    }
  }
  std::string row_string;
  /*
    Space for constant strings "INSERT INTO ... VALUES ()" with
    reserve for comments, modificators and future changes.
    */
  const size_t INSERT_INTO_MAX_SIZE = 200;

  row_string.reserve(
      INSERT_INTO_MAX_SIZE + row_group->m_source_table->get_schema().size() +
      row_group->m_source_table->get_name().size() + row_data_length);

  if (m_options->m_insert_type_replace) row_string += "REPLACE INTO ";
  /*
   for mysql.innodb_table_stats, mysql.innodb_index_stats tables always
   dump as INSERT IGNORE INTO
  */
  else if (m_options->m_insert_type_ignore ||
           innodb_stats_tables(row_group->m_source_table->get_schema(),
                               row_group->m_source_table->get_name()))
    row_string += "INSERT IGNORE INTO ";
  else
    row_string += "INSERT INTO ";
  row_string += this->get_quoted_object_full_name(row_group->m_source_table);
  if (m_options->m_dump_column_names || row_group->m_has_generated_columns) {
    row_string += " (";
    const std::vector<Mysql_field> &fields = row_group->m_fields;
    for (std::vector<Mysql_field>::const_iterator field_iterator =
             fields.begin();
         field_iterator != fields.end(); ++field_iterator) {
      if (field_iterator != fields.begin()) row_string += ',';
      row_string += this->quote_name(field_iterator->get_name());
    }
    row_string += ')';
  }
  row_string += " VALUES ";

  CHARSET_INFO *charset_info = this->get_charset();

  std::vector<bool> is_blob;
  for (std::vector<Mysql_field>::const_iterator it =
           row_group->m_fields.begin();
       it != row_group->m_fields.end(); ++it) {
    is_blob.push_back(it->get_character_set_nr() == my_charset_bin.number &&
                      (it->get_type() == MYSQL_TYPE_BIT ||
                       it->get_type() == MYSQL_TYPE_STRING ||
                       it->get_type() == MYSQL_TYPE_VAR_STRING ||
                       it->get_type() == MYSQL_TYPE_VARCHAR ||
                       it->get_type() == MYSQL_TYPE_BLOB ||
                       it->get_type() == MYSQL_TYPE_LONG_BLOB ||
                       it->get_type() == MYSQL_TYPE_MEDIUM_BLOB ||
                       it->get_type() == MYSQL_TYPE_TINY_BLOB ||
                       it->get_type() == MYSQL_TYPE_GEOMETRY));
  }

  for (std::vector<Row *>::const_iterator row_iterator =
           row_group->m_rows.begin();
       row_iterator != row_group->m_rows.end(); ++row_iterator) {
    Row *row = *row_iterator;

    if (row_iterator != row_group->m_rows.begin()) row_string += ',';
    row_string += '(';

    size_t columns = row->m_row_data.size();
    for (size_t column = 0; column < columns; ++column) {
      if (column > 0) row_string += ',';

      size_t column_length;
      const char *column_data =
          row->m_row_data.get_buffer(column, column_length);

      if (row->m_row_data.is_value_null(column))
        row_string += "NULL";
      else if (column_length == 0)
        row_string += "''";
      else if (row_group->m_fields[column].get_additional_flags() & NUM_FLAG) {
        if (column_length >= 1 &&
            (my_isalpha(charset_info, column_data[0]) ||
             (column_length >= 2 && column_data[0] == '-' &&
              my_isalpha(charset_info, column_data[1])))) {
          row_string += "NULL";
        } else if (row_group->m_fields[column].get_type() ==
                   MYSQL_TYPE_DECIMAL) {
          row_string += '\'';
          row_string.append(column_data, column_length);
          row_string += '\'';
        } else
          row_string.append(column_data, column_length);
      } else if (m_options->m_hex_blob && is_blob[column]) {
        row_string += "0x";
        Mysql::Tools::Base::Mysql_query_runner::append_hex_string(
            &row_string, column_data, column_length);
      } else {
        if (is_blob[column]) row_string += "_binary ";
        row_string += '\"';
        if (m_escaping_runner)
          m_escaping_runner->append_escape_string(&row_string, column_data,
                                                  column_length);
        else
          row_string.append(column_data, column_length);
        row_string += '\"';
      }
    }

    row_string += ')';
  }

  row_string += ";\n";

  this->append_output(row_string);
  /*
    user account is dumped in the form of INSERT statements, thus need
    to append FLUSH PRIVILEGES
  */
  if (!use_show_create_user) {
    std::string schema = row_group->m_source_table->get_schema();
    std::string name = row_group->m_source_table->get_name();
    if ((schema == "mysql") && (name == "user")) {
      this->append_output("/*! FLUSH PRIVILEGES */;\n");
    }
  }
}

void Sql_formatter::format_table_indexes(
    Table_deferred_indexes_dump_task *table_indexes_dump_task) {
  Table *table = table_indexes_dump_task->get_related_table();
  if (m_options->m_deffer_table_indexes) {
    /*
      Tables can have indexes  which can refer to columns from
      other tables (ex: foreign keys). In that case we need to
      emit 'USE db' statement as the referenced table may not have
      been created
    */
    bool use_added = false;
    std::string alter_base_string =
        "ALTER TABLE " + this->get_quoted_object_full_name(table) + " ADD ";
    for (std::vector<std::string>::const_iterator it =
             table->get_indexes_sql_definition().begin();
         it != table->get_indexes_sql_definition().end(); ++it) {
      if (!use_added) {
        this->append_output("USE " + this->quote_name(table->get_schema()) +
                            ";\n");
        use_added = true;
      }
      this->append_output(alter_base_string + (*it) + ";\n");
    }
  }
  if (m_options->m_add_locks) this->append_output("UNLOCK TABLES;\n");
}

void Sql_formatter::format_table_definition(
    Table_definition_dump_task *table_definition_dump_task) {
  Table *table = table_definition_dump_task->get_related_table();

  /*
   do not dump DDLs for mysql.innodb_table_stats,
   mysql.innodb_index_stats tables
  */
  if (innodb_stats_tables(table->get_schema(), table->get_name())) return;
  bool use_added = false;
  if (m_options->m_drop_table)
    this->append_output("DROP TABLE IF EXISTS " +
                        this->get_quoted_object_full_name(table) + ";\n");
  if (m_options->m_deffer_table_indexes == 0 && !use_added) {
    use_added = true;
    this->append_output("USE " + this->quote_name(table->get_schema()) + ";\n");
  }
  if (!m_options->m_suppress_create_table)
    this->append_output((m_options->m_deffer_table_indexes
                             ? table->get_sql_definition_without_indexes()
                             : table->get_sql_formatted_definition()) +
                        ";\n");

  if (m_options->m_add_locks)
    this->append_output("LOCK TABLES " +
                        this->get_quoted_object_full_name(table) + " WRITE;\n");
}

void Sql_formatter::format_database_start(
    Database_start_dump_task *database_definition_dump_task) {
  Database *database = database_definition_dump_task->get_related_database();
  if (m_options->m_drop_database)
    this->append_output("DROP DATABASE IF EXISTS " +
                        this->quote_name(database->get_name()) + ";\n");
  if (!m_options->m_suppress_create_database)
    this->append_output(database->get_sql_formatted_definition() + ";\n");
}

void Sql_formatter::format_dump_end(Dump_end_dump_task *) {
  std::ostringstream out;
  std::time_t sys_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  // Convert to calendar time. time_string ends with '\n'.
  std::string time_string = std::ctime(&sys_time);

  if (m_options->m_timezone_consistent)
    out << "SET TIME_ZONE=@OLD_TIME_ZONE;\n";
  if (m_options->m_charsets_consistent)
    out << "SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT;\n"
           "SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS;\n"
           "SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION;\n";
  out << "SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;\n"
         "SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;\n"
         "SET SQL_MODE=@OLD_SQL_MODE;\n";
  if (m_options->m_innodb_stats_tables_included)
    out << "SET GLOBAL INNODB_STATS_AUTO_RECALC="
        << "@OLD_INNODB_STATS_AUTO_RECALC;\n";
  out << "-- Dump end time: " << time_string;

  this->append_output(out.str());
}

void Sql_formatter::format_dump_start(
    Dump_start_dump_task *dump_start_dump_task) {
  // Convert to system time.
  std::time_t sys_time =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  // Convert to calendar time. time_string ends with '\n'.
  std::string time_string = std::ctime(&sys_time);

  std::ostringstream out;
  out << "-- Dump created by MySQL pump utility, version: " MYSQL_SERVER_VERSION
         ", " SYSTEM_TYPE " (" MACHINE_TYPE ")\n"
      << "-- Dump start time: " << time_string
      << "-- Server version: " << this->get_server_version_string() << "\n\n"
      << "SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;\n"
         "SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, "
         "FOREIGN_KEY_CHECKS=0;\n"
      << "SET @OLD_SQL_MODE=@@SQL_MODE;\n"
         "SET SQL_MODE=\"NO_AUTO_VALUE_ON_ZERO\";\n";

  /* disable binlog */
  out << "SET @@SESSION.SQL_LOG_BIN= 0;\n";

  if (m_options->m_timezone_consistent)
    out << "SET @OLD_TIME_ZONE=@@TIME_ZONE;\n"
           "SET TIME_ZONE='+00:00';\n";
  if (m_options->m_charsets_consistent)
    out << "SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT;\n"
           "SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS;\n"
           "SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION;\n"
           "SET NAMES "
        << this->get_charset()->csname << ";\n";

  if (m_options->m_innodb_stats_tables_included)
    out << "SET @OLD_INNODB_STATS_AUTO_RECALC="
        << "@@INNODB_STATS_AUTO_RECALC;\n"
        << "SET GLOBAL INNODB_STATS_AUTO_RECALC=OFF;\n";

  if (dump_start_dump_task->m_gtid_mode == "OFF" &&
      m_options->m_gtid_purged == enum_gtid_purged_mode::GTID_PURGED_ON) {
    m_options->m_mysql_chain_element_options->get_program()->error(
        Mysql::Tools::Base::Message_data(
            1, "Server has GTIDs disabled.\n",
            Mysql::Tools::Base::Message_type_error));
    return;
  }
  if (dump_start_dump_task->m_gtid_mode != "OFF") {
    if (m_options->m_gtid_purged == enum_gtid_purged_mode::GTID_PURGED_ON ||
        m_options->m_gtid_purged == enum_gtid_purged_mode::GTID_PURGED_AUTO) {
      if (!m_mysqldump_tool_options->m_dump_all_databases &&
          m_options->m_gtid_purged == enum_gtid_purged_mode::GTID_PURGED_AUTO) {
        m_options->m_mysql_chain_element_options->get_program()->error(
            Mysql::Tools::Base::Message_data(
                1,
                "A partial dump from a server that is using GTID-based "
                "replication "
                "requires the --set-gtid-purged=[ON|OFF] option to be "
                "specified. Use ON "
                "if the intention is to deploy a new replication slave using "
                "only some "
                "of the data from the dumped server. Use OFF if the intention "
                "is to "
                "repair a table by copying it within a topology, and use OFF "
                "if the "
                "intention is to copy a table between replication topologies "
                "that are "
                "disjoint and will remain so.\n",
                Mysql::Tools::Base::Message_type_error));
        return;
      }
      std::string gtid_output("SET @@GLOBAL.GTID_PURGED=/*!80000 '+'*/ '");
      gtid_output += (dump_start_dump_task->m_gtid_executed + "';\n");
      out << gtid_output;
    }
  }

  this->append_output(out.str());
}

void Sql_formatter::format_plain_sql_object(
    Abstract_plain_sql_object_dump_task *plain_sql_dump_task) {
  View *new_view_task = dynamic_cast<View *>(plain_sql_dump_task);
  if (new_view_task != nullptr) {
    /*
     DROP VIEW statement followed by CREATE VIEW must be written to output
     as an atomic operation, else there is a possibility of bug#21399236.
     It happens when we DROP VIEW v1, and it uses column from view v2, which
     might get dropped before creation of real v1 view, and thus result in
     error during restore.
   */
    format_sql_objects_definer(plain_sql_dump_task, "VIEW");
    this->append_output(
        "DROP VIEW IF EXISTS " +
        this->get_quoted_object_full_name(new_view_task) + ";\n" +
        plain_sql_dump_task->get_sql_formatted_definition() + ";\n");
    return;
  }

  Mysql_function *new_func_task =
      dynamic_cast<Mysql_function *>(plain_sql_dump_task);
  if (new_func_task != nullptr)
    format_sql_objects_definer(plain_sql_dump_task, "FUNCTION");

  Stored_procedure *new_proc_task =
      dynamic_cast<Stored_procedure *>(plain_sql_dump_task);
  if (new_proc_task != nullptr)
    format_sql_objects_definer(plain_sql_dump_task, "PROCEDURE");

  Privilege *new_priv_task = dynamic_cast<Privilege *>(plain_sql_dump_task);
  if (new_priv_task != nullptr) {
    if (m_options->m_drop_user)
      this->append_output(
          "DROP USER " +
          (dynamic_cast<Abstract_data_object *>(new_priv_task))->get_name() +
          ";\n");
  }

  Column_statistic *new_col_stats_task =
      dynamic_cast<Column_statistic *>(plain_sql_dump_task);
  if (new_col_stats_task != nullptr) {
    if (m_options->m_column_statistics)
      this->append_output(plain_sql_dump_task->get_sql_formatted_definition() +
                          ";\n");
    return;
  }

  this->append_output(plain_sql_dump_task->get_sql_formatted_definition() +
                      ";\n");
}

void Sql_formatter::format_sql_objects_definer(
    Abstract_plain_sql_object_dump_task *plain_sql_dump_task,
    std::string object_type) {
  if (m_options->m_skip_definer) {
    std::istringstream ddl_stream(
        plain_sql_dump_task->get_sql_formatted_definition());
    std::string new_sql_stmt;
    bool is_replaced = false;
    for (std::string object_sql; std::getline(ddl_stream, object_sql);) {
      size_t object_pos = object_sql.find(object_type);
      size_t definer_pos = object_sql.find("DEFINER");
      if (object_pos != std::string::npos && definer_pos != std::string::npos &&
          definer_pos <= object_pos && !is_replaced) {
        object_sql.replace(definer_pos, (object_pos - definer_pos), "");
        new_sql_stmt += object_sql + "\n";
        is_replaced = true;
      } else
        new_sql_stmt += object_sql + "\n";
    }
    plain_sql_dump_task->set_sql_formatted_definition(new_sql_stmt);
  }
}

/**
  Check if the table is innodb stats table in mysql database.

  @param [in] db           Database name
  @param [in] table        Table name

  @retval true if it is innodb stats table else false
*/
bool Sql_formatter::innodb_stats_tables(std::string db, std::string table) {
  return ((db == "mysql") &&
          ((table == "innodb_table_stats") || (table == "innodb_index_stats")));
}

void Sql_formatter::format_object(Item_processing_data *item_to_process) {
  this->object_processing_starts(item_to_process);

  // format_row_group is placed first, as it is most occurring task.
  if (this->try_process_task<Row_group_dump_task>(
          item_to_process, &Sql_formatter::format_row_group) ||
      this->try_process_task<Table_definition_dump_task>(
          item_to_process, &Sql_formatter::format_table_definition) ||
      this->try_process_task<Table_deferred_indexes_dump_task>(
          item_to_process, &Sql_formatter::format_table_indexes) ||
      this->try_process_task<Dump_start_dump_task>(
          item_to_process, &Sql_formatter::format_dump_start) ||
      this->try_process_task<Dump_end_dump_task>(
          item_to_process, &Sql_formatter::format_dump_end) ||
      this->try_process_task<Database_start_dump_task>(
          item_to_process, &Sql_formatter::format_database_start)
      /*
        Abstract_plain_sql_object_dump_task must be last, as so of above derive
        from it too.
        */
      || this->try_process_task<Abstract_plain_sql_object_dump_task>(
             item_to_process, &Sql_formatter::format_plain_sql_object)) {
    // Item was processed. No further action required.
  }

  this->object_processing_ends(item_to_process);

  return;
}

Sql_formatter::Sql_formatter(
    I_connection_provider *connection_provider,
    std::function<bool(const Mysql::Tools::Base::Message_data &)>
        *message_handler,
    Simple_id_generator *object_id_generator,
    const Mysqldump_tool_chain_maker_options *mysqldump_tool_options,
    const Sql_formatter_options *options)
    : Abstract_output_writer_wrapper(message_handler, object_id_generator),
      Abstract_mysql_chain_element_extension(
          connection_provider, message_handler,
          options->m_mysql_chain_element_options),
      m_mysqldump_tool_options(mysqldump_tool_options),
      m_options(options) {
  m_escaping_runner = this->get_runner();
}

Sql_formatter::~Sql_formatter() {
  if (m_escaping_runner) delete m_escaping_runner;
}
