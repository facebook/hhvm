/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RESOURCEGROUPS_RESOURCE_GROUP_BASIC_TYPES_H_
#define RESOURCEGROUPS_RESOURCE_GROUP_BASIC_TYPES_H_

#include "lex_string.h"
#include "mysql_com.h"                                     // NAME_LEN
#include "sql/resourcegroups/platform/thread_attrs_api.h"  // platform::cpu_id_t

namespace resourcegroups {
// Definitions for resource group basic types.
enum class Type { SYSTEM_RESOURCE_GROUP = 1, USER_RESOURCE_GROUP };
struct Range {
  Range() {}
  Range(platform::cpu_id_t start, platform::cpu_id_t end)
      : m_start(start), m_end(end) {}
  platform::cpu_id_t m_start;
  platform::cpu_id_t m_end;
};

/**
  To support logging of multiple warn conditions, these values have been
  represented as powers of two.
*/

#define WARN_RESOURCE_GROUP_UNSUPPORTED 0x00001
#define WARN_RESOURCE_GROUP_UNSUPPORTED_HINT 0x0002
#define WARN_RESOURCE_GROUP_TYPE_MISMATCH 0x0004
#define WARN_RESOURCE_GROUP_NOT_EXISTS 0x0008
#define WARN_RESOURCE_GROUP_ACCESS_DENIED 0x0010

class Resource_group;
struct Resource_group_ctx {
  Resource_group *m_cur_resource_group;
  char m_switch_resource_group_str[NAME_CHAR_LEN + 1];
  int m_warn;
};
}  // namespace resourcegroups
#endif  // RESOURCEGROUPS_RESOURCE_GROUP_BASIC_TYPES_H_
