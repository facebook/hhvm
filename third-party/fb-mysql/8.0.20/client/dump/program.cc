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

#include "client/dump/program.h"

#include <errno.h>
#include <chrono>
#include <functional>

#include "client/dump/i_chain_maker.h"
#include "client/dump/i_connection_provider.h"
#include "client/dump/i_crawler.h"
#include "client/dump/i_progress_watcher.h"
#include "client/dump/mysql_crawler.h"
#include "client/dump/mysqldump_tool_chain_maker.h"
#include "client/dump/simple_id_generator.h"
#include "client/dump/single_transaction_connection_provider.h"
#include "client/dump/standard_progress_watcher.h"
#include "client/dump/thread_specific_connection_provider.h"

using namespace Mysql::Tools::Dump;
using std::placeholders::_1;

void Program::close_redirected_stderr() {
  if (m_stderr != nullptr) fclose(m_stderr);
}

void Program::error_log_file_callback(char *) {
  if (!m_error_log_file.has_value()) return;
  this->close_redirected_stderr();
  m_stderr = freopen(m_error_log_file.value().c_str(), "a", stderr);
  if (m_stderr == nullptr) {
    this->error(Mysql::Tools::Base::Message_data(
        errno,
        "Cannot append error log to specified file: \"" +
            m_error_log_file.value() + "\"",
        Mysql::Tools::Base::Message_type_error));
  }
}

bool Program::message_handler(const Mysql::Tools::Base::Message_data &message) {
  this->error(message);
  return false;
}

void Program::error(const Mysql::Tools::Base::Message_data &message) {
  std::cerr << this->get_name() << ": [" << message.get_message_type_string()
            << "] (" << message.get_code() << ") " << message.get_message()
            << std::endl;

  if (message.get_message_type() == Mysql::Tools::Base::Message_type_error) {
    std::cerr << "Dump process encountered error and will not continue."
              << std::endl;
    m_error_code.store((int)message.get_code());
  }
}

void Program::create_options() {
  this->create_new_option(&m_error_log_file, "log-error-file",
                          "Append warnings and errors to specified file.")
      ->add_callback(new std::function<void(char *)>(
          std::bind(&Program::error_log_file_callback, this, _1)));
  this->create_new_option(
          &m_watch_progress, "watch-progress",
          "Shows periodically dump process progress information on error "
          "output. "
          "Progress information include both completed and total number of "
          "tables, rows and other objects collected.")
      ->set_value(true);
  this->create_new_option(
      &m_single_transaction, "single-transaction",
      "Creates a consistent snapshot by dumping all tables in a single "
      "transaction. Works ONLY for tables stored in storage engines which "
      "support multiversioning (currently only InnoDB does); the dump is NOT "
      "guaranteed to be consistent for other storage engines. "
      "While a --single-transaction dump is in process, to ensure a valid "
      "dump file (correct table contents and binary log position), no other "
      "connection should use the following statements: ALTER TABLE, DROP "
      "TABLE, RENAME TABLE, TRUNCATE TABLE, as consistent snapshot is not "
      "isolated from them. This option is mutually exclusive with "
      "--add-locks option.");
}

void Program::check_mutually_exclusive_options() {
  /*
    In case of --add-locks we dont allow parallelism
  */
  if (m_mysqldump_tool_chain_maker_options->m_default_parallelism ||
      m_mysqldump_tool_chain_maker_options
          ->get_parallel_schemas_thread_count()) {
    if (m_mysqldump_tool_chain_maker_options->m_formatter_options->m_add_locks)
      m_mysql_chain_element_options->get_program()->error(
          Mysql::Tools::Base::Message_data(
              1,
              "Usage of --add-locks "
              "is mutually exclusive with parallelism.",
              Mysql::Tools::Base::Message_type_error));
  }
}

int Program::get_total_connections() {
  /*
    total thread count for mysqlpump would be as below:
     1 main thread +
     default queues thread (specified by default parallelism) +
     total parallel-schemas without threads specified * dp +
     total threads mentioned in parallel-schemas
  */

  int dp = m_mysqldump_tool_chain_maker_options->m_default_parallelism;
  return (1 + dp +
          m_mysqldump_tool_chain_maker_options
              ->get_parallel_schemas_thread_count() +
          (m_mysqldump_tool_chain_maker_options
               ->get_parallel_schemas_with_default_thread_count() *
           dp));
}

int Program::get_error_code() { return m_error_code.load(); }

int Program::execute(const std::vector<std::string> &positional_options) {
  I_connection_provider *connection_provider = nullptr;
  int num_connections = get_total_connections();

  std::function<bool(const Mysql::Tools::Base::Message_data &)>
      *message_handler =
          new std::function<bool(const Mysql::Tools::Base::Message_data &)>(
              std::bind(&Program::message_handler, this, _1));

  try {
    connection_provider = m_single_transaction
                              ? new Single_transaction_connection_provider(
                                    this, num_connections, message_handler)
                              : new Thread_specific_connection_provider(this);
  } catch (const std::exception &e) {
    this->error(Mysql::Tools::Base::Message_data(
        0, "Error during creating connection.",
        Mysql::Tools::Base::Message_type_error));
  }

  Mysql::Tools::Base::Mysql_query_runner *runner =
      connection_provider->get_runner(message_handler);
  if (!runner) {
    delete message_handler;
    delete connection_provider;
    return 0;
  }

  ulong server_version =
      mysql_get_server_version(runner->get_low_level_connection());
  if (server_version < 50646) {
    std::cerr << "Server version is not compatible. Server version should "
                 "be 5.6.46 or above.";
    delete runner;
    delete message_handler;
    delete connection_provider;
    return 0;
  }
  use_show_create_user = (server_version > 50705);

  Simple_id_generator *id_generator = new Simple_id_generator();

  std::chrono::high_resolution_clock::time_point start_time =
      std::chrono::high_resolution_clock::now();

  I_progress_watcher *progress_watcher = nullptr;

  if (m_watch_progress) {
    progress_watcher =
        new Standard_progress_watcher(message_handler, id_generator);
  }
  I_crawler *crawler =
      new Mysql_crawler(connection_provider, message_handler, id_generator,
                        m_mysql_chain_element_options, this);
  m_mysqldump_tool_chain_maker_options->process_positional_options(
      positional_options);
  check_mutually_exclusive_options();
  I_chain_maker *chain_maker = new Mysqldump_tool_chain_maker(
      connection_provider, message_handler, id_generator,
      m_mysqldump_tool_chain_maker_options, this);

  crawler->register_chain_maker(chain_maker);
  if (progress_watcher != nullptr) {
    crawler->register_progress_watcher(progress_watcher);
    chain_maker->register_progress_watcher(progress_watcher);
  }

  crawler->enumerate_objects();

  delete runner;
  delete crawler;
  if (progress_watcher != nullptr) delete progress_watcher;
  delete id_generator;
  delete connection_provider;
  delete message_handler;
  delete chain_maker;

  if (!get_error_code()) {
    std::cerr << "Dump completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::high_resolution_clock::now() - start_time)
                     .count()
              << std::endl;
  }
  return get_error_code();
}

std::string Program::get_description() {
  return "MySQL utility for dumping data from databases to external file.";
}

int Program::get_first_release_year() { return 2014; }

std::string Program::get_version() { return "1.0.0"; }

Program::~Program() {
  delete m_mysql_chain_element_options;
  delete m_mysqldump_tool_chain_maker_options;
  this->close_redirected_stderr();
}

void Program::short_usage() {
  std::cout << "Usage: " << get_name() << " [OPTIONS] [--all-databases]"
            << std::endl;
  std::cout << "OR     " << get_name()
            << " [OPTIONS] --databases DB1 [DB2 DB3...]" << std::endl;
  std::cout << "OR     " << get_name() << " [OPTIONS] database [tables]"
            << std::endl;
}

Program::Program()
    : Abstract_connection_program(), m_stderr(nullptr), m_error_code(0) {
  m_mysql_chain_element_options = new Mysql_chain_element_options(this);
  m_mysqldump_tool_chain_maker_options =
      new Mysqldump_tool_chain_maker_options(m_mysql_chain_element_options);

  this->add_provider(m_mysql_chain_element_options);
  this->add_provider(m_mysqldump_tool_chain_maker_options);
}

const char *load_default_groups[] = {
    "client", /* Read settings how to connect to server. */
    /*
     Read special settings for mysql_dump.
     This section will be deprecated.
    */
    "mysql_dump",
    /* Read config options from mysqlpump section. */
    "mysqlpump", nullptr};

int main(int argc, char **argv) {
  Program program;
  program.run(argc, argv);
  return 0;
}
