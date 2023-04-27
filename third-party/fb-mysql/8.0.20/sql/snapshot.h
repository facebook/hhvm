/* Copyright (c) 2020, Percona and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef SQL_SNAPSHOT_INCLUDED
#define SQL_SNAPSHOT_INCLUDED

enum snapshot_operation {
  SNAPSHOT_CREATE,
  SNAPSHOT_ATTACH,
  SNAPSHOT_RELEASE,
  SNAPSHOT_NONE
};

struct snapshot_info_st {
  std::string binlog_file;
  ulonglong binlog_pos = 0;
  std::string gtid_executed;
  ulonglong snapshot_id = 0;
  snapshot_operation op = SNAPSHOT_NONE;
  ulonglong snapshot_hlc = 0;
};

class explicit_snapshot {
 protected:
  virtual ~explicit_snapshot() {}

 public:
  snapshot_info_st ss_info;
  explicit_snapshot(snapshot_info_st ss_info) : ss_info(ss_info) {}
};

#endif /* SQL_SNAPSHOT_INCLUDED */
