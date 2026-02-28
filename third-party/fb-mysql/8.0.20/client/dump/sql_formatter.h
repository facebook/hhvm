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

#ifndef SQL_FORMATTER_INCLUDED
#define SQL_FORMATTER_INCLUDED

#include <functional>

#include "client/dump/abstract_mysql_chain_element_extension.h"
#include "client/dump/abstract_output_writer_wrapper.h"
#include "client/dump/abstract_plain_sql_object_dump_task.h"
#include "client/dump/database_end_dump_task.h"
#include "client/dump/database_start_dump_task.h"
#include "client/dump/dump_end_dump_task.h"
#include "client/dump/dump_start_dump_task.h"
#include "client/dump/i_data_formatter.h"
#include "client/dump/mysqldump_tool_chain_maker_options.h"
#include "client/dump/row_group_dump_task.h"
#include "client/dump/sql_formatter_options.h"
#include "client/dump/table_deferred_indexes_dump_task.h"
#include "client/dump/table_definition_dump_task.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Prints object data in SQL format.
 */
class Sql_formatter : public Abstract_output_writer_wrapper,
                      public Abstract_mysql_chain_element_extension,
                      public virtual I_data_formatter {
 public:
  Sql_formatter(
      I_connection_provider *connection_provider,
      std::function<bool(const Mysql::Tools::Base::Message_data &)>
          *message_handler,
      Simple_id_generator *object_id_generator,
      const Mysqldump_tool_chain_maker_options *mysqldump_tool_options,
      const Sql_formatter_options *options);

  ~Sql_formatter();

  /**
    Creates string representation for output of DB object related to specified
    dump task object.
   */
  void format_object(Item_processing_data *item_to_process);

  // Fix "inherits ... via dominance" warnings
  void register_progress_watcher(I_progress_watcher *new_progress_watcher) {
    Abstract_chain_element::register_progress_watcher(new_progress_watcher);
  }

  // Fix "inherits ... via dominance" warnings
  uint64 get_id() const { return Abstract_chain_element::get_id(); }

 protected:
  // Fix "inherits ... via dominance" warnings
  void item_completion_in_child_callback(Item_processing_data *item_processed) {
    Abstract_chain_element::item_completion_in_child_callback(item_processed);
  }

 private:
  void format_plain_sql_object(
      Abstract_plain_sql_object_dump_task *plain_sql_dump_task);

  void format_dump_start(Dump_start_dump_task *dump_start_dump_task);

  void format_dump_end(Dump_end_dump_task *dump_start_dump_task);

  void format_database_start(
      Database_start_dump_task *database_definition_dump_task);

  void format_table_definition(
      Table_definition_dump_task *table_definition_dump_task);

  void format_table_indexes(
      Table_deferred_indexes_dump_task *table_indexes_dump_task);

  void format_row_group(Row_group_dump_task *row_group);

  void format_sql_objects_definer(Abstract_plain_sql_object_dump_task *,
                                  std::string);

  bool innodb_stats_tables(std::string db, std::string table);

  Mysql::Tools::Base::Mysql_query_runner *m_escaping_runner;
  const Mysqldump_tool_chain_maker_options *m_mysqldump_tool_options;
  const Sql_formatter_options *m_options;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
