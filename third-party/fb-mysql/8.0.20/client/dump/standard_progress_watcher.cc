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

#include "client/dump/standard_progress_watcher.h"

#include <functional>

using namespace Mysql::Tools::Dump;

void Standard_progress_watcher::process_progress_step(
    Abstract_progress_watcher::Progress_data &) {
  std::cerr << "Dump progress: " << m_last_progress.m_table_count << "/"
            << m_total.m_table_count << " tables, "
            << m_last_progress.m_row_count << "/" << m_total.m_row_count
            << " rows" << std::endl;
}

Standard_progress_watcher::Standard_progress_watcher(
    std::function<bool(const Mysql::Tools::Base::Message_data &)>
        *message_handler,
    Simple_id_generator *object_id_generator)
    : Abstract_progress_watcher(message_handler, object_id_generator) {}
