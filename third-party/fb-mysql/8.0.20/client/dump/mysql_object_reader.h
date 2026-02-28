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

#ifndef MYSQL_OBJECT_READER_INCLUDED
#define MYSQL_OBJECT_READER_INCLUDED

#include <functional>

#include "client/dump/abstract_data_formatter_wrapper.h"
#include "client/dump/abstract_mysql_chain_element_extension.h"
#include "client/dump/i_object_reader.h"
#include "client/dump/mysql_field.h"
#include "client/dump/mysql_object_reader_options.h"
#include "client/dump/row_group_dump_task.h"
#include "client/dump/table_rows_dump_task.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Parses any DB object(excluding rows and privileges for DB objects) data using
  connection to MySQL server.
 */
class Mysql_object_reader : public Abstract_data_formatter_wrapper,
                            public I_object_reader,
                            public Abstract_mysql_chain_element_extension {
 public:
  Mysql_object_reader(
      I_connection_provider *connection_provider,
      std::function<bool(const Mysql::Tools::Base::Message_data &)>
          *message_handler,
      Simple_id_generator *object_id_generator,
      const Mysql_object_reader_options *options);

  void read_object(Item_processing_data *item_to_process);

  void format_rows(Item_processing_data *item_to_process,
                   Row_group_dump_task *row_group);

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
  void read_table_rows_task(Table_rows_dump_task *table_rows_dump_task,
                            Item_processing_data *item_to_process);

  const Mysql_object_reader_options *m_options;

  class Rows_fetching_context {
   public:
    Rows_fetching_context(Mysql_object_reader *parent,
                          Item_processing_data *item_processing,
                          bool has_generated_column);

    int64 result_callback(
        const Mysql::Tools::Base::Mysql_query_runner::Row &row_data);

    void process_buffer();
    bool is_all_rows_processed();

   private:
    void acquire_fields_information(MYSQL_RES *mysql_result);

    Mysql_object_reader *m_parent;
    Item_processing_data *m_item_processing;
    Row_group_dump_task m_row_group;
    std::vector<Mysql_field> m_fields;
  };
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
