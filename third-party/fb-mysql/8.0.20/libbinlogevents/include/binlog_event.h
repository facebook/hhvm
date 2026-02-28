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

/**
  @addtogroup Replication
  @{

  @file binlog_event.h

  @brief Contains the classes representing events occurring in the replication
  stream. Each event is represented as a byte sequence with logical divisions
  as event header, event specific data and event footer. The header and footer
  are common to all the events and are represented as two different subclasses.
*/

#ifndef BINLOG_EVENT_INCLUDED
#define BINLOG_EVENT_INCLUDED

#include <stdlib.h>
#include <sys/types.h>
#include <zlib.h>  //for checksum calculations
#include <climits>
#include <cstdio>
#include <iostream>

#include "debug_vars.h"
#include "event_reader.h"
#include "my_io.h"

#if defined(_WIN32)
#include <Winsock2.h>
#else
#include <sys/times.h>
#endif
/*
  The symbols below are a part of the common definitions between
  the MySQL server and the client. Since they should not be a part of
  this library but the server, these should be placed in a header file
  common to the library and the MySQL server code, so that if they are
  updated in the server code, it is reflected in the libbinlogevent also.

  TODO(WL#7984): Moving the binlog constant in library libbinlogevents into a
                 separate file and make them const variables
*/
#ifndef SYSTEM_CHARSET_MBMAXLEN
#define SYSTEM_CHARSET_MBMAXLEN 3
#endif
#ifndef NAME_CHAR_LEN
#define NAME_CHAR_LEN 64 /* Field/table name length */
#endif
#ifndef NAME_LEN
#define NAME_LEN (NAME_CHAR_LEN * SYSTEM_CHARSET_MBMAXLEN)
#endif
/* Length of the server_version_split array in FDE class */
#ifndef ST_SERVER_VER_SPLIT_LEN
#define ST_SERVER_VER_SPLIT_LEN 3
#endif

/*
   Do not decrease the value of BIN_LOG_HEADER_SIZE.
   Do not even increase it before checking code.
*/
#ifndef BIN_LOG_HEADER_SIZE
#define BIN_LOG_HEADER_SIZE 4U
#endif

/**
   binlog_version 3 is MySQL 4.x; 4 is MySQL 5.0.0.
   Compared to version 3, version 4 has:
   - a different Start_event, which includes info about the binary log
   (sizes of headers); this info is included for better compatibility if the
   master's MySQL version is different from the slave's.
*/
#define BINLOG_VERSION 4

/*
  Constants used by Query_event.
*/

/**
   The maximum number of updated databases that a status of
   Query-log-event can carry.  It can be redefined within a range
   [1.. OVER_MAX_DBS_IN_EVENT_MTS].
*/
#define MAX_DBS_IN_EVENT_MTS 16

/**
   When the actual number of databases exceeds MAX_DBS_IN_EVENT_MTS
   the value of OVER_MAX_DBS_IN_EVENT_MTS is is put into the
   mts_accessed_dbs status.
*/
#define OVER_MAX_DBS_IN_EVENT_MTS 254

/**
  Max number of possible extra bytes in a replication event compared to a
  packet (i.e. a query) sent from client to master;
  First, an auxiliary log_event status vars estimation:
*/
#define MAX_SIZE_LOG_EVENT_STATUS                                             \
  (1U + 4 /* type, flags2 */ + 1U + 8 /* type, sql_mode */ + 1U + 1 +         \
   255 /* type, length, catalog */ + 1U + 4 /* type, auto_increment */ + 1U + \
   6 /* type, charset */ + 1U + 1 + 255 /* type, length, time_zone */ + 1U +  \
   2 /* type, lc_time_names_number */ + 1U +                                  \
   2 /* type, charset_database_number */ + 1U +                               \
   8 /* type, table_map_for_update */ + 1U +                                  \
   4 /* type, master_data_written */ + /* type, db_1, db_2, ... */            \
   1U + (MAX_DBS_IN_EVENT_MTS * (1 + NAME_LEN)) + 3U +                        \
   /* type, microseconds */ +1U + 32 * 3 + /* type, user_len, user */         \
   1 + 255 /* host_len, host */ + 1U + 1 /* type, explicit_def..ts*/ + 1U +   \
   8 /* type, xid of DDL */ + 1U +                                            \
   2 /* type, default_collation_for_utf8mb4_number */ +                       \
   1 /* sql_require_primary_key */ + 1 /* type, default_table_encryption */)

/**
   Uninitialized timestamp value (for either last committed or sequence number).
   Often carries meaning of the minimum value in the logical timestamp domain.
*/
const int64_t SEQ_UNINIT = 0;

/** We use 7 bytes, 1 bit being used as a flag. */
#define MAX_COMMIT_TIMESTAMP_VALUE (1ULL << 55)
/**
  Used to determine whether the original_commit_timestamp is already known or if
  it still needs to be determined when computing it.
*/
const int64_t UNDEFINED_COMMIT_TIMESTAMP = MAX_COMMIT_TIMESTAMP_VALUE;

const uint32_t UNDEFINED_SERVER_VERSION = 999999;
const uint32_t UNKNOWN_SERVER_VERSION = 0;

/** Setting this flag will mark an event as Ignorable */
#define LOG_EVENT_IGNORABLE_F 0x80

/**
  In case the variable is updated,
  make sure to update it in $MYSQL_SOURCE_DIR/my_io.h.
*/
#ifndef FN_REFLEN
#define FN_REFLEN 512 /* Max length of full path-name */
#endif

/**
   Splits server 'version' string into three numeric pieces stored
   into 'split_versions':
   X.Y.Zabc (X,Y,Z numbers, a not a digit) -> {X,Y,Z}
   X.Yabc -> {X,Y,0}

   @param version        String representing server version
   @param split_versions Array with each element containing one split of the
                         input version string
*/
inline void do_server_version_split(const char *version,
                                    unsigned char split_versions[3]) {
  const char *p = version;
  char *r;
  unsigned long number;
  for (unsigned int i = 0; i <= 2; i++) {
    number = strtoul(p, &r, 10);
    /*
      It is an invalid version if any version number greater than 255 or
      first number is not followed by '.'.
    */
    if (number < 256 && (*r == '.' || i != 0))
      split_versions[i] = static_cast<unsigned char>(number);
    else {
      split_versions[0] = 0;
      split_versions[1] = 0;
      split_versions[2] = 0;
      break;
    }

    p = r;
    if (*r == '.') p++;  // skip the dot
  }
}

/**
   Transforms the server version from 'XX.YY.ZZ-suffix' into an integer in the
   format XXYYZZ.

   @param version        String representing server version
   @return               The server version in the format XXYYZZ
*/
inline uint32_t do_server_version_int(const char *version) {
  unsigned char version_split[3];
  do_server_version_split(version, version_split);
  uint32_t ret = static_cast<uint32_t>(version_split[0]) * 10000 +
                 static_cast<uint32_t>(version_split[1]) * 100 +
                 static_cast<uint32_t>(version_split[2]);
  return ret;
}

/**
  Calculate the version product from the numeric pieces representing the server
  version:
  For a server version X.Y.Zabc (X,Y,Z numbers, a not a digit), the input is
  {X,Y,Z}. This is converted to XYZ in bit representation.

  @param  version_split Array containing the version information of the server
  @return               The version product of the server
*/
inline unsigned long version_product(const unsigned char *version_split) {
  return ((version_split[0] * 256 + version_split[1]) * 256 + version_split[2]);
}

/**
   Replication event checksum is introduced in the following "checksum-home"
   version. The checksum-aware servers extract FD's version to decide whether
   the FD event  carries checksum info.
*/
extern const unsigned char checksum_version_split[3];
extern const unsigned long checksum_version_product;
/**
  @namespace binary_log

  The namespace contains classes representing events that can occur in a
  replication stream.
*/
namespace binary_log {
/*
   This flag only makes sense for Format_description_event. It is set
   when the event is written, and *reset* when a binlog file is
   closed (yes, it's the only case when MySQL modifies an already written
   part of the binlog).  Thus it is a reliable indicator that the binlog was
   closed correctly.  (Stop_event is not enough, there's always a
   small chance that mysqld crashes in the middle of insert and end of
   the binlog would look like a Stop_event).

   This flag is used to detect a restart after a crash, and to provide
   "unbreakable" binlog. The problem is that on a crash storage engines
   rollback automatically, while binlog does not.  To solve this we use this
   flag and automatically append ROLLBACK to every non-closed binlog (append
   virtually, on reading, file itself is not changed). If this flag is found,
   mysqlbinlog simply prints "ROLLBACK". Replication master does not abort on
   binlog corruption, but takes it as EOF, and replication slave forces a
   rollback in this case.

   Note, that old binlogs does not have this flag set, so we get a
   a backward-compatible behaviour.
*/
#define LOG_EVENT_BINLOG_IN_USE_F 0x1

/**
  Enumeration type for the different types of log events.
*/
enum Log_event_type {
  /**
    Every time you add a type, you have to
    - Assign it a number explicitly. Otherwise it will cause trouble
      if a event type before is deprecated and removed directly from
      the enum.
    - Fix Format_description_event::Format_description_event().
  */
  UNKNOWN_EVENT = 0,
  /*
    Deprecated since mysql 8.0.2. It is just a placeholder,
    should not be used anywhere else.
  */
  START_EVENT_V3 = 1,
  QUERY_EVENT = 2,
  STOP_EVENT = 3,
  ROTATE_EVENT = 4,
  INTVAR_EVENT = 5,

  METADATA_EVENT = 7, /* SLAVE_EVENT is repurposed as METADATA_EVENT */

  APPEND_BLOCK_EVENT = 9,
  DELETE_FILE_EVENT = 11,

  RAND_EVENT = 13,
  USER_VAR_EVENT = 14,
  FORMAT_DESCRIPTION_EVENT = 15,
  XID_EVENT = 16,
  BEGIN_LOAD_QUERY_EVENT = 17,
  EXECUTE_LOAD_QUERY_EVENT = 18,

  TABLE_MAP_EVENT = 19,

  /**
    The V1 event numbers are used from 5.1.16 until mysql-5.6.
  */
  WRITE_ROWS_EVENT_V1 = 23,
  UPDATE_ROWS_EVENT_V1 = 24,
  DELETE_ROWS_EVENT_V1 = 25,

  /**
    Something out of the ordinary happened on the master
   */
  INCIDENT_EVENT = 26,

  /**
    Heartbeat event to be send by master at its idle time
    to ensure master's online status to slave
  */
  HEARTBEAT_LOG_EVENT = 27,

  /**
    In some situations, it is necessary to send over ignorable
    data to the slave: data that a slave can handle in case there
    is code for handling it, but which can be ignored if it is not
    recognized.
  */
  IGNORABLE_LOG_EVENT = 28,
  ROWS_QUERY_LOG_EVENT = 29,

  /** Version 2 of the Row events */
  WRITE_ROWS_EVENT = 30,
  UPDATE_ROWS_EVENT = 31,
  DELETE_ROWS_EVENT = 32,

  GTID_LOG_EVENT = 33,
  ANONYMOUS_GTID_LOG_EVENT = 34,

  PREVIOUS_GTIDS_LOG_EVENT = 35,

  TRANSACTION_CONTEXT_EVENT = 36,

  VIEW_CHANGE_EVENT = 37,

  /* Prepared XA transaction terminal event similar to Xid */
  XA_PREPARE_LOG_EVENT = 38,

  /**
    Extension of UPDATE_ROWS_EVENT, allowing partial values according
    to binlog_row_value_options.
  */
  PARTIAL_UPDATE_ROWS_EVENT = 39,

  TRANSACTION_PAYLOAD_EVENT = 40,

  /**
    Add new events here - right above this comment!
    Existing events (except ENUM_END_EVENT) should never change their numbers
  */
  ENUM_END_EVENT /* end marker */
};

/**
  Struct to pass basic information about a event: type, query, is it ignorable
*/
struct Log_event_basic_info {
  Log_event_type event_type{UNKNOWN_EVENT};
  const char *query{nullptr};
  size_t query_length{0};
  bool ignorable_event{false};
};

/**
 The length of the array server_version, which is used to store the version
 of MySQL server.
 We could have used SERVER_VERSION_LENGTH, but this introduces an
 obscure dependency - if somebody decided to change SERVER_VERSION_LENGTH
 this would break the replication protocol
 both of these are used to initialize the array server_version
 SERVER_VERSION_LENGTH is used for global array server_version
 and ST_SERVER_VER_LEN for the Start_event_v3 member server_version
*/

#define ST_SERVER_VER_LEN 50

/*
   Event header offsets;
   these point to places inside the fixed header.
*/
#define EVENT_TYPE_OFFSET 4
#define SERVER_ID_OFFSET 5
#define EVENT_LEN_OFFSET 9
#define LOG_POS_OFFSET 13
#define FLAGS_OFFSET 17

/** start event post-header (for v3 and v4) */
#define ST_BINLOG_VER_OFFSET 0
#define ST_SERVER_VER_OFFSET 2
#define ST_CREATED_OFFSET (ST_SERVER_VER_OFFSET + ST_SERVER_VER_LEN)
#define ST_COMMON_HEADER_LEN_OFFSET (ST_CREATED_OFFSET + 4)

#define LOG_EVENT_HEADER_LEN 19U /* the fixed header length */

/**
   Fixed header length, where 4.x and 5.0 agree. That is, 5.0 may have a longer
   header (it will for sure when we have the unique event's ID), but at least
   the first 19 bytes are the same in 4.x and 5.0. So when we have the unique
   event's ID, LOG_EVENT_HEADER_LEN will be something like 26, but
   LOG_EVENT_MINIMAL_HEADER_LEN will remain 19.
*/
#define LOG_EVENT_MINIMAL_HEADER_LEN 19U

/**
  Enumeration spcifying checksum algorithm used to encode a binary log event
*/
enum enum_binlog_checksum_alg {
  /**
    Events are without checksum though its generator is checksum-capable
    New Master (NM).
  */
  BINLOG_CHECKSUM_ALG_OFF = 0,
  /** CRC32 of zlib algorithm */
  BINLOG_CHECKSUM_ALG_CRC32 = 1,
  /** the cut line: valid alg range is [1, 0x7f] */
  BINLOG_CHECKSUM_ALG_ENUM_END,
  /**
    Special value to tag undetermined yet checksum or events from
    checksum-unaware servers
  */
  BINLOG_CHECKSUM_ALG_UNDEF = 255
};

#define CHECKSUM_CRC32_SIGNATURE_LEN 4

/**
   defined statically while there is just one alg implemented
*/
#define BINLOG_CHECKSUM_LEN CHECKSUM_CRC32_SIGNATURE_LEN
#define BINLOG_CHECKSUM_ALG_DESC_LEN 1 /* 1 byte checksum alg descriptor */
#define LOG_EVENT_HEADER_SIZE 20

/**
  Calculate a long checksum for a memoryblock.

  @param crc       start value for crc
  @param pos       pointer to memory block
  @param length    length of the block

  @return checksum for a memory block
*/
inline uint32_t checksum_crc32(uint32_t crc, const unsigned char *pos,
                               size_t length) {
  BAPI_ASSERT(length <= UINT_MAX);
  return static_cast<uint32_t>(crc32(static_cast<unsigned int>(crc), pos,
                                     static_cast<unsigned int>(length)));
}

/*
  Forward declaration of Format_description_event class to be used in class
  Log_event_header
*/
class Format_description_event;

/**
  @class Log_event_footer

  The footer, in the current version of the MySQL server, only contains
  the checksum algorithm descriptor. The descriptor is contained in the
  FDE of the binary log. This is common for all the events contained in
  that binary log, and defines the algorithm used to checksum
  the events contained in the binary log.

  @anchor Table_common_footer
  The footer contains the following:
  <table>
  <caption>Common-Footer</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>checkusm_alg</td>
    <td>enum_checksum_alg</td>
    <td>Algorithm used to checksum the events contained in the binary log</td>
  </tr>

  </table>

  @note checksum *value* is not generated with the event. On master's side, it
        is calculated right before writing the event into the binary log. The
        data_written field of the event is adjusted (+BINLOG_CHECKSUM_LEN) to
        consider the checksum value as part of the event. On reading the event,
        if the Format Description Event (FDE) used when serializing the event
        tells that the events have checksum information, the checksum value can
        be retrieved from a particular offset of the serialized event buffer
        (event length - BINLOG_CHECKSUM_LEN) and checked for corruption by
        computing a new value over the event buffer. It is not required after
        that. Therefore, the checksum value is not required to be stored in the
        instance as a class member.
*/
class Log_event_footer {
 public:
  /**
    Wrapper to call get_checksum_alg(const char *, ulong) function based on the
    event reader object (which knows both buffer and buffer length).

    @param[in] reader The Event_reader object associated to the event buffer
                      of the FD event.

    @retval BINLOG_CHECKSUM_ALG_UNDEF originator is checksum-unaware
                                      (effectively no checksum).
    @retval BINLOG_CHECKSUM_ALG_OFF no checksum.
    @retval other the actual checksum algorithm descriptor.
  */
  static enum_binlog_checksum_alg get_checksum_alg(Event_reader &reader);

  /**
    The method returns the checksum algorithm used to checksum the binary log
    from a Format Description Event serialized buffer.

    For MySQL server versions < 5.6, the algorithm is undefined.

    @param buf buffer holding serialized FD event.
    @param len length of the event buffer.

    @retval BINLOG_CHECKSUM_ALG_UNDEF originator is checksum-unaware
                                      (effectively no checksum).
    @retval BINLOG_CHECKSUM_ALG_OFF no checksum.
    @retval other the actual checksum algorithm descriptor.
  */
  static enum_binlog_checksum_alg get_checksum_alg(const char *buf,
                                                   unsigned long len);

  static bool event_checksum_test(unsigned char *buf, unsigned long event_len,
                                  enum_binlog_checksum_alg alg);

  /* Constructors */
  Log_event_footer() : checksum_alg(BINLOG_CHECKSUM_ALG_UNDEF) {}

  /**
    This ctor will create a new object of Log_event_footer, and will adjust
    the event reader buffer size with respect to CRC usage.

    @param reader the Event_reader containing the serialized event (including
                  header, event data and optional checksum information).
    @param event_type the type of the event the footer belongs to.
    @param fde An FDE event, used to get information about CRC.
  */
  Log_event_footer(Event_reader &reader, Log_event_type event_type,
                   const Format_description_event *fde);

  explicit Log_event_footer(enum_binlog_checksum_alg checksum_alg_arg)
      : checksum_alg(checksum_alg_arg) {}

  /**
     @verbatim
     Master side:
     The value is set by caller of FD(Format Description) constructor
     In the FD case it's propagated into the last byte
     of post_header_len[].
     Slave side:
     On the slave side the value is assigned from post_header_len[last]
     of the last seen FD event.
     @endverbatim
     TODO(WL#7546): Revisit this comment when encoder is moved in libbinlogevent
  */
  enum_binlog_checksum_alg checksum_alg;
};

/**
  @class Log_event_header

  The Common-Header always has the same form and length within one
  version of MySQL.  Each event type specifies a format and length
  of the Post-Header.  The length of the Common-Header is the same
  for all events of the same type.

  @anchor Table_common_header
    The binary format of Common-Header is as follows:
  <table>
  <caption>Common-Header</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>when</td>
    <td>4 byte unsigned integer, represented by type struct timeval</td>
    <td>The time when the query started, in seconds since 1970.
    </td>
  </tr>

  <tr>
    <td>type_code</td>
    <td>1 byte enumeration</td>
    <td>See enum #Log_event_type.</td>
  </tr>

  <tr>
    <td>unmasked_server_id</td>
    <td>4 byte unsigned integer</td>
    <td>Server ID of the server that created the event.</td>
  </tr>

  <tr>
    <td>data_written</td>
    <td>4 byte unsigned integer</td>
    <td>The total size of this event, in bytes.  In other words, this
    is the sum of the sizes of Common-Header, Post-Header, and Body.
    </td>
  </tr>

  <tr>
    <td>log_pos</td>
    <td>4 byte unsigned integer</td>
    <td>The position of the next event in the master binary log, in
    bytes from the beginning of the file.  In a binlog that is not a
    relay log, this is just the position of the next event, in bytes
    from the beginning of the file.  In a relay log, this is
    the position of the next event in the master's binlog.
    </td>
  </tr>

  <tr>
    <td>flags</td>
    <td>2 byte bitfield</td>
    <td>16 or less flags depending on the version of the binary log.</td>
  </tr>
  </table>

  Summing up the numbers above, we see that the total size of the
  common header is 19 bytes.
*/
class Log_event_header {
 public:
  /*
    Timestamp on the master(for debugging and replication of
    NOW()/TIMESTAMP).  It is important for queries and LOAD DATA
    INFILE. This is set at the event's creation time, except for Query
    and Load (and other events) events where this is set at the query's
    execution time, which guarantees good replication (otherwise, we
    could have a query and its event with different timestamps).
  */
  struct timeval when;

  /**
    Event type extracted from the header. In the server, it is decoded
    by read_log_event(), but adding here for complete decoding.
  */
  Log_event_type type_code;

  /*
    The server id read from the Binlog.
  */
  unsigned int unmasked_server_id;

  /* Length of an event, which will be written by write() function */
  size_t data_written;

  /*
    The offset in the log where this event originally appeared (it is
    preserved in relay logs, making SHOW SLAVE STATUS able to print
    coordinates of the event in the master's binlog).
  */
  unsigned long long log_pos;

  /*
    16 or less flags depending on the version of the binary log.
    See the definitions above for LOG_EVENT_TIME_F,
    LOG_EVENT_FORCED_ROTATE_F, LOG_EVENT_THREAD_SPECIFIC_F, and
    LOG_EVENT_SUPPRESS_USE_F for notes.
  */
  uint16_t flags;

  /**
    The following type definition is to be used whenever data is placed
    and manipulated in a common buffer. Use this typedef for buffers
    that contain data containing binary and character data.
  */
  typedef unsigned char Byte;

  explicit Log_event_header(Log_event_type type_code_arg = ENUM_END_EVENT)
      : type_code(type_code_arg), data_written(0), log_pos(0), flags(0) {
    when.tv_sec = 0;
    when.tv_usec = 0;
  }
  /**
    Log_event_header constructor.

    @param reader the Event_reader containing the serialized event (including
                  header, event data and optional checksum information).
  */
  Log_event_header(Event_reader &reader);

  /**
    The get_is_valid function is related to event specific sanity checks to
    determine that the object was initialized without errors.

    Note that a given event object may be valid at some point (ancestor
    event type initialization was fine) but be turned invalid in a later
    stage.

    @return True if the event object is valid, false otherwise.
  */

  bool get_is_valid() { return m_is_valid; }

  /**
    Set if the event object shall be considered valid or not.

    @param is_valid if the event object shall be considered valid.
  */

  void set_is_valid(bool is_valid) { m_is_valid = is_valid; }

 private:
  /*
    As errors might happen when de-serializing events, the m_is_valid variable
    will hold information about the validity of the event.

    An invalid event shall never be applied/dumped/displayed, as its
    interpretation (accessing its contents) might lead to using invalid
    memory pointers.
  */
  bool m_is_valid;
};

/**
  This is the abstract base class for binary log events.

  @section Binary_log_event_binary_format Binary Format

  @anchor Binary_log_event_format
  Any @c Binary_log_event saved on disk consists of the following four
  components.

  - Common-Header
  - Post-Header
  - Body
  - Footer

  Common header has the same format and length in a given MySQL version. It is
  documented @ref Table_common_header "here".

  The Body may be of different format and length even for different events of
  the same type. The binary formats of Post-Header and Body are documented
  separately in each subclass.

  Footer is common to all the events in a given MySQL version. It is documented
  @ref Table_common_footer "here".

  @anchor packed_integer
  - Some events, used for RBR use a special format for efficient representation
  of unsigned integers, called Packed Integer.  A Packed Integer has the
  capacity of storing up to 8-byte integers, while small integers
  still can use 1, 3, or 4 bytes.  The value of the first byte
  determines how to read the number, according to the following table:

  <table>
  <caption>Format of Packed Integer</caption>

  <tr>
    <th>First byte</th>
    <th>Format</th>
  </tr>

  <tr>
    <td>0-250</td>
    <td>The first byte is the number (in the range 0-250), and no more
    bytes are used.</td>
  </tr>

  <tr>
    <td>252</td>
    <td>Two more bytes are used.  The number is in the range
    251-0xffff.</td>
  </tr>

  <tr>
    <td>253</td>
    <td>Three more bytes are used.  The number is in the range
    0xffff-0xffffff.</td>
  </tr>

  <tr>
    <td>254</td>
    <td>Eight more bytes are used.  The number is in the range
    0xffffff-0xffffffffffffffff.</td>
  </tr>

  </table>

  - Strings are stored in various formats.  The format of each string
  is documented separately.

*/
class Binary_log_event {
 public:
  /*
     The number of types we handle in Format_description_event (UNKNOWN_EVENT
     is not to be handled, it does not exist in binlogs, it does not have a
     format).
  */
  static const int LOG_EVENT_TYPES = (ENUM_END_EVENT - 1);

  /**
    The lengths for the fixed data part of each event.
    This is an enum that provides post-header lengths for all events.
  */
  enum enum_post_header_length {
    // where 3.23, 4.x and 5.0 agree
    QUERY_HEADER_MINIMAL_LEN = (4 + 4 + 1 + 2),
    // where 5.0 differs: 2 for length of N-bytes vars.
    QUERY_HEADER_LEN = (QUERY_HEADER_MINIMAL_LEN + 2),
    STOP_HEADER_LEN = 0,
    START_V3_HEADER_LEN = (2 + ST_SERVER_VER_LEN + 4),
    // this is FROZEN (the Rotate post-header is frozen)
    ROTATE_HEADER_LEN = 8,
    INTVAR_HEADER_LEN = 0,
    APPEND_BLOCK_HEADER_LEN = 4,
    DELETE_FILE_HEADER_LEN = 4,
    RAND_HEADER_LEN = 0,
    USER_VAR_HEADER_LEN = 0,
    FORMAT_DESCRIPTION_HEADER_LEN = (START_V3_HEADER_LEN + 1 + LOG_EVENT_TYPES),
    XID_HEADER_LEN = 0,
    BEGIN_LOAD_QUERY_HEADER_LEN = APPEND_BLOCK_HEADER_LEN,
    ROWS_HEADER_LEN_V1 = 8,
    TABLE_MAP_HEADER_LEN = 8,
    EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN = (4 + 4 + 4 + 1),
    EXECUTE_LOAD_QUERY_HEADER_LEN =
        (QUERY_HEADER_LEN + EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN),
    INCIDENT_HEADER_LEN = 2,
    HEARTBEAT_HEADER_LEN = 0,
    IGNORABLE_HEADER_LEN = 0,
    ROWS_HEADER_LEN_V2 = 10,
    TRANSACTION_CONTEXT_HEADER_LEN = 18,
    VIEW_CHANGE_HEADER_LEN = 52,
    XA_PREPARE_HEADER_LEN = 0,
    TRANSACTION_PAYLOAD_HEADER_LEN = 0,
  };  // end enum_post_header_length
 protected:
  /**
    This constructor is used to initialize the type_code of header object
    m_header.
    We set the type code to ENUM_END_EVENT so that the decoder
    asserts if event type has not been modified by the sub classes
  */
  explicit Binary_log_event(Log_event_type type_code)
      : m_reader(nullptr, 0), m_header(type_code) {}

  /**
    This constructor will create a new object of Log_event_header and initialize
    the variable m_header, which in turn will be used to initialize Log_event's
    member common_header.
    It will also advance the Event_reader cursor after decoding the header (it
    is done through the constructor of Log_event_header) and will be pointing to
    the start of event data.

    @param buf  Contains the serialized event.
    @param fde  An FDE event used to get checksum information of non FDE events.
  */
  Binary_log_event(const char **buf, const Format_description_event *fde);

 public:
#ifndef HAVE_MYSYS
  /*
    The print_event_info functions are used in the free standing version of
    the library only. Since MySQL server does not use them, and it does not
    link to standard input/output library on Windows 32 bit system ,these
    methods are commented out when the library(libbinlogevents) is built
    with the server.
  */
  /**
    Returns short information about the event
  */
  virtual void print_event_info(std::ostream &info) = 0;
  /**
    Returns detailed information about the event
  */
  virtual void print_long_info(std::ostream &info) = 0;
#endif
  virtual ~Binary_log_event() = 0;

  Binary_log_event(const Binary_log_event &) = default;
  Binary_log_event(Binary_log_event &&) = default;
  Binary_log_event &operator=(const Binary_log_event &) = default;
  Binary_log_event &operator=(Binary_log_event &&) = default;

  /**
   * Helper method
   */
  enum Log_event_type get_event_type() const { return m_header.type_code; }

  /**
    Return a const pointer to the header of the log event
  */
  const Log_event_header *header() const { return &m_header; }
  /**
    Return a non-const pointer to the header of the log event
  */
  Log_event_header *header() { return &m_header; }
  /**
    Return a const pointer to the footer of the log event
  */
  const Log_event_footer *footer() const { return &m_footer; }
  /**
    Return a non-const pointer to the footer of the log event
  */
  Log_event_footer *footer() { return &m_footer; }
  /**
    Returns a reference to the event Event_reader object.
  */
  Event_reader &reader() { return m_reader; }

 private:
  /*
    All the accesses to the event buffer shall be performed by using m_reader
    methods.
  */
  Event_reader m_reader;
  Log_event_header m_header;
  Log_event_footer m_footer;
};

/**
  @class Unknown_event

  An unknown event should never occur. It is never written to a binary log.
  If an event is read from a binary log that cannot be recognized as
  something else, it is treated as UNKNOWN_EVENT.

  The Post-Header and Body for this event type are empty; it only has
  the Common-Header.
*/
class Unknown_event : public Binary_log_event {
 public:
  /**
    This is the minimal constructor, and set the type_code as
    UNKNOWN_EVENT in the header object in Binary_log_event
  */
  Unknown_event() : Binary_log_event(UNKNOWN_EVENT) {}

  Unknown_event(const char *buf, const Format_description_event *fde);
#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &info);
  void print_long_info(std::ostream &info);
#endif
};
}  // end namespace binary_log
/**
  @} (end of group Replication)
*/
#endif /* BINLOG_EVENT_INCLUDED */
