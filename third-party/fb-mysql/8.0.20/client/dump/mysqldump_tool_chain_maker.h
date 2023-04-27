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

#ifndef MYSQLDUMP_TOOL_CHAIN_MAKER_INCLUDED
#define MYSQLDUMP_TOOL_CHAIN_MAKER_INCLUDED

#include <functional>
#include <map>
#include <vector>

#include "client/dump/abstract_chain_element.h"
#include "client/dump/abstract_mysql_chain_element_extension.h"
#include "client/dump/chain_data.h"
#include "client/dump/i_chain_maker.h"
#include "client/dump/i_dump_task.h"
#include "client/dump/i_object_reader.h"
#include "client/dump/mysql_object_reader.h"
#include "client/dump/mysqldump_tool_chain_maker_options.h"
#include "client/dump/object_queue.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Chain maker implemented in Mysql_dump application, constructs chain based on
  command line options that are compatible with these available in previous
  implementation.
 */
class Mysqldump_tool_chain_maker
    : public I_chain_maker,
      public Abstract_chain_element,
      public Abstract_mysql_chain_element_extension {
 public:
  Mysqldump_tool_chain_maker(
      I_connection_provider *connection_provider,
      std::function<bool(const Mysql::Tools::Base::Message_data &)>
          *message_handler,
      Simple_id_generator *object_id_generator,
      Mysqldump_tool_chain_maker_options *options,
      Mysql::Tools::Base::Abstract_program *program);

  ~Mysqldump_tool_chain_maker();

  I_object_reader *create_chain(Chain_data *chain_data, I_dump_task *dump_task);

  void delete_chain(uint64 chain_id, I_object_reader *chain);

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
  void mysql_thread_callback(bool is_starting);

  void stop_queues();

  Mysqldump_tool_chain_maker_options *m_options;

  Mysql_object_reader *m_main_object_reader;
  std::map<int, Object_queue *> m_object_queues;
  std::vector<I_chain_element *> m_all_created_elements;
  Mysql::Tools::Base::Abstract_program *m_program;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
