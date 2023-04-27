/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "m_ctype.h"
#include "my_dbug.h"
#include "sql/rpl_gtid.h"

// const int Gtid_specification::MAX_TEXT_LENGTH;

#ifdef MYSQL_SERVER

enum_return_status Gtid_specification::parse(Sid_map *sid_map,
                                             const char *text) {
  DBUG_TRACE;
  DBUG_ASSERT(text != nullptr);
  if (my_strcasecmp(&my_charset_latin1, text, "AUTOMATIC") == 0) {
    type = AUTOMATIC_GTID;
    gtid.sidno = 0;
    gtid.gno = 0;
  } else if (my_strcasecmp(&my_charset_latin1, text, "ANONYMOUS") == 0) {
    type = ANONYMOUS_GTID;
    gtid.sidno = 0;
    gtid.gno = 0;
  } else {
    PROPAGATE_REPORTED_ERROR(gtid.parse(sid_map, text));
    type = ASSIGNED_GTID;
  }
  RETURN_OK;
}

bool Gtid_specification::is_valid(const char *text) {
  DBUG_TRACE;
  DBUG_ASSERT(text != nullptr);
  if (my_strcasecmp(&my_charset_latin1, text, "AUTOMATIC") == 0)
    return true;
  else if (my_strcasecmp(&my_charset_latin1, text, "ANONYMOUS") == 0)
    return true;
  else
    return Gtid::is_valid(text);
}

#endif  // ifdef MYSQL_SERVER

int Gtid_specification::to_string(const rpl_sid *sid, char *buf) const {
  DBUG_TRACE;
  switch (type) {
    case AUTOMATIC_GTID:
      strcpy(buf, "AUTOMATIC");
      return 9;
    case NOT_YET_DETERMINED_GTID:
      /*
        This can happen if user issues SELECT @@SESSION.GTID_NEXT
        immediately after a BINLOG statement containing a
        Format_description_log_event.
      */
      strcpy(buf, "NOT_YET_DETERMINED");
      return 18;
    case ANONYMOUS_GTID:
      strcpy(buf, "ANONYMOUS");
      return 9;
    /*
      UNDEFINED_GTID must be printed like ASSIGNED_GTID because of
      SELECT @@SESSION.GTID_NEXT.
    */
    case UNDEFINED_GTID:
    case ASSIGNED_GTID:
      return gtid.to_string(*sid, buf);
  }
  DBUG_ASSERT(0);
  return 0;
}

int Gtid_specification::to_string(const Sid_map *sid_map, char *buf,
                                  bool need_lock) const {
  return to_string(type == ASSIGNED_GTID || type == UNDEFINED_GTID
                       ? &sid_map->sidno_to_sid(gtid.sidno, need_lock)
                       : nullptr,
                   buf);
}
