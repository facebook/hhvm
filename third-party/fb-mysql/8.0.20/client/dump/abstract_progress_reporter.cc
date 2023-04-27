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

#include "client/dump/abstract_progress_reporter.h"

#include "client/dump/i_progress_watcher.h"

using namespace Mysql::Tools::Dump;

void Abstract_progress_reporter::register_progress_watcher(
    I_progress_watcher *new_progress_watcher) {
  m_progress_watchers.push_back(new_progress_watcher);
}

bool Abstract_progress_reporter::have_progress_watcher() {
  return m_progress_watchers.size() > 0;
}

void Abstract_progress_reporter::report_new_chain_created(
    Item_processing_data *new_chain_creator) {
  for (std::vector<I_progress_watcher *>::iterator it =
           m_progress_watchers.begin();
       it != m_progress_watchers.end(); ++it) {
    (*it)->new_chain_created(new_chain_creator);
  }
}

void Abstract_progress_reporter::report_object_processing_started(
    Item_processing_data *process_data) {
  for (std::vector<I_progress_watcher *>::iterator it =
           m_progress_watchers.begin();
       it != m_progress_watchers.end(); ++it) {
    (*it)->object_processing_started(process_data);
  }
}

void Abstract_progress_reporter::report_object_processing_ended(
    Item_processing_data *finished_process_data) {
  for (std::vector<I_progress_watcher *>::iterator it =
           m_progress_watchers.begin();
       it != m_progress_watchers.end(); ++it) {
    (*it)->object_processing_ended(finished_process_data);
  }
}

void Abstract_progress_reporter::report_crawler_completed(I_crawler *crawler) {
  for (std::vector<I_progress_watcher *>::iterator it =
           m_progress_watchers.begin();
       it != m_progress_watchers.end(); ++it) {
    (*it)->crawler_completed(crawler);
  }
}

void Abstract_progress_reporter::register_progress_watchers_in_child(
    I_progress_reporter *reporter) {
  for (std::vector<I_progress_watcher *>::iterator it =
           m_progress_watchers.begin();
       it != m_progress_watchers.end(); ++it) {
    reporter->register_progress_watcher(*it);
  }
}
