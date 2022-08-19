/*
   Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
   Standalone program to read a MySQL binary log (or relay log).

   Should be able to read any file of these categories, even with
   --start-position.
   An important fact: the Format_desc event of the log is at most the 3rd event
   of the log; if it is the 3rd then there is this combination:
   Format_desc_of_slave, Rotate_of_master, Format_desc_of_master.
*/

#include "client/mysqlbinlog.h"

#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <map>
#include <string>
#include <utility>

#include "caching_sha2_passwordopt-vars.h"
#include "client/client_priv.h"
#include "compression.h"
#include "libbinlogevents/include/codecs/factory.h"
#include "libbinlogevents/include/compression/factory.h"
#include "libbinlogevents/include/compression/iterator.h"
#include "libbinlogevents/include/trx_boundary_parser.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_default.h"
#include "my_dir.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_time.h"
#include "prealloced_array.h"
#include "print_version.h"
#include "sql/binlog_reader.h"
#include "sql/log_event.h"
#include "sql/my_decimal.h"
#include "sql/rpl_constants.h"
#include "sql/rpl_gtid.h"
#include "sql_common.h"
#include "sql_string.h"
#include "sslopt-vars.h"
#include "typelib.h"
#include "welcome_copyright_notice.h"  // ORACLE_WELCOME_COPYRIGHT_NOTICE

#include <tuple>

using std::map;
using std::max;
using std::min;
using std::string;

/**
  For storing information of the Format_description_event of the currently
  active binlog. it will be changed each time a new Format_description_event is
  found in the binlog.
*/
Format_description_event glob_description_event(BINLOG_VERSION, server_version);

/**
  This class abstracts the rewriting of databases for RBR events.
 */
class Database_rewrite {
 public:
  using Rewrite_result =
      std::tuple<unsigned char *, std::size_t, std::size_t, bool>;

 private:
  class Transaction_payload_content_rewriter {
    using Rewrite_payload_result = std::tuple<unsigned char *, std::size_t,
                                              std::size_t, std::size_t, bool>;

   private:
    /**
      The event rewriter reference.
     */
    Database_rewrite &m_event_rewriter;

    /**
      Expands the buffer if needed.
    */
    std::tuple<unsigned char *, std::size_t, bool> reserve(
        unsigned char *buffer, std::size_t capacity, std::size_t size) {
      if (size > capacity) {
        auto outsize{size};
        outsize = round(((size + BINLOG_CHECKSUM_LEN) / 1024.0) + 1) * 1024;
        buffer = (unsigned char *)realloc(buffer, outsize);
        if (!buffer) {
          return std::make_tuple(nullptr, 0, true);
        }
        return std::make_tuple(buffer, outsize, false);
      } else
        return std::make_tuple(buffer, capacity, false);
    }

    class Buffer_realloc_manager {
     private:
      unsigned char **m_buffer{nullptr};

     public:
      Buffer_realloc_manager(unsigned char **buffer) : m_buffer{buffer} {}
      ~Buffer_realloc_manager() {
        if (m_buffer != nullptr) free(*m_buffer);
      }
      void release() { m_buffer = nullptr; }
    };

    Rewrite_payload_result rewrite_inner_events(
        binary_log::transaction::compression::type compression_type,
        const char *orig_payload, std::size_t orig_payload_size,
        std::size_t orig_payload_uncompressed_size,
        const binary_log::Format_description_event &fde) {
      // to return error or not
      auto err{false};
      auto error_val = Rewrite_payload_result{nullptr, 0, 0, 0, true};

      // output variables
      unsigned char *obuffer{nullptr};
      std::size_t obuffer_size{0};
      std::size_t obuffer_capacity{0};
      std::size_t obuffer_size_uncompressed{0};

      // temporary buffer for holding uncompressed and rewritten events
      unsigned char *ibuffer{nullptr};
      std::size_t ibuffer_capacity{0};

      // RAII objects
      Buffer_realloc_manager obuffer_dealloc_guard(&obuffer);
      Buffer_realloc_manager ibuffer_dealloc_guard(&ibuffer);

      // iterator to decompress events
      binary_log::transaction::compression::Iterable_buffer it(
          orig_payload, orig_payload_size, orig_payload_uncompressed_size,
          compression_type);

      // compressor to compress this again
      auto compressor =
          binary_log::transaction::compression::Factory::build_compressor(
              compression_type);

      compressor->set_buffer(obuffer, obuffer_size);
      compressor->reserve(orig_payload_uncompressed_size);
      compressor->open();

      // rewrite and compress
      for (auto ptr : it) {
        std::size_t ev_len{uint4korr(ptr + EVENT_LEN_OFFSET)};

        // reserve input buffer size (we are modifying the input buffer contents
        // before compressing it back).
        std::tie(ibuffer, ibuffer_capacity, err) =
            reserve(ibuffer, ibuffer_capacity, ev_len);
        if (err) return error_val;
        memcpy(ibuffer, ptr, ev_len);

        // rewrite the database name if needed
        std::tie(ibuffer, ibuffer_capacity, ev_len, err) =
            m_event_rewriter.rewrite_event(ibuffer, ibuffer_capacity, ev_len,
                                           fde);
        if (err) return error_val;

        auto left{ev_len};
        while (left > 0 && !err) {
          auto pos{ibuffer + (ev_len - left)};
          std::tie(left, err) = compressor->compress(pos, left);
        }

        if (err) return error_val;
        obuffer_size_uncompressed += ev_len;
      }

      compressor->close();
      std::tie(obuffer, obuffer_size, obuffer_capacity) =
          compressor->get_buffer();

      // do not dispose of the obuffer (disable RAII for obuffer)
      obuffer_dealloc_guard.release();

      // set the new one and adjust event settings
      return Rewrite_payload_result{obuffer, obuffer_capacity, obuffer_size,
                                    obuffer_size_uncompressed, false};
    }

   public:
    Transaction_payload_content_rewriter(Database_rewrite &rewriter)
        : m_event_rewriter(rewriter) {}

    /**
      This member function SHALL decompress, rewrite the contents of the
      payload event, compress it again and then re-encode it.

      @param buffer the buffer holding this event encoded.
      @param buffer_capacity the capacity of the buffer.
      @param fde The format description event to decode this event.

      @return a tuple with the result of the rewrite.
     */
    Rewrite_result rewrite_transaction_payload(
        unsigned char *buffer, std::size_t buffer_capacity,
        binary_log::Format_description_event const &fde) {
      DBUG_ASSERT(buffer[EVENT_TYPE_OFFSET] ==
                  binary_log::TRANSACTION_PAYLOAD_EVENT);
      binary_log::Transaction_payload_event tpe((const char *)buffer, &fde);

      auto orig_payload{tpe.get_payload()};
      auto orig_payload_size{tpe.get_payload_size()};
      auto orig_payload_uncompressed_size{tpe.get_uncompressed_size()};
      auto orig_payload_compression_type{tpe.get_compression_type()};

      unsigned char *rewritten_payload{nullptr};
      std::size_t rewritten_payload_size{0};
      std::size_t rewritten_payload_capacity{0};
      std::size_t rewritten_payload_uncompressed_size{0};

      auto rewrite_payload_res{false};
      auto has_crc{fde.footer()->checksum_alg ==
                   binary_log::BINLOG_CHECKSUM_ALG_CRC32};

      // Rewrite its contents as needed
      std::tie(rewritten_payload, rewritten_payload_capacity,
               rewritten_payload_size, rewritten_payload_uncompressed_size,
               rewrite_payload_res) =
          rewrite_inner_events(orig_payload_compression_type, orig_payload,
                               orig_payload_size,
                               orig_payload_uncompressed_size, fde);

      if (rewrite_payload_res) return Rewrite_result{nullptr, 0, 0, true};

      // create a new TPE with the new buffer
      binary_log::Transaction_payload_event new_tpe(
          reinterpret_cast<const char *>(rewritten_payload),
          rewritten_payload_size, orig_payload_compression_type,
          rewritten_payload_uncompressed_size);

      // start encoding it
      auto codec =
          binary_log::codecs::Factory::build_codec(tpe.header()->type_code);
      uchar tpe_buffer[binary_log::Transaction_payload_event::MAX_DATA_LENGTH];
      auto result = codec->encode(new_tpe, tpe_buffer, sizeof(tpe_buffer));
      if (result.second == true) return Rewrite_result{nullptr, 0, 0, true};

      // Now adjust the event buffer itself
      auto new_data_size = result.first + rewritten_payload_size;
      auto new_event_size = LOG_EVENT_HEADER_LEN + new_data_size;
      if (has_crc) new_event_size += BINLOG_CHECKSUM_LEN;
      if (new_event_size > buffer_capacity)
        buffer = (unsigned char *)my_realloc(PSI_NOT_INSTRUMENTED, buffer,
                                             new_event_size, MYF(0));

      // now write everything into the event buffer
      auto ptr = buffer;

      // preserve the current event header, but adjust the event size
      int4store(ptr + EVENT_LEN_OFFSET, new_event_size);
      ptr += LOG_EVENT_HEADER_LEN;

      // add the new tpe header
      memmove(ptr, tpe_buffer, result.first);
      ptr += result.first;

      // add the new payload
      memmove(ptr, rewritten_payload, rewritten_payload_size);
      ptr += rewritten_payload_size;

      // now can free the new payload, as we have moved it to the
      // event buffer
      free(rewritten_payload);

      // recalculate checksum
      if (has_crc) {
        ha_checksum crc{0};
        uchar buf[BINLOG_CHECKSUM_LEN];
        crc = checksum_crc32(crc, buffer, new_event_size - BINLOG_CHECKSUM_LEN);
        int4store(buf, crc);
        memcpy(ptr, buf, sizeof(buf));
      }

      return Rewrite_result{buffer, new_event_size, new_event_size, false};
    }
  };

 protected:
  /**
    A map that establishes the relationship between from the source
    database name that is to be rewritten into the target one.

    The key of the map is the "from" database name. The value of the
    map is is the "to" database name that we are rewritting the
    name into.
   */
  std::map<std::string, std::string> m_dict;

  /**
    A special rewriter for those transactions that are enclosed in a
    Transaction_payload event.
   */
  std::unique_ptr<Transaction_payload_content_rewriter>
      m_transaction_payload_rewriter{nullptr};

  /**
    This function gets the offset in the buffer for the dbname and
    dbname length.

    @param buffer the event buffer
    @param buffer_size the event length
    @param fde the format description event to decode parts of this buffer

    @return a tuple containing:
            - dbname offset
            - dbname length offset
            - boolean specifying whether this is an event that needs rewrite
              checks
            - boolean specifying whether an error was found
  */
  std::tuple<my_off_t, my_off_t, bool, bool> get_dbname_and_dblen_offsets(
      const unsigned char *buffer, size_t buffer_size,
      binary_log::Format_description_event const &fde) {
    my_off_t off_dbname = 0;
    my_off_t off_dbname_len = 0;
    bool error = false;
    bool needs_rewrite_check = false;
    auto event_type = (Log_event_type)buffer[EVENT_TYPE_OFFSET];

    switch (event_type) {
      case binary_log::TABLE_MAP_EVENT: {
        /*
          Before rewriting:

          +-------------+-----------+----------+------+----------------+
          |common_header|post_header|old_db_len|old_db|event data...   |
          +-------------+-----------+----------+------+----------------+

          Note that table map log event uses only one byte for database length.

        */
        off_dbname_len = fde.common_header_len +
                         fde.post_header_len[binary_log::TABLE_MAP_EVENT - 1];
        off_dbname = off_dbname_len + 1;
        needs_rewrite_check = true;
      } break;
      case binary_log::EXECUTE_LOAD_QUERY_EVENT:
      case binary_log::QUERY_EVENT: {
        /*
          The QUERY_EVENT buffer structure:

          Before Rewriting :

            +-------------+-----------+-----------+------+------+
            |common_header|post_header|status_vars|old_db|...   |
            +-------------+-----------+-----------+------+------+

          After Rewriting :

            +-------------+-----------+-----------+------+------+
            |common_header|post_header|status_vars|new_db|...   |
            +-------------+-----------+-----------+------+------+

          The db_len is inside the post header, more specifically:

            +---------+---------+------+--------+--------+------+
            |thread_id|exec_time|db_len|err_code|status_vars_len|
            +---------+---------+------+--------+--------+------+

          Thence we need to change the post header and the payload,
          which is the one carrying the database name.

          In case the new database name is longer than the old database
          length, it will reallocate the buffer.
        */
        uint8 common_header_len = fde.common_header_len;
        uint8 query_header_len =
            fde.post_header_len[binary_log::QUERY_EVENT - 1];
        const unsigned char *ptr = buffer;
        uint sv_len = 0;

        DBUG_EXECUTE_IF("simulate_corrupt_event_len", buffer_size = 0;);
        /* Error if the event content is too small */
        if (buffer_size < (common_header_len + query_header_len)) {
          error = true;
          goto end;
        }

        /* Check if there are status variables in the event */
        if ((query_header_len -
             binary_log::Query_event::QUERY_HEADER_MINIMAL_LEN) > 0) {
          sv_len = uint2korr(ptr + common_header_len +
                             binary_log::Query_event::Q_STATUS_VARS_LEN_OFFSET);
        }

        /* now we have a pointer to the position where the database is. */
        off_dbname_len =
            common_header_len + binary_log::Query_event::Q_DB_LEN_OFFSET;
        off_dbname = common_header_len + query_header_len + sv_len;

        if (off_dbname_len > buffer_size || off_dbname > buffer_size) {
          error = true;
          goto end;
        }

        if (event_type == binary_log::EXECUTE_LOAD_QUERY_EVENT)
          off_dbname += Binary_log_event::EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN;
        needs_rewrite_check = true;
      } break;
      default:
        break;
    }

  end:
    return std::make_tuple(off_dbname, off_dbname_len, needs_rewrite_check,
                           error);
  }

  Rewrite_result rewrite_event(unsigned char *buffer, size_t buffer_capacity,
                               size_t data_size,
                               binary_log::Format_description_event const &fde,
                               bool recalculate_crc = false) {
    auto the_buffer{buffer};
    auto the_buffer_capacity{buffer_capacity};
    auto the_data_size{data_size};
    std::string from{};
    std::string to{};
    int64_t delta{0};
    unsigned char *dbname_ptr{nullptr};
    unsigned char *dbname_len_ptr{nullptr};

    bool error{false};
    bool needs_rewrite{false};
    size_t offset_dbname_len{0};
    size_t offset_dbname{0};
    uint8_t dbname_len{0};
    const char *dbname{nullptr};

    std::tie(offset_dbname, offset_dbname_len, needs_rewrite, error) =
        get_dbname_and_dblen_offsets(buffer, data_size, fde);
    if (error || !needs_rewrite) goto end;

    // build the "from"
    dbname_len = static_cast<uint8_t>(buffer[offset_dbname_len]);
    dbname = reinterpret_cast<const char *>(buffer + offset_dbname);
    from = std::string(dbname, dbname_len);

    // check if we need to continue
    if (!is_rewrite_needed(from)) goto end;

    // if we do, we need to find the name to rewrite to (the "to")
    to = m_dict[from];

    // need to adjust the buffer layout or even reallocate
    delta = to.size() - from.size();
    // need to reallocate
    if ((delta + data_size) > buffer_capacity) {
      the_buffer_capacity = buffer_capacity + delta;
      the_buffer = (unsigned char *)my_realloc(PSI_NOT_INSTRUMENTED, buffer,
                                               the_buffer_capacity, MYF(0));
      /* purecov: begin inspected */
      if (!the_buffer) {
        // OOM
        error = true;
        goto end;
      }
      /* purecov: end */
    }

    // adjust the size of the event
    the_data_size += delta;

    // need to move bytes around in the buffer if needed
    if (the_data_size != data_size) {
      unsigned char *to_tail_ptr = the_buffer + offset_dbname + to.size();
      unsigned char *from_tail_ptr = the_buffer + offset_dbname + from.size();
      size_t to_tail_size = data_size - (offset_dbname + from.size());

      // move the tail (so we do not risk overwriting it)
      memmove(to_tail_ptr, from_tail_ptr, to_tail_size);
    }

    dbname_ptr = the_buffer + offset_dbname;
    memcpy(dbname_ptr, to.c_str(), to.size());

    DBUG_ASSERT(to.size() < UINT8_MAX);
    dbname_len_ptr = the_buffer + offset_dbname_len;
    *dbname_len_ptr = (char)to.size();

    // Update event length in header.
    int4store(the_buffer + EVENT_LEN_OFFSET, the_data_size);

    // now recalculate the checksum
    if (recalculate_crc) {
      auto ptr = the_buffer + the_data_size - BINLOG_CHECKSUM_LEN;
      ha_checksum crc{};
      uchar buf[BINLOG_CHECKSUM_LEN];
      crc = checksum_crc32(crc, the_buffer, (ptr - the_buffer));
      int4store(buf, crc);
      memcpy(ptr, buf, sizeof(buf));
    }

  end:
    return std::make_tuple(the_buffer, the_buffer_capacity, the_data_size,
                           error);
  }

  /**
    This function shall return true if the event needs to be processed for
    rewriting the database.

    @param event_type the event type code.

    @return true if the database needs to be rewritten.
   */
  bool is_rewrite_needed_for_event(Log_event_type event_type) {
    switch (event_type) {
      case binary_log::TABLE_MAP_EVENT:
      case binary_log::EXECUTE_LOAD_QUERY_EVENT:
      case binary_log::QUERY_EVENT:
      case binary_log::TRANSACTION_PAYLOAD_EVENT:
        return true;
      default:
        return false;
    }
  }

 public:
  Database_rewrite() = default;

  ~Database_rewrite() { m_dict.clear(); }

  /**
    Shall register a rule to rewrite from one database name to another.
    @param from the database name to rewrite from.
    @param to the database name to rewrite to.
   */
  void register_rule(std::string from, std::string to) {
    m_dict.insert(std::pair<std::string, std::string>(from, to));
  }

  /**
    Shall unregister a rewrite rule for a given database. If the name is
    not registered, then no action is taken and no error reported.
    The name of database to be used in this invokation is the original
    database name.

    @param from the original database name used when the rewrite rule
                was registered.
   */
  void unregister_rule(std::string from) { m_dict.erase(from); }

  /**
    Returns true if this database name needs to be rewritten.

    @param dbname The database name.
    @return true if a database name rewrite is needed, false otherwise.
   */
  bool is_rewrite_needed(std::string dbname) {
    return !m_dict.empty() && m_dict.find(dbname) != m_dict.end();
  }

  /**
    Shall rewrite the database name in the given buffer. This function
    is called when rewriting events in raw_mode.

    @param buffer the full event still not decoded.
    @param buffer_capacity the event buffer size.
    @param data_size the size of the buffer filled with meaningful data.
    @param fde the format description event to decode the event.
    @param skip_transaction_payload_event Whether to skip the
                                          Transaction_payload_event or not

    @return a tuple containing:
            - A pointer to the buffer after the changes (if any).
            - The buffer capacity size updated.
            - The event data size.
            - A boolean specifying whether there was an error or not.
   */
  Rewrite_result rewrite_raw(unsigned char *buffer, size_t buffer_capacity,
                             size_t data_size,
                             binary_log::Format_description_event const &fde,
                             bool skip_transaction_payload_event = false) {
    DBUG_ASSERT(buffer_capacity >= data_size);
    auto event_type = (Log_event_type)buffer[EVENT_TYPE_OFFSET];
    if (m_dict.empty() || !is_rewrite_needed_for_event(event_type))
      return Rewrite_result{buffer, buffer_capacity, data_size, false};

    switch (event_type) {
      case binary_log::TRANSACTION_PAYLOAD_EVENT: {
        if (!skip_transaction_payload_event) {
          if (m_transaction_payload_rewriter == nullptr)
            m_transaction_payload_rewriter =
                std::make_unique<Transaction_payload_content_rewriter>(*this);
          return m_transaction_payload_rewriter->rewrite_transaction_payload(
              buffer, buffer_capacity, fde);
        } else
          return Rewrite_result{buffer, buffer_capacity, buffer_capacity,
                                false};
      }
      default: {
        bool recalculate_crc =
            fde.footer()->checksum_alg == binary_log::BINLOG_CHECKSUM_ALG_CRC32;
        return rewrite_event(buffer, buffer_capacity, data_size, fde,
                             recalculate_crc);
      }
    }
  }

  /**
    Rewrites the event database if needed. This function is called when
    rewriting events not in raw mode.

    @param buffer the full event still not decoded.
    @param buffer_capacity the event buffer size.
    @param data_size the size of the buffer filled with meaningful data.
    @param fde the format description event to decode the event.

    @return a tuple with the pointer to the buffer with the database rewritten,
            the rewritten buffer capacity, the rewritten buffer meaningful
            bytes, and whether there was an error or not.
   */
  Rewrite_result rewrite(unsigned char *buffer, size_t buffer_capacity,
                         size_t data_size,
                         binary_log::Format_description_event const &fde) {
    return rewrite_raw(buffer, buffer_capacity, data_size, fde, true);
  }
};

/**
  The database rewriter handler for Table map and Query log events.
 */
Database_rewrite global_database_rewriter;

/*
  The character set used should be equal to the one used in mysqld.cc for
  server rewrite-db
*/
#define mysqld_charset &my_charset_latin1

#define CLIENT_CAPABILITIES \
  (CLIENT_LONG_PASSWORD | CLIENT_LONG_FLAG | CLIENT_LOCAL_FILES)

char server_version[SERVER_VERSION_LENGTH];
ulong filter_server_id = 0;

/*
  This strucure is used to store the event and the log postion of the events
  which is later used to print the event details from correct log postions.
  The Log_event *event is used to store the pointer to the current event and
  the event_pos is used to store the current event log postion.
*/
struct buff_event_info {
  Log_event *event = nullptr;
  my_off_t event_pos = 0;
};

/*
  One statement can result in a sequence of several events: Intvar_log_events,
  User_var_log_events, and Rand_log_events, followed by one
  Query_log_event. If statements are filtered out, the filter has to be
  checked for the Query_log_event. So we have to buffer the Intvar,
  User_var, and Rand events and their corresponding log postions until we see
  the Query_log_event. This dynamic array buff_ev is used to buffer a structure
  which stores such an event and the corresponding log position.
*/
typedef Prealloced_array<buff_event_info, 16> Buff_ev;
Buff_ev *buff_ev(PSI_NOT_INSTRUMENTED);

// needed by net_serv.c
ulong bytes_sent = 0L, bytes_received = 0L;
ulong mysqld_net_retry_count = 10L;
ulong open_files_limit;
ulong opt_binlog_rows_event_max_size;
uint test_flags = 0;
static uint opt_protocol = 0;
static uint opt_compress = 0;
static uint opt_print_sql_string = 0;
static uint opt_use_dscp = 0;
static FILE *result_file;

#ifndef DBUG_OFF
static const char *default_dbug_option = "d:t:o,/tmp/mysqlbinlog.trace";
#endif
static const char *load_default_groups[] = {"mysqlbinlog", "client", nullptr};

static bool one_database = false, disable_log_bin = false;
static bool opt_hexdump = false;
const char *base64_output_mode_names[] = {"NEVER", "AUTO", "UNSPEC",
                                          "DECODE-ROWS", NullS};
TYPELIB base64_output_mode_typelib = {
    array_elements(base64_output_mode_names) - 1, "", base64_output_mode_names,
    nullptr};
static enum_base64_output_mode opt_base64_output_mode = BASE64_OUTPUT_UNSPEC;
static char *opt_base64_output_mode_str = nullptr;
static bool opt_remote_alias = false;
const char *remote_proto_names[] = {"BINLOG-DUMP-NON-GTIDS",
                                    "BINLOG-DUMP-GTIDS", NullS};
TYPELIB remote_proto_typelib = {array_elements(remote_proto_names) - 1, "",
                                remote_proto_names, nullptr};
static enum enum_remote_proto {
  BINLOG_DUMP_NON_GTID = 0,
  BINLOG_DUMP_GTID = 1,
  BINLOG_LOCAL = 2
} opt_remote_proto = BINLOG_LOCAL;
static char *opt_remote_proto_str = nullptr;
static char *database = nullptr;
static char *output_file = nullptr;
static char *rewrite = nullptr;
bool force_opt = false, short_form = false, idempotent_mode = false;
static bool debug_info_flag, debug_check_flag;
static bool force_if_open_opt = true, raw_mode = false;
static bool to_last_remote_log = false, stop_never = false;
static bool opt_verify_binlog_checksum = true;
static bool opt_skip_verify_if_open = true;
static ulonglong offset = 0;
static int64 stop_never_slave_server_id = -1;
static int64 connection_server_id = -1;
static char *host = nullptr;
static int port = 0;
static uint my_end_arg;
static const char *sock = nullptr;
static char *opt_plugin_dir = nullptr, *opt_default_auth = nullptr;

#if defined(_WIN32)
static char *shared_memory_base_name = nullptr;
#endif
static char *user = nullptr;
static char *pass = nullptr;
static char *opt_bind_addr = nullptr;
static char *charset = nullptr;

static uint verbose = 0;

static ulonglong start_position, stop_position;
#define start_position_mot ((my_off_t)start_position)
#define stop_position_mot ((my_off_t)stop_position)

static char *start_datetime_str, *stop_datetime_str;
static my_time_t start_datetime = 0, stop_datetime = MY_TIME_T_MAX;
static ulonglong rec_count = 0;
static MYSQL *mysql = nullptr;
static char *dirname_for_local_load = nullptr;
static uint opt_server_id_bits = 0;
ulong opt_server_id_mask = 0;
Sid_map *global_sid_map = nullptr;
Checkable_rwlock *global_sid_lock = nullptr;
Gtid_set *gtid_set_included = nullptr;
Gtid_set *gtid_set_excluded = nullptr;
static uint opt_zstd_compress_level = default_zstd_compression_level;
static char *opt_compress_algorithm = nullptr;
Gtid_set *gtid_set_stop = nullptr;
static bool opt_print_gtids = false;

static bool opt_print_table_metadata;

/**
 * Used for --opt_skip_empty_trans
 */
static Log_event *begin_query_ev_cache = nullptr;
static string cur_database = "";

enum class Check_database_decision : char {
  EMPTY_EVENT_DATABASE = 2,
  CHANGED = 1,
  OK = 0,
  ERROR = -1
};

/**
  Exit status for functions in this file.
*/
enum Exit_status {
  /** No error occurred and execution should continue. */
  OK_CONTINUE = 0,
  /** An error occurred and execution should stop. */
  ERROR_STOP,
  /** No error occurred but execution should stop. */
  OK_STOP
};

/*
  Options that will be used to filter out events.
*/
static char *opt_include_gtids_str = nullptr, *opt_exclude_gtids_str = nullptr,
            *opt_include_gtids_from_file_str = nullptr,
            *opt_exclude_gtids_from_file_str = nullptr,
            *opt_start_gtid_str = nullptr, *opt_find_gtid_str = nullptr,
            *opt_stop_gtid_str = nullptr;
static char *opt_index_file_str = nullptr;
Gtid_set_map previous_gtid_set_map;
static bool opt_skip_gtids = false;
static bool opt_skip_rows_query =
    false; /* Placeholder for skip_rows_query diff */
static bool opt_skip_empty_trans = 0;
static bool filter_based_on_gtids = false;
static bool opt_require_row_format = false;

/* It is set to true when BEGIN is found, and false when the transaction ends.
 */
static bool in_transaction = false;
/* It is set to true when GTID is found, and false when the transaction ends. */
static bool seen_gtid = false;

static uint opt_receive_buffer_size = 0;

static Exit_status dump_local_log_entries(PRINT_EVENT_INFO *print_event_info,
                                          const char *logname);
static Exit_status dump_remote_log_entries(PRINT_EVENT_INFO *print_event_info,
                                           const char *logname);
static Exit_status dump_single_log(PRINT_EVENT_INFO *print_event_info,
                                   const char *logname);
static Exit_status dump_multiple_logs(int argc, char **argv);
static Exit_status safe_connect();

struct buff_event_info buff_event;

// the last seen rows_query event buffered so we can check database/table
// filters in the table_map/query event before printing it
static buff_event_info last_rows_query_event;
// the `temp_buf` data member of the last seen rows_query event, this stores the
// serialized event and is used to print the event in base64 format
static char *last_rows_query_event_temp_buf = nullptr;

class Load_log_processor {
  char target_dir_name[FN_REFLEN];
  size_t target_dir_name_len;

  /*
    When we see first event corresponding to some LOAD DATA statement in
    binlog, we create temporary file to store data to be loaded.
    We add name of this file to file_names set using its file_id as index.
  */
  struct File_name_record {
    char *fname;
  };

  typedef std::map<uint, File_name_record> File_names;
  File_names file_names;

  /**
    Looks for a non-existing filename by adding a numerical suffix to
    the given base name, creates the generated file, and returns the
    filename by modifying the filename argument.

    @param[in,out] filename Base filename

    @param[in,out] file_name_end Pointer to last character of
    filename.  The numerical suffix will be written to this position.
    Note that there must be a least five bytes of allocated memory
    after file_name_end.

    @retval -1 Error (can't find new filename).
    @retval >=0 Found file.
  */
  File create_unique_file(char *filename, char *file_name_end) {
    File res;
    /* If we have to try more than 1000 times, something is seriously wrong */
    for (uint version = 0; version < 1000; version++) {
      sprintf(file_name_end, "-%x", version);
      if ((res = my_create(filename, 0, O_CREAT | O_EXCL | O_WRONLY, MYF(0))) !=
          -1)
        return res;
    }
    return -1;
  }

 public:
  Load_log_processor() : file_names() {}
  ~Load_log_processor() {}

  void init_by_dir_name(const char *dir) {
    target_dir_name_len =
        (convert_dirname(target_dir_name, dir, NullS) - target_dir_name);
  }
  void init_by_cur_dir() {
    if (my_getwd(target_dir_name, sizeof(target_dir_name), MYF(MY_WME)))
      exit(1);
    target_dir_name_len = strlen(target_dir_name);
  }
  void destroy() {
    File_names::iterator iter = file_names.begin();
    File_names::iterator end = file_names.end();
    for (; iter != end; ++iter) {
      File_name_record *ptr = &iter->second;
      if (ptr->fname) {
        my_free(ptr->fname);
        memset(ptr, 0, sizeof(File_name_record));
      }
    }

    file_names.clear();
  }

  /**
    Obtain file name of temporary file for LOAD DATA statement by its
    file_id and remove it from this Load_log_processor's list of events.

    @param[in] file_id Identifier for the LOAD DATA statement.

    Checks whether we have already seen Begin_load_query event for
    this file_id. If yes, returns the file name of the corresponding
    temporary file and removes the filename from the array of active
    temporary files.  From this moment, the caller is responsible for
    freeing the memory occupied by this name.

    @return String with the name of the temporary file, or NULL if we
    have not seen any Begin_load_query_event with this file_id.
  */
  char *grab_fname(uint file_id) {
    File_name_record *ptr;
    char *res = nullptr;

    File_names::iterator it = file_names.find(file_id);
    if (it == file_names.end()) return nullptr;
    ptr = &((*it).second);
    res = ptr->fname;
    memset(ptr, 0, sizeof(File_name_record));
    return res;
  }
  Exit_status process(Begin_load_query_log_event *ce);
  Exit_status process(Append_block_log_event *ae);
  Exit_status process_first_event(const char *bname, size_t blen,
                                  const uchar *block, size_t block_len,
                                  uint file_id);
};

/**
  Process the first event in the sequence of events representing a
  LOAD DATA statement.

  Creates a temporary file to be used in LOAD DATA and writes first block of
  data to it. Registers its file name in the array of active temporary files.

  @param bname Base name for temporary file to be created.
  @param blen Base name length.
  @param block First block of data to be loaded.
  @param block_len First block length.
  @param file_id Identifies the LOAD DATA statement.
  this type of event.

  @retval ERROR_STOP An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
*/
Exit_status Load_log_processor::process_first_event(const char *bname,
                                                    size_t blen,
                                                    const uchar *block,
                                                    size_t block_len,
                                                    uint file_id) {
  size_t full_len = target_dir_name_len + blen + 9 + 9 + 1;
  Exit_status retval = OK_CONTINUE;
  char *fname, *ptr;
  File file;
  File_name_record rec;
  DBUG_TRACE;

  if (!(fname =
            (char *)my_malloc(PSI_NOT_INSTRUMENTED, full_len, MYF(MY_WME)))) {
    error("Out of memory.");
    return ERROR_STOP;
  }

  memcpy(fname, target_dir_name, target_dir_name_len);
  ptr = fname + target_dir_name_len;
  memcpy(ptr, bname, blen);
  ptr += blen;
  ptr += sprintf(ptr, "-%x", file_id);

  if ((file = create_unique_file(fname, ptr)) < 0) {
    error("Could not construct local filename %s%s.", target_dir_name, bname);
    my_free(fname);
    return ERROR_STOP;
  }

  rec.fname = fname;

  /*
     fname is freed in process_event()
     after Execute_load_query_log_event or Execute_load_log_event
     will have been processed, otherwise in Load_log_processor::destroy()
  */
  file_names[file_id] = rec;

  if (my_write(file, pointer_cast<const uchar *>(block), block_len,
               MYF(MY_WME | MY_NABP))) {
    error("Failed writing to file.");
    retval = ERROR_STOP;
  }
  if (my_close(file, MYF(MY_WME))) {
    error("Failed closing file.");
    retval = ERROR_STOP;
  }
  return retval;
}

/**
  Process the given Begin_load_query_log_event.

  @see Load_log_processor::process_first_event(const char*,uint,const
  char*,uint,uint)

  @param blqe Begin_load_query_log_event to process.

  @retval ERROR_STOP An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
*/
Exit_status Load_log_processor::process(Begin_load_query_log_event *blqe) {
  return process_first_event("SQL_LOAD_MB", 11, blqe->block, blqe->block_len,
                             blqe->file_id);
}

/**
  Process the given Append_block_log_event.

  Appends the chunk of the file contents specified by the event to the
  file created by a previous Begin_load_query_log_event.

  If the file_id for the event does not correspond to any file
  previously registered through a Begin_load_query_log_event,
  this member function will print a warning and
  return OK_CONTINUE.  It is safe to return OK_CONTINUE, because no
  query will be written for this event.  We should not print an error
  and fail, since the missing file_id could be because a (valid)
  --start-position has been specified after the Begin_load_query_log_event but
  before this Append event.

  @param ae Append_block_log_event to process.

  @retval ERROR_STOP An error occurred - the program should terminate.

  @retval OK_CONTINUE No error, the program should continue.
*/
Exit_status Load_log_processor::process(Append_block_log_event *ae) {
  DBUG_TRACE;
  File_names::iterator it = file_names.find(ae->file_id);
  const char *fname = ((it != file_names.end()) ? (*it).second.fname : nullptr);

  if (fname) {
    File file;
    Exit_status retval = OK_CONTINUE;
    if (((file = my_open(fname, O_APPEND | O_WRONLY, MYF(MY_WME))) < 0)) {
      error("Failed opening file %s", fname);
      return ERROR_STOP;
    }
    if (my_write(file, (uchar *)ae->block, ae->block_len,
                 MYF(MY_WME | MY_NABP))) {
      error("Failed writing to file %s", fname);
      retval = ERROR_STOP;
    }
    if (my_close(file, MYF(MY_WME))) {
      error("Failed closing file %s", fname);
      retval = ERROR_STOP;
    }
    return retval;
  }

  /*
    There is no Begin_load_query_log_event (a bad binlog or a big
    --start-position). Assuming it's a big --start-position, we just do
    nothing and print a warning.
  */
  warning(
      "Ignoring Append_block as there is no "
      "Begin_load_query_log_event for file_id: %u",
      ae->file_id);
  return OK_CONTINUE;
}

static Load_log_processor load_processor;

/**
  Replace windows-style backslashes by forward slashes so it can be
  consumed by the mysql client, which requires Unix path.

  @todo This is only useful under windows, so may be ifdef'ed out on
  other systems.  /Sven

  @todo If a Begin_load_query_log_event contains a filename with a
  backslash (valid under unix), then we have problems under windows.
  /Sven

  @param[in,out] fname Filename to modify. The filename is modified
  in-place.
*/
static void convert_path_to_forward_slashes(char *fname) {
  while (*fname) {
    if (*fname == '\\') *fname = '/';
    fname++;
  }
}

/**
  Indicates whether the given database should be filtered out,
  according to the --database=X option.

  @param log_dbname Name of database.

  @return nonzero if the database with the given name should be
  filtered out, 0 otherwise.
*/
static bool shall_skip_database(const char *log_dbname) {
  return one_database && (log_dbname != nullptr) &&
         strcmp(log_dbname, database);
}

/**
  Checks whether to stop receiving more events based on the stop-gtids option.
  Note that we stop if the current event is *outside* stop-gtids, as opposed to
  if the current event is *within* stop-gtids. This is a bit counter-intuitive
  but unfortunately it is too late to address this now.

  @param[in] ev Pointer to the event to be checked.

  @return true if it's time to stop receiving
          false, otherwise.
*/
static bool shall_stop_gtids(Log_event *ev) {
  if (opt_stop_gtid_str != NULL) {
    if (ev->get_type_code() == binary_log::GTID_LOG_EVENT ||
        ev->get_type_code() == binary_log::ANONYMOUS_GTID_LOG_EVENT) {
      Gtid_log_event *gtid = (Gtid_log_event *)ev;
      return !gtid_set_stop->contains_gtid(gtid->get_sidno(true),
                                           gtid->get_gno());
    }
  }
  return false;
}

/**
  Checks whether the given event should be filtered out,
  according to the include-gtids, exclude-gtids and
  skip-gtids options.

  @param[in] ev Pointer to the event to be checked.
  @param[out] cached_gtid Store the gtid here.

  @return true if the event should be filtered out,
          false, otherwise.
*/
static bool shall_skip_gtids(const Log_event *ev, Gtid *cached_gtid) {
  bool filtered = false;

  switch (ev->get_type_code()) {
    case binary_log::GTID_LOG_EVENT:
    case binary_log::ANONYMOUS_GTID_LOG_EVENT: {
      Gtid_log_event *gtid =
          const_cast<Gtid_log_event *>(down_cast<const Gtid_log_event *>(ev));
      if (gtid->get_sidno(true) > 0 && gtid->get_gno() != 0) {
        cached_gtid->set(gtid->get_sidno(true), gtid->get_gno());
      }
      if (opt_include_gtids_str != nullptr) {
        filtered = filtered || !gtid_set_included->contains_gtid(
                                   gtid->get_sidno(true), gtid->get_gno());
      }

      if (opt_exclude_gtids_str != nullptr) {
        filtered = filtered || gtid_set_excluded->contains_gtid(
                                   gtid->get_sidno(true), gtid->get_gno());
      }
      filter_based_on_gtids = filtered;
      filtered = filtered || opt_skip_gtids;
    } break;
    /* Skip previous gtids if --skip-gtids is set. */
    case binary_log::PREVIOUS_GTIDS_LOG_EVENT:
      filtered = opt_skip_gtids;
      if (opt_print_gtids) {
        const Previous_gtids_log_event *pgev =
            down_cast<const Previous_gtids_log_event *>(ev);
        global_sid_lock->rdlock();
        pgev->add_to_set(gtid_set_excluded);
        global_sid_lock->unlock();
      }
      break;

    /*
      Transaction boundaries reset the global filtering flag.

      Since in the relay log a transaction can span multiple
      log files, we do not reset filter_based_on_gtids flag when
      processing control events (they can appear in the middle
      of a transaction). But then, if:

        FILE1: ... GTID BEGIN QUERY QUERY COMMIT ROTATE
        FILE2: FD BEGIN QUERY QUERY COMMIT

      Events on the second file would not be outputted, even
      though they should.
    */
    case binary_log::XID_EVENT:
      filtered = filter_based_on_gtids;
      filter_based_on_gtids = false;
      break;
    case binary_log::QUERY_EVENT:
      filtered = filter_based_on_gtids;
      if (down_cast<const Query_log_event *>(ev)->ends_group())
        filter_based_on_gtids = false;
      break;

    /*
      Never skip STOP, FD, ROTATE, IGNORABLE or INCIDENT events.
      SLAVE_EVENT and START_EVENT_V3 are there for completion.

      Although in the binlog transactions do not span multiple
      log files, in the relay-log, that can happen. As such,
      we need to explicitly state that we do not filter these
      events, because there is a chance that they appear in the
      middle of a filtered transaction, e.g.:

         FILE1: ... GTID BEGIN QUERY QUERY ROTATE
         FILE2: FD QUERY QUERY COMMIT GTID BEGIN ...

      In this case, ROTATE and FD events should be processed and
      outputted.
    */
    case binary_log::STOP_EVENT:
    case binary_log::FORMAT_DESCRIPTION_EVENT:
    case binary_log::ROTATE_EVENT:
    case binary_log::IGNORABLE_LOG_EVENT:
    case binary_log::INCIDENT_EVENT:
      filtered = false;
      break;
    case binary_log::METADATA_EVENT:
      filtered = filter_based_on_gtids;
      if (((const Metadata_log_event *)ev)
              ->does_exist(
                  Metadata_log_event::Metadata_event_types::PREV_HLC_TYPE)) {
        /* Filter metadata event if it contains prev hlc timestamp */
        filtered = true;
      }
      break;
    default:
      filtered = filter_based_on_gtids;
      break;
  }

  return filtered;
}

/**
  Helper function that prints the cached begin query event to the output

  @param[in] print_event_info print event info pointer

  @retval True   OK
  @retval False  ERROR
*/
static bool print_cached_begin_query(PRINT_EVENT_INFO *print_event_info) {
  begin_query_ev_cache->print(result_file, print_event_info);
  auto head = &print_event_info->head_cache;
  if (head->error == -1) {
    return false;
  }
  return true;
}

/**
  Checks whether a given event within a transaction has changed the database
  that the current transaction is operating on. If the event within a
  transaction has changed the database of the current transaction, an error
  message will be issued. This check is only valid when --skip-empty-trans
  is specified. This function also will check and print the previous cached
  BEGIN event since it's possible that the previous BEGIN event doesn't carry
  a session database name.

  @param[in] ev_database the database that the current event is operating on.
  @param[in] print_event_info print event info pointer

  @retval CHANGED if the database of the current transaction has been changed by
  the current event.
  @retval OK if the database of the current transaction has not been changed
  by the current event.
  @retval ERROR if there is an error on writing the cached BEGIN event out.
*/
static Check_database_decision ev_database_changed(
    const string &ev_database, PRINT_EVENT_INFO *print_event_info) {
  if (opt_skip_empty_trans  // When --skip-empty-trans is enabled
      && in_transaction)    // When the event is within a transaction
  {
    /**
      This function is called at every event that carries a database name other
      than BEGIN query. So the ev_database name should not be empty
     */
    if (ev_database.empty()) {
      error(
          "The current event does not carry a session database name. This is "
          "not supported");
      return Check_database_decision::EMPTY_EVENT_DATABASE;
    }
    /**
      If the previous BEGIN query doesn't have a database name, we change the
      current transaction's database name to the ev_database, and also flush
      the previous BEGIN event if it should not be skipped.
     */
    if (begin_query_ev_cache != nullptr && cur_database.empty()) {
      cur_database = ev_database;
      if (!shall_skip_database(cur_database.c_str()) &&
          !print_cached_begin_query(print_event_info)) {
        return Check_database_decision::ERROR;
      }
      return Check_database_decision::OK;
    }
    // When the event database is changed
    if (cur_database != ev_database) {
      error(
          "The database used for the current "
          "transaction has been changed since "
          "BEGIN. This is not supported! The database "
          "of the current transaction that is updated from the "
          "BEGIN query event is: %s, however, the current event's "
          "database is: %s",
          cur_database.c_str(), ev_database.c_str());

      return Check_database_decision::CHANGED;
    }
  }
  return Check_database_decision::OK;
}

/**
  Print auxiliary statements ending a binary log (or a logical binary log
  within a sequence of relay logs; see below).

  There are two kinds of log files which can be printed by mysqlbinlog
  binlog file   - generated by mysql server when binlog is ON.
  relaylog file - generated by slave IO thread. It just stores binlog
                  replicated from master with an extra header(FD event,
                  Previous_gtid_log_event) and a tail(rotate event).
  when printing the events in relay logs, the purpose is to print
  the events generated by master, but not slave.

  There are three types of FD events:
  - Slave FD event: has F_RELAY_LOG set and end_log_pos > 0
  - Real master FD event: has F_RELAY_LOG cleared and end_log_pos > 0
  - Fake master FD event: has F_RELAY_LOG cleared and end_log_pos == 0

  (Two remarks:

  - The server_id of a slave FD event is the slave's server_id, and
    the server_id of a master FD event (real or fake) is the
    master's server_id. But this does not help to distinguish the
    types in case replicate-same-server-id is enabled.  So to
    determine the type of event we need to check the F_RELAY_LOG
    flag.

  - A fake master FD event may be generated by master's dump
    thread (then it takes the first event of the binlog and sets
    end_log_pos=0), or by the slave (then it takes the last known
    real FD event and sets end_log_pos=0.)  There is no way to
    distinguish master-generated fake master FD events from
    slave-generated fake master FD events.
  )

  There are 8 cases where we rotate a relay log:

  R1. After FLUSH [RELAY] LOGS
  R2. When mysqld receives SIGHUP
  R3. When relay log size grows too big
  R4. Immediately after START SLAVE
  R5. When slave IO thread reconnects without user doing
      START SLAVE/STOP SLAVE
  R6. When master dump thread starts a new binlog
  R7. CHANGE MASTER which deletes all relay logs
  R8. RESET SLAVE

  (Remark: CHANGE MASTER which does not delete any relay log,
  does not cause any rotation at all.)

  The 8 cases generate the three types of FD events as follows:
  - In all cases, a slave FD event is generated.
  - In cases R1 and R2, if the slave has been connected
    previously, the slave client thread that issues
    FLUSH (or the thread that handles the SIGHUP) generates a
    fake master FD event. If the slave has not been connected
    previously, there is no master FD event.
  - In case R3, the slave IO thread generates a fake master FD
    event.
  - In cases R4 and R5, if AUTOPOSITION=0 and MASTER_LOG_POS>4,
    the master dump thread generates a fake master FD event.
  - In cases R4 and R5, if AUTOPOSITION=1 or MASTER_LOG_POS<=4,
    the master dump thread generates a real master FD event.
  - In case R6, the master dump thread generates a real master FD
    event.
  - In cases R7 and R8, the slave does not generate any master FD
    event.

  We define the term 'logical binlog' as a sequence of events in
  relay logs, such that a single logical binlog may span multiple
  relay log files, and any two logical binlogs are separated by a
  real master FD event.

  A transaction's events will never be divided into two binlog files or
  two logical binlogs. But a transaction may span multiple relay logs, in which
  case a faked FD will appear in the middle of the transaction. they may be
  divided by fake master FD event and/or slave FD events.

  * Example 1

    relay-log.1
    ...
    GTID_NEXT=1
    BEGIN;

    relay-log.2
    ...
    faked Format_description_event
    INSERT ...
    COMMIT;

    For above case, it has only one logical binlog. The events
    in both relay-log.1 and relay-log.2 belong to the same logical binlog.

  * Example 2

    relay-log.1
    ...
    GTID_NEXT=1
    BEGIN;      // It is a partial transaction at the end of logical binlog

    relay-log.2
    ...
    real Format_description_event
    GTID_NEXT=1
    BEGIN;
    ...

    For above case, it has two logical binlogs. Events in relay-log.1
    and relay-log.2 belong to two different logical binlog.

  Logical binlog is handled in a similar way as a binlog file. At the end of a
  binlog file, at the end of a logical binlog or at the end of mysqlbinlog it
  should
  - rollback the last transaction if it is not complete
  - rollback the last gtid if the last event is a gtid_log_event
  - set gtid_next to AUTOMATIC

  This function is called two places:
  - Before printing a real Format_description_log_event(excluding the
    first Format_description_log_event), while mysqlbinlog is in the middle
    of printing all log files(binlog or relaylog).
  - At the end of mysqlbinlog, just after printing all log files(binlog or
    relaylog).

  @param[in,out] print_event_info Context state determining how to print.
*/
void end_binlog(PRINT_EVENT_INFO *print_event_info) {
  if (in_transaction) {
    fprintf(result_file, "ROLLBACK /* added by mysqlbinlog */ %s\n",
            print_event_info->delimiter);
  } else if (seen_gtid && !opt_skip_gtids) {
    /*
      If we are here, then we have seen only GTID_LOG_EVENT
      of a transaction and did not see even a BEGIN event
      (in_transaction flag is false). So generate BEGIN event
      also along with ROLLBACK event.
    */
    fprintf(result_file,
            "BEGIN /*added by mysqlbinlog */ %s\n"
            "ROLLBACK /* added by mysqlbinlog */ %s\n",
            print_event_info->delimiter, print_event_info->delimiter);
  }

  if (!opt_skip_gtids)
    fprintf(result_file, "%sAUTOMATIC' /* added by mysqlbinlog */ %s\n",
            Gtid_log_event::SET_STRING_PREFIX, print_event_info->delimiter);

  seen_gtid = false;
  in_transaction = false;
}

void handle_last_rows_query_event(bool print,
                                  PRINT_EVENT_INFO *print_event_info) {
  if (!last_rows_query_event.event) return;
  auto old_temp_buf = last_rows_query_event.event->temp_buf;
  last_rows_query_event.event->register_temp_buf(
      last_rows_query_event_temp_buf);
  if (print) {
    my_off_t temp_log_pos = last_rows_query_event.event_pos;
    auto old_hexdump_from = print_event_info->hexdump_from;
    print_event_info->hexdump_from = (opt_hexdump ? temp_log_pos : 0);
    last_rows_query_event.event->print(result_file, print_event_info);
    print_event_info->hexdump_from = old_hexdump_from;
  }
  last_rows_query_event.event->register_temp_buf(old_temp_buf);
  my_free(last_rows_query_event_temp_buf);
  last_rows_query_event_temp_buf = nullptr;
  delete last_rows_query_event.event;
  last_rows_query_event.event = nullptr;
  last_rows_query_event.event_pos = 0;
}

// Helper for next function
static bool encounter_gtid(Gtid const &cached_gtid) {
  global_sid_lock->rdlock();
  if (!cached_gtid.is_empty()) {
    if (gtid_set_excluded->ensure_sidno(cached_gtid.sidno) !=
        RETURN_STATUS_OK) {
      global_sid_lock->unlock();
      return true;
    }
    gtid_set_excluded->_add_gtid(cached_gtid);
  }
  global_sid_lock->unlock();
  return false;
}

/**
  Print the given event, and either delete it or delegate the deletion
  to someone else.

  The deletion may be delegated in these cases:
  - the event is a Create_file_log_event, and is saved in load_processor.
  - the event is an Intvar, Rand or User_var event, it will be kept until
    the subsequent Query_log_event.
  - the event is a Table_map_log_event, it will be kept until the subsequent
    Rows_log_event.
  @param[in,out] print_event_info Parameters and context state
  determining how to print.
  @param[in] ev Log_event to process.
  @param[in] pos Offset from beginning of binlog file.
  @param[in] logname Name of input binlog.
  @param[in] skip_pos_check skip filename and position check.

  @retval ERROR_STOP An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
  @retval OK_STOP No error, but the end of the specified range of
  events to process has been reached and the program should terminate.
*/
static Exit_status process_event(PRINT_EVENT_INFO *print_event_info,
                                 Log_event *ev, my_off_t pos,
                                 const char *logname,
                                 bool skip_pos_check = false) {
  char ll_buff[21];
  Log_event_type ev_type = ev->get_type_code();
  DBUG_TRACE;
  Exit_status retval = OK_CONTINUE;
  IO_CACHE *const head = &print_event_info->head_cache;
  static Gtid cached_gtid;

  /*
    Format events are not concerned by --offset and such, we always need to
    read them to be able to process the wanted events.
  */
  if (((rec_count >= offset) &&
       ((my_time_t)(ev->common_header->when.tv_sec) >= start_datetime)) ||
      (ev_type == binary_log::FORMAT_DESCRIPTION_EVENT)) {
    if (ev_type != binary_log::FORMAT_DESCRIPTION_EVENT) {
      /*
        We have found an event after start_datetime, from now on print
        everything (in case the binlog has timestamps increasing and
        decreasing, we do this to avoid cutting the middle).
      */
      start_datetime = 0;
      offset = 0;  // print everything and protect against cycling rec_count
      /*
        Skip events according to the --server-id flag.  However, don't
        skip format_description or rotate events, because they they
        are really "global" events that are relevant for the entire
        binlog, even if they have a server_id.  Also, we have to read
        the format_description event so that we can parse subsequent
        events.
      */
      if (ev_type != binary_log::ROTATE_EVENT && filter_server_id &&
          (filter_server_id != ev->server_id))
        goto end;
    }

    // reached stop position, but make sure that we print all events
    // that have the same position (compressed events)
    if (!skip_pos_check && (pos >= stop_position_mot)) {
      /* end the program */
      retval = OK_STOP;
      goto end;
    }

    // reached stop time
    if (((my_time_t)(ev->common_header->when.tv_sec) >= stop_datetime) ||
        shall_stop_gtids(ev)) {
      /* end the program */
      retval = OK_STOP;
      goto end;
    }
    if (!short_form)
      my_b_printf(&print_event_info->head_cache, "# at %s\n",
                  llstr(pos, ll_buff));

    if (!opt_hexdump)
      print_event_info->hexdump_from = 0; /* Disabled */
    else
      print_event_info->hexdump_from = pos;

    DBUG_PRINT("debug", ("event_type: %s", ev->get_type_str()));

    if (shall_skip_gtids(ev, &cached_gtid)) goto end;

    switch (ev_type) {
      case binary_log::TRANSACTION_PAYLOAD_EVENT:
        ev->print(result_file, print_event_info);
        if (head->error == -1) goto err;
        break;
      case binary_log::QUERY_EVENT: {
        Query_log_event *qle = (Query_log_event *)ev;
        bool parent_query_skips =
            !qle->is_trans_keyword() && shall_skip_database(qle->db);
        bool ends_group = ((Query_log_event *)ev)->ends_group();
        bool starts_group = ((Query_log_event *)ev)->starts_group();

        if (!starts_group &&
            ev_database_changed(string(((Query_log_event *)ev)->db),
                                print_event_info) !=
                Check_database_decision::OK) {
          goto err;
        }

        for (size_t i = 0; i < buff_ev->size(); i++) {
          buff_event_info pop_event_array = buff_ev->at(i);
          Log_event *temp_event = pop_event_array.event;
          my_off_t temp_log_pos = pop_event_array.event_pos;
          print_event_info->hexdump_from = (opt_hexdump ? temp_log_pos : 0);
          if (!parent_query_skips)
            temp_event->print(result_file, print_event_info);
          delete temp_event;
        }

        // dbug_case: rows_query event comes in SBR only when
        // binlog_trx_meta_data is enabled and it comes just before the real
        // query event (i.e. not a trx keyword like BEGIN, COMMIT etc.)
        DBUG_ASSERT(
            !last_rows_query_event.event ||
            (static_cast<Rows_query_log_event *>(last_rows_query_event.event)
                 ->has_trx_meta_data() &&
             !((Query_log_event *)ev)->is_trans_keyword()));

        // when binlog_trx_meta_data is enabled we will get rows_query event
        // before query events
        handle_last_rows_query_event(
            !opt_skip_rows_query && !parent_query_skips, print_event_info);

        print_event_info->hexdump_from = (opt_hexdump ? pos : 0);
        buff_ev->clear();

        if (parent_query_skips) {
          /*
            Even though there would be no need to set the flag here,
            since parent_query_skips is never true when handling "COMMIT"
            statements in the Query_log_event, we still need to handle DDL,
            which causes a commit itself.
          */

          if (seen_gtid && !in_transaction && !starts_group && !ends_group) {
            /*
              For DDLs, print the COMMIT right away.
            */
            fprintf(result_file, "COMMIT /* added by mysqlbinlog */%s\n",
                    print_event_info->delimiter);
            print_event_info->skipped_event_in_transaction = false;
            in_transaction = false;
            seen_gtid = false;
          } else
            print_event_info->skipped_event_in_transaction = true;
          goto end;
        }

        if (ends_group) {
          in_transaction = false;
          print_event_info->skipped_event_in_transaction = false;
          seen_gtid = false;

          /*
            Skip the COMMIT/ROLLBACK event of the extra databases
            when the option --skip-empty-trans is enabled. We also clear the
            cur_database at the end of currrent transaction
           */
          if (opt_skip_empty_trans) {
            bool skip = shall_skip_database(cur_database.c_str());
            // Delete and reset the cache pointer
            delete begin_query_ev_cache;
            begin_query_ev_cache = nullptr;
            // Reset the database tracking as well
            cur_database = "";
            if (skip) break;
          }
          if (opt_print_gtids && encounter_gtid(cached_gtid)) goto err;
        } else if (starts_group) {
          in_transaction = true;

          if (opt_skip_empty_trans) {
            if (cur_database != "") {
              error(
                  "The database used from the previous transaction has not "
                  "been cleared. This probably means that the previous "
                  "transaction has not hit COMMIT/ROLLBACK yet.");
              goto err;
            }
            /*
             * cur_database is always assigned at the BEGIN of a transaction and
             * cleared at the COMMIT/ROLLBACK of a transaction.
             */
            cur_database = string(((Query_log_event *)ev)->db);
            if (cur_database.empty()) {
              // The cache should definitely be cleared
              assert(begin_query_ev_cache == nullptr);
              // Cache the new BEGIN event
              begin_query_ev_cache = ev;
              // Do not destroy, we will handle the detroy ourselves
              ev = nullptr;
              break;
            }
            /*
             * skip the BEGIN query of the extra databases when the option
             * --skip-empty-trans is enabled
             */
            if (shall_skip_database(cur_database.c_str())) goto end;
          }
        } else {
          /*
            We are not in a transaction and are not seeing a BEGIN or
            COMMIT. So this is an implicitly committing DDL.
           */
          if (!in_transaction) seen_gtid = false;
        }

        ev->print(result_file, print_event_info);
        if (head->error == -1) goto err;
        break;
      }

      case binary_log::INTVAR_EVENT: {
        buff_event.event = ev;
        buff_event.event_pos = pos;
        buff_ev->push_back(buff_event);
        ev = nullptr;
        break;
      }

      case binary_log::RAND_EVENT: {
        buff_event.event = ev;
        buff_event.event_pos = pos;
        buff_ev->push_back(buff_event);
        ev = nullptr;
        break;
      }

      case binary_log::USER_VAR_EVENT: {
        buff_event.event = ev;
        buff_event.event_pos = pos;
        buff_ev->push_back(buff_event);
        ev = nullptr;
        break;
      }
      case binary_log::APPEND_BLOCK_EVENT:
        /*
          Append_block_log_events can safely print themselves even if
          the subsequent call load_processor.process fails, because the
          output of Append_block_log_event::print is only a comment.
        */
        ev->print(result_file, print_event_info);

        if (opt_print_gtids && encounter_gtid(cached_gtid)) goto err;

        if (head->error == -1) goto err;
        if ((retval = load_processor.process((Append_block_log_event *)ev)) !=
            OK_CONTINUE)
          goto end;
        break;
      case binary_log::FORMAT_DESCRIPTION_EVENT: {
        /*
          end_binlog is not called on faked fd and relay log's fd.
          Faked FD's log_pos is always 0.
          Faked FD happens in below cases:
          - first FD sent from master to slave if dump request's position is
            greater than 4(when using COM_BINLOG_DUMP, autoposition is 0).
          - Slave fakes a master's FD when rotating relay log through
            'FLUSH LOGS | FLUSH RELAY LOGS', or get the signal SIGHUP.
        */
        if (!ev->is_relay_log_event()) {
          /* Assignment copy. We need this to be able to check later,
             for instance, that this FD has checksums enabled. */
          glob_description_event =
              dynamic_cast<Format_description_event &>(*ev);
          static bool is_first_fd = true;

          /*
            Before starting next binlog or logical binlog, it should end the
            previous binlog first. For detail, see the comment of end_binlog().
          */
          if (ev->common_header->log_pos > 0 && !is_first_fd)
            end_binlog(print_event_info);

          is_first_fd = false;
        }

        print_event_info->common_header_len =
            dynamic_cast<Format_description_event *>(ev)->common_header_len;
        ev->print(result_file, print_event_info);

        if (head->error == -1) goto err;
        if (!force_if_open_opt &&
            (ev->common_header->flags & LOG_EVENT_BINLOG_IN_USE_F)) {
          error(
              "Attempting to dump binlog '%s', which was not closed properly. "
              "Most probably, mysqld is still writing it, or it crashed. "
              "Rerun with --force-if-open to ignore this problem.",
              logname);
          goto err;
        }
        break;
      }
      case binary_log::BEGIN_LOAD_QUERY_EVENT:
        ev->print(result_file, print_event_info);
        if (head->error == -1) goto err;
        if ((retval = load_processor.process(
                 (Begin_load_query_log_event *)ev)) != OK_CONTINUE)
          goto end;
        break;
      case binary_log::EXECUTE_LOAD_QUERY_EVENT: {
        Execute_load_query_log_event *exlq = (Execute_load_query_log_event *)ev;
        char *fname = load_processor.grab_fname(exlq->file_id);
        if (ev_database_changed(string(exlq->db), print_event_info) !=
            Check_database_decision::OK) {
          goto err;
        }
        if (shall_skip_database(exlq->db))
          print_event_info->skipped_event_in_transaction = true;
        else {
          if (fname) {
            convert_path_to_forward_slashes(fname);
            exlq->print(result_file, print_event_info, fname);
            if (head->error == -1) {
              if (fname) my_free(fname);
              goto err;
            }
          } else
            warning(
                "Ignoring Execute_load_query since there is no "
                "Begin_load_query event for file_id: %u",
                exlq->file_id);
        }

        if (fname) my_free(fname);
        break;
      }
      case binary_log::ROWS_QUERY_LOG_EVENT: {
        if (last_rows_query_event.event || last_rows_query_event_temp_buf) {
          error(
              "Found a buffered rows_query event while processing one. This "
              "might mean that the binlog contains consecutive rows_query "
              "events. We expect a table_map or query event after rows_query.");
          goto err;
        }
        // case: we can have rows_query event containing trx metadata in SBR, to
        // avoid unflushed event warning we'll skip setting this flag when the
        // rows_query event contains trx metadata
        auto rq = static_cast<Rows_query_log_event *>(ev);
        print_event_info->have_unflushed_events = !rq->has_trx_meta_data();
        last_rows_query_event.event = rq;
        last_rows_query_event.event_pos = pos;
        if (ev->temp_buf) {
          last_rows_query_event_temp_buf = static_cast<char *>(
              my_malloc(PSI_NOT_INSTRUMENTED, ev->common_header->data_written,
                        MYF(MY_WME)));
          memcpy(last_rows_query_event_temp_buf, ev->temp_buf,
                 ev->common_header->data_written);
        } else {
          last_rows_query_event_temp_buf = nullptr;
        }
        // event is cached and not freed
        ev = nullptr;

        break;
      }
      case binary_log::TABLE_MAP_EVENT: {
        Table_map_log_event *map = ((Table_map_log_event *)ev);
        if (ev_database_changed(string(map->get_db_name()), print_event_info) !=
            Check_database_decision::OK) {
          goto err;
        }
        if (shall_skip_database(map->get_db_name())) {
          print_event_info->skipped_event_in_transaction = true;
          print_event_info->m_table_map_ignored.set_table(map->get_table_id(),
                                                          map);
          // case: this event is skipped so clean up the buffered rows_query
          handle_last_rows_query_event(false, print_event_info);
          // event is cached and not freed
          ev = nullptr;
          goto end;
        }
        // case: this event was not skipped, so let's print the buffered
        // rows_query
        handle_last_rows_query_event(!opt_skip_rows_query, print_event_info);
      }
      // Fall through
      case binary_log::WRITE_ROWS_EVENT:
      case binary_log::DELETE_ROWS_EVENT:
      case binary_log::UPDATE_ROWS_EVENT:
      case binary_log::WRITE_ROWS_EVENT_V1:
      case binary_log::UPDATE_ROWS_EVENT_V1:
      case binary_log::DELETE_ROWS_EVENT_V1:
      case binary_log::PARTIAL_UPDATE_ROWS_EVENT: {
        // dbug_case: we should have handled the buffered rows_query now that
        // we're handling rows events
        DBUG_ASSERT(!last_rows_query_event.event);
        bool stmt_end = false;
        Table_map_log_event *ignored_map = nullptr;
        if (ev_type == binary_log::WRITE_ROWS_EVENT ||
            ev_type == binary_log::DELETE_ROWS_EVENT ||
            ev_type == binary_log::UPDATE_ROWS_EVENT ||
            ev_type == binary_log::WRITE_ROWS_EVENT_V1 ||
            ev_type == binary_log::DELETE_ROWS_EVENT_V1 ||
            ev_type == binary_log::UPDATE_ROWS_EVENT_V1 ||
            ev_type == binary_log::PARTIAL_UPDATE_ROWS_EVENT) {
          Rows_log_event *new_ev = (Rows_log_event *)ev;
          if (new_ev->get_flags(Rows_log_event::STMT_END_F)) stmt_end = true;
          ignored_map = print_event_info->m_table_map_ignored.get_table(
              new_ev->get_table_id());
        }

        bool skip_event = (ignored_map != nullptr);
        /*
          end of statement check:
          i) destroy/free ignored maps
          ii) if skip event
                a) set the unflushed_events flag to false
                b) since we are skipping the last event,
                   append END-MARKER(') to body cache (if required)
                c) flush cache now
         */
        if (stmt_end) {
          /*
            Now is safe to clear ignored map (clear_tables will also
            delete original table map events stored in the map).
          */
          if (print_event_info->m_table_map_ignored.count() > 0)
            print_event_info->m_table_map_ignored.clear_tables();

          /*
             One needs to take into account an event that gets
             filtered but was last event in the statement. If this is
             the case, previous rows events that were written into
             IO_CACHEs still need to be copied from cache to
             result_file (as it would happen in ev->print(...) if
             event was not skipped).
          */
          if (skip_event) {
            // set the unflushed_events flag to false
            print_event_info->have_unflushed_events = false;

            // append END-MARKER(') with delimiter
            IO_CACHE *const body_cache = &print_event_info->body_cache;
            if (my_b_tell(body_cache))
              my_b_printf(body_cache, "'%s\n", print_event_info->delimiter);

            // flush cache
            if ((copy_event_cache_to_file_and_reinit(
                     &print_event_info->head_cache, result_file,
                     stop_never /* flush result_file */) ||
                 copy_event_cache_to_file_and_reinit(
                     &print_event_info->body_cache, result_file,
                     stop_never /* flush result_file */) ||
                 copy_event_cache_to_file_and_reinit(
                     &print_event_info->footer_cache, result_file,
                     stop_never /* flush result_file */)))
              goto err;
          }
        }

        /* skip the event check */
        if (skip_event) {
          print_event_info->skipped_event_in_transaction = true;
          goto end;
        }

        /*
          These events must be printed in base64 format, if printed.
          base64 format requires a FD event to be safe, so if no FD
          event has been printed, we give an error.  Except if user
          passed --short-form, because --short-form disables printing
          row events.
        */
        if (!print_event_info->printed_fd_event && !short_form &&
            ev_type != binary_log::TABLE_MAP_EVENT &&
            ev_type != binary_log::ROWS_QUERY_LOG_EVENT &&
            opt_base64_output_mode != BASE64_OUTPUT_DECODE_ROWS) {
          const char *type_str = ev->get_type_str();
          if (opt_base64_output_mode == BASE64_OUTPUT_NEVER)
            error(
                "--base64-output=never specified, but binlog contains a "
                "%s event which must be printed in base64.",
                type_str);
          else
            error(
                "malformed binlog: it does not contain any "
                "Format_description_log_event. I now found a %s event, which "
                "is not safe to process without a "
                "Format_description_log_event.",
                type_str);
          goto err;
        }

        ev->print(result_file, print_event_info);

        print_event_info->have_unflushed_events = true;

        /* Flush head,body and footer cache to result_file */
        if (stmt_end) {
          print_event_info->have_unflushed_events = false;
          if (copy_event_cache_to_file_and_reinit(
                  &print_event_info->head_cache, result_file,
                  stop_never /* flush result file */) ||
              copy_event_cache_to_file_and_reinit(
                  &print_event_info->body_cache, result_file,
                  stop_never /* flush result file */) ||
              copy_event_cache_to_file_and_reinit(
                  &print_event_info->footer_cache, result_file,
                  stop_never /* flush result file */))
            goto err;
          goto end;
        }
        break;
      }
      case binary_log::ANONYMOUS_GTID_LOG_EVENT:
      case binary_log::GTID_LOG_EVENT: {
        seen_gtid = true;
        if (print_event_info->skipped_event_in_transaction == true)
          fprintf(result_file, "COMMIT /* added by mysqlbinlog */%s\n",
                  print_event_info->delimiter);
        print_event_info->skipped_event_in_transaction = false;

        ev->print(result_file, print_event_info);
        if (head->error == -1) goto err;
        break;
      }
      case binary_log::XID_EVENT: {
        in_transaction = false;
        print_event_info->skipped_event_in_transaction = false;
        seen_gtid = false;
        /*
         * Skip the extra XID event (COMMIT) from other databases when the
         * option --skip-empty-trans is enabled. We also clear the
         * cur_database at the end of currrent transaction
         */
        if (opt_skip_empty_trans) {
          bool skip = shall_skip_database(cur_database.c_str());
          // Reset current transaction's database name tracking
          cur_database = "";
          // Delete and reset the cached BEGIN event
          delete begin_query_ev_cache;
          begin_query_ev_cache = nullptr;
          if (skip) break;
        }
        ev->print(result_file, print_event_info);
        if (head->error == -1) goto err;
        break;
      }
      case binary_log::METADATA_EVENT: {
        ev->print(result_file, print_event_info);
        if (head->error == -1) goto err;

        /* Copy and flush head cache and body cache */
        if (copy_event_cache_to_file_and_reinit(&print_event_info->head_cache,
                                                result_file, stop_never) ||
            copy_event_cache_to_file_and_reinit(&print_event_info->body_cache,
                                                result_file, stop_never))
          goto err;

        goto end;
      } break;
      case binary_log::PREVIOUS_GTIDS_LOG_EVENT:
        if (one_database && !opt_skip_gtids)
          warning(
              "The option --database has been used. It may filter "
              "parts of transactions, but will include the GTIDs in "
              "any case. If you want to exclude or include transactions, "
              "you should use the options --exclude-gtids or "
              "--include-gtids, respectively, instead.");
        /* fall through */
      default:
        ev->print(result_file, print_event_info);
        if (head->error == -1) goto err;
    }
    /* Flush head cache to result_file for every event */
    if (copy_event_cache_to_file_and_reinit(&print_event_info->head_cache,
                                            result_file,
                                            stop_never /* flush result_file */))
      goto err;
  }

  goto end;

err:
  retval = ERROR_STOP;
end:
  rec_count++;
  /*
    Destroy the log_event object.
  */
  delete ev;
  return retval;
}

static struct my_option my_long_options[] = {
    {"help", '?', "Display this help and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"base64-output", OPT_BASE64_OUTPUT_MODE,
     /* 'unspec' is not mentioned because it is just a placeholder. */
     "Determine when the output statements should be base64-encoded BINLOG "
     "statements: 'never' disables it and works only for binlogs without "
     "row-based events; 'decode-rows' decodes row events into commented "
     "pseudo-SQL "
     "statements if the --verbose option is also given; 'auto' prints base64 "
     "only when necessary (i.e., for row-based events and format description "
     "events).  If no --base64-output[=name] option is given at all, the "
     "default is 'auto'.",
     &opt_base64_output_mode_str, &opt_base64_output_mode_str, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"bind-address", 0, "IP address to bind to.", (uchar **)&opt_bind_addr,
     (uchar **)&opt_bind_addr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    /*
      mysqlbinlog needs charsets knowledge, to be able to convert a charset
      number found in binlog to a charset name (to be able to print things
      like this:
      SET @`a`:=_cp850 0x4DFC6C6C6572 COLLATE `cp850_general_ci`;
    */
    {"character-sets-dir", OPT_CHARSETS_DIR,
     "Directory for character set files.", &charsets_dir, &charsets_dir,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"use_dscp", 0, "Use DSCP QOS header in binlog send", &opt_use_dscp,
     &opt_use_dscp, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
    {"database", 'd', "List entries for just this database (local log only).",
     &database, &database, nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"rewrite-db", OPT_REWRITE_DB,
     "Rewrite the row event to point so that "
     "it can be applied to a new database",
     &rewrite, &rewrite, nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"skip-empty-trans", OPT_MYSQLBINLOG_SKIP_EMPTY_TRANS,
     "Do not print empty transactions from databases other than the "
     "selected database, requires --database "
     "and --skip-gtids to be specified.",
     &opt_skip_empty_trans, &opt_skip_empty_trans, 0, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
#ifdef DBUG_OFF
    {"debug", '#', "This is a non-debug version. Catch this and exit.", 0, 0, 0,
     GET_DISABLED, OPT_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-check", OPT_DEBUG_CHECK,
     "This is a non-debug version. Catch this and exit.", 0, 0, 0, GET_DISABLED,
     NO_ARG, 0, 0, 0, 0, 0, 0},
    {"debug-info", OPT_DEBUG_INFO,
     "This is a non-debug version. Catch this and exit.", 0, 0, 0, GET_DISABLED,
     NO_ARG, 0, 0, 0, 0, 0, 0},
#else
    {"debug", '#', "Output debug log.", &default_dbug_option,
     &default_dbug_option, nullptr, GET_STR, OPT_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"debug-check", OPT_DEBUG_CHECK,
     "Check memory and open file usage at exit .", &debug_check_flag,
     &debug_check_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"debug-info", OPT_DEBUG_INFO, "Print some debug info at exit.",
     &debug_info_flag, &debug_info_flag, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
#endif
    {"default_auth", OPT_DEFAULT_AUTH,
     "Default authentication client-side plugin to use.", &opt_default_auth,
     &opt_default_auth, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"disable-log-bin", 'D',
     "Disable binary log. This is useful, if you "
     "enabled --to-last-log and are sending the output to the same MySQL "
     "server. "
     "This way you could avoid an endless loop. You would also like to use it "
     "when restoring after a crash to avoid duplication of the statements you "
     "already have. NOTE: you will need a SUPER privilege to use this option.",
     &disable_log_bin, &disable_log_bin, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"force-if-open", 'F', "Force if binlog was not closed properly.",
     &force_if_open_opt, &force_if_open_opt, nullptr, GET_BOOL, NO_ARG, 1, 0, 0,
     nullptr, 0, nullptr},
    {"force-read", 'f', "Force reading unknown binlog events.", &force_opt,
     &force_opt, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"hexdump", 'H', "Augment output with hexadecimal and ASCII event dump.",
     &opt_hexdump, &opt_hexdump, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"host", 'h', "Get the binlog from server.", &host, &host, nullptr,
     GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"idempotent", 'i',
     "Notify the server to use idempotent mode before "
     "applying Row Events",
     &idempotent_mode, &idempotent_mode, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"local-load", 'l',
     "Prepare local temporary files for LOAD DATA INFILE in the specified "
     "directory.",
     &dirname_for_local_load, &dirname_for_local_load, nullptr, GET_STR_ALLOC,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"offset", 'o', "Skip the first N entries.", &offset, &offset, nullptr,
     GET_ULL, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"password", 'p', "Password to connect to remote server.", nullptr, nullptr,
     nullptr, GET_PASSWORD, OPT_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"plugin_dir", OPT_PLUGIN_DIR, "Directory for client-side plugins.",
     &opt_plugin_dir, &opt_plugin_dir, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"port", 'P',
     "Port number to use for connection or 0 for default to, in "
     "order of preference, my.cnf, $MYSQL_TCP_PORT, "
#if MYSQL_PORT_DEFAULT == 0
     "/etc/services, "
#endif
     "built-in default (" STRINGIFY_ARG(MYSQL_PORT) ").",
     &port, &port, nullptr, GET_INT, REQUIRED_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"print_gtids", OPT_PRINT_GTIDS, "Print encountered gtid set to stderr.",
     &opt_print_gtids, &opt_print_gtids, 0, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"protocol", OPT_MYSQL_PROTOCOL,
     "The protocol to use for connection (tcp, socket, pipe, memory).", nullptr,
     nullptr, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"read-from-remote-server", 'R',
     "Read binary logs from a MySQL server. "
     "This is an alias for read-from-remote-master=BINLOG-DUMP-NON-GTIDS.",
     &opt_remote_alias, &opt_remote_alias, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"read-from-remote-master", OPT_REMOTE_PROTO,
     "Read binary logs from a MySQL server through the COM_BINLOG_DUMP or "
     "COM_BINLOG_DUMP_GTID commands by setting the option to either "
     "BINLOG-DUMP-NON-GTIDS or BINLOG-DUMP-GTIDS, respectively. If "
     "--read-from-remote-master=BINLOG-DUMP-GTIDS is combined with "
     "--exclude-gtids, transactions can be filtered out on the master "
     "avoiding unnecessary network traffic.",
     &opt_remote_proto_str, &opt_remote_proto_str, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"raw", OPT_RAW_OUTPUT,
     "Requires -R. Output raw binlog data instead of SQL "
     "statements, output is to log files.",
     &raw_mode, &raw_mode, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"result-file", 'r',
     "Direct output to a given file. With --raw this is a "
     "prefix for the file names.",
     &output_file, &output_file, nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"server-id", OPT_SERVER_ID,
     "Extract only binlog entries created by the server having the given id.",
     &filter_server_id, &filter_server_id, nullptr, GET_ULONG, REQUIRED_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"server-id-bits", 0, "Set number of significant bits in server-id",
     &opt_server_id_bits, &opt_server_id_bits,
     /* Default + Max 32 bits, minimum 7 bits */
     nullptr, GET_UINT, REQUIRED_ARG, 32, 7, 32, nullptr, 0, nullptr},
    {"set-charset", OPT_SET_CHARSET,
     "Add 'SET NAMES character_set' to the output.", &charset, &charset,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
#if defined(_WIN32)
    {"shared-memory-base-name", OPT_SHARED_MEMORY_BASE_NAME,
     "Base name of shared memory.", &shared_memory_base_name,
     &shared_memory_base_name, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
#endif
    {"short-form", 's',
     "Just show regular queries: no extra info and no "
     "row-based events. This is for testing only, and should not be used in "
     "production systems. If you want to suppress base64-output, consider "
     "using --base64-output=never instead.",
     &short_form, &short_form, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"socket", 'S', "The socket file to use for connection.", &sock, &sock,
     nullptr, GET_STR, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
#include "caching_sha2_passwordopt-longopts.h"
#include "sslopt-longopts.h"

    {"start-datetime", OPT_START_DATETIME,
     "Start reading the binlog at first event having a datetime equal or "
     "posterior to the argument; the argument must be a date and time "
     "in the local time zone, in any format accepted by the MySQL server "
     "for DATETIME and TIMESTAMP types, for example: 2004-12-25 11:25:56 "
     "(you should probably use quotes for your shell to set it properly).",
     &start_datetime_str, &start_datetime_str, nullptr, GET_STR_ALLOC,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"start-position", 'j',
     "Start reading the binlog at position N. Applies to the first binlog "
     "passed on the command line.",
     &start_position, &start_position, nullptr, GET_ULL, REQUIRED_ARG,
     BIN_LOG_HEADER_SIZE, BIN_LOG_HEADER_SIZE,
     /* COM_BINLOG_DUMP accepts only 4 bytes for the position */
     (ulonglong)(~(uint32)0), nullptr, 0, nullptr},
    {"stop-datetime", OPT_STOP_DATETIME,
     "Stop reading the binlog at first event having a datetime equal or "
     "posterior to the argument; the argument must be a date and time "
     "in the local time zone, in any format accepted by the MySQL server "
     "for DATETIME and TIMESTAMP types, for example: 2004-12-25 11:25:56 "
     "(you should probably use quotes for your shell to set it properly).",
     &stop_datetime_str, &stop_datetime_str, nullptr, GET_STR_ALLOC,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"stop-never", OPT_STOP_NEVER,
     "Wait for more data from the server "
     "instead of stopping at the end of the last log. Implicitly sets "
     "--to-last-log but instead of stopping at the end of the last log "
     "it continues to wait till the server disconnects.",
     &stop_never, &stop_never, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"stop-never-slave-server-id", OPT_WAIT_SERVER_ID,
     "The slave server_id used for --read-from-remote-server --stop-never."
     " This option cannot be used together with connection-server-id.",
     &stop_never_slave_server_id, &stop_never_slave_server_id, nullptr, GET_LL,
     REQUIRED_ARG, -1, -1, 0xFFFFFFFFLL, nullptr, 0, nullptr},
    {"connection-server-id", OPT_CONNECTION_SERVER_ID,
     "The slave server_id used for --read-from-remote-server."
     " This option cannot be used together with stop-never-slave-server-id.",
     &connection_server_id, &connection_server_id, nullptr, GET_LL,
     REQUIRED_ARG, -1, -1, 0xFFFFFFFFLL, nullptr, 0, nullptr},
    {"stop-position", OPT_STOP_POSITION,
     "Stop reading the binlog at position N. Applies to the last binlog "
     "passed on the command line.",
     &stop_position, &stop_position, nullptr, GET_ULL, REQUIRED_ARG,
     (longlong)(~(my_off_t)0), BIN_LOG_HEADER_SIZE, (ulonglong)(~(my_off_t)0),
     nullptr, 0, nullptr},
    {"to-last-log", 't',
     "Requires -R. Will not stop at the end of the "
     "requested binlog but rather continue printing until the end of the last "
     "binlog of the MySQL server. If you send the output to the same MySQL "
     "server, that may lead to an endless loop.",
     &to_last_remote_log, &to_last_remote_log, nullptr, GET_BOOL, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr},
    {"user", 'u', "Connect to the remote server as username.", &user, &user,
     nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"verbose", 'v',
     "Reconstruct pseudo-SQL statements out of row events. "
     "-v -v adds comments on column data types.",
     nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"version", 'V', "Print version and exit.", nullptr, nullptr, nullptr,
     GET_NO_ARG, NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"open_files_limit", OPT_OPEN_FILES_LIMIT,
     "Used to reserve file descriptors for use by this program.",
     &open_files_limit, &open_files_limit, nullptr, GET_ULONG, REQUIRED_ARG,
     MY_NFILE, 8, OS_FILE_LIMIT, nullptr, 1, nullptr},
    {"verify-binlog-checksum", 'c', "Verify checksum binlog events.",
     (uchar **)&opt_verify_binlog_checksum,
     (uchar **)&opt_verify_binlog_checksum, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"skip-verify-if-open", 'n',
     "Skip verify checksum binlog events "
     "if binlog file is open.",
     (uchar **)&opt_skip_verify_if_open, (uchar **)&opt_skip_verify_if_open, 0,
     GET_BOOL, NO_ARG, 1, 0, 0, nullptr, 0, nullptr},
    {"binlog-row-event-max-size", OPT_BINLOG_ROWS_EVENT_MAX_SIZE,
     "The maximum size of a row-based binary log event in bytes. Rows will be "
     "grouped into events smaller than this size if possible. "
     "This value must be a multiple of 256.",
     &opt_binlog_rows_event_max_size, &opt_binlog_rows_event_max_size, nullptr,
     GET_ULONG, REQUIRED_ARG,
     /* def_value 4GB */ UINT_MAX, /* min_value */ 256,
     /* max_value */ ULONG_MAX, /* arg_source */ nullptr,
     /* block_size */ 256, /* app_type */ nullptr},
    {"skip-gtids", OPT_MYSQLBINLOG_SKIP_GTIDS,
     "Do not preserve Global Transaction Identifiers; instead make the server "
     "execute the transactions as if they were new.",
     &opt_skip_gtids, &opt_skip_gtids, nullptr, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {"include-gtids", OPT_MYSQLBINLOG_INCLUDE_GTIDS,
     "Print events whose Global Transaction Identifiers "
     "were provided.",
     &opt_include_gtids_str, &opt_include_gtids_str, nullptr, GET_STR_ALLOC,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"include-gtids-from-file", OPT_MYSQLBINLOG_INCLUDE_GTIDS_FROM_FILE,
     "Print events whose Global Transaction Identifiers "
     "were provided.",
     &opt_include_gtids_from_file_str, &opt_include_gtids_from_file_str,
     nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"exclude-gtids", OPT_MYSQLBINLOG_EXCLUDE_GTIDS,
     "Print all events but those whose Global Transaction "
     "Identifiers were provided.",
     &opt_exclude_gtids_str, &opt_exclude_gtids_str, nullptr, GET_STR_ALLOC,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"exclude-gtids-from-file", OPT_MYSQLBINLOG_EXCLUDE_GTIDS_FROM_FILE,
     "Print all events but those whose Global Transaction "
     "Identifiers were provided.",
     &opt_exclude_gtids_from_file_str, &opt_exclude_gtids_from_file_str,
     nullptr, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"print-table-metadata", OPT_PRINT_TABLE_METADATA,
     "Print metadata stored in Table_map_log_event", &opt_print_table_metadata,
     &opt_print_table_metadata, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr, 0,
     nullptr},
    {"compress", 'C', "Use compression in server/client protocol.",
     &opt_compress, &opt_compress, nullptr, GET_BOOL, NO_ARG, 0, 0, 0, nullptr,
     0, nullptr},
    {"compression-algorithms", 0,
     "Use compression algorithm in server/client protocol. Valid values "
     "are any combination of 'zstd','zlib','uncompressed'.",
     &opt_compress_algorithm, &opt_compress_algorithm, nullptr, GET_STR,
     REQUIRED_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"zstd-compression-level", 0,
     "Use this compression level in the client/server protocol, in case "
     "--compression-algorithms=zstd. Valid range is between 1 and 22, "
     "inclusive. Default is 3.",
     &opt_zstd_compress_level, &opt_zstd_compress_level, nullptr, GET_UINT,
     REQUIRED_ARG, 3, 1, 22, nullptr, 0, nullptr},
    {"require-row-format", 0,
     "Fail when printing an event that was not logged using row format or "
     "other forbidden events like Load instructions or the creation/deletion "
     "of temporary tables.",
     &opt_require_row_format, &opt_require_row_format, nullptr, GET_BOOL,
     NO_ARG, 0, 0, 0, nullptr, 0, nullptr},
    {"receive-buffer-size", OPT_RECEIVE_BUFFER_SIZE,
     "The size of input buffer for the socket used while "
     "receving events from the server",
     &opt_receive_buffer_size, &opt_receive_buffer_size, nullptr, GET_UINT,
     REQUIRED_ARG,
     /* def_val */ 1024 * 1024, /* min_value */ 1024, /* max_value */ UINT_MAX,
     nullptr, /* block_size */ 1024, nullptr},
    {"start-gtid", OPT_START_GTID,
     "Binlog dump from the given gtid. This requires index-file option or "
     "--read-from-remote-master=BINLOG-DUMP-GTIDS option. ",
     &opt_start_gtid_str, &opt_start_gtid_str, 0, GET_STR_ALLOC, REQUIRED_ARG,
     0, 0, 0, nullptr, 0, nullptr},
    {"stop-gtid", OPT_STOP_GTID,
     "Binlog dump stop if outside the given gtid. This requires index-file "
     "option or --read-from-remote-master=BINLOG-DUMP-GTIDS option. ",
     &opt_stop_gtid_str, &opt_stop_gtid_str, 0, GET_STR_ALLOC, REQUIRED_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"find-gtid-position", OPT_FIND_GTID_POSITION,
     "Prints binlog file name and starting position of Gtid_log_event "
     "corresponding to the given gtid. If the input string is a gtid set, "
     "mysqlbinlog prints latest binlog file name whose previous gtid set "
     "is a subset of the input gtid set. This requires index-file option.",
     &opt_find_gtid_str, &opt_find_gtid_str, 0, GET_STR_ALLOC, REQUIRED_ARG, 0,
     0, 0, nullptr, 0, nullptr},
    {"index-file", OPT_INDEX_FILE,
     "Path to the index file, required "
     "for initialization of previous gtid sets (local log only).",
     &opt_index_file_str, &opt_index_file_str, 0, GET_STR_ALLOC, REQUIRED_ARG,
     0, 0, 0, nullptr, 0, nullptr},
    {"sql-string", 0, "Print out sql string instead of quoted string",
     &opt_print_sql_string, &opt_print_sql_string, 0, GET_BOOL, NO_ARG, 0, 0, 0,
     nullptr, 0, nullptr},
    {nullptr, 0, nullptr, nullptr, nullptr, nullptr, GET_NO_ARG, NO_ARG, 0, 0,
     0, nullptr, 0, nullptr},
};

/**
  Auxiliary function used by error() and warning().

  Prints the given text (normally "WARNING: " or "ERROR: "), followed
  by the given vprintf-style string, followed by a newline.

  @param format Printf-style format string.
  @param args List of arguments for the format string.
  @param msg Text to print before the string.
*/
void error_or_warning(const char *format, va_list args, const char *msg) {
  fprintf(stderr, "%s: ", msg);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
}

/**
  Prints a message to stderr, prefixed with the text "ERROR: " and
  suffixed with a newline.

  @param format Printf-style format string, followed by printf
  varargs.
*/
void error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  error_or_warning(format, args, "ERROR");
  va_end(args);
}

/**
  This function is used in log_event.cc to report errors.

  @param format Printf-style format string, followed by printf
  varargs.
*/
void sql_print_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  error_or_warning(format, args, "ERROR");
  va_end(args);
}

/**
  Prints a message to stderr, prefixed with the text "WARNING: " and
  suffixed with a newline.

  @param format Printf-style format string, followed by printf
  varargs.
*/
void warning(const char *format, ...) {
  va_list args;
  va_start(args, format);
  error_or_warning(format, args, "WARNING");
  va_end(args);
}

/**
  Frees memory for global variables in this file.
*/
static void cleanup() {
  my_free(pass);
  my_free(database);
  my_free(rewrite);
  my_free(host);
  my_free(user);
  my_free(dirname_for_local_load);

  for (size_t i = 0; i < buff_ev->size(); i++) {
    buff_event_info pop_event_array = buff_ev->at(i);
    delete (pop_event_array.event);
  }
  delete buff_ev;

  delete begin_query_ev_cache;
  begin_query_ev_cache = nullptr;

  if (mysql) mysql_close(mysql);
}

static void usage() {
  print_version();
  puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
  printf(
      "\
Dumps a MySQL binary log in a format usable for viewing or for piping to\n\
the mysql command line client.\n\n");
  printf("Usage: %s [options] log-files\n", my_progname);
  my_print_help(my_long_options);
  print_defaults("my", load_default_groups);
  my_print_variables(my_long_options);
}

static my_time_t convert_str_to_timestamp(const char *str) {
  MYSQL_TIME_STATUS status;
  MYSQL_TIME l_time;
  long dummy_my_timezone;
  bool dummy_in_dst_time_gap;
  /* We require a total specification (date AND time) */
  if (str_to_datetime(str, strlen(str), &l_time, 0, &status) ||
      l_time.time_type != MYSQL_TIMESTAMP_DATETIME || status.warnings) {
    error("Incorrect date and time argument: %s", str);
    exit(1);
  }
  /*
    Note that Feb 30th, Apr 31st cause no error messages and are mapped to
    the next existing day, like in mysqld. Maybe this could be changed when
    mysqld is changed too (with its "strict" mode?).
  */
  return my_system_gmt_sec(l_time, &dummy_my_timezone, &dummy_in_dst_time_gap);
}

extern "C" bool get_one_option(int optid, const struct my_option *opt,
                               char *argument) {
  bool tty_password = false;
  switch (optid) {
#ifndef DBUG_OFF
    case '#':
      DBUG_PUSH(argument ? argument : default_dbug_option);
      break;
#endif
#include "sslopt-case.h"

    case 'd':
      one_database = true;
      break;
    case OPT_REWRITE_DB: {
      char *from_db = argument, *p, *to_db;
      if (!(p = strstr(argument, "->"))) {
        sql_print_error(
            "Bad syntax in mysqlbinlog-rewrite-db - missing '->'!\n");
        return true;
      }
      to_db = p + 2;
      while (p > argument && my_isspace(mysqld_charset, p[-1])) p--;
      *p = 0;
      if (!*from_db) {
        sql_print_error(
            "Bad syntax in mysqlbinlog-rewrite-db - empty FROM db!\n");
        return true;
      }
      while (*to_db && my_isspace(mysqld_charset, *to_db)) to_db++;
      if (!*to_db) {
        sql_print_error(
            "Bad syntax in mysqlbinlog-rewrite-db - empty TO db!\n");
        return true;
      }
      /* Add the database to the mapping */
      global_database_rewriter.register_rule(from_db, to_db);
      break;
    }
    case 'p':
      if (argument == disabled_my_option) {
        // Don't require password
        static char empty_password[] = {'\0'};
        DBUG_ASSERT(empty_password[0] ==
                    '\0');  // Check that it has not been overwritten
        argument = empty_password;
      }
      if (argument) {
        my_free(pass);
        char *start = argument;
        pass = my_strdup(PSI_NOT_INSTRUMENTED, argument, MYF(MY_FAE));
        while (*argument) *argument++ = 'x'; /* Destroy argument */
        if (*start) start[1] = 0;            /* Cut length of argument */
      } else
        tty_password = true;
      break;
    case 'R':
      opt_remote_alias = true;
      opt_remote_proto = BINLOG_DUMP_NON_GTID;
      break;
    case OPT_REMOTE_PROTO:
      opt_remote_proto = (enum_remote_proto)(
          find_type_or_exit(argument, &remote_proto_typelib, opt->name) - 1);
      break;
    case OPT_MYSQL_PROTOCOL:
      opt_protocol =
          find_type_or_exit(argument, &sql_protocol_typelib, opt->name);
      break;
    case OPT_START_DATETIME:
      start_datetime = convert_str_to_timestamp(start_datetime_str);
      break;
    case OPT_STOP_DATETIME:
      stop_datetime = convert_str_to_timestamp(stop_datetime_str);
      break;
    case OPT_BASE64_OUTPUT_MODE:
      opt_base64_output_mode = (enum_base64_output_mode)(
          find_type_or_exit(argument, &base64_output_mode_typelib, opt->name) -
          1);
      break;
    case 'v':
      if (argument == disabled_my_option)
        verbose = 0;
      else
        verbose++;
      break;
    case 'V':
      print_version();
      exit(0);
    case OPT_STOP_NEVER:
      /* wait-for-data implicitly sets to-last-log */
      to_last_remote_log = true;
      break;
    case '?':
      usage();
      exit(0);
    case 's':
      warning(CLIENT_WARN_DEPRECATED_NO_REPLACEMENT_MSG("--short-form"));
      short_form = true;
      break;
    case OPT_WAIT_SERVER_ID:
      warning(CLIENT_WARN_DEPRECATED_MSG("--stop-never-slave-server-id",
                                         "--connection-server-id"));
      break;
  }
  if (tty_password) pass = get_tty_password(NullS);

  return false;
}

static int parse_args(int *argc, char ***argv) {
  int ho_error;

  result_file = stdout;
  if ((ho_error = handle_options(argc, argv, my_long_options, get_one_option)))
    exit(ho_error);
  if (debug_info_flag) my_end_arg = MY_CHECK_ERROR | MY_GIVE_INFO;
  if (debug_check_flag) my_end_arg = MY_CHECK_ERROR;
  return 0;
}

/**
  Create and initialize the global mysql object, and connect to the
  server.

  @retval ERROR_STOP An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
*/
static Exit_status safe_connect() {
  /*
    A possible old connection's resources are reclaimed now
    at new connect attempt. The final safe_connect resources
    are mysql_closed at the end of program, explicitly.
  */
  mysql_close(mysql);
  mysql = mysql_init(nullptr);

  if (!mysql) {
    error("Failed on mysql_init.");
    return ERROR_STOP;
  }

  if (SSL_SET_OPTIONS(mysql)) {
    error("%s", SSL_SET_OPTIONS_ERROR);
    return ERROR_STOP;
  }
  if (opt_plugin_dir && *opt_plugin_dir)
    mysql_options(mysql, MYSQL_PLUGIN_DIR, opt_plugin_dir);

  if (opt_compress_algorithm)
    mysql_options(mysql, MYSQL_OPT_COMPRESSION_ALGORITHMS,
                  opt_compress_algorithm);

  mysql_options(mysql, MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
                &opt_zstd_compress_level);

  if (opt_default_auth && *opt_default_auth)
    mysql_options(mysql, MYSQL_DEFAULT_AUTH, opt_default_auth);

  if (opt_protocol)
    mysql_options(mysql, MYSQL_OPT_PROTOCOL, (char *)&opt_protocol);
  if (opt_bind_addr) mysql_options(mysql, MYSQL_OPT_BIND, opt_bind_addr);

  if (opt_compress) mysql_options(mysql, MYSQL_OPT_COMPRESS, NullS);

#if defined(_WIN32)
  if (shared_memory_base_name)
    mysql_options(mysql, MYSQL_SHARED_MEMORY_BASE_NAME,
                  shared_memory_base_name);
#endif
  mysql_options(mysql, MYSQL_OPT_CONNECT_ATTR_RESET, nullptr);
  mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "program_name",
                 "mysqlbinlog");
  mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "_client_role",
                 "binary_log_listener");
  set_server_public_key(mysql);
  set_get_server_public_key_option(mysql);

  mysql_options(mysql, MYSQL_OPT_NET_RECEIVE_BUFFER_SIZE,
                &opt_receive_buffer_size);

  if (!mysql_real_connect(mysql, host, user, pass, 0, port, sock, 0)) {
    error("Failed on connect: %s", mysql_error(mysql));
    return ERROR_STOP;
  }
  mysql->reconnect = true;
  return OK_CONTINUE;
}

/**
  High-level function for dumping a named binlog.

  This function calls dump_remote_log_entries() or
  dump_local_log_entries() to do the job.

  @param[in] logname Name of input binlog.

  @retval ERROR_STOP An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
  @retval OK_STOP No error, but the end of the specified range of
  events to process has been reached and the program should terminate.
*/
static Exit_status dump_single_log(PRINT_EVENT_INFO *print_event_info,
                                   const char *logname) {
  DBUG_TRACE;

  Exit_status rc = OK_CONTINUE;

  switch (opt_remote_proto) {
    case BINLOG_LOCAL:
      rc = dump_local_log_entries(print_event_info, logname);
      break;
    case BINLOG_DUMP_NON_GTID:
    case BINLOG_DUMP_GTID:
      rc = dump_remote_log_entries(print_event_info, logname);
      break;
    default:
      DBUG_ASSERT(0);
      break;
  }
  return rc;
}

static Exit_status dump_multiple_logs(int argc, char **argv) {
  DBUG_TRACE;
  Exit_status rc = OK_CONTINUE;

  PRINT_EVENT_INFO print_event_info;
  if (!print_event_info.init_ok()) return ERROR_STOP;
  /*
     Set safe delimiter, to dump things
     like CREATE PROCEDURE safely
  */
  if (!raw_mode) {
    fprintf(result_file, "DELIMITER /*!*/;\n");
  }
  my_stpcpy(print_event_info.delimiter, "/*!*/;");

  print_event_info.verbose = short_form ? 0 : verbose;
  print_event_info.short_form = short_form;
  print_event_info.base64_output_mode = opt_base64_output_mode;
  print_event_info.skip_gtids = opt_skip_gtids;
  print_event_info.print_table_metadata = opt_print_table_metadata;
  print_event_info.require_row_format = opt_require_row_format;

  // Dump all logs.
  my_off_t save_stop_position = stop_position;
  stop_position = ~(my_off_t)0;
  for (int i = 0; i < argc; i++) {
    if (i == argc - 1)  // last log, --stop-position applies
      stop_position = save_stop_position;
    if ((rc = dump_single_log(&print_event_info, argv[i])) != OK_CONTINUE)
      break;

    // For next log, --start-position does not apply
    start_position = BIN_LOG_HEADER_SIZE;
  }

  if (!buff_ev->empty())
    warning(
        "The range of printed events ends with an Intvar_event, "
        "Rand_event or User_var_event with no matching Query_log_event. "
        "This might be because the last statement was not fully written "
        "to the log, or because you are using a --stop-position or "
        "--stop-datetime that refers to an event in the middle of a "
        "statement. The event(s) from the partial statement have not been "
        "written to output. ");

  else if (print_event_info.have_unflushed_events)
    warning(
        "The range of printed events ends with a row event or "
        "a table map event that does not have the STMT_END_F "
        "flag set. This might be because the last statement "
        "was not fully written to the log, or because you are "
        "using a --stop-position or --stop-datetime that refers "
        "to an event in the middle of a statement. The event(s) "
        "from the partial statement have not been written to output.");

  /* Set delimiter back to semicolon */
  if (!raw_mode) {
    /*
     * When --skip-empty-trans is enabled, we want to avoid the automatically
     * added COMMIT message when an event is skipped and no COMMIT event has
     * been hit. This is because --skip-empty-trans only supports operating on
     * a single database within a transaction. In other words, we do not want
     * to print a COMMIT message at the end if the interrupted transaction
     * (that has not hit COMMIT event) needs to be skipped anyway.
     */
    if (print_event_info.skipped_event_in_transaction && !opt_skip_empty_trans)
      fprintf(result_file, "COMMIT /* added by mysqlbinlog */%s\n",
              print_event_info.delimiter);

    end_binlog(&print_event_info);

    fprintf(result_file, "DELIMITER ;\n");
    my_stpcpy(print_event_info.delimiter, ";");
  }
  return rc;
}

/**
  When reading a remote binlog, this function is used to grab the
  Format_description_log_event in the beginning of the stream.

  It will not work for a binlog which mixes format. TODO: fix this.

  @retval ERROR_STOP An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
*/
static Exit_status check_master_version() {
  DBUG_TRACE;
  MYSQL_RES *res = nullptr;
  MYSQL_ROW row;
  const char *version;

  if (mysql_query(mysql, "SELECT VERSION()") ||
      !(res = mysql_store_result(mysql))) {
    error(
        "Could not find server version: "
        "Query failed when checking master version: %s",
        mysql_error(mysql));
    return ERROR_STOP;
  }
  if (!(row = mysql_fetch_row(res))) {
    error(
        "Could not find server version: "
        "Master returned no rows for SELECT VERSION().");
    goto err;
  }

  if (!(version = row[0])) {
    error(
        "Could not find server version: "
        "Master reported NULL for the version.");
    goto err;
  }
  /*
     Make a notice to the server that this client
     is checksum-aware. It does not need the first fake Rotate
     necessary checksummed.
     That preference is specified below.
  */
  if (mysql_query(mysql, "SET @master_binlog_checksum='NONE'")) {
    error(
        "Could not notify master about checksum awareness."
        "Master returned '%s'",
        mysql_error(mysql));
    goto err;
  }

  switch (*version) {
    case '5':
    case '8':
    case '9':
      /* The server is soon going to send us its Format_description event .*/
      glob_description_event =
          Format_description_event(BINLOG_VERSION, server_version);
      break;
    default:
      error(
          "Could not find server version: "
          "Master reported unrecognized MySQL version '%s'.",
          version);
      goto err;
  }

  mysql_free_result(res);
  return OK_CONTINUE;

err:
  mysql_free_result(res);
  return ERROR_STOP;
}

static uint get_dump_flags() { return stop_never ? 0 : BINLOG_DUMP_NON_BLOCK; }

/**
  Callback function for mysql_binlog_open().

  Sets gtid data in the command packet.

  @param rpl              Replication stream information.
  @param packet_gtid_set  Pointer to command packet where gtid
                          data should be stored.
*/
static void fix_gtid_set(MYSQL_RPL *rpl, uchar *packet_gtid_set) {
  Gtid_set *gtid_set = (Gtid_set *)rpl->gtid_set_arg;

  gtid_set->encode(packet_gtid_set);

  /*
    Note: we acquire lock in the dump_remote_log_entries()
    just before mysql_binlog_open() call if GTID used.
  */
  global_sid_lock->assert_some_rdlock();
  global_sid_lock->unlock();
}

/*
  A RAII class created to handle the memory of Log_event object
  created in the dump_remote_log_entries method.
*/
class Destroy_log_event_guard {
 public:
  Log_event **ev_del;
  Destroy_log_event_guard(Log_event **ev_arg) { ev_del = ev_arg; }
  ~Destroy_log_event_guard() {
    if (*ev_del != nullptr) delete *ev_del;
  }
};

/**
  Requests binlog dump from a remote server and prints the events it
  receives.

  @param[in,out] print_event_info Parameters and context state
  determining how to print.
  @param[in] logname Name of input binlog.

  @retval ERROR_STOP An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
  @retval OK_STOP No error, but the end of the specified range of
  events to process has been reached and the program should terminate.
*/
static Exit_status dump_remote_log_entries(PRINT_EVENT_INFO *print_event_info,
                                           const char *logname) {
  uint server_id = 0;
  my_off_t old_off = start_position_mot;
  char log_file_name[FN_REFLEN + 1];
  char cur_log_file_name[FN_REFLEN + 1];
  Exit_status retval = OK_CONTINUE;
  unsigned char *event_buf = nullptr;
  ulong event_len;

  DBUG_TRACE;

  log_file_name[0] = 0;

  /*
    Even if we already read one binlog (case of >=2 binlogs on command line),
    we cannot re-use the same connection as before, because it is now dead
    (COM_BINLOG_DUMP kills the thread when it finishes).
  */
  if ((retval = safe_connect()) != OK_CONTINUE) return retval;

  if ((retval = check_master_version()) != OK_CONTINUE) return retval;

  if (opt_use_dscp && mysql_query(mysql, "SET SESSION dscp_on_socket=9")) {
    error(
        "Could not notify master about DSCP."
        "Master returned '%s'",
        mysql_error(mysql));
    return ERROR_STOP;
  }

  /*
    Fake a server ID to log continously. This will show as a
    slave on the mysql server.
  */
  if (to_last_remote_log && stop_never) {
    if (stop_never_slave_server_id == -1)
      server_id = 1;
    else
      server_id = static_cast<uint>(stop_never_slave_server_id);
  } else
    server_id = 0;

  if (connection_server_id != -1)
    server_id = static_cast<uint>(connection_server_id);

  /*
    Ignore HEARBEAT events. They can show up if mysqlbinlog is
    running with:

      --read-from-remote-server
      --read-from-remote-master=BINLOG-DUMP-GTIDS'
      --stop-never
      --stop-never-slave-server-id

    i.e., acting as a fake slave.
  */
  MYSQL_RPL rpl = {0,
                   logname,
                   start_position,
                   server_id,
                   get_dump_flags() | MYSQL_RPL_SKIP_HEARTBEAT |
                       (opt_start_gtid_str ? USING_START_GTID_PROTOCOL : 0),
                   0,
                   nullptr,
                   nullptr,
                   0,
                   nullptr};

  if (opt_remote_proto != BINLOG_DUMP_NON_GTID) {
    rpl.flags |= MYSQL_RPL_GTID;

    global_sid_lock->rdlock();
    rpl.gtid_set_encoded_size = gtid_set_excluded->get_encoded_length();
    rpl.fix_gtid_set = fix_gtid_set;
    rpl.gtid_set_arg = (void *)gtid_set_excluded;
  }

  if (mysql_binlog_open(mysql, &rpl)) {
    error("Open binlog error: %s", mysql_error(mysql));
    return ERROR_STOP;
  }

  Transaction_boundary_parser transaction_parser(
      Transaction_boundary_parser::TRX_BOUNDARY_PARSER_RECEIVER);
  transaction_parser.reset();

  for (;;) {
    bool res{false};
    if (mysql_binlog_fetch(mysql, &rpl))  // Error packet
    {
      error("Got error reading packet from server: %s", mysql_error(mysql));
      return ERROR_STOP;
    } else if (rpl.size == 0)  // EOF
      break;

    DBUG_PRINT("info", ("len: %lu  net->read_pos[5]: %d\n", rpl.size,
                        mysql->net.read_pos[5]));
    /*
      In raw mode We only need the full event details if it is a
      ROTATE_EVENT or FORMAT_DESCRIPTION_EVENT
    */

    Log_event_type type = (Log_event_type)rpl.buffer[1 + EVENT_TYPE_OFFSET];
    Log_event *ev = nullptr;
    Destroy_log_event_guard del(&ev);
    event_len = rpl.size - 1;
    if (!(event_buf = (unsigned char *)my_malloc(key_memory_log_event,
                                                 event_len + 1, MYF(0)))) {
      error("Out of memory.");
      return ERROR_STOP;
    }
    memcpy(event_buf, rpl.buffer + 1, event_len);

    std::tie(event_buf, std::ignore, event_len, res) =
        raw_mode ? global_database_rewriter.rewrite_raw(
                       event_buf, event_len, event_len, glob_description_event)
                 : global_database_rewriter.rewrite(
                       event_buf, event_len, event_len, glob_description_event);
    if (res) {
      error("Got a fatal error while applying rewrite db filter.");
      my_free(event_buf);
      return ERROR_STOP;
    }

    if (!raw_mode || (type == binary_log::ROTATE_EVENT) ||
        (type == binary_log::FORMAT_DESCRIPTION_EVENT)) {
      Binlog_read_error read_error = binlog_event_deserialize(
          reinterpret_cast<unsigned char *>(event_buf), event_len,
          &glob_description_event, opt_verify_binlog_checksum, &ev);

      if (read_error.has_error()) {
        error("Could not construct log event object: %s", read_error.get_str());
        my_free(event_buf);
        return ERROR_STOP;
      }
      ev->register_temp_buf((char *)event_buf);
    }

    {
      /*
        If this is a Rotate event, maybe it's the end of the requested binlog;
        in this case we are done (stop transfer).
        This is suitable for binlogs, not relay logs (but for now we don't read
        relay logs remotely because the server is not able to do that). If one
        day we read relay logs remotely, then we will have a problem with the
        detection below: relay logs contain Rotate events which are about the
        binlogs, so which would trigger the end-detection below.
      */
      if (type == binary_log::ROTATE_EVENT) {
        Rotate_log_event *rev = (Rotate_log_event *)ev;
        /*
          If this is a fake Rotate event, and not about our log, we can stop
          transfer. If this a real Rotate event (so it's not about our log,
          it's in our log describing the next log), we print it (because it's
          part of our log) and then we will stop when we receive the fake one
          soon.
        */
        if (raw_mode) {
          if (output_file != nullptr) {
            snprintf(log_file_name, sizeof(log_file_name), "%s%s", output_file,
                     rev->new_log_ident);
          } else {
            my_stpcpy(log_file_name, rev->new_log_ident);
          }
        }

        if (rev->common_header->when.tv_sec == 0) {
          if (!to_last_remote_log) {
            // logname is NULL if mysqlbinlog is run with
            // --exclude-gtids or --start-gtid. In these cases
            // the first fake rotate event should not be considered as end of
            // binlog file.
            if (logname == NULL &&
                (opt_exclude_gtids_str != NULL || opt_start_gtid_str != NULL)) {
              // store the first fake rotate event.
              // mysqlbinlog exists after reading this binlog file.
              // If --to-last-log is used, then mysqlbinlog will not
              // stop after reading this binlog file, but continue reading
              // all the binary logs.
              strcpy(cur_log_file_name, rev->new_log_ident);
              logname = cur_log_file_name;
            }

            if ((rev->ident_len != rpl.file_name_length) ||
                memcmp(rev->new_log_ident, logname, rpl.file_name_length)) {
              return OK_CONTINUE;
            }
            /*
              Otherwise, this is a fake Rotate for our log, at the very
              beginning for sure. Skip it, because it was not in the original
              log. If we are running with to_last_remote_log, we print it,
              because it serves as a useful marker between binlogs then.
            */
            continue;
          }
          /*
             Reset the value of '# at pos' field shown against first event of
             next binlog file (fake rotate) picked by mysqlbinlog --to-last-log
         */
          old_off = start_position_mot;
          rpl.size = 1;  // fake Rotate, so don't increment old_off
          event_len = 0;
        }
      } else if (type == binary_log::FORMAT_DESCRIPTION_EVENT) {
        /*
          This could be an fake Format_description_log_event that server
          (5.0+) automatically sends to a slave on connect, before sending
          a first event at the requested position.  If this is the case,
          don't increment old_off. Real Format_description_log_event always
          starts from BIN_LOG_HEADER_SIZE position.
        */
        // fake event when not in raw mode, don't increment old_off
        if ((old_off != BIN_LOG_HEADER_SIZE) && (!raw_mode)) {
          rpl.size = 1;
          event_len = 0;
        }
        if (raw_mode) {
          if (result_file && (result_file != stdout))
            my_fclose(result_file, MYF(0));
          if (!(result_file = my_fopen(
                    log_file_name, O_WRONLY | MY_FOPEN_BINARY, MYF(MY_WME))) ||
              DBUG_EVALUATE_IF("simulate_create_log_file_error_for_FD_event", 1,
                               0)) {
            error("Could not create log file '%s'", log_file_name);
            return ERROR_STOP;
          }
          DBUG_EXECUTE_IF("simulate_result_file_write_error_for_FD_event",
                          DBUG_SET("+d,simulate_fwrite_error"););
          if (my_fwrite(result_file, (const uchar *)BINLOG_MAGIC,
                        BIN_LOG_HEADER_SIZE, MYF(MY_NABP))) {
            error("Could not write into log file '%s'", log_file_name);
            return ERROR_STOP;
          }
        }
        glob_description_event = dynamic_cast<Format_description_event &>(*ev);
      }

      if (opt_require_row_format) {
        bool info_error{false};
        binary_log::Log_event_basic_info log_event_info;
        std::tie(info_error, log_event_info) = extract_log_event_basic_info(
            (const char *)event_buf, event_len, &glob_description_event);

        if (!info_error) {
          transaction_parser.feed_event(log_event_info, false);
          if (transaction_parser.check_row_logging_constraints(
                  log_event_info)) {
            error(
                "Event being written violates the --require-row-format "
                "parameter constraints.");
            return ERROR_STOP;
          }
        } else {
          error("Unexpected event being evaluated under --require-row-format.");
          return ERROR_STOP;
        }
      }

      if (raw_mode) {
        DBUG_EXECUTE_IF("simulate_result_file_write_error",
                        DBUG_SET("+d,simulate_fwrite_error"););
        if (my_fwrite(result_file, (const uchar *)event_buf, event_len,
                      MYF(MY_NABP))) {
          error("Could not write into log file '%s'", log_file_name);
          retval = ERROR_STOP;
        }
        if (!ev) my_free(event_buf);

        /* Flush result_file after every event */
        fflush(result_file);
      } else {
        retval = process_event(print_event_info, ev, old_off, logname);
        // The event's deletion has been handled in process_event. To prevent
        // that Destroy_log_event_guard deletes it again, we have to set it to
        // NULL
        ev = nullptr;
      }

      if (retval != OK_CONTINUE) return retval;
    }
    /*
      Let's adjust offset for remote log as for local log to produce
      similar text and to have --stop-position to work identically.
    */
    old_off += rpl.size - 1;
  }
  mysql_binlog_close(mysql, &rpl);
  return OK_CONTINUE;
}

/**
   Two things are done in this class:
   - rewrite the database name in event_data if rewrite option is configured.
   - Skip the extra BINLOG_MAGIC when reading event data if
     m_multiple_binlog_magic is set. It is used for the case when users feed
     more than one binlog files through stdin.
 */
class Mysqlbinlog_event_data_istream : public Binlog_event_data_istream {
 public:
  using Binlog_event_data_istream::Binlog_event_data_istream;

  template <class ALLOCATOR>
  bool read_event_data(unsigned char **buffer, unsigned int *length,
                       ALLOCATOR *allocator, bool verify_checksum,
                       enum_binlog_checksum_alg checksum_alg) {
    return Binlog_event_data_istream::read_event_data(
               buffer, length, allocator, verify_checksum, checksum_alg) ||
           rewrite_db(buffer, length);
  }

  void set_multi_binlog_magic() { m_multi_binlog_magic = true; }

 private:
  bool m_multi_binlog_magic = false;

  bool rewrite_db(unsigned char **buffer, unsigned int *length) {
    bool ret{false};
    size_t buffer_capacity{0};
    std::tie(*buffer, buffer_capacity, *length, ret) =
        global_database_rewriter.rewrite(*buffer, *length, *length,
                                         glob_description_event);
    if (ret) {
      error("Error applying filter while reading event");
      ret = m_error->set_type(Binlog_read_error::MEM_ALLOCATE);
      if (buffer_capacity > 0) {
        my_free(*buffer);
        *buffer = nullptr;
        *length = 0;
      }
    }
    return ret;
  }

  bool read_event_header() override {
    if (Binlog_event_data_istream::read_event_header()) return true;
    /*
      If there are more than one binlog files in the stdin, it checks and skips
      the binlog magic heads of following binlog files.
    */
    if (m_multi_binlog_magic &&
        memcmp(m_header, BINLOG_MAGIC, BINLOG_MAGIC_SIZE) == 0) {
      size_t header_len = LOG_EVENT_MINIMAL_HEADER_LEN - BINLOG_MAGIC_SIZE;

      // Remove BINLOG_MAGIC from m_header
      memmove(m_header, m_header + BINLOG_MAGIC_SIZE, header_len);
      // Read the left BINLOG_MAGIC_SIZE bytes of the header
      return read_fixed_length<Binlog_read_error::TRUNC_EVENT>(
          m_header + header_len, BINLOG_MAGIC_SIZE);
    }
    return false;
  }
};

/**
   It makes Stdin_istream support seek(only seek forward). So stdin can be used
   as a Basic_seekable_istream.
*/
class Stdin_binlog_istream : public Basic_seekable_istream,
                             public Stdin_istream {
 public:
  ssize_t read(unsigned char *buffer, size_t length) override {
    longlong ret = Stdin_istream::read(buffer, length);
    if (ret > 0) m_position += ret;
    return ret;
  }

  bool seek(my_off_t position) override {
    DBUG_ASSERT(position > m_position);
    if (Stdin_istream::skip(position - m_position)) {
      error("Failed to skip %llu bytes from stdin", position - m_position);
      return true;
    }
    m_position = position;
    return false;
  }

  /* purecov: begin inspected */
  /** Stdin has no length. It should never be called. */
  my_off_t length() override {
    DBUG_ASSERT(0);
    return 0;
  }
  /* purecov: end */

 private:
  /**
    Stores the position of the stdin stream it is reading. It is exact same to
    the count of bytes it has read.
  */
  my_off_t m_position = 0;
};

class Mysqlbinlog_ifile : public Basic_binlog_ifile {
 public:
  using Basic_binlog_ifile::Basic_binlog_ifile;

 private:
  std::unique_ptr<Basic_seekable_istream> open_file(
      const char *file_name) override {
    if (file_name && strcmp(file_name, "-") != 0) {
      IO_CACHE_istream *iocache = new IO_CACHE_istream;
      if (iocache->open(
#ifdef HAVE_PSI_INTERFACE
              PSI_NOT_INSTRUMENTED, PSI_NOT_INSTRUMENTED,
#endif
              file_name, MYF(MY_WME | MY_NABP))) {
        delete iocache;
        return nullptr;
      }
      return std::unique_ptr<Basic_seekable_istream>(iocache);
    } else {
      std::string errmsg;
      Stdin_binlog_istream *standard_in = new Stdin_binlog_istream;
      if (standard_in->open(&errmsg)) {
        error("%s", errmsg.c_str());
        delete standard_in;
        return nullptr;
      }
      return std::unique_ptr<Basic_seekable_istream>(standard_in);
    }
  }
};

typedef Basic_binlog_file_reader<
    Mysqlbinlog_ifile, Mysqlbinlog_event_data_istream,
    Binlog_event_object_istream, Default_binlog_event_allocator>
    Mysqlbinlog_file_reader;

/**
  Reads a local binlog and prints the events it sees.

  @param[in] logname Name of input binlog.

  @param[in,out] print_event_info Parameters and context state
  determining how to print.

  @retval ERROR_STOP An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
  @retval OK_STOP No error, but the end of the specified range of
  events to process has been reached and the program should terminate.
*/
static Exit_status dump_local_log_entries(PRINT_EVENT_INFO *print_event_info,
                                          const char *logname) {
  Exit_status retval = OK_CONTINUE;

  ulong max_event_size = 0;
  mysql_get_option(nullptr, MYSQL_OPT_MAX_ALLOWED_PACKET, &max_event_size);
  Mysqlbinlog_file_reader mysqlbinlog_file_reader(opt_verify_binlog_checksum,
                                                  max_event_size);

  Format_description_log_event *fdle = nullptr;
  if (mysqlbinlog_file_reader.open(logname, start_position, &fdle)) {
    error("%s", mysqlbinlog_file_reader.get_error_str());
    return ERROR_STOP;
  }

  Transaction_boundary_parser transaction_parser(
      Transaction_boundary_parser::TRX_BOUNDARY_PARSER_APPLIER);
  transaction_parser.reset();

  if (fdle != nullptr) {
    retval = process_event(print_event_info, fdle,
                           mysqlbinlog_file_reader.event_start_pos(), logname);
    if (retval != OK_CONTINUE) return retval;
  }

  if (strcmp(logname, "-") == 0)
    mysqlbinlog_file_reader.event_data_istream()->set_multi_binlog_magic();

  for (;;) {
    char llbuff[21];
    my_off_t old_off = mysqlbinlog_file_reader.position();

    Log_event *ev = mysqlbinlog_file_reader.read_event_object();
    if (ev == nullptr) {
      /*
        if binlog wasn't closed properly ("in use" flag is set) don't complain
        about a corruption, but treat it as EOF and move to the next binlog.
      */
      if ((opt_skip_verify_if_open &&
           (mysqlbinlog_file_reader.format_description_event()
                ->header()
                ->flags &
            LOG_EVENT_BINLOG_IN_USE_F)) ||
          mysqlbinlog_file_reader.get_error_type() ==
              Binlog_read_error::READ_EOF)
        return retval;

      error(
          "Could not read entry at offset %s: "
          "Error in log format or read error 1.",
          llstr(old_off, llbuff));
      error("%s", mysqlbinlog_file_reader.get_error_str());
      return ERROR_STOP;
    }

    if (opt_require_row_format) {
      bool info_error{false};
      binary_log::Log_event_basic_info log_event_info;
      std::tie(info_error, log_event_info) = extract_log_event_basic_info(ev);

      if (!info_error) {
        transaction_parser.feed_event(log_event_info, false);
        if (transaction_parser.check_row_logging_constraints(log_event_info)) {
          error(
              "Event being written violates the --require-row-format "
              "parameter constraints.");
          delete ev;
          return ERROR_STOP;
        }
      } else {
        error("Unexpected event being evaluated under --require-row-format.");
        delete ev;
        return ERROR_STOP;
      }
    }

    if ((retval = process_event(print_event_info, ev, old_off, logname)) !=
        OK_CONTINUE)
      return retval;
  }

  /* NOTREACHED */

  return retval;
}

/* Read gtids file and store the value in the argument 'buffer'.
 *
 * @params gtids_file  name of the gtids file
 * @params buffer[OUT] output buffer to store the gtids string
 *
 * @returns FALSE if the reading of the gtids file is successful.
 *          TRUE  otherwise.
 */
static bool read_gtids_file(char *gtids_file, char **buffer) {
  MY_STAT stat_arg;
  int fd;
  /* get the size of the gtids file in bytes */
  if (!my_stat(gtids_file, &stat_arg, MYF(MY_WME))) return true;

  /* open the gtids file */
  if ((fd = my_open(gtids_file, O_RDONLY, MYF(MY_WME))) < 0) return true;

  /* allocate buffer */
  if (!((*buffer) = (char *)my_malloc(
            PSI_NOT_INSTRUMENTED, (size_t)stat_arg.st_size + 1, MYF(MY_WME)))) {
    return true;
  }

  /* read the contents of the file and store in buffer */
  if (my_read(fd, (uchar *)(*buffer), (size_t)stat_arg.st_size,
              MYF(MY_FNABP))) {
    return true;
  }

  (*buffer)[stat_arg.st_size] = '\0';
  (void)my_close(fd, MYF(0));

  return false;
}

/* Post processing of arguments to check for conflicts and other setups */
static int args_post_process(void) {
  DBUG_TRACE;

  if (opt_remote_alias && opt_remote_proto != BINLOG_DUMP_NON_GTID) {
    error(
        "The option read-from-remote-server cannot be used when "
        "read-from-remote-master is defined and is not equal to "
        "BINLOG-DUMP-NON-GTIDS");
    return ERROR_STOP;
  }

  if (raw_mode) {
    if (one_database)
      warning("The --database option is ignored with --raw mode");

    if (opt_remote_proto == BINLOG_LOCAL) {
      error(
          "The --raw flag requires one of --read-from-remote-master or "
          "--read-from-remote-server");
      return ERROR_STOP;
    }

    if (opt_include_gtids_str != nullptr ||
        opt_include_gtids_from_file_str != nullptr) {
      error("You cannot use --include-gtids%s and --raw together.",
            opt_include_gtids_from_file_str ? "-from-file" : "");
      return ERROR_STOP;
    }

    if (opt_remote_proto == BINLOG_DUMP_NON_GTID &&
        (opt_exclude_gtids_str != nullptr ||
         opt_exclude_gtids_from_file_str != nullptr)) {
      error(
          "You cannot use both of --exclude-gtids%s and --raw together "
          "with one of --read-from-remote-server or "
          "--read-from-remote-master=BINLOG-DUMP-NON-GTID.",
          opt_exclude_gtids_from_file_str ? "-from-file" : "");
      return ERROR_STOP;
    }

    if (stop_position != (ulonglong)(~(my_off_t)0))
      warning("The --stop-position option is ignored in raw mode");

    if (stop_datetime != MY_TIME_T_MAX)
      warning("The --stop-datetime option is ignored in raw mode");
  } else if (output_file) {
    if (!(result_file =
              my_fopen(output_file, O_WRONLY | MY_FOPEN_BINARY, MYF(MY_WME)))) {
      error("Could not create log file '%s'", output_file);
      return ERROR_STOP;
    }
  }

  if (opt_start_gtid_str != nullptr &&
      (opt_exclude_gtids_str != nullptr ||
       opt_exclude_gtids_from_file_str != nullptr)) {
    error("--start-gtid and --exclude-gtids%s should not be used together",
          opt_exclude_gtids_from_file_str ? "-from-file" : "");
    return ERROR_STOP;
  }

  global_sid_lock->rdlock();

  if (opt_include_gtids_str != nullptr) {
    if (gtid_set_included->add_gtid_text(opt_include_gtids_str) !=
        RETURN_STATUS_OK) {
      error("Could not configure --include-gtids%s '%s'",
            opt_include_gtids_from_file_str ? "-from-file" : "",
            opt_include_gtids_from_file_str ? opt_include_gtids_from_file_str
                                            : opt_include_gtids_str);
      global_sid_lock->unlock();
      return ERROR_STOP;
    }
  }

  if (opt_exclude_gtids_str != nullptr) {
    if (gtid_set_excluded->add_gtid_text(opt_exclude_gtids_str) !=
        RETURN_STATUS_OK) {
      error("Could not configure --exclude-gtids%s '%s'",
            opt_exclude_gtids_from_file_str ? "-from-file" : "",
            opt_exclude_gtids_from_file_str ? opt_exclude_gtids_from_file_str
                                            : opt_exclude_gtids_str);
      global_sid_lock->unlock();
      return ERROR_STOP;
    }
  }

  if (opt_skip_empty_trans != 0 && (database == NULL || opt_skip_gtids == 0)) {
    error(
        "--skip_empty_trans requires --database and "
        "--skip-gtids options to be specified");
    return ERROR_STOP;
  }

  global_sid_lock->unlock();

  if (connection_server_id == 0 && stop_never)
    error("Cannot set --server-id=0 when --stop-never is specified.");
  if (connection_server_id != -1 && stop_never_slave_server_id != -1)
    error("Cannot set --connection-server-id= %" PRId64
          " and"
          "--stop-never-slave-server-id= %" PRId64 ". ",
          connection_server_id, stop_never_slave_server_id);

  if (opt_start_gtid_str != nullptr && opt_find_gtid_str != nullptr) {
    error(
        "--start-gtid and --find-gtid-position options should not be "
        "used together");
    return ERROR_STOP;
  }

  if (opt_find_gtid_str != nullptr && opt_index_file_str == nullptr) {
    error("--find-gtid-position requires --index-file option");
    return ERROR_STOP;
  }

  if (opt_start_gtid_str != nullptr && opt_index_file_str == nullptr &&
      opt_remote_proto != BINLOG_DUMP_GTID) {
    error(
        "--start-gtid requires --index-file option or "
        "--read-from-remote-master=BINLOG_DUMP_GTID option");
    return ERROR_STOP;
  }

  if (opt_start_gtid_str != nullptr && opt_remote_proto == BINLOG_DUMP_GTID) {
    global_sid_lock->rdlock();
    // Update gtid_set_excluded as it will be sent to server side indicating
    // the starting point when USING_START_GTID_PROTOCOL is set.
    if (gtid_set_excluded->add_gtid_text(opt_start_gtid_str) !=
        RETURN_STATUS_OK) {
      error("Could not configure --start-gtid '%s'", opt_start_gtid_str);
      global_sid_lock->unlock();
      return ERROR_STOP;
    }
    global_sid_lock->unlock();
  }

  if (opt_stop_gtid_str != NULL) {
    if (opt_index_file_str == NULL && opt_remote_proto != BINLOG_DUMP_GTID) {
      error(
          "--stop-gtid requires --index-file option or "
          "--read-from-remote-master=BINLOG_DUMP_GTID option");
      return ERROR_STOP;
    }

    global_sid_lock->rdlock();
    if (gtid_set_stop->add_gtid_text(opt_stop_gtid_str) != RETURN_STATUS_OK) {
      error("Could not configure --stop-gtid '%s'", opt_stop_gtid_str);
      global_sid_lock->unlock();
      return ERROR_STOP;
    }
    global_sid_lock->unlock();
  }

  return OK_CONTINUE;
}

/**
   GTID cleanup destroys objects and reset their pointer.
   Function is reentrant.
*/
inline void gtid_client_cleanup() {
  delete global_sid_lock;
  delete global_sid_map;
  delete gtid_set_excluded;
  delete gtid_set_included;
  delete gtid_set_stop;
  global_sid_lock = nullptr;
  global_sid_map = nullptr;
  gtid_set_excluded = nullptr;
  gtid_set_included = nullptr;
  gtid_set_stop = nullptr;
}

/**
   GTID initialization.

   @return true if allocation does not succeed
           false if OK
*/
inline bool gtid_client_init() {
  bool res = (!(global_sid_lock = new Checkable_rwlock) ||
              !(global_sid_map = new Sid_map(global_sid_lock)) ||
              !(gtid_set_excluded = new Gtid_set(global_sid_map)) ||
              !(gtid_set_included = new Gtid_set(global_sid_map)) ||
              !(gtid_set_stop = new Gtid_set(global_sid_map)));
  if (res) {
    gtid_client_cleanup();
  }
  return res;
}

/**
   Parses the index file and builds the previous_gtid_set_map

   @param index_file_path path to index file

   @return true  Failure
           false Success
*/
static bool init_previous_gtid_set_map(char *index_file_path) {
  DBUG_ENTER("init_previous_gtid_set_map");
  char file_name_and_gtid_set_length[FILE_AND_GTID_SET_LENGTH];
  uchar *previous_gtid_set_in_file = NULL;
  int length;
  int index_dir_len = dirname_length(opt_index_file_str);
  bool ret = false;

  IO_CACHE index_file;
  File file = -1;

  if ((file = my_open(index_file_path, O_RDONLY, MYF(MY_WME))) < 0) {
    error("Error opening index file");
    DBUG_RETURN(true);
  }

  if (init_io_cache(&index_file, file, IO_SIZE, READ_CACHE, 0, 0,
                    MYF(MY_WME))) {
    error("Error initializing index file cache");
    ret = true;
    my_close(file, MYF(MY_WME));
    DBUG_RETURN(true);
  }

  my_b_seek(&index_file, 0);
  while ((length = my_b_gets(&index_file, file_name_and_gtid_set_length,
                             sizeof(file_name_and_gtid_set_length))) >= 1) {
    file_name_and_gtid_set_length[length - 1] = 0;
    uint gtid_string_length = 0;
    char *save_ptr = strchr(file_name_and_gtid_set_length, ' ');
    if (save_ptr != NULL) {
      *save_ptr = 0;  // replace ' ' with '\0'
      save_ptr++;
      gtid_string_length = atol(save_ptr);
    }
    if (gtid_string_length > 0) {
      previous_gtid_set_in_file = (uchar *)my_malloc(
          PSI_NOT_INSTRUMENTED, gtid_string_length + 1, MYF(MY_WME));
      if (previous_gtid_set_in_file == NULL) {
        error("Malloc Failed to allocate %d bytes of memory",
              gtid_string_length + 1);
        ret = true;
        goto end;
      }
      if (my_b_read(&index_file, previous_gtid_set_in_file,
                    gtid_string_length + 1)) {
        error(
            "Previous gtid set of binlog %s is corrupted in binlog "
            "index file",
            file_name_and_gtid_set_length);
        my_free(previous_gtid_set_in_file);
        ret = true;
        goto end;
      }
    }
    int binlog_dir_len = dirname_length(file_name_and_gtid_set_length);
    char binlog_file[FN_REFLEN];
    strcpy(binlog_file, opt_index_file_str);
    strcpy(binlog_file + index_dir_len,
           file_name_and_gtid_set_length + binlog_dir_len);
    previous_gtid_set_map.insert(std::pair<string, string>(
        string(binlog_file),
        string((char *)previous_gtid_set_in_file, gtid_string_length)));
    my_free(previous_gtid_set_in_file);
  }

end:
  /*
    Since the end_io_cache() writes to the file errors may happen.
   */
  if (end_io_cache(&index_file)) {
    ret = true;
  }

  my_close(file, MYF(MY_WME));

  DBUG_RETURN(ret);
}

/**
  Finds starting binlog to start the dump by iterating over
  previous_gtid_set_map. Dumps events starting from the given GTID
  if find_position is false, otherwise prints binlog name and starting
  position of Gtid_log_event corresponding to the given GTID.

  @param gtid_string   GTID to start the dump
  @param find_position true if using find-gtid-position
                       false if using start-gtid

  @retval ERROR_STOP  An error occurred - the program should terminate.
  @retval OK_CONTINUE No error, the program should continue.
  @retval OK_STOP     No error, but the end of the specified range of
                      events to process has been reached and the program
                      should terminate.
*/
static Exit_status start_gtid_dump(char *gtid_string, bool find_position) {
  DBUG_ENTER("start_gtid_dump");
  Exit_status retval = OK_CONTINUE;
  Sid_map sid_map(NULL);
  bool searching_gtid_set = false;
  my_off_t pos = BIN_LOG_HEADER_SIZE;
  Gtid gtid;
  Gtid_set gtid_set(&sid_map);
  std::map<std::string, std::string>::reverse_iterator rit;
  std::map<std::string, std::string>::iterator it, last_log_it;
  it = previous_gtid_set_map.end();
  last_log_it = --previous_gtid_set_map.end();

  Gtid_set previous_gtid_set(&sid_map);
  if (!find_position || Gtid::is_valid(gtid_string)) {
    if (gtid.parse(&sid_map, gtid_string) != RETURN_STATUS_OK) {
      error("Couldn't find position of a malformed Gtid %s", gtid_string);
      DBUG_RETURN(ERROR_STOP);
    }
  } else {
    // Check if the value given to find-gtid-position is gtid set.
    if (gtid_set.add_gtid_text(gtid_string) != RETURN_STATUS_OK) {
      error("Couldn't find position of a malformed Gtid %s", gtid_string);
      DBUG_RETURN(ERROR_STOP);
    }
    searching_gtid_set = true;
  }

  for (rit = previous_gtid_set_map.rbegin();
       rit != previous_gtid_set_map.rend(); ++rit) {
    previous_gtid_set.add_gtid_encoding((const uchar *)rit->second.c_str(),
                                        rit->second.length());
    if (searching_gtid_set ? previous_gtid_set.is_subset(&gtid_set)
                           : !previous_gtid_set.contains_gtid(gtid)) {
      it = previous_gtid_set_map.find(rit->first.c_str());
      break;
    }
    previous_gtid_set.clear();
  }

  if (it == previous_gtid_set_map.end()) {
    error("Requested Gtid is purged and so cannot be found in binary logs");
    DBUG_RETURN(ERROR_STOP);
  }

  if (!searching_gtid_set) {
    pos = find_gtid_pos_in_log<Mysqlbinlog_file_reader>(it->first.c_str(), gtid,
                                                        &sid_map);
    if (pos == 0) {
      error(
          "Request gtid is not in executed set and so cannot be "
          "found in binary logs");
      DBUG_RETURN(ERROR_STOP);
    }
  }

  if (find_position) {
    int dir_len = dirname_length(it->first.c_str());
    fprintf(result_file, "Log_name: %s\n", it->first.c_str() + dir_len);
    if (!searching_gtid_set) fprintf(result_file, "Position: %llu", pos);
    DBUG_RETURN(OK_CONTINUE);
  }

  my_off_t save_stop_position = stop_position;
  stop_position = ULLONG_MAX;
  start_position = pos;

  while (it != previous_gtid_set_map.end()) {
    char *args = const_cast<char *>(it->first.c_str());
    if ((retval = dump_multiple_logs(1, &args)) != OK_CONTINUE)
      DBUG_RETURN(retval);
    start_position = BIN_LOG_HEADER_SIZE;
    if (++it == last_log_it) stop_position = save_stop_position;
  }
  DBUG_RETURN(retval);
}

int main(int argc, char **argv) {
  Exit_status retval = OK_CONTINUE;
  MY_INIT(argv[0]);
  DBUG_TRACE;
  DBUG_PROCESS(argv[0]);

  my_init_time();  // for time functions
  tzset();         // set tzname
  /*
    A pointer of type Log_event can point to
     INTVAR
     USER_VAR
     RANDOM
    events.
  */
  buff_ev = new Buff_ev(PSI_NOT_INSTRUMENTED);

  my_getopt_use_args_separator = true;
  MEM_ROOT alloc{PSI_NOT_INSTRUMENTED, 512};
  if (load_defaults("my", load_default_groups, &argc, &argv, &alloc)) exit(1);
  my_getopt_use_args_separator = false;

  parse_args(&argc, &argv);

  if (!argc && opt_find_gtid_str == nullptr && opt_start_gtid_str == nullptr &&
      (opt_exclude_gtids_str == nullptr ||
       opt_exclude_gtids_from_file_str != NULL)) {
    usage();
    my_end(my_end_arg);
    return EXIT_FAILURE;
  }

  my_b_set_print_sql_string(opt_print_sql_string);

  if (gtid_client_init()) {
    error("Could not initialize GTID structuress.");
    return EXIT_FAILURE;
  }
  if ((argc == 1) && (stop_position != (ulonglong)(~(my_off_t)0)) &&
      (!strcmp(argv[0], "-"))) {
    error("stop_position not allowed when input is STDIN");
    return EXIT_FAILURE;
  }

  if (opt_include_gtids_str != nullptr &&
      opt_include_gtids_from_file_str != nullptr) {
    error(
        "--include-gtids and --include-gtids-from-file should not be used "
        "together");
    return EXIT_FAILURE;
  }

  /* if '--include-gtids-from-file' is specified then read the gtids string
   * value from the file and store it in 'opt_include_gtids_str'
   */
  if (opt_include_gtids_str == nullptr &&
      opt_include_gtids_from_file_str != nullptr) {
    if (strlen(opt_include_gtids_from_file_str) > 0 &&
        read_gtids_file(opt_include_gtids_from_file_str,
                        &opt_include_gtids_str)) {
      error("Could not read include-gtids-from-file %s",
            opt_include_gtids_from_file_str);
      return EXIT_FAILURE;
    }
  }

  if (opt_exclude_gtids_str != nullptr &&
      opt_exclude_gtids_from_file_str != nullptr) {
    error(
        "--exclude-gtids and --exclude-gtids-from-file should not be used "
        "together");
    return ERROR_STOP;
  }

  /* if '--exclude-gtids-from-file' is specified then read the gtids string
   * value from the file and store it in 'opt_exclude_gtids_str'
   */
  if (opt_exclude_gtids_str == nullptr &&
      opt_exclude_gtids_from_file_str != nullptr) {
    if (strlen(opt_exclude_gtids_from_file_str) > 0 &&
        read_gtids_file(opt_exclude_gtids_from_file_str,
                        &opt_exclude_gtids_str)) {
      error("Could not read exclude-gtids-from-file %s",
            opt_exclude_gtids_from_file_str);
      return EXIT_FAILURE;
    }
  }

  umask(((~my_umask) & 0666));
  /* Check for argument conflicts and do any post-processing */
  if (args_post_process() == ERROR_STOP) return EXIT_FAILURE;

  if (opt_base64_output_mode == BASE64_OUTPUT_UNSPEC)
    opt_base64_output_mode = BASE64_OUTPUT_AUTO;

  opt_server_id_mask =
      (opt_server_id_bits == 32) ? ~ulong(0) : (1 << opt_server_id_bits) - 1;

  my_set_max_open_files(open_files_limit);

  MY_TMPDIR tmpdir;
  tmpdir.list = nullptr;
  if (!dirname_for_local_load) {
    if (init_tmpdir(&tmpdir, nullptr)) return EXIT_FAILURE;
    dirname_for_local_load =
        my_strdup(PSI_NOT_INSTRUMENTED, my_tmpdir(&tmpdir), MY_WME);
  }

  if (dirname_for_local_load)
    load_processor.init_by_dir_name(dirname_for_local_load);
  else
    load_processor.init_by_cur_dir();

  if (!raw_mode && opt_find_gtid_str == nullptr) {
    fprintf(result_file, "/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=1*/;\n");

    if (disable_log_bin)
      fprintf(
          result_file,
          "/*!32316 SET @OLD_SQL_LOG_BIN=@@SQL_LOG_BIN, SQL_LOG_BIN=0*/;\n");

    /*
      In mysqlbinlog|mysql, don't want mysql to be disconnected after each
      transaction (which would be the case with GLOBAL.COMPLETION_TYPE==2).
    */
    fprintf(result_file,
            "/*!50003 SET @OLD_COMPLETION_TYPE=@@COMPLETION_TYPE,"
            "COMPLETION_TYPE=0*/;\n");

    if (charset)
      fprintf(
          result_file,
          "\n/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;"
          "\n/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS "
          "*/;"
          "\n/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;"
          "\n/*!40101 SET NAMES %s */;\n",
          charset);
  }
  /*
    In case '--idempotent' or '-i' options has been used, we will notify the
    server to use idempotent mode for the following events.
   */
  if (idempotent_mode)
    fprintf(result_file,
            "/*!50700 SET @@SESSION.RBR_EXEC_MODE=IDEMPOTENT*/;\n\n");

  if (opt_require_row_format) {
    fprintf(result_file, "/*!80019 SET @@SESSION.REQUIRE_ROW_FORMAT=1*/;\n\n");
  }

  if (opt_start_gtid_str != nullptr || opt_find_gtid_str != nullptr) {
    if (opt_start_gtid_str != nullptr && opt_remote_proto == BINLOG_DUMP_GTID) {
      char *args = const_cast<char *>("");
      retval = dump_multiple_logs(1, &args);
    } else {
      if (init_previous_gtid_set_map(opt_index_file_str)) {
        error(
            "initialization of the previous gtid log events from index file "
            "failed");
        exit(1);
      }
      if (opt_start_gtid_str)
        retval = start_gtid_dump(opt_start_gtid_str, false);
      else if (opt_find_gtid_str)
        retval = start_gtid_dump(opt_find_gtid_str, true);
    }
  } else if (!argc && opt_exclude_gtids_str != NULL) {
    // server can automatically find binlog file name when using
    // auto_position using --exclude-gtids. So passing binlog file
    // name to mysqlbinlog is not necessary in this case.
    char *args = NULL;
    retval = dump_multiple_logs(1, &args);
  } else {
    retval = dump_multiple_logs(argc, argv);
  }

  if (!raw_mode && opt_find_gtid_str == nullptr) {
    fprintf(result_file, "# End of log file\n");

    fprintf(result_file,
            "/*!50003 SET COMPLETION_TYPE=@OLD_COMPLETION_TYPE*/;\n");
    if (disable_log_bin)
      fprintf(result_file, "/*!32316 SET SQL_LOG_BIN=@OLD_SQL_LOG_BIN*/;\n");

    if (charset)
      fprintf(
          result_file,
          "/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;\n"
          "/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;\n"
          "/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;\n");

    fprintf(result_file, "/*!50530 SET @@SESSION.PSEUDO_SLAVE_MODE=0*/;\n");
  }

  // print encountered gtids to stderr
  if (opt_print_gtids) {
    global_sid_lock->rdlock();
    char *encountered_gtids = nullptr;
    gtid_set_excluded->to_string(&encountered_gtids);
    if (encountered_gtids) {
      // Replace new lines with spaces. Easy for parsing the output.
      for (char *ptr = encountered_gtids; *ptr; ++ptr) {
        if (*ptr == '\n') *ptr = ' ';
      }
      // NO_LINT_DEBUG
      fprintf(stderr, "Executed gtids: %s\n", encountered_gtids);
      my_free(encountered_gtids);
    }
    global_sid_lock->unlock();
  }

  /*
    We should unset the RBR_EXEC_MODE since the user may concatenate output of
    multiple runs of mysqlbinlog, all of which may not run in idempotent mode.
   */
  if (idempotent_mode)
    fprintf(result_file, "/*!50700 SET @@SESSION.RBR_EXEC_MODE=STRICT*/;\n");

  if (tmpdir.list) free_tmpdir(&tmpdir);
  if (result_file && (result_file != stdout)) my_fclose(result_file, MYF(0));
  cleanup();

  load_processor.destroy();
  /* We cannot free DBUG, it is used in global destructors after exit(). */
  my_end(my_end_arg | MY_DONT_FREE_DBUG);
  gtid_client_cleanup();

  return (retval == ERROR_STOP ? EXIT_FAILURE : EXIT_SUCCESS);
}

#ifndef MYSQL_SERVER
void Transaction_payload_log_event::print(FILE *,
                                          PRINT_EVENT_INFO *info) const {
  DBUG_TRACE;

  bool has_crc{(glob_description_event.footer()->checksum_alg ==
                binary_log::BINLOG_CHECKSUM_ALG_CRC32)};
  Format_description_event fde_no_crc = glob_description_event;
  fde_no_crc.footer()->checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_OFF;

  bool error{false};
  IO_CACHE *const head = &info->head_cache;
  size_t current_buffer_size = 1024;
  auto buffer = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, current_buffer_size,
                                   MYF(MY_WME));
  if (!info->short_form) {
    std::ostringstream oss;
    oss << "\tTransaction_Payload\t" << to_string() << std::endl;
    oss << "# Start of compressed events!" << std::endl;
    print_header(head, info, false);
    my_b_printf(head, "%s", oss.str().c_str());
  }

  // print the payload
  binary_log::transaction::compression::Iterable_buffer it(
      m_payload, m_payload_size, m_uncompressed_size, m_compression_type);

  for (auto ptr : it) {
    Log_event *ev = nullptr;
    bool is_deferred_event = false;

    // fix the checksum part
    size_t event_len = uint4korr(ptr + EVENT_LEN_OFFSET);

    // resize the buffer we are using to handle the event if needed
    if (event_len > current_buffer_size) {
      current_buffer_size =
          round(((event_len + BINLOG_CHECKSUM_LEN) / 1024.0) + 1) * 1024;
      buffer = (uchar *)my_realloc(PSI_NOT_INSTRUMENTED, buffer,
                                   current_buffer_size, MYF(0));

      /* purecov: begin inspected */
      if (!buffer) {
        // OOM
        head->error = -1;
        my_b_printf(head, "# Out of memory!");
        goto end;
      }
      /* purecov: end */
    }

    memcpy(buffer, ptr, event_len);

    // rewrite the database name if needed
    std::tie(buffer, current_buffer_size, event_len, error) =
        global_database_rewriter.rewrite(buffer, current_buffer_size, event_len,
                                         fde_no_crc);

    /* purecov: begin inspected */
    if (error) {
      head->error = -1;
      my_b_printf(head, "# Error while rewriting db for compressed events!");
      goto end;
    }
    /* purecov: end */

    // update the CRC
    if (has_crc) {
      int4store(buffer + EVENT_LEN_OFFSET, event_len + BINLOG_CHECKSUM_LEN);
      int4store(buffer + event_len, checksum_crc32(0, buffer, event_len));
      event_len += BINLOG_CHECKSUM_LEN;
    }

    // now deserialize the event
    if (binlog_event_deserialize((const unsigned char *)buffer, event_len,
                                 &glob_description_event, true, &ev)) {
      /* purecov: begin inspected */
      head->error = -1;
      my_b_printf(
          head, "# Error while handling compressed events! Corrupted binlog?");
      goto end;
      /* purecov: end */
    }

    switch (ev->get_type_code()) {
      // Statement Based Replication
      //   deferred events have to keep a copy of the buffer
      //   they are output only when the correct event comes
      //   later (Query_log_event)
      case binary_log::INTVAR_EVENT: /* purecov: inspected */
      case binary_log::RAND_EVENT:
      case binary_log::USER_VAR_EVENT:
        is_deferred_event = true; /* purecov: inspected */
        break;                    /* purecov: inspected */
      default:
        is_deferred_event = false;
        break;
    }

    ev->register_temp_buf((char *)buffer, is_deferred_event);
    ev->common_header->log_pos = header()->log_pos;

    // TODO: make this iterative, not recursive (process_event may rely
    // on global vars, and this may cause problems).
    process_event(info, ev, header()->log_pos, "", true);

    // lets make the buffer be allocated again, as the current
    // buffer ownership has been handed over to the defferred event
    if (is_deferred_event) {
      buffer = nullptr;        /* purecov: inspected */
      current_buffer_size = 0; /* purecov: inspected */
    }
  }

  if (!info->short_form) my_b_printf(head, "# End of compressed events!\n");

end:
  my_free(buffer);
}
#endif
