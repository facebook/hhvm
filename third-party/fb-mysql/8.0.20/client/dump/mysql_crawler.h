/*
  Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_CRAWLER_INCLUDED
#define MYSQL_CRAWLER_INCLUDED

#include <functional>

#include "client/base/abstract_program.h"
#include "client/base/message_data.h"
#include "client/dump/abstract_crawler.h"
#include "client/dump/abstract_dump_task.h"
#include "client/dump/abstract_mysql_chain_element_extension.h"
#include "client/dump/database.h"
#include "client/dump/database_end_dump_task.h"
#include "client/dump/database_start_dump_task.h"
#include "client/dump/dump_end_dump_task.h"
#include "client/dump/dump_start_dump_task.h"
#include "client/dump/i_connection_provider.h"
#include "client/dump/mysql_chain_element_options.h"
#include "client/dump/simple_id_generator.h"
#include "client/dump/table.h"
#include "client/dump/tables_definition_ready_dump_task.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Searches DB objects using connection to MYSQL server.
 */
class Mysql_crawler : public Abstract_crawler,
                      public Abstract_mysql_chain_element_extension {
 public:
  Mysql_crawler(I_connection_provider *connection_provider,
                std::function<bool(const Mysql::Tools::Base::Message_data &)>
                    *message_handler,
                Simple_id_generator *object_id_generator,
                Mysql_chain_element_options *options,
                Mysql::Tools::Base::Abstract_program *program);
  /**
    Enumerates all objects it can access, gets chains from all registered
    chain_maker for each object and then execute each chain.
   */
  virtual void enumerate_objects();

  // Fix "inherits ... via dominance" warnings
  void register_progress_watcher(I_progress_watcher *new_progress_watcher) {
    Abstract_crawler::register_progress_watcher(new_progress_watcher);
  }

  // Fix "inherits ... via dominance" warnings
  uint64 get_id() const { return Abstract_crawler::get_id(); }

 protected:
  // Fix "inherits ... via dominance" warnings
  void item_completion_in_child_callback(Item_processing_data *item_processed) {
    Abstract_crawler::item_completion_in_child_callback(item_processed);
  }

 private:
  void enumerate_database_objects(const Database &db);

  void enumerate_tables(const Database &db);

  void enumerate_table_triggers(const Table &table,
                                Abstract_dump_task *dependency);

  void enumerate_column_statistics(const Table &table,
                                   Abstract_dump_task *dependency);

  void enumerate_views(const Database &db);

  template <typename TObject>
  void enumerate_functions(const Database &db, std::string type);

  void enumerate_event_scheduler_events(const Database &db);

  void enumerate_users();

  /**
    Rewrite statement, enclosing it with version specific comment and with
    DEFINER clause enclosed in version-specific comment.

    This function parses any CREATE statement and encloses DEFINER-clause in
    version-specific comment:
      input query:     CREATE DEFINER=a@b FUNCTION ...
      rewritten query: / *!50003 CREATE * / / *!50020 DEFINER=a@b * / / *!50003
      FUNCTION ... * /
   */
  std::string get_version_specific_statement(std::string create_string,
                                             const std::string &keyword,
                                             std::string main_version,
                                             std::string definer_version);

  Dump_start_dump_task *m_dump_start_task;
  Dump_end_dump_task *m_dump_end_task;
  Database_start_dump_task *m_current_database_start_dump_task;
  Database_end_dump_task *m_current_database_end_dump_task;
  Tables_definition_ready_dump_task *m_tables_definition_ready_dump_task;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
