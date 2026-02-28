/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <string.h>
#include <memory>
#include <unordered_map>
#include <utility>

#include "libbinlogevents/include/control_events.h"
#include "map_helpers.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"  // IWYU pragma: keep
#include "prealloced_array.h"
#include "sql/rpl_gtid.h"
#include "sql/thr_malloc.h"

#ifndef MYSQL_SERVER
#include "client/mysqlbinlog.h"  // IWYU pragma: keep
#endif

PSI_memory_key key_memory_Sid_map_Node;

Sid_map::Sid_map(Checkable_rwlock *_sid_lock)
    : sid_lock(_sid_lock),
      _sidno_to_sid(key_memory_Sid_map_Node),
      _sorted(key_memory_Sid_map_Node) {
  DBUG_TRACE;
}

Sid_map::~Sid_map() { DBUG_TRACE; }

enum_return_status Sid_map::clear() {
  DBUG_TRACE;
  _sid_to_sidno.clear();
  _sidno_to_sid.clear();
  _sorted.clear();
  RETURN_OK;
}

rpl_sidno Sid_map::add_sid(const rpl_sid &sid) {
  DBUG_TRACE;
#ifndef DBUG_OFF
  char buf[binary_log::Uuid::TEXT_LENGTH + 1];
  sid.to_string(buf);
  DBUG_PRINT("info", ("SID=%s", buf));
#endif
  if (sid_lock) sid_lock->assert_some_lock();
  auto it = _sid_to_sidno.find(sid);
  if (it != _sid_to_sidno.end()) {
    DBUG_PRINT("info", ("existed as sidno=%d", it->second->sidno));
    return it->second->sidno;
  }

  bool is_wrlock = false;
  if (sid_lock) {
    is_wrlock = sid_lock->is_wrlock();
    if (!is_wrlock) {
      sid_lock->unlock();
      sid_lock->wrlock();
    }
  }
  DBUG_PRINT("info", ("is_wrlock=%d sid_lock=%p", is_wrlock, sid_lock));
  rpl_sidno sidno;
  it = _sid_to_sidno.find(sid);
  if (it != _sid_to_sidno.end())
    sidno = it->second->sidno;
  else {
    sidno = get_max_sidno() + 1;
    if (add_node(sidno, sid) != RETURN_STATUS_OK) sidno = -1;
  }

  if (sid_lock) {
    if (!is_wrlock) {
      sid_lock->unlock();
      sid_lock->rdlock();
    }
  }
  return sidno;
}

enum_return_status Sid_map::add_node(rpl_sidno sidno, const rpl_sid &sid) {
  DBUG_TRACE;
  if (sid_lock) sid_lock->assert_some_wrlock();
  unique_ptr_my_free<Node> node(
      (Node *)my_malloc(key_memory_Sid_map_Node, sizeof(Node), MYF(MY_WME)));
  if (node == nullptr) RETURN_REPORTED_ERROR;

  node->sidno = sidno;
  node->sid = sid;
  if (!_sidno_to_sid.push_back(node.get())) {
    if (!_sorted.push_back(sidno)) {
      if (_sid_to_sidno.emplace(node->sid, std::move(node)).second) {
#ifdef MYSQL_SERVER
        /*
          If this is the global_sid_map, we take the opportunity to
          resize all arrays in gtid_state while holding the wrlock.
        */
        if (this != global_sid_map ||
            gtid_state->ensure_sidno() == RETURN_STATUS_OK)
#endif
        {
          // We have added one element to the end of _sorted.  Now we
          // bubble it down to the sorted position.
          int sorted_i = sidno - 1;
          rpl_sidno *prev_sorted_p = &_sorted[sorted_i];
          sorted_i--;
          while (sorted_i >= 0) {
            rpl_sidno *sorted_p = &_sorted[sorted_i];
            const rpl_sid &other_sid = sidno_to_sid(*sorted_p);
            if (memcmp(sid.bytes, other_sid.bytes,
                       binary_log::Uuid::BYTE_LENGTH) >= 0)
              break;
            memcpy(prev_sorted_p, sorted_p, sizeof(rpl_sidno));
            sorted_i--;
            prev_sorted_p = sorted_p;
          }
          memcpy(prev_sorted_p, &sidno, sizeof(rpl_sidno));
          RETURN_OK;
        }
      }
      _sorted.pop_back();
    }
    _sidno_to_sid.pop_back();
  }

  BINLOG_ERROR(("Out of memory."), (ER_OUT_OF_RESOURCES, MYF(0)));
  RETURN_REPORTED_ERROR;
}

enum_return_status Sid_map::copy(Sid_map *dest) {
  DBUG_TRACE;
  enum_return_status return_status = RETURN_STATUS_OK;

  rpl_sidno max_sidno = get_max_sidno();
  for (rpl_sidno sidno = 1;
       sidno <= max_sidno && return_status == RETURN_STATUS_OK; sidno++) {
    rpl_sid sid;
    sid.copy_from(sidno_to_sid(sidno));
    return_status = dest->add_node(sidno, sid);
  }

  return return_status;
}
