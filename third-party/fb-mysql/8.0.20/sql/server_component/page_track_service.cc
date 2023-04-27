/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <sql/handler.h>
#include <sql/sql_class.h>
#include "mysql_page_track_imp.h"

/** Check if thd has backup privilige.
@param[in]	thd	thread context
@return 0 if thd has backup privilege, else 1 */
bool check_backup_privilege(MYSQL_THD thd) {
  auto sctx = thd->security_context();

  if (!(sctx->has_global_grant(STRING_WITH_LEN("BACKUP_ADMIN")).first)) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "BACKUP_ADMIN");
    return (true);
  }
  return (false);
}

DEFINE_METHOD(int, Page_track_implementation::start,
              (MYSQL_THD opaque_thd, Page_Track_SE se_type,
               uint64_t *start_id)) {
  if (check_backup_privilege(opaque_thd)) {
    return (ER_SPECIFIC_ACCESS_DENIED_ERROR);
  }

  DBUG_ASSERT(se_type == PAGE_TRACK_SE_INNODB);

  enum legacy_db_type db_type;

  switch (se_type) {
    case PAGE_TRACK_SE_INNODB:
      db_type = DB_TYPE_INNODB;
      break;
  }

  handlerton *ddse = ha_resolve_by_legacy_type(opaque_thd, db_type);

  return (ddse->page_track.start(start_id));
}

DEFINE_METHOD(int, Page_track_implementation::stop,
              (MYSQL_THD opaque_thd, Page_Track_SE se_type,
               uint64_t *stop_id)) {
  if (check_backup_privilege(opaque_thd)) {
    return (ER_SPECIFIC_ACCESS_DENIED_ERROR);
  }

  DBUG_ASSERT(se_type == PAGE_TRACK_SE_INNODB);

  enum legacy_db_type db_type = DB_TYPE_UNKNOWN;

  switch (se_type) {
    case PAGE_TRACK_SE_INNODB:
      db_type = DB_TYPE_INNODB;
      break;
  }

  handlerton *ddse = ha_resolve_by_legacy_type(opaque_thd, db_type);

  return (ddse->page_track.stop(stop_id));
}

DEFINE_METHOD(int, Page_track_implementation::purge,
              (MYSQL_THD opaque_thd, Page_Track_SE se_type,
               uint64_t *purge_id)) {
  if (check_backup_privilege(opaque_thd)) {
    return (ER_SPECIFIC_ACCESS_DENIED_ERROR);
  }

  DBUG_ASSERT(se_type == PAGE_TRACK_SE_INNODB);

  enum legacy_db_type db_type = DB_TYPE_UNKNOWN;

  switch (se_type) {
    case PAGE_TRACK_SE_INNODB:
      db_type = DB_TYPE_INNODB;
      break;
  }

  handlerton *ddse = ha_resolve_by_legacy_type(opaque_thd, db_type);

  return (ddse->page_track.purge(purge_id));
}

DEFINE_METHOD(int, Page_track_implementation::get_page_ids,
              (MYSQL_THD opaque_thd, Page_Track_SE se_type, uint64_t *start_id,
               uint64_t *stop_id, unsigned char *buffer, size_t buffer_len,
               Page_Track_Callback cbk_func, void *cbk_ctx)) {
  if (check_backup_privilege(opaque_thd)) {
    return (ER_SPECIFIC_ACCESS_DENIED_ERROR);
  }

  DBUG_ASSERT(se_type == PAGE_TRACK_SE_INNODB);

  enum legacy_db_type db_type = DB_TYPE_UNKNOWN;

  switch (se_type) {
    case PAGE_TRACK_SE_INNODB:
      db_type = DB_TYPE_INNODB;
      break;
  }

  handlerton *ddse = ha_resolve_by_legacy_type(opaque_thd, db_type);

  return (ddse->page_track.get_page_ids(cbk_func, cbk_ctx, start_id, stop_id,
                                        buffer, buffer_len));
}

DEFINE_METHOD(int, Page_track_implementation::get_num_page_ids,
              (MYSQL_THD opaque_thd, Page_Track_SE se_type, uint64_t *start_id,
               uint64_t *stop_id, uint64_t *num_pages)) {
  if (check_backup_privilege(opaque_thd)) {
    return (ER_SPECIFIC_ACCESS_DENIED_ERROR);
  }

  DBUG_ASSERT(se_type == PAGE_TRACK_SE_INNODB);

  enum legacy_db_type db_type = DB_TYPE_UNKNOWN;

  switch (se_type) {
    case PAGE_TRACK_SE_INNODB:
      db_type = DB_TYPE_INNODB;
      break;
  }

  handlerton *ddse = ha_resolve_by_legacy_type(opaque_thd, db_type);

  return (ddse->page_track.get_num_page_ids(start_id, stop_id, num_pages));
}

DEFINE_METHOD(int, Page_track_implementation::get_status,
              (MYSQL_THD opaque_thd, Page_Track_SE se_type,
               uint64_t *initial_start_id, uint64_t *last_start_id)) {
  if (check_backup_privilege(opaque_thd)) {
    return (ER_SPECIFIC_ACCESS_DENIED_ERROR);
  }

  DBUG_ASSERT(se_type == PAGE_TRACK_SE_INNODB);

  *initial_start_id = 0;
  *last_start_id = 0;

  enum legacy_db_type db_type = DB_TYPE_UNKNOWN;

  switch (se_type) {
    case PAGE_TRACK_SE_INNODB:
      db_type = DB_TYPE_INNODB;
      break;
  }

  handlerton *ddse = ha_resolve_by_legacy_type(opaque_thd, db_type);

  /** Vector of a pair of (ID, bool) where ID is the start/stop point and bool
  is true if the ID is a start point else false */
  static std::vector<std::pair<uint64_t, bool>> status;
  status.clear();

  ddse->page_track.get_status(status);

  if (status.size() == 0) {
    return (false);
  }

  bool is_active = (status.back().second == true);

  if (is_active) {
    *last_start_id = status.back().first;

    for (auto rit = status.rbegin(); rit != status.rend(); ++rit) {
      if (rit->second == false) {
        break;
      }
      *initial_start_id = rit->first;
    }
  }

  return (is_active);
}
