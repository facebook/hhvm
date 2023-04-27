/*
   Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_binlog.h"

#include <stddef.h>
#include <sys/types.h>
#include <utility>

#include "base64.h"  // base64_needed_decoded_length
#include "lex_string.h"
#include "libbinlogevents/include/binlog_event.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog_reader.h"
#include "sql/log_event.h"  // Format_description_log_event
#include "sql/psi_memory_key.h"
#include "sql/rpl_info_factory.h"  // Rpl_info_factory
#include "sql/rpl_info_handler.h"
#include "sql/rpl_rli.h"  // Relay_log_info
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/system_variables.h"

/**
  Check if the event type is allowed in a BINLOG statement.

  @retval 0 if the event type is ok.
  @retval 1 if the event type is not ok.
*/
static int check_event_type(int type, Relay_log_info *rli) {
  Format_description_log_event *fd_event = rli->get_rli_description_event();

  switch (type) {
    case binary_log::FORMAT_DESCRIPTION_EVENT:
      /*
        We need a preliminary FD event in order to parse the FD event,
        if we don't already have one.
      */
      if (!fd_event) {
        fd_event = new Format_description_log_event();
        if (rli->set_rli_description_event(fd_event)) {
          delete fd_event;
          return 1;
        }
      }

      /* It is always allowed to execute FD events. */
      return 0;

    case binary_log::ROWS_QUERY_LOG_EVENT:
    case binary_log::TABLE_MAP_EVENT:
    case binary_log::WRITE_ROWS_EVENT:
    case binary_log::UPDATE_ROWS_EVENT:
    case binary_log::DELETE_ROWS_EVENT:
    case binary_log::WRITE_ROWS_EVENT_V1:
    case binary_log::UPDATE_ROWS_EVENT_V1:
    case binary_log::DELETE_ROWS_EVENT_V1:
    case binary_log::PARTIAL_UPDATE_ROWS_EVENT:
    case binary_log::METADATA_EVENT:
      /*
        Row events are only allowed if a Format_description_event has
        already been seen.
      */
      if (fd_event)
        return 0;
      else {
        my_error(ER_NO_FORMAT_DESCRIPTION_EVENT_BEFORE_BINLOG_STATEMENT, MYF(0),
                 Log_event::get_type_str((Log_event_type)type));
        return 1;
      }
      break;

    default:
      /*
        It is not meaningful to execute other events than row-events and
        FD events. It would even be dangerous to execute Stop_log_event
        and Rotate_log_event since they call flush_relay_log_info, which
        is not allowed to call by other threads than the slave SQL
        thread when the slave SQL thread is running.
      */
      my_error(ER_ONLY_FD_AND_RBR_EVENTS_ALLOWED_IN_BINLOG_STATEMENT, MYF(0),
               Log_event::get_type_str((Log_event_type)type));
      return 1;
  }
}

/**
  Execute a BINLOG statement.

  To execute the BINLOG command properly the server needs to know
  which format the BINLOG command's event is in.  Therefore, the first
  BINLOG statement seen must be a base64 encoding of the
  Format_description_log_event, as outputted by mysqlbinlog.  This
  Format_description_log_event is cached in
  rli->rli_description_event.

  @param thd Pointer to THD object for the client thread executing the
  statement.
*/

void mysql_client_binlog_statement(THD *thd) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("binlog base64: '%*s'",
                      (int)(thd->lex->binlog_stmt_arg.length < 2048
                                ? thd->lex->binlog_stmt_arg.length
                                : 2048),
                      thd->lex->binlog_stmt_arg.str));

  Security_context *sctx = thd->security_context();
  if (!(sctx->check_access(SUPER_ACL) ||
        sctx->has_global_grant(STRING_WITH_LEN("BINLOG_ADMIN")).first ||
        sctx->has_global_grant(STRING_WITH_LEN("REPLICATION_APPLIER")).first)) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
             "SUPER, BINLOG_ADMIN or REPLICATION_APPLIER");
    return;
  }

  size_t coded_len = thd->lex->binlog_stmt_arg.length;
  if (!coded_len) {
    my_error(ER_SYNTAX_ERROR, MYF(0));
    return;
  }
  size_t decoded_len = base64_needed_decoded_length(coded_len);

  /*
    option_bits will be changed when applying the event. But we don't expect
    it be changed permanently after BINLOG statement, so backup it first.
    It will be restored at the end of this function.
  */
  ulonglong thd_options = thd->variables.option_bits;

  /*
    Allocation
  */
  int err = 0;
  Relay_log_info *rli = thd->rli_fake;
  if (!rli) {
    /*
      We create a Relay_log_info object with a INFO_REPOSITORY_DUMMY because
      to process a BINLOG command a real repository is not necessary. In the
      future, we need to improve the code around the BINLOG command as only a
      small part of the object is required to execute it. / Alfranio
    */

    /* when trying to create an rli from a client, there is no channel*/
    if ((rli = Rpl_info_factory::create_rli(INFO_REPOSITORY_DUMMY, false,
                                            (const char *)"", true))) {
      thd->rli_fake = rli;
      rli->info_thd = thd;
    }
    if (opt_rbr_column_type_mismatch_whitelist) {
      const auto &list =
          split_into_set(opt_rbr_column_type_mismatch_whitelist, ',');
      rli->set_rbr_column_type_mismatch_whitelist(list);
    } else
      rli->set_rbr_column_type_mismatch_whitelist(
          std::unordered_set<std::string>());
  }

  const char *error = nullptr;
  char *buf = (char *)my_malloc(key_memory_binlog_statement_buffer, decoded_len,
                                MYF(MY_WME));
  Log_event *ev = nullptr;

  /*
    Out of memory check
  */
  if (!(rli && buf)) {
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 1); /* needed 1 bytes */
    goto end;
  }

  DBUG_ASSERT(rli->belongs_to_client());

  for (char const *strptr = thd->lex->binlog_stmt_arg.str;
       strptr <
       thd->lex->binlog_stmt_arg.str + thd->lex->binlog_stmt_arg.length;) {
    char const *endptr = nullptr;
    int64 bytes_decoded = base64_decode(strptr, coded_len, buf, &endptr,
                                        MY_BASE64_DECODE_ALLOW_MULTIPLE_CHUNKS);

    DBUG_PRINT("info",
               ("bytes_decoded: %" PRId64 "  strptr: %p  endptr: %p ('%c':%d)",
                bytes_decoded, strptr, endptr, *endptr, *endptr));

    if (bytes_decoded < 0) {
      my_error(ER_BASE64_DECODE_ERROR, MYF(0));
      goto end;
    } else if (bytes_decoded == 0)
      break;  // If no bytes where read, the string contained only whitespace

    DBUG_ASSERT(bytes_decoded > 0);
    DBUG_ASSERT(endptr > strptr);
    coded_len -= endptr - strptr;
    strptr = endptr;

    /*
      Now we have one or more events stored in the buffer. The size of
      the buffer is computed based on how much base64-encoded data
      there were, so there should be ample space for the data (maybe
      even too much, since a statement can consist of a considerable
      number of events).

      TODO: Switch to use a stream-based base64 encoder/decoder in
      order to be able to read exactly what is necessary.
    */

    DBUG_PRINT("info",
               ("binlog base64 decoded_len: %lu  bytes_decoded: %" PRId64,
                (ulong)decoded_len, bytes_decoded));

    /*
      Now we start to read events of the buffer, until there are no
      more.
    */
    for (char *bufptr = buf; bytes_decoded > 0;) {
      /*
        Checking that the first event in the buffer is not truncated.
      */
      ulong event_len;
      if (bytes_decoded < EVENT_LEN_OFFSET + 4 ||
          (event_len = uint4korr(bufptr + EVENT_LEN_OFFSET)) >
              (uint)bytes_decoded) {
        my_error(ER_SYNTAX_ERROR, MYF(0));
        goto end;
      }
      DBUG_PRINT("info", ("event_len=%lu, bytes_decoded=%" PRId64, event_len,
                          bytes_decoded));

      if (check_event_type(bufptr[EVENT_TYPE_OFFSET], rli)) goto end;

      Binlog_read_error binlog_read_error = binlog_event_deserialize(
          reinterpret_cast<unsigned char *>(bufptr), event_len,
          rli->get_rli_description_event(), false, &ev);
      if (binlog_read_error.has_error()) {
        DBUG_PRINT("info",
                   ("binlog base64 err=%s", binlog_read_error.get_str()));
        /*
          This could actually be an out-of-memory, but it is more likely
          caused by a bad statement
        */
        my_error(ER_SYNTAX_ERROR, MYF(0));
        goto end;
      }

      bytes_decoded -= event_len;
      bufptr += event_len;

      DBUG_PRINT("info", ("ev->common_header()=%d", ev->get_type_code()));
      ev->thd = thd;
      /*
        We go directly to the application phase, since we don't need
        to check if the event shall be skipped or not.

        Neither do we have to update the log positions, since that is
        not used at all: the rli_fake instance is used only for error
        reporting.
      */
      err = ev->apply_event(rli);
      /*
        Format_description_log_event should not be deleted because it
        will be used to read info about the relay log's format; it
        will be deleted when the SQL thread does not need it,
        i.e. when this thread terminates.
        ROWS_QUERY_LOG_EVENT if present in rli is deleted at the end
        of the event but ones with trx meta data are deleted here.
      */
      if (ev->get_type_code() != binary_log::FORMAT_DESCRIPTION_EVENT &&
          ev->get_type_code() != binary_log::ROWS_QUERY_LOG_EVENT) {
        delete ev;
        ev = nullptr;
      }
      if (err) {
        /*
          TODO: Maybe a better error message since the BINLOG statement
          now contains several events.
        */
        my_error(ER_UNKNOWN_ERROR, MYF(0));
        goto end;
      }
    }
  }

  DBUG_PRINT("info", ("binlog base64 execution finished successfully"));
  my_ok(thd);

end:
  if (rli) {
    if ((error || err) && rli->rows_query_ev) {
      delete rli->rows_query_ev;
      rli->rows_query_ev = nullptr;
    }
    rli->slave_close_thread_tables(thd);
  }
  thd->variables.option_bits = thd_options;
  my_free(buf);
}
