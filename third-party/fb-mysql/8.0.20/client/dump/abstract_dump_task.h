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

#ifndef ABSTRACT_DUMP_TASK_INCLUDED
#define ABSTRACT_DUMP_TASK_INCLUDED

#include <functional>
#include <mutex>
#include <vector>

#include "client/dump/abstract_data_object.h"
#include "client/dump/abstract_simple_dump_task.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Base class for most individual dump process tasks, not suitable for
  lightweight dump tasks (e.g. Row).
*/
class Abstract_dump_task : public Abstract_simple_dump_task {
 public:
  Abstract_dump_task(Abstract_data_object *related_object);

  virtual ~Abstract_dump_task();

  I_data_object *get_related_db_object() const;

  std::vector<const Abstract_dump_task *> get_dependencies() const;

  std::vector<Abstract_dump_task *> get_dependents() const;

  void add_dependency(Abstract_dump_task *dependency);

  bool can_be_executed() const;

  void set_completed();

  /**
    Registers callback to be called once this task is able to be executed.
   */
  void register_execution_availability_callback(
      std::function<void(const Abstract_dump_task *)> *availability_callback);

 private:
  void check_execution_availability();

  Abstract_data_object *m_related_object;
  std::vector<const Abstract_dump_task *> m_dependencies;
  std::vector<Abstract_dump_task *> m_dependents;
  std::vector<std::function<void(const Abstract_dump_task *)> *>
      m_availability_callbacks;
  std::mutex m_task_mutex;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
