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

#ifndef ITEM_PROCESSING_DATA_INCLUDED
#define ITEM_PROCESSING_DATA_INCLUDED

#include <atomic>
#include <functional>

#include "client/dump/chain_data.h"
#include "client/dump/i_chain_element.h"
#include "client/dump/i_dump_task.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Data structure for objects that are processed in any chain.
 */
class Item_processing_data {
 public:
  Item_processing_data(
      Chain_data *chain_data, I_dump_task *process_task_object,
      I_chain_element *chain_element,
      const std::function<void(Item_processing_data *)> *completion_callback,
      Item_processing_data *parent_item_data);

  ~Item_processing_data();

  /**
    Returns chain data in which this item is being processed.
   */
  Chain_data *get_chain() const;

  Item_processing_data *get_parent_item_data() const;

  void start_processing();

  bool end_processing();

  I_dump_task *get_process_task_object() const;

  I_chain_element *get_processing_chain_element() const;

  bool have_completion_callback();

  bool had_chain_created() const;

  void set_had_chain_created();

  void set_chain(Chain_data *);

  bool call_completion_callback_at_end();

 private:
  /**
    Chain in which current processing item is processed.
   */
  Chain_data *m_chain_data;
  /**
    Instance of task object that is being processing.
   */
  I_dump_task *m_process_task_object;
  /**
    Instance of chain element that is processing specified element.
   */
  I_chain_element *m_chain_element;
  /**
    Callback to call after element is fully processed to the output. Can be
    NULL.
   */
  const std::function<void(Item_processing_data *)> *m_completion_callback;
  /**
    Link to item process information of parent module execution, if exists.
   */
  Item_processing_data *m_parent_item_data;
  /**
    Number of modules that have pending or are executing this task.
   */
  std::atomic<uint32_t> m_active_executions;
  /**
    Indicates if this item led to creation of at least one new chain.
  */
  bool m_had_chain_created;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
