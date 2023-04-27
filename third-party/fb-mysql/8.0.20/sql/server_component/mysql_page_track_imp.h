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

#ifndef MYSQL_PAGE_TRACK_H
#define MYSQL_PAGE_TRACK_H

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/page_track_service.h>

/**
  An implementation of page tracking service.
*/
class Page_track_implementation {
 public:
  /**
    Service API to start page tracking.

    @param[in]     opaque_thd    Current thread context.
    @param[in]     se_type       SE for which to enable tracking
    @param[out]    start_id      SE specific sequence number [LSN for InnoDB]
    indicating when the tracking was started

    @return Operation status.
      @retval 0 Success
      @retval other ER_* mysql error. Get error details from THD.
  */
  static DEFINE_METHOD(int, start,
                       (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                        uint64_t *start_id));

  /**
    Service API to stop page tracking.

    @param[in]     opaque_thd   Current thread context.
    @param[in]     se_type      SE for which to enable tracking
    @param[out]    stop_id      SE specific sequence number [LSN for InnoDB]
    indicating when the tracking was stopped

    @return Operation status.
      @retval 0 Success
      @retval other ER_* mysql error. Get error details from THD.
  */
  static DEFINE_METHOD(int, stop,
                       (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                        uint64_t *stop_id));

  /**
    Service API to purge page tracking data.

    @param[in]     opaque_thd   Current thread context.
    @param[in]     se_type      SE for which to enable tracking
    @param[in,out] purge_id     SE specific sequence number [LSN for Innodb]
    initially indicating till where the data needs to be purged and finally
    updated to until where it was actually purged

    @return Operation status.
      @retval 0 Success
      @retval other ER_* mysql error. Get error details from THD.
  */
  static DEFINE_METHOD(int, purge,
                       (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                        uint64_t *purge_id));

  /**
    Service API to get tracked pages

    @param[in]     opaque_thd   Current thread context.
    @param[in]     se_type      SE for which to enable tracking
    @param[in,out] start_id     SE specific sequence number [LSN for InnoDB]
    from where the pages tracked would be returned.
    @note The range might get expanded and the actual start_id used for the
    querying will be updated.
    @param[in,out] stop_id      SE specific sequence number [LSN for InnoDB]
    until where the pages tracked would be returned.
    @note The range might get expanded and the actual stop_id used for the
    querying will be updated.
    @param[out]    buffer       allocated buffer to copy page IDs
    @param[in]     buffer_len   length of buffer in bytes
    @param[in]     cbk_func     callback function return page IDs
    @param[in]     cbk_ctx      caller's context for callback

    @return Operation status.
      @retval 0 Success
      @retval other ER_* mysql error. Get error details from THD.
  */
  static DEFINE_METHOD(int, get_page_ids,
                       (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                        uint64_t *start_id, uint64_t *stop_id,
                        unsigned char *buffer, size_t buffer_len,
                        Page_Track_Callback cbk_func, void *cbk_ctx));

  /**
    Service API to get approximate number of pages tracked in the given range.

    @param[in]     opaque_thd   Current thread context.
    @param[in]     se_type      SE for which to enable tracking
    @param[in,out] start_id     SE specific sequence number [LSN for InnoDB]
    from where the pages tracked would be returned.
    @note the range might get expanded and the actual start_id used for the
    querying will be updated.
    @param[in,out] stop_id      SE specific sequence number [LSN for InnoDB]
    until where the pages tracked would be returned.
    @note the range might get expanded and the actual stop_id used for the
    querying will be updated.
    @param[out]    num_pages    number of pages tracked

    @return Operation status.
      @retval 0 Success
      @retval other ER_* mysql error. Get error details from THD.
  */
  static DEFINE_METHOD(int, get_num_page_ids,
                       (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                        uint64_t *start_id, uint64_t *stop_id,
                        uint64_t *num_pages));

  /**
    API to check if page tracking is active or not and to return start id if
    it's active.

    @param[in]     opaque_thd   Current thread context.
    @param[in]     se_type      SE for which to enable tracking
    @param[out]    initial_start_id     SE specific sequence number [LSN for
    Innodb] which denotes the start id at which page tracking was started if
    it's active
    @param[out]    last_start_id        SE specific sequence number [LSN for
    Innodb] which denotes the start id the last time the start request was
    issued
    @return if page tracking is active or not
      @retval true if page tracking is active
      @retval false if page tracking is not active
  */
  static DEFINE_METHOD(int, get_status,
                       (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                        uint64_t *initial_start_id, uint64_t *last_start_id));
};

#endif /* MYSQL_PAGE_TRACK_H */
