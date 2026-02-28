/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "statement_events.h"
#include <algorithm>  // std::min() for Windows
#include "event_reader_macros.h"

namespace binary_log {

/******************************************************************************
                     Query_event methods
******************************************************************************/
/**
  The simplest constructor that could possibly work.  This is used for
  creating static objects that have a special meaning and are invisible
  to the log.
*/
Query_event::Query_event(Log_event_type type_arg)
    : Binary_log_event(type_arg),
      query(nullptr),
      db(nullptr),
      user(nullptr),
      user_len(0),
      host(nullptr),
      host_len(0),
      db_len(0),
      q_len(0) {}

/**
  The constructor used by MySQL master to create a query event, to be
  written to the binary log.
*/
Query_event::Query_event(
    const char *query_arg, const char *catalog_arg, const char *db_arg,
    uint32_t query_length, unsigned long thread_id_arg,
    unsigned long long sql_mode_arg, unsigned long auto_increment_increment_arg,
    unsigned long auto_increment_offset_arg, unsigned int number,
    unsigned long long table_map_for_update_arg, int errcode)
    : Binary_log_event(QUERY_EVENT),
      query(query_arg),
      db(db_arg),
      catalog(catalog_arg),
      user(nullptr),
      user_len(0),
      host(nullptr),
      host_len(0),
      thread_id(thread_id_arg),
      db_len(0),
      error_code(errcode),
      status_vars_len(0),
      q_len(query_length),
      flags2_inited(true),
      sql_mode_inited(true),
      charset_inited(true),
      sql_mode(sql_mode_arg),
      auto_increment_increment(
          static_cast<uint16_t>(auto_increment_increment_arg)),
      auto_increment_offset(static_cast<uint16_t>(auto_increment_offset_arg)),
      time_zone_len(0),
      lc_time_names_number(number),
      charset_database_number(0),
      table_map_for_update(table_map_for_update_arg),
      explicit_defaults_ts(TERNARY_UNSET),
      mts_accessed_dbs(0),
      ddl_xid(INVALID_XID),
      default_collation_for_utf8mb4_number(0),
      sql_require_primary_key(0xff),
      default_table_encryption(0xff) {}

/**
  Utility function for the Query_event constructor.
  The function copies n bytes from the source string and moves the
  destination pointer by the number of bytes copied.

  @param dst Pointer to the buffer into which the string is to be copied
  @param src Source string
  @param len The number of bytes to be copied
*/
static void copy_str_and_move(Log_event_header::Byte **dst, const char **src,
                              size_t len) {
  memcpy(*dst, *src, len);
  *src = reinterpret_cast<const char *>(*dst);
  (*dst) += len;
  *(*dst)++ = 0;
}

/**
  The event occurs when an updating statement is done.
*/
Query_event::Query_event(const char *buf, const Format_description_event *fde,
                         Log_event_type event_type)
    : Binary_log_event(&buf, fde),
      query(nullptr),
      db(nullptr),
      catalog(nullptr),
      time_zone_str(nullptr),
      user(nullptr),
      user_len(0),
      host(nullptr),
      host_len(0),
      db_len(0),
      status_vars_len(0),
      q_len(0),
      flags2_inited(false),
      sql_mode_inited(false),
      charset_inited(false),
      auto_increment_increment(1),
      auto_increment_offset(1),
      time_zone_len(0),
      catalog_len(0),
      lc_time_names_number(0),
      charset_database_number(0),
      table_map_for_update(0),
      explicit_defaults_ts(TERNARY_UNSET),
      mts_accessed_dbs(OVER_MAX_DBS_IN_EVENT_MTS),
      ddl_xid(INVALID_XID),
      default_collation_for_utf8mb4_number(0),
      sql_require_primary_key(0xff),
      default_table_encryption(0xff) {
  BAPI_ENTER("Query_event::Query_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);

  uint8_t post_header_len = fde->post_header_len[event_type - 1];
  uint64_t end_variable_part = 0;
  data_len = READER_CALL(available_to_read);
  data_len = data_len - post_header_len;

  READER_TRY_SET(thread_id, read_and_letoh<uint32_t>);
  READER_TRY_SET(query_exec_time, read_and_letoh<uint32_t>);
  READER_TRY_SET(db_len, read<uint8_t>);
  READER_TRY_SET(error_code, read_and_letoh<uint16_t>);
  /*
    5.0 format starts here.
    Depending on the format, we may or not have affected/warnings etc
    The remnant post-header to be parsed has length:
  */
  if (post_header_len > QUERY_HEADER_MINIMAL_LEN) {
    READER_TRY_SET(status_vars_len, read_and_letoh<uint16_t>);
    /*
      Check if status variable length is corrupt and will lead to very
      wrong data. We could be even more strict and require data_len to
      be even bigger, but this will suffice to catch most corruption
      errors that can lead to a crash.
    */
    if (status_vars_len >
        std::min<unsigned long>(data_len, MAX_SIZE_LOG_EVENT_STATUS))
      READER_THROW("Invalid status vars length in Query event");
  } else
    /* formats before 5.0 are not supported anymore */
    READER_THROW("Unsupported Query event format");

  /*
    We have parsed everything we know in the post header for QUERY_EVENT,
    the rest of post header is either comes from older version MySQL or
    dedicated to derived events (e.g. Execute_load_query...)
  */
  READER_CALL(go_to, fde->common_header_len + post_header_len);

  data_len = data_len - status_vars_len;

  /* variable-part: the status vars; only in MySQL 5.0  */
  end_variable_part = READER_CALL(position) + status_vars_len;
  while (READER_CALL(position) < end_variable_part) {
    uint8_t variable_type;
    READER_TRY_SET(variable_type, read<uint8_t>);
    switch (variable_type) {
      case Q_FLAGS2_CODE:
        flags2_inited = true;
        READER_TRY_SET(flags2, read_and_letoh<uint32_t>);
        break;
      case Q_SQL_MODE_CODE:
        sql_mode_inited = true;
        READER_TRY_SET(sql_mode, read_and_letoh<uint64_t>);
        break;
      case Q_CATALOG_NZ_CODE:
        READER_TRY_SET(catalog_len, read<uint8_t>);
        if (catalog_len) {
          catalog = READER_CALL(ptr);
          READER_TRY_CALL(forward, catalog_len);
        }
        break;
      case Q_AUTO_INCREMENT:
        READER_TRY_SET(auto_increment_increment, read_and_letoh<uint16_t>);
        READER_TRY_SET(auto_increment_offset, read_and_letoh<uint16_t>);
        break;
      case Q_CHARSET_CODE:
        charset_inited = true;
        READER_TRY_CALL(memcpy<char *>, charset, 6);
        break;
      case Q_TIME_ZONE_CODE:
        READER_TRY_SET(time_zone_len, read<uint8_t>);
        if (time_zone_len) {
          time_zone_str = READER_CALL(ptr);
          READER_TRY_CALL(forward, time_zone_len);
        }
        break;
      case Q_CATALOG_CODE: /* for 5.0.x where 0<=x<=3 masters */
        READER_TRY_SET(catalog_len, read<uint8_t>);
        if (catalog_len) {
          catalog = READER_CALL(ptr);
          READER_TRY_CALL(forward,
                          catalog_len + 1);  // leap over end 0
        }
        break;
      case Q_LC_TIME_NAMES_CODE:
        READER_TRY_SET(lc_time_names_number, read_and_letoh<uint16_t>);
        break;
      case Q_CHARSET_DATABASE_CODE:
        READER_TRY_SET(charset_database_number, read_and_letoh<uint16_t>);
        break;
      case Q_TABLE_MAP_FOR_UPDATE_CODE:
        READER_TRY_SET(table_map_for_update, read_and_letoh<uint64_t>);
        break;
      case Q_MICROSECONDS: {
        READER_TRY_SET(header()->when.tv_usec, read_and_letoh<uint32_t>, 3);
        break;
      }
      case Q_INVOKER: {
        READER_TRY_SET(user_len, read<uint8_t>);
        if (user_len == 0) {
          user = (const char *)"";
        } else {
          user = READER_CALL(ptr);
          READER_TRY_CALL(forward, user_len);
        }

        READER_TRY_SET(host_len, read<uint8_t>);
        if (host_len == 0) {
          host = (const char *)"";
        } else {
          host = READER_CALL(ptr);
          READER_TRY_CALL(forward, host_len);
        }
        break;
      }
      case Q_UPDATED_DB_NAMES: {
        unsigned char i = 0;
#ifndef DBUG_OFF
        bool is_corruption_injected = false;
#endif

        READER_TRY_SET(mts_accessed_dbs, read<uint8_t>);
        /*
           Notice, the following check is positive also in case of
           the master's MAX_DBS_IN_EVENT_MTS > the slave's one and the event
           contains e.g the master's MAX_DBS_IN_EVENT_MTS db:s.
        */
        if (mts_accessed_dbs > MAX_DBS_IN_EVENT_MTS) {
          mts_accessed_dbs = OVER_MAX_DBS_IN_EVENT_MTS;
          break;
        }

        if (mts_accessed_dbs == 0)
          READER_THROW("Invalid MTS accessed databases");

        for (i = 0;
             i < mts_accessed_dbs && READER_CALL(position) < end_variable_part;
             i++) {
          uint64_t remaining;
#ifndef DBUG_OFF
          /*
            This is specific to mysql test run on the server
            for the keyword "query_log_event_mts_corrupt_db_names"
          */
          if (binary_log_debug::debug_query_mts_corrupt_db_names) {
            if (mts_accessed_dbs == 2) {
              char *pos = const_cast<char *>(READER_CALL(ptr));
              BAPI_ASSERT(pos[sizeof("d?") - 1] == 0);
              pos[sizeof("d?") - 1] = 'a';
              is_corruption_injected = true;
            }
          }
#endif
          if (READER_CALL(position) >= end_variable_part)
            READER_THROW("Error reading MTS accessed databases");

          remaining = end_variable_part - READER_CALL(position);
          READER_TRY_CALL(strncpyz, mts_accessed_db_names[i],
                          std::min<unsigned long>(NAME_LEN, remaining),
                          NAME_LEN);
        }
        if (i != mts_accessed_dbs
#ifndef DBUG_OFF
            || is_corruption_injected
#endif
        )
          READER_THROW("Invalid Q_UPDATED_DB_NAMES");
        break;
      }
      case Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP: {
        char value;
        READER_TRY_SET(value, read<uint8_t>);
        explicit_defaults_ts = value == 0 ? TERNARY_OFF : TERNARY_ON;
        break;
      }
      case Q_DDL_LOGGED_WITH_XID:
        /*
          Like in Xid_log_event case, the xid value is not used on the slave
          so the number does not really need to respect endiness.
        */
        READER_TRY_SET(ddl_xid, read_and_letoh<uint64_t>);
        break;
      case Q_DEFAULT_COLLATION_FOR_UTF8MB4:
        READER_TRY_SET(default_collation_for_utf8mb4_number,
                       read_and_letoh<uint16_t>);
        break;
      case Q_SQL_REQUIRE_PRIMARY_KEY:
        READER_TRY_SET(sql_require_primary_key, read<uint8_t>);
        break;
      case Q_DEFAULT_TABLE_ENCRYPTION:
        READER_TRY_SET(default_table_encryption, read<uint8_t>);
        break;
      default:
        /* That's why you must write status vars in growing order of code */
        READER_CALL(go_to, end_variable_part);  // Break loop
    }
  }

  /*
    if time_zone_len or catalog_len are 0, then time_zone and catalog
    are uninitialized at this point.  shouldn't they point to the
    zero-length null-terminated strings we allocated space for in the
    my_alloc call above? /sven
  */

  /* A 2nd variable part; this is common to all versions */
  db = READER_CALL(ptr);
  READER_TRY_CALL(forward, db_len + 1);
  q_len = data_len - db_len - 1;
  if (q_len != READER_CALL(available_to_read))
    READER_THROW("Invalid query length");
  query = READER_CALL(ptr, q_len);

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

/**
  Layout for the data buffer is as follows
  <pre>
  +--------+-----------+------+------+---------+----+-------+----+
  | catlog | time_zone | user | host | db name | \0 | Query | \0 |
  +--------+-----------+------+------+---------+----+-------+----+
  </pre>
*/
int Query_event::fill_data_buf(Log_event_header::Byte *buf,
                               unsigned long buf_len) {
  if (!buf) return 0;
  /* We need to check the buffer size */
  if (buf_len < catalog_len + 1 + time_zone_len + 1 + user_len + 1 + host_len +
                    1 + data_len)
    return 0;

  if (data_len && (data_len < db_len || data_len < q_len ||
                   data_len != (db_len + q_len + 1)))
    return 0;

  unsigned char *start = buf;
  /*
    Note: catalog_len is one less than "catalog.length()"
    if Q_CATALOG flag is set
   */
  if (catalog_len)  // If catalog is given
    /*
      This covers both the cases, where the catalog_nz flag is set of unset.
      The end 0 will be a part of the string catalog in the second case,
      hence using catalog.length() instead of catalog_len makes the flags
      catalog_nz redundant.
     */
    copy_str_and_move(&start, &catalog, catalog_len);
  if (time_zone_len > 0)
    copy_str_and_move(&start, &time_zone_str, time_zone_len);
  if (user_len > 0) copy_str_and_move(&start, &user, user_len);
  if (host_len > 0) copy_str_and_move(&start, &host, host_len);
  if (data_len) {
    if (db_len > 0 && db) copy_str_and_move(&start, &db, db_len);
    if (q_len > 0 && query) copy_str_and_move(&start, &query, q_len);
  }
  return 1;
}

User_var_event::User_var_event(const char *buf,
                               const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("User_var_event::User_var_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  READER_TRY_CALL(forward, fde->post_header_len[USER_VAR_EVENT - 1]);

  READER_TRY_SET(name_len, read_and_letoh<uint32_t>);
  if (name_len == 0) READER_THROW("Invalid name length");
  name = READER_CALL(strndup<const char *>, name_len);
  READER_TRY_SET(is_null, read<uint8_t>);

  flags = User_var_event::UNDEF_F;  // defaults to UNDEF_F
  if (is_null) {
    type = STRING_RESULT;
    /*
     *my_charset_bin.number= 63, and my_charset_bin is defined in server
     *so replacing it with its value.
     */
    charset_number = 63;
    val_len = 0;
    val = nullptr;
  } else {
    uint8_t type_tmp;
    READER_TRY_SET(type_tmp, read<uint8_t>);
    type = (Value_type)type_tmp;
    switch (type) {
      case STRING_RESULT:
      case DECIMAL_RESULT:
      case REAL_RESULT:
      case INT_RESULT:
        break;
      default:
        READER_THROW("Invalid type found while deserializing User_var_event");
    }
    READER_TRY_SET(charset_number, read_and_letoh<uint32_t>);
    READER_TRY_SET(val_len, read_and_letoh<uint32_t>);
    val = const_cast<char *>(READER_CALL(ptr, val_len));
    // val[0] is precision and val[1] is scale so precision >= scale for decimal
    if (type == DECIMAL_RESULT) {
      if (val[0] < val[1])
        READER_THROW(
            "Invalid User value found while deserializing User_var_event");
    }
    /**
      We need to check if this is from an old server
      that did not pack information for flags.
      We do this by checking if there are extra bytes
      after the packed value. If there are we take the
      extra byte and it's value is assumed to contain
      the flags value.

      Old events will not have this extra byte, thence,
      we keep the flags set to UNDEF_F.
    */
    if (READER_CALL(available_to_read) > 0) {
      READER_TRY_SET(flags, read<uint8_t>);
    }
  }

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

User_var_event::~User_var_event() { bapi_free(const_cast<char *>(name)); }

/**
  Constructor receives a packet from the MySQL master or the binary
  log and decodes it to create an Intvar_event.
  Written every time a statement uses an AUTO_INCREMENT column or the
  LAST_INSERT_ID() function; precedes other events for the statement.
  This is written only before a QUERY_EVENT and is not used with row-based
  logging. An INTVAR_EVENT is written with a "subtype" in the event data part:

  * INSERT_ID_EVENT indicates the value to use for an AUTO_INCREMENT column in
    the next statement.

  * LAST_INSERT_ID_EVENT indicates the value to use for the LAST_INSERT_ID()
    function in the next statement.
*/
Intvar_event::Intvar_event(const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("Intvar_event::Intvar_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  READER_TRY_CALL(forward, fde->post_header_len[INTVAR_EVENT - 1]);

  READER_TRY_SET(type, read<uint8_t>);
  READER_TRY_SET(val, read_and_letoh<uint64_t>);

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Rand_event::Rand_event(const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("Rand_event::Rand_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  READER_TRY_CALL(forward, fde->post_header_len[RAND_EVENT - 1]);

  READER_TRY_SET(seed1, read_and_letoh<uint64_t>);
  READER_TRY_SET(seed2, read_and_letoh<uint64_t>);

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

#ifndef HAVE_MYSYS
void Query_event::print_event_info(std::ostream &info) {
  if (memcmp(query, "BEGIN", 5) != 0 && memcmp(query, "COMMIT", 6) != 0) {
    info << "use `" << db << "`; ";
  }
  info << query;
}

void Query_event::print_long_info(std::ostream &info) {
  info << "Timestamp: " << header()->when.tv_sec;
  info << "\tThread id: " << (int)thread_id;
  info << "\tExec time: " << (int)query_exec_time;
  info << "\nDatabase: " << db;
  info << "\tQuery: ";
  this->print_event_info(info);
}

void User_var_event::print_event_info(std::ostream &info) {
  info << "@`" << name << "`=";
  if (type == STRING_RESULT)
    info << val;
  else
    info << "<Binary encoded value>";
  // TODO: value is binary encoded, requires decoding
}

void User_var_event::print_long_info(std::ostream &info) {
  info << "Timestamp: " << header()->when.tv_sec;
  info << "\tType: " << get_value_type_string(type);
  info << "\n";
  this->print_event_info(info);
}

void Intvar_event::print_event_info(std::ostream &info) {
  info << get_var_type_string();
  info << "\tValue: " << val;
}

void Intvar_event::print_long_info(std::ostream &info) {
  info << "Timestamp: " << header()->when.tv_sec;
  info << "\t";
  this->print_event_info(info);
}

void Rand_event::print_event_info(std::ostream &info) {
  info << " SEED1 is " << seed1;
  info << " SEED2 is " << seed2;
}
void Rand_event::print_long_info(std::ostream &info) {
  info << "Timestamp: " << header()->when.tv_sec;
  info << "\t";
  this->print_event_info(info);
}

#endif
}  // namespace binary_log
