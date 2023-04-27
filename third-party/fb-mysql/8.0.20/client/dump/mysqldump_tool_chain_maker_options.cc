/*
  Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "client/dump/mysqldump_tool_chain_maker_options.h"

#include <functional>
#include <sstream>
#include <string>

#include "m_ctype.h"

using namespace Mysql::Tools::Dump;
using std::placeholders::_1;

bool Mysql::Tools::Dump::use_show_create_user;

void Mysqldump_tool_chain_maker_options::parallel_schemas_callback(char *) {
  std::vector<std::string> schemas;
  std::istringstream schema_stream(m_parallel_schemas_string.value());
  for (std::string schema; std::getline(schema_stream, schema, ',');)
    schemas.push_back(schema);
  if (schemas.size() == 0) return;
  int threads = 0;
  std::string &first_part = schemas[0];
  size_t i;
  for (i = 0;
       i < first_part.size() && first_part[i] >= '0' && first_part[i] <= '9';
       ++i) {
    threads = threads * 10 + first_part[i] - '0';
  }
  if (i < first_part.size() && first_part[i] == ':') {
    first_part = first_part.substr(i + 1);
  } else {
    threads = 0;
  }
  if (threads)
    m_parallel_thread_count += threads;
  else
    m_def_thread_count++;
  m_last_parallel_schemas_queue_id++;
  m_object_queue_threads.insert(
      std::make_pair(m_last_parallel_schemas_queue_id, threads));

  for (std::vector<std::string>::iterator it = schemas.begin();
       it != schemas.end(); ++it) {
    m_database_to_object_queue_id.insert(
        std::make_pair(*it, m_last_parallel_schemas_queue_id));
  }
}

bool Mysqldump_tool_chain_maker_options::is_object_included_in_dump(
    Abstract_data_object *object) {
  return m_object_filter.is_object_included_in_dump(object);
}

int Mysqldump_tool_chain_maker_options::get_object_queue_threads_count(
    int object_queue_id) {
  std::map<int, int>::iterator it =
      m_object_queue_threads.find(object_queue_id);
  if (it != m_object_queue_threads.end() && it->second != 0) {
    return it->second;
  }
  return m_default_parallelism;
}

int Mysqldump_tool_chain_maker_options::get_object_queue_id_for_schema(
    const std::string &schema) {
  std::map<std::string, int>::iterator it =
      m_database_to_object_queue_id.find(schema);
  if (it != m_database_to_object_queue_id.end()) {
    return it->second;
  }
  return 0;
}

void Mysqldump_tool_chain_maker_options::process_positional_options(
    std::vector<std::string> positional_options) {
  if ((m_dump_all_databases ? 1 : 0) + (m_dump_selected_databases ? 1 : 0) >
      1) {
    m_mysql_chain_element_options->get_program()->error(
        Mysql::Tools::Base::Message_data(
            1,
            "Usage of --all-databases and "
            "--databases are mutually  exclusive.",
            Mysql::Tools::Base::Message_type_error));
  } else if (m_dump_all_databases ||
             (!m_dump_selected_databases && positional_options.size() == 0)) {
    if (positional_options.size() > 0) {
      m_mysql_chain_element_options->get_program()->error(
          Mysql::Tools::Base::Message_data(
              1,
              "Positional options specified, "
              "while disalowed by usage of --all-databases.",
              Mysql::Tools::Base::Message_type_error));
    }
  } else if (m_dump_selected_databases) {
    if (positional_options.size() < 1) {
      m_mysql_chain_element_options->get_program()->error(
          Mysql::Tools::Base::Message_data(
              1,
              "No positional options "
              "specified, while expected by usage of --databases.",
              Mysql::Tools::Base::Message_type_error));
    } else {
      for (int i = positional_options.size(); --i >= 0;)
        m_object_filter.m_databases_included.push_back(
            std::make_pair("", positional_options[i]));
    }
  } else {
    std::string db_name = positional_options[0];

    m_object_filter.m_databases_included.push_back(std::make_pair("", db_name));
    for (int i = positional_options.size(); --i >= 1;)
      m_object_filter.m_tables_included.push_back(
          std::make_pair(db_name, positional_options[i]));
  }

  /*
    INFORMATION_SCHEMA DB content dump is only used to reload the data
    into another tables for analysis purpose. This feature is not the
    core responsibility of mysqlpump tool. INFORMATION_SCHEMA DB
    content can even be dumped using other methods like SELECT INTO
    OUTFILE... for such purpose.
    Hence reporting error if INFORMATION_SCHEMA DB is in databases list.
  */
  for (auto database : m_object_filter.m_databases_included) {
    auto db_name = std::get<1>(database);

    if (!my_strcasecmp(&my_charset_latin1, db_name.c_str(),
                       INFORMATION_SCHEMA_DB_NAME)) {
      m_mysql_chain_element_options->get_program()->error(
          Mysql::Tools::Base::Message_data(
              1,
              "Dumping "
              "\'INFORMATION_SCHEMA\' DB content is not supported.",
              Mysql::Tools::Base::Message_type_error));
    }
  }

  /*
    We add standard exclusions only if objects are included by default, i.e.
    there are exclusions or there is no exclusions and inclusions.
  */
  if (m_object_filter.m_tables_excluded.size() > 0 ||
      m_object_filter.m_tables_included.size() == 0) {
    m_object_filter.m_tables_excluded.push_back(
        std::make_pair("mysql", "apply_status"));
    m_object_filter.m_tables_excluded.push_back(
        std::make_pair("mysql", "schema"));
    m_object_filter.m_tables_excluded.push_back(
        std::make_pair("mysql", "general_log"));
    m_object_filter.m_tables_excluded.push_back(
        std::make_pair("mysql", "slow_log"));
    m_object_filter.m_tables_excluded.push_back(
        std::make_pair("mysql", "slave_master_info"));
    m_object_filter.m_tables_excluded.push_back(
        std::make_pair("mysql", "slave_relay_log_info"));
    /*
      We filter out all the tables which store account and privilge
      information. ex: mysql.user, mysql.db, mysql.tables_priv,
      mysql.columns_priv, mysql.procs_priv, mysql.proxies_priv
      mysql.default_roles, mysql.global_grants, mysql_role_edges.
      We do not filter password_history since the password history
      will be lost this way.
    */
    if (use_show_create_user) {
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "user"));
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "db"));
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "tables_priv"));
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "columns_priv"));
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "procs_priv"));
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "proxies_priv"));
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "default_roles"));
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "global_grants"));
      m_object_filter.m_tables_excluded.push_back(
          std::make_pair("mysql", "role_edges"));
    }
  }
  if (m_object_filter.m_databases_excluded.size() > 0 ||
      m_object_filter.m_databases_included.size() == 0) {
    m_object_filter.m_databases_excluded.push_back(
        std::make_pair("", INFORMATION_SCHEMA_DB_NAME));
    m_object_filter.m_databases_excluded.push_back(
        std::make_pair("", PERFORMANCE_SCHEMA_DB_NAME_MACRO));
    m_object_filter.m_databases_excluded.push_back(
        std::make_pair("", "ndbinfo"));
    m_object_filter.m_databases_excluded.push_back(std::make_pair("", "sys"));
  }

  if (m_dump_all_databases)
    m_formatter_options->m_innodb_stats_tables_included = true;
  else if (m_dump_selected_databases) {
    for (auto database : m_object_filter.m_databases_included) {
      auto db_name = std::get<1>(database);
      if (!my_strcasecmp(&my_charset_latin1, db_name.c_str(), "mysql")) {
        m_formatter_options->m_innodb_stats_tables_included = true;
        break;
      }
    }
  }
  /* check positional options */
  else
    for (auto database : m_object_filter.m_databases_included) {
      auto db_name = std::get<1>(database);
      if (!my_strcasecmp(&my_charset_latin1, db_name.c_str(), "mysql")) {
        if (m_object_filter.m_tables_included.size() == 0)
          m_formatter_options->m_innodb_stats_tables_included = true;
        for (auto table : m_object_filter.m_tables_included) {
          auto table_name = std::get<1>(table);
          if (!my_strcasecmp(&my_charset_latin1, table_name.c_str(),
                             "innodb_table_stats") ||
              !my_strcasecmp(&my_charset_latin1, table_name.c_str(),
                             "innodb_index_stats")) {
            m_formatter_options->m_innodb_stats_tables_included = true;
            break;
          }
        }
      }
    }
}

void Mysqldump_tool_chain_maker_options::create_options() {
  this->create_new_option(
          &m_dump_all_databases, "all-databases",
          "Dump all databases. This is default behaviour if no positional "
          "options "
          "are specified. Specifying this option is mutually exclusive with "
          "--databases.")
      ->set_short_character('A');
  this->create_new_option(
          &m_dump_selected_databases, "databases",
          "Dump selected databases, specified in positional options. "
          "Specifying "
          "this option is mutually exclusive with --all-databases.")
      ->set_short_character('B');
  this->create_new_option(
          &m_parallel_schemas_string, "parallel-schemas",
          "[N:]<list of: schema_name separated with ','>. Process tables in "
          "specified schemas using separate queue handled by "
          "--default-parallelism threads or N threads, if N is specified. Can "
          "be "
          "used multiple times to specify more parallel processes.")
      ->add_callback(new std::function<void(char *)>(std::bind(
          &Mysqldump_tool_chain_maker_options::parallel_schemas_callback, this,
          _1)));
  this->create_new_option(
          &m_default_parallelism, "default-parallelism",
          "Specifies number of threads to process each parallel queue for "
          "values "
          "N > 0. if N is 0 then no queue will be used. Default value is 2. "
          "If N > 1 then objects in dump file can have lines intersected. "
          "Usage "
          "of values greater than 1 is mutually exclusive with "
          "--single-transaction.")
      ->set_value(2);
  this->create_new_option(
      &m_result_file, "result-file",
      "Direct all output generated for all objects to a given file.");
  this->create_new_option(
      &m_compress_output_algorithm, "compress-output",
      "Compresses all output files with LZ4 or ZLIB compression algorithm.");
  this->create_new_option(&m_skip_rows_data, "skip-dump-rows",
                          "Skip dumping rows of all tables to output.")
      ->set_short_character('d');
}

Mysqldump_tool_chain_maker_options::~Mysqldump_tool_chain_maker_options() {
  delete m_formatter_options;
  delete m_object_reader_options;
}

Mysqldump_tool_chain_maker_options::Mysqldump_tool_chain_maker_options(
    const Mysql_chain_element_options *mysql_chain_element_options)
    : m_mysql_chain_element_options(mysql_chain_element_options),
      m_formatter_options(
          new Sql_formatter_options(mysql_chain_element_options)),
      m_object_reader_options(
          new Mysql_object_reader_options(mysql_chain_element_options)),
      m_last_parallel_schemas_queue_id(0),
      m_def_thread_count(0),
      m_parallel_thread_count(0),
      m_object_filter(mysql_chain_element_options->get_program()) {
  this->add_provider(m_formatter_options);
  this->add_provider(m_object_reader_options);
  this->add_provider(&m_object_filter);
}

int Mysqldump_tool_chain_maker_options::
    get_parallel_schemas_with_default_thread_count() {
  return m_def_thread_count;
}

int Mysqldump_tool_chain_maker_options::get_parallel_schemas_thread_count() {
  return m_parallel_thread_count;
}
