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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @brief

  This defines the API used to call functions to the page tracking service.
  When implementing such a service, refer to mysql_page_track.h instead!

*/

#ifndef MYSQL_PAGE_TRACK_SERVICE_H
#define MYSQL_PAGE_TRACK_SERVICE_H

#include <mysql/components/service.h>
#include <functional>

#include <stdint.h>

#ifdef __cplusplus
class THD;
#define MYSQL_THD THD *
#else
#define MYSQL_THD void *
#endif

/**
  SE for the page tracking. Currently supports only in InnoDB.
*/
typedef enum { PAGE_TRACK_SE_INNODB = 1 } Page_Track_SE;

/**
  Page tracking callback function.

  @param[in]     thd        Current thread context
  @param[in]     buffer     buffer filled with 8 byte page ids; the format is
  specific to SE. For InnoDB it is space_id (4 bytes) followed by page number
  (4 bytes)
  @param[in]     buf_len    length of buffer in bytes
  @param[in]     num_pages  number of valid page IDs in buffer
  @param[in,out] user_ctx   user context passed to page tracking function

  @return Operation status.
*/
typedef int (*Page_Track_Callback)(MYSQL_THD thd, const unsigned char *buffer,
                                   size_t buf_len, int num_pages,
                                   void *user_ctx);

BEGIN_SERVICE_DEFINITION(mysql_page_track)

/**
  Service API to start page tracking

  @param[in]     opaque_thd    Current thread context.
  @param[in]     se_type       SE for which to enable tracking
  @param[out]    start_id      SE specific sequence number [LSN for InnoDB]
  indicating when the tracking was started

  @return Operation status.
    @retval 0 Success
    @retval other ER_* mysql error. Get error details from THD.
*/

DECLARE_METHOD(int, start,
               (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                uint64_t *start_id));

/**
  Service API to stop page tracking

  @param[in]     opaque_thd   Current thread context.
  @param[in]     se_type      SE for which to enable tracking
  @param[out]    stop_id      SE specific sequence number [LSN for InnoDB]
  indicating when the tracking was stopped

  @return Operation status.
    @retval 0 Success
    @retval other ER_* mysql error. Get error details from THD.
*/

DECLARE_METHOD(int, stop,
               (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                uint64_t *stop_id));

/**
  Service API to purge page tracking data.

  @param[in]     opaque_thd   Current thread context.
  @param[in]     se_type      SE for which to enable tracking
  @param[in,out] purge_id     SE specific sequence number [LSN for InnoDB]
  initially indicating till where the data needs to be purged and finally
  updated to until where it was actually purged

  @return Operation status.
    @retval 0 Success
    @retval other ER_* mysql error. Get error details from THD.
*/
DECLARE_METHOD(int, purge,
               (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                uint64_t *purge_id));

/**
  Service API to get tracked pages

  @param[in]     opaque_thd   Current thread context.
  @param[in]     se_type      SE for which to enable tracking
  @param[in,out] start_id     SE specific sequence number [LSN for InnoDB] from
  where the pages tracked would be returned.
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

DECLARE_METHOD(int, get_page_ids,
               (MYSQL_THD opaque_thd, Page_Track_SE se_type, uint64_t *start_id,
                uint64_t *stop_id, unsigned char *buffer, size_t buffer_len,
                Page_Track_Callback cbk_func, void *cbk_ctx));

/**
  Service API to get approximate number of pages tracked in the given range.

  @param[in]     opaque_thd   Current thread context.
  @param[in]     se_type      SE for which to enable tracking
  @param[in,out] start_id     SE specific sequence number [LSN for InnoDB] from
  where the pages tracked would be returned.
  @note the range might get expanded and the actual start_id used for the
  querying will be updated.
  @param[in,out] stop_id      SE specific sequence number [LSN for InnoDB]
  until where the pages tracked would be returned.
  @note the range might get expanded and the actual stop_id used for the
  querying will be updated.
  @param[out]    num_pages	number of pages tracked

  @return Operation status.
    @retval 0 Success
    @retval other ER_* mysql error. Get error details from THD.
*/
DECLARE_METHOD(int, get_num_page_ids,
               (MYSQL_THD opaque_thd, Page_Track_SE se_type, uint64_t *start_id,
                uint64_t *stop_id, uint64_t *num_pages));

/**
  API to check if the page tracking is active or not and to return start id if
  it's active.

  @param[in]     opaque_thd   Current thread context.
  @param[in]     se_type      SE for which to enable tracking
  @param[out]    initial_start_id     SE specific sequence number [LSN for
  InnoDB] which denotes the start id at which page tracking was started if it's
  active
  @param[out]    last_start_id        SE specific sequence number [LSN for
  InnoDB] which denotes the start id the last time the start request was issued
  @return if page tracking is active or not
    @retval true if page tracking is active
    @retval false if page tracking is not active
*/
DECLARE_METHOD(int, get_status,
               (MYSQL_THD opaque_thd, Page_Track_SE se_type,
                uint64_t *initial_start_id, uint64_t *last_start_id));

END_SERVICE_DEFINITION(mysql_page_track)

#endif
