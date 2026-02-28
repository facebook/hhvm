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

#ifndef ABSTRACT_PROGRESS_WATCHER_INCLUDED
#define ABSTRACT_PROGRESS_WATCHER_INCLUDED

#include <atomic>
#include <chrono>
#include <functional>

#include "client/dump/abstract_chain_element.h"
#include "client/dump/i_progress_watcher.h"
#include "my_inttypes.h"

namespace Mysql {
namespace Tools {
namespace Dump {

/**
  Gathers information about progress of current dump progress and format
  messages on progress.Also it should expose API for receiving processed
  progress information: collected objects and rows information along with time
  elapsed, ETA.
 */
class Abstract_progress_watcher : public virtual I_progress_watcher,
                                  public Abstract_chain_element {
 public:
  void new_chain_created(Item_processing_data *new_chain_process_data);

  void object_processing_started(Item_processing_data *process_data);

  void object_processing_ended(Item_processing_data *finished_process_data);

  void crawler_completed(I_crawler *crawler);

  // Fix "inherits ... via dominance" warnings
  void register_progress_watcher(I_progress_watcher *new_progress_watcher) {
    Abstract_chain_element::register_progress_watcher(new_progress_watcher);
  }

  // Fix "inherits ... via dominance" warnings
  uint64 get_id() const { return Abstract_chain_element::get_id(); }

 protected:
  Abstract_progress_watcher(
      std::function<bool(const Mysql::Tools::Base::Message_data &)>
          *message_handler,
      Simple_id_generator *object_id_generator);

  class Progress_data {
   public:
    Progress_data();
    Progress_data(const Progress_data &to_copy);
    Progress_data &operator=(const Progress_data &to_copy);
    Progress_data operator-(const Progress_data &to_subtract);
    std::atomic<uint64_t> m_table_count;
    std::atomic<uint64_t> m_row_data;
    std::atomic<uint64_t> m_row_count;
  };

  virtual void process_progress_step(Progress_data &change) = 0;

  Progress_data m_total;
  Progress_data m_progress;
  Progress_data m_last_progress;

  // Fix "inherits ... via dominance" warnings
  void item_completion_in_child_callback(Item_processing_data *item_processed) {
    Abstract_chain_element::item_completion_in_child_callback(item_processed);
  }

 private:
  /**
    Throttles progress changes to be reported to progress_changed() about 1 in
    second. It uses 10 stages, each 100ms long, in each there is number of
    iterations to prevent calling std::chrono::system_clock::now() on each
    function call.
   */
  void progress_changed();

  static const int STAGES = 10;
  static const int REPORT_DELAY_MS = 1000;

  std::chrono::system_clock::time_point m_last_stage_time;
  std::atomic<int64_t> m_step_countdown;
  std::atomic<int64_t> m_stage_countdown;
  int64 m_last_step_countdown;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
