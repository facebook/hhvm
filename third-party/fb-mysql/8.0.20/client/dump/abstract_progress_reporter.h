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

#ifndef ABSTRACT_PROGRESS_REPORTER_INCLUDED
#define ABSTRACT_PROGRESS_REPORTER_INCLUDED

#include <vector>

#include "client/dump/i_crawler.h"
#include "client/dump/i_progress_reporter.h"

namespace Mysql {
namespace Tools {
namespace Dump {

class Abstract_progress_reporter : public virtual I_progress_reporter {
 public:
  /**
    Add new Progress Watcher to report to.
  */
  void register_progress_watcher(I_progress_watcher *new_progress_watcher);

 protected:
  /**
    Specifies if have any Progress Watcher registered.
   */
  bool have_progress_watcher();

  /**
    Reports new non-empty chain being created by Chain Maker or new row
    fetched from table by Table Reader. Called from Crawler or Table Reader.
   */
  void report_new_chain_created(Item_processing_data *new_chain_creator);
  /**
    Report new object(table, row or any other) was started processing by
    specified Object Reader, Table Reader, Formatter or Row Formatter. Reported
    by these types. Is not reported by queues on enqueue but on dequeue.
   */
  void report_object_processing_started(Item_processing_data *process_data);
  /**
    Report object(table, row or any other) finished being processed. In case of
    table, this does not necessarily mean that all rows were processed. That
    does not necessarily mean t3hat object was successfully written by
    Output Writers.
   */
  void report_object_processing_ended(
      Item_processing_data *finished_process_data);
  /**
    Reports crawler ended enumerating objects and creating chains for them.
   */
  virtual void report_crawler_completed(I_crawler *crawler);

  void register_progress_watchers_in_child(I_progress_reporter *reporter);

 private:
  std::vector<I_progress_watcher *> m_progress_watchers;
};

}  // namespace Dump
}  // namespace Tools
}  // namespace Mysql

#endif
