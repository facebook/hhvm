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

#ifndef ABSTRACT_CHAIN_ELEMENT_INCLUDED
#define ABSTRACT_CHAIN_ELEMENT_INCLUDED

#include <stddef.h>
#include <atomic>
#include <functional>

#include "client/base/message_data.h"
#include "client/dump/abstract_progress_reporter.h"
#include "client/dump/i_chain_element.h"
#include "client/dump/item_processing_data.h"
#include "client/dump/simple_id_generator.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

class Abstract_chain_element : public virtual I_chain_element,
                               public virtual Abstract_progress_reporter {
 public:
  /**
    Returns an application unique ID of this chain element object. This helps
    progress watching with multiple parts of chain during all objects
    processing.
   */
  uint64 get_id() const;

  /** Disable move assignment to avoid Wvirtual-move-assign warning */
  Abstract_chain_element &operator=(Abstract_chain_element &&other) = delete;

  // Fix "inherits ... via dominance" warnings
  void register_progress_watcher(I_progress_watcher *new_progress_watcher) {
    Abstract_progress_reporter::register_progress_watcher(new_progress_watcher);
  }

 protected:
  Abstract_chain_element(
      std::function<bool(const Mysql::Tools::Base::Message_data &)>
          *message_handler,
      Simple_id_generator *object_id_generator);

  /**
    Process task object with specified function if that task object can be
    casted to type TType. Returns true if object was processed.
   */
  template <typename TType, typename TClass>
  bool try_process_task(
      Item_processing_data *item_to_process,
      void (TClass::*processing_func)(TType *, Item_processing_data *)) {
    TType *casted_object =
        dynamic_cast<TType *>(item_to_process->get_process_task_object());
    if (casted_object != nullptr)
      (((TClass *)this)->*processing_func)(casted_object, item_to_process);
    return casted_object != nullptr;
  }

  /**
    Process task object with specified function if that task object can be
    casted to type TType. Returns true if object was processed.
   */
  template <typename TType, typename TClass>
  bool try_process_task(Item_processing_data *item_to_process,
                        void (TClass::*processing_func)(TType *)) {
    TType *casted_object =
        dynamic_cast<TType *>(item_to_process->get_process_task_object());
    if (casted_object != nullptr)
      (((TClass *)this)->*processing_func)(casted_object);
    return casted_object != nullptr;
  }

  void object_processing_starts(Item_processing_data *item_to_process);

  Item_processing_data *object_to_be_processed_in_child(
      Item_processing_data *current_item_data,
      I_chain_element *child_chain_element);

  Item_processing_data *new_task_created(I_dump_task *dump_task_created);

  Item_processing_data *new_chain_created(
      Chain_data *new_chain_data, Item_processing_data *parent_processing_data,
      I_chain_element *child_chain_element);

  Item_processing_data *new_chain_created(
      Item_processing_data *current_item_data, I_dump_task *dump_task_created);

  void object_processing_ends(Item_processing_data *processed_item);

  uint64 generate_new_object_id();

  Simple_id_generator *get_object_id_generator() const;

  /**
    Passes message to message callback.
   */
  void pass_message(const Mysql::Tools::Base::Message_data &message_data);

  std::function<bool(const Mysql::Tools::Base::Message_data &)>
      *get_message_handler() const;

 protected:
  virtual bool need_callbacks_in_child();

  // Must be protected to allow subclasses to call explicitly.
  void item_completion_in_child_callback(Item_processing_data *item_processed);

 private:
  /**
    Wrapper on item_completion_in_child_callback which allows creation of
    pointer to function which will then fetch correct virtual method pointer.
   */
  void item_completion_in_child_callback_wrapper(
      Item_processing_data *item_processed);
  /**
    Wrapper on item_completion_in_child_callback_wrapper which also sets
    precessed task to be fully completed.
   */
  void item_completion_in_child_completes_task_callback(
      Item_processing_data *item_processed);

  Item_processing_data *task_to_be_processed_in_child(
      Item_processing_data *current_item_data,
      I_chain_element *child_chain_element, I_dump_task *task_to_be_processed,
      std::function<void(Item_processing_data *)> *callback);

  uint64 m_id;
  std::function<bool(const Mysql::Tools::Base::Message_data &)>
      *m_message_handler;
  std::function<void(Item_processing_data *)> m_item_processed_callback;
  std::function<void(Item_processing_data *)>
      m_item_processed_complete_callback;
  Simple_id_generator *m_object_id_generator;

  /**
    Stores next chain element ID to be used. Used as ID generator.
   */
  static std::atomic<uint64_t> next_id;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
