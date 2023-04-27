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

#ifndef ABSTRACT_CRAWLER_INCLUDED
#define ABSTRACT_CRAWLER_INCLUDED

#include <atomic>
#include <functional>

#include "client/base/abstract_program.h"
#include "client/dump/abstract_chain_element.h"
#include "client/dump/i_chain_maker.h"
#include "client/dump/i_crawler.h"
#include "client/dump/i_dump_task.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

class Abstract_crawler : public Abstract_chain_element,
                         public virtual I_crawler {
 public:
  /**
    Adds new Chain Maker to ask for chains for found objects.
   */
  virtual void register_chain_maker(I_chain_maker *new_chain_maker);

  // Fix "inherits ... via dominance" warnings
  void register_progress_watcher(I_progress_watcher *new_progress_watcher) {
    Abstract_chain_element::register_progress_watcher(new_progress_watcher);
  }

  // Fix "inherits ... via dominance" warnings
  uint64 get_id() const { return Abstract_chain_element::get_id(); }

  ~Abstract_crawler();

 protected:
  Abstract_crawler(std::function<bool(const Mysql::Tools::Base::Message_data &)>
                       *message_handler,
                   Simple_id_generator *object_id_generator,
                   Mysql::Tools::Base::Abstract_program *program);
  /**
    Routine for performing common work on each enumerated DB object.
   */
  void process_dump_task(I_dump_task *new_dump_task);

  void wait_for_tasks_completion();

  bool need_callbacks_in_child();

  // Fix "inherits ... via dominance" warnings
  void item_completion_in_child_callback(Item_processing_data *item_processed) {
    Abstract_chain_element::item_completion_in_child_callback(item_processed);
  }

  Mysql::Tools::Base::Abstract_program *get_program();

 private:
  std::vector<I_chain_maker *> m_chain_makers;
  std::vector<I_dump_task *> m_dump_tasks_created;
  /**
    Stores next chain ID to be used. Used as ID generator.
   */
  static std::atomic<uint64_t> next_chain_id;
  Mysql::Tools::Base::Abstract_program *m_program;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
