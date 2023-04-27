/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file

  Low level functions for storing data to be send to the MySQL client.
  The actual communication is handled by the net_xxx functions in net_serv.cc
*/

/* clang-format off */
/**
  @page page_protocol_basics Protocol Basics

  This is a description of the basic building blocks used by the MySQL protocol:
  - @subpage page_protocol_basic_data_types
  - @subpage page_protocol_basic_packets
  - @subpage page_protocol_basic_response_packets
  - @subpage page_protocol_basic_character_set
  - @subpage page_protocol_basic_compression
  - @subpage page_protocol_basic_tls
  - @subpage page_protocol_basic_expired_passwords
*/


/**
  @page page_protocol_basic_data_types Basic Data Types

  The protocol has a few basic types that are used throughout the protocol:
  - @subpage page_protocol_basic_dt_integers
  - @subpage page_protocol_basic_dt_strings
*/

/**
  @page page_protocol_basic_dt_integers Integer Types

  The MySQL %Protocol has a set of possible encodings for integers.

  @section sect_protocol_basic_dt_int_fixed Protocol::FixedLengthInteger

  Fixed-Length Integer Types
  ============================

  A fixed-length unsigned integer stores its value in a series of
  bytes with the least significant byte first.

  The MySQL uses the following fixed-length unsigned integer variants:
  - @anchor a_protocol_type_int1 int\<1>:
    1 byte @ref sect_protocol_basic_dt_int_fixed.
  - @anchor a_protocol_type_int2 int\<2\>:
    2 byte @ref sect_protocol_basic_dt_int_fixed.  See int2store()
  - @anchor a_protocol_type_int3 int\<3\>:
    3 byte @ref sect_protocol_basic_dt_int_fixed.  See int3store()
  - @anchor a_protocol_type_int4 int\<4\>:
    4 byte @ref sect_protocol_basic_dt_int_fixed.  See int4store()
  - @anchor a_protocol_type_int6 int\<6\>:
    6 byte @ref sect_protocol_basic_dt_int_fixed.  See int6store()
  - @anchor a_protocol_type_int8 int\<8\>:
    8 byte @ref sect_protocol_basic_dt_int_fixed.  See int8store()

  See int3store() for an example.


  @section sect_protocol_basic_dt_int_le Protocol::LengthEncodedInteger

  Length-Encoded Integer Type
  ==============================

  An integer that consumes 1, 3, 4, or 9 bytes, depending on its numeric value

  To convert a number value into a length-encoded integer:

  Greater or equal |     Lower than | Stored as
  -----------------|----------------|-------------------------
                 0 |            251 | `1-byte integer`
               251 | 2<sup>16</sup> | `0xFC + 2-byte integer`
    2<sup>16</sup> | 2<sup>24</sup> | `0xFD + 3-byte integer`
    2<sup>24</sup> | 2<sup>64</sup> | `0xFE + 8-byte integer`

   Similarly, to convert a length-encoded integer into its numeric value
   check the first byte.

   @warning
   If the first byte of a packet is a length-encoded integer and
   its byte value is `0xFE`, you must check the length of the packet to
   verify that it has enough space for a 8-byte integer.
   If not, it may be an EOF_Packet instead.
*/

/**
  @page page_protocol_basic_dt_strings String Types

  Strings are sequences of bytes and appear in a few forms in the protocol.

  @section sect_protocol_basic_dt_string_fix Protocol::FixedLengthString

  Fixed-length strings have a known, hardcoded length.

  An example is the sql-state of the @ref page_protocol_basic_err_packet
  which is always 5 bytes long.

  @section sect_protocol_basic_dt_string_null Protocol::NullTerminatedString

  Strings that are terminated by a `00` byte.

  @section sect_protocol_basic_dt_string_var Protocol::VariableLengthString

  The length of the string is determined by another field or is calculated
  at runtime

  @section sect_protocol_basic_dt_string_le Protocol::LengthEncodedString

  A length encoded string is a string that is prefixed with length encoded
  integer describing the length of the string.

  It is a special case of @ref sect_protocol_basic_dt_string_var

  @section sect_protocol_basic_dt_string_eof Protocol::RestOfPacketString

  If a string is the last component of a packet, its length can be calculated
  from the overall packet length minus the current position.
*/

/**
  @page page_protocol_basic_response_packets Generic Response Packets

  For most commands the client sends to the server, the server returns one
  of these packets in response:
  - @subpage page_protocol_basic_ok_packet
  - @subpage page_protocol_basic_err_packet
  - @subpage page_protocol_basic_eof_packet
*/

/**
  @page page_protocol_command_phase %Command Phase

  In the command phase, the client sends a command packet with
  the sequence-id [00]:

  ~~~~~~~~
  13 00 00 00 03 53 ...
  01 00 00 00 01
              ^^- command-byte
           ^^---- sequence-id == 0
  ~~~~~~~~

  The first byte of the payload describes the command-type.
  See ::enum_server_command for the list of commands supported.

  The commands belong to one of the following sub-protocols

  - @subpage page_protocol_command_phase_text
  - @subpage page_protocol_command_phase_utility
  - @subpage page_protocol_command_phase_ps
  - @subpage page_protocol_command_phase_sp

  @sa ::dispatch_command
*/


/**
   @page page_protocol_command_phase_utility Utility Commands

   - @subpage page_protocol_com_quit
   - @subpage page_protocol_com_init_db
   - @subpage page_protocol_com_field_list
   - @subpage page_protocol_com_refresh
   - @subpage page_protocol_com_statistics
   - @subpage page_protocol_com_process_info
   - @subpage page_protocol_com_process_kill
   - @subpage page_protocol_com_debug
   - @subpage page_protocol_com_ping
   - @subpage page_protocol_com_change_user
   - @subpage page_protocol_com_reset_connection
   - @subpage page_protocol_com_set_option
*/


/**
   @page page_protocol_command_phase_text Text Protocol

   - @subpage page_protocol_com_query
*/

/**
  @page page_protocol_command_phase_ps Prepared Statements

  The prepared statement protocol was introduced in MySQL 4.1 and adds a
  few new commands:
    - @subpage page_protocol_com_stmt_prepare
    - @subpage page_protocol_com_stmt_execute
    - @subpage page_protocol_com_stmt_fetch
    - @subpage page_protocol_com_stmt_close
    - @subpage page_protocol_com_stmt_reset
    - @subpage page_protocol_com_stmt_send_long_data

  It also defines a more compact resultset format that is used instead of
  @ref page_protocol_com_query_response_text_resultset to return the results.

  @note Keep in mind that not all SQL statements can be prepared.

  @sa [WL#2871](https://dev.mysql.com/worklog/task/?id=2871)
*/

/**
  @page page_protocol_command_phase_sp Stored Programs

  In MySQL 5.0 the protocol was extended to handle:
   - @ref sect_protocol_command_phase_sp_multi_resultset
   - @ref sect_protocol_command_phase_sp_multi_statement

   @section sect_protocol_command_phase_sp_multi_resultset Multi-Resultset

   Multi-resultsets are sent by a stored program if more than one resultset was
   generated inside of it. e.g.:
   ~~~~~~~~~~~~
   CREATE TEMPORARY TABLE ins ( id INT );
   DROP PROCEDURE IF EXISTS multi;
   DELIMITER $$
   CREATE PROCEDURE multi() BEGIN
     SELECT 1;
     SELECT 1;
     INSERT INTO ins VALUES (1);
     INSERT INTO ins VALUES (2);
   END$$
   DELIMITER ;

   CALL multi();
   DROP TABLE ins;
   ~~~~~~~~~~~~

   results in:
   - a resultset
   ~~~~~~~~~~~~~
   01 00 00 01 01 17 00 00    02 03 64 65 66 00 00 00    ..........def...
   01 31 00 0c 3f 00 01 00    00 00 08 81 00 00 00 00    .1..?...........
   05 00 00 03 fe 00 00 0a    00 02 00 00 04 01 31 05    ..............1.
   00 00 05 fe 00 00 0a 00                               ........
   ~~~~~~~~~~~~~
      - see the @ref page_protocol_basic_eof_packet
        `05 00 00 03 fe 00 00 0a 00` with its status-flag being `0x0A`
   - another resultset:
   ~~~~~~~~~~~~~
   01 00 00 06 01 17 00 00    07 03 64 65 66 00 00 00    ..........def...
   01 31 00 0c 3f 00 01 00    00 00 08 81 00 00 00 00    .1..?...........
   05 00 00 08 fe 00 00 0a    00 02 00 00 09 01 31 05    ..............1.
   00 00 0a fe 00 00 0a 00                               ........
   ~~~~~~~~~~~~~
      - see the @ref page_protocol_basic_eof_packet
        `05 00 00 03 fe 00 00 0a 00` with its status-flag being `0x0A`
   - and a closing empty resultset, an @ref page_protocol_basic_ok_packet
   ~~~~~~~~~~~~~
   07 00 00 0b 00 01 00 02    00 00 00                   ...........
   ~~~~~~~~~~~~~

   If the ::SERVER_MORE_RESULTS_EXISTS flag ise set, that indicates more
   resultsets will follow.

   The trailing @ref page_protocol_basic_ok_packet is the response to the
   `CALL` statement and contains the `affected_rows` count of the last
   statement. In our case we inserted 2 rows, but only the `affected_rows`
   of the last `INSERT` statement is returned as part of the
   @ref page_protocol_basic_ok_packet. If the last statement is a `SELECT`,
   the `affected_rows` count is 0.

   As of MySQL 5.7.5, the resultset is followed by an
   @ref page_protocol_basic_ok_packet, and this
   @ref page_protocol_basic_ok_packet has the ::SERVER_MORE_RESULTS_EXISTS
   flag set to start the processing of the next resultset.

   The client has to announce that it wants multi-resultsets by either setting
   the ::CLIENT_MULTI_RESULTS or ::CLIENT_PS_MULTI_RESULTS capabilitiy flags.

   @subsection sect_protocol_command_phase_sp_multi_resultset_out_params OUT Parameter Set

   Starting with MySQL 5.5.3, prepared statements can bind OUT parameters of
   stored procedures. They are returned as an extra resultset in the
   multi-resultset response. The client announces it can handle OUT parameters
   by settting the ::CLIENT_PS_MULTI_RESULTS capability.

   To distinguish a normal resultset from an OUT parameter set, the
   @ref page_protocol_basic_eof_packet or (if ::CLIENT_DEPRECATE_EOF capability
   flag is set) @ref page_protocol_basic_ok_packet that follows its field
   definition has the ::SERVER_PS_OUT_PARAMS flag set.

   @note The closing @ref page_protocol_basic_eof_packet does NOT have either
   ::SERVER_PS_OUT_PARAMS flag nor the ::SERVER_MORE_RESULTS_EXISTS flag set.
   Only the first @ref page_protocol_basic_eof_packet has.

   @section sect_protocol_command_phase_sp_multi_statement Multi-Statement

   A multi-statement is permitting ::COM_QUERY to send more than one query to
   the server, separated by `;` characters.

   The client musst announce that it wants multi-statements by either setting
   the ::CLIENT_MULTI_STATEMENTS capability or by using
   @ref page_protocol_com_set_option
*/

/**
  @page page_protocol_com_set_option COM_SET_OPTION

  @brief Sets options for the current connection

  ::COM_SET_OPTION enables and disables server capabilities for the current
  connection.

  @note Only ::CLIENT_MULTI_STATEMENTS can be set to a
  value defined in ::enum_mysql_set_option.

  @return @ref page_protocol_basic_ok_packet on success,
    @ref page_protocol_basic_err_packet otherwise.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>status</td>
      <td>[0x1A] COM_SET_OPTION</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>option_operation</td>
      <td>One of ::enum_mysql_set_option</td></tr>
  </table>

  @sa ::mysql_set_server_option, ::mysql_parse
*/

/**
  @page page_protocol_connection_lifecycle Connection Lifecycle

  The MySQL protocol is a stateful protocol. When a connection is established
  the server initiates a @ref page_protocol_connection_phase. Once that is
  performed the connection enters the \ref page_protocol_command_phase. The
  @ref page_protocol_command_phase ends when the connection terminates.

  The connection can also enter @ref page_protocol_replication from
  @ref page_protocol_connection_phase if one of the replication commands
  is sent.

  @startuml
  [*] --> ConnectionState : client connects
  ConnectionState: Authentication
  ConnectionState --> [*] : Error or client disconnects
  ConnectionState --> CommandState : Successful authentication
  CommandState: RPC commands read/execute loop
  CommandState --> [*] : client or server closes the connection
  CommandState --> ReplicationMode : Replication command received
  ReplicationMode: binlog data streamed
  ReplicationMode --> [*] : client or server closes the connection
  @enduml

  Further reading:
  - @subpage page_protocol_connection_phase
  - @subpage page_protocol_command_phase
  - @subpage page_protocol_replication

*/

/**
  @page page_protocol_basic_character_set Character Set

  MySQL has a very flexible character set support as documented in
  [Character Set Support](http://dev.mysql.com/doc/refman/5.7/en/charset.html).
  The list of character sets and their IDs can be queried as follows:

<pre>
  SELECT id, collation_name FROM information_schema.collations ORDER BY id;
  +----+-------------------+
  | id | collation_name    |
  +----+-------------------+
  |  1 | big5_chinese_ci   |
  |  2 | latin2_czech_cs   |
  |  3 | dec8_swedish_ci   |
  |  4 | cp850_general_ci  |
  |  5 | latin1_german1_ci |
  |  6 | hp8_english_ci    |
  |  7 | koi8r_general_ci  |
  |  8 | latin1_swedish_ci |
  |  9 | latin2_general_ci |
  | 10 | swe7_swedish_ci   |
  +----+-------------------+
</pre>

  The following table shows a few common character sets.

  Number |  Hex  | Character Set Name
  -------|-------|-------------------
       8 |  0x08 | @ref my_charset_latin1 "latin1_swedish_ci"
      33 |  0x21 | @ref my_charset_utf8_general_ci "utf8_general_ci"
      63 |  0x3f | @ref my_charset_bin "binary"


  @anchor a_protocol_character_set Protocol::CharacterSet
  ----------------------

  A character set is defined in the protocol as a integer.
  Fields:
     - charset_nr (2) -- number of the character set and collation
*/
/* clang-format on */

/**
  @defgroup group_cs Client/Server Protocol

  Client/server protocol related structures,
  macros, globals and functions
*/

#include "sql/protocol_classic.h"

#include <openssl/ssl.h>

#include <string.h>
#include <zlib.h>  // crc32
#include <algorithm>
#include <limits>

#include "decimal.h"
#include "errmsg.h"  // CR_*
#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "my_time.h"
#include "mysql/com_data.h"
#include "mysql/psi/mysql_socket.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "mysys_err.h"
#include "sql/derror.h"
#include "sql/field.h"
#include "sql/item.h"
#include "sql/item_func.h"  // Item_func_set_user_var
#include "sql/my_decimal.h"
#include "sql/mysqld.h"  // global_system_variables
#include "sql/session_tracker.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_prepare.h"  // Prepared_statement
#include "sql/system_variables.h"
#include "sql_string.h"
#include "template_utils.h"

using std::max;
using std::min;

static const char *CHECKSUM = "checksum";
static const unsigned int PACKET_BUFFER_EXTRA_ALLOC = 1024;
static bool net_send_error_packet(THD *, uint, const char *, const char *);
static bool net_send_error_packet(NET *, uint, const char *, const char *, bool,
                                  ulong, const CHARSET_INFO *);
static bool write_eof_packet(THD *, NET *, uint, uint);
static ulong get_ps_param_len(enum enum_field_types, uchar *, ulong, ulong *,
                              bool *);

/**
  Ensures that the packet buffer has enough capacity to hold a string of the
  given length.

  @param length  the length of the string
  @param[in,out] packet  the buffer
  @return true if memory could not be allocated, false on success
*/
static bool ensure_packet_capacity(size_t length, String *packet) {
  size_t packet_length = packet->length();
  /*
     The +9 comes from that strings of length longer than 16M require
     9 bytes to be stored (see net_store_length).
  */
  return packet_length + 9 + length > packet->alloced_length() &&
         packet->mem_realloc(packet_length + 9 + length);
}

/**
  Store length and data in a network packet buffer.

  @param from    the data to store
  @param length  the length of the data
  @param[in,out] packet  the buffer
  @return true if there is not enough memory, false on success
*/
static inline bool net_store_data(const uchar *from, size_t length,
                                  String *packet) {
  if (ensure_packet_capacity(length, packet)) return true;
  size_t packet_length = packet->length();
  uchar *to = net_store_length((uchar *)packet->ptr() + packet_length, length);
  if (length > 0) memcpy(to, from, length);
  packet->length((uint)(to + length - (uchar *)packet->ptr()));
  return false;
}

/**
  Stores a string in the network buffer. The string is padded with zeros if it
  is shorter than the specified padded length.

  @param data           the string to store
  @param data_length    the length of the string
  @param padded_length  the length of the zero-padded string
  @param[in,out] packet the network buffer
*/
static bool net_store_zero_padded_data(const char *data, size_t data_length,
                                       size_t padded_length, String *packet) {
  const size_t zeros =
      padded_length > data_length ? padded_length - data_length : 0;
  const size_t full_length = data_length + zeros;
  if (ensure_packet_capacity(full_length, packet)) return true;
  uchar *to = net_store_length(
      pointer_cast<uchar *>(packet->ptr()) + packet->length(), full_length);
  memset(to, '0', zeros);
  if (data_length > 0)
    memcpy(to + zeros, pointer_cast<const uchar *>(data), data_length);
  packet->length(pointer_cast<char *>(to) + full_length - packet->ptr());
  return false;
}

/**
  net_store_data() - extended version with character set conversion.

  It is optimized for short strings whose length after
  conversion is guaranteed to be less than 251, which occupies
  exactly one byte to store length. It allows not to use
  the "convert" member as a temporary buffer, conversion
  is done directly to the "packet" member.
  The limit 251 is good enough to optimize send_result_set_metadata()
  because column, table, database names fit into this limit.
*/

bool Protocol_classic::net_store_data_with_conversion(
    const uchar *from, size_t length, const CHARSET_INFO *from_cs,
    const CHARSET_INFO *to_cs) {
  uint dummy_errors;
  /* Calculate maxumum possible result length */
  size_t conv_length = to_cs->mbmaxlen * length / from_cs->mbminlen;
  if (conv_length > 250) {
    /*
      For strings with conv_length greater than 250 bytes
      we don't know how many bytes we will need to store length: one or two,
      because we don't know result length until conversion is done.
      For example, when converting from utf8 (mbmaxlen=3) to latin1,
      conv_length=300 means that the result length can vary between 100 to 300.
      length=100 needs one byte, length=300 needs to bytes.

      Thus conversion directly to "packet" is not worthy.
      Let's use "convert" as a temporary buffer.
    */
    return (convert.copy(pointer_cast<const char *>(from), length, from_cs,
                         to_cs, &dummy_errors) ||
            net_store_data(pointer_cast<const uchar *>(convert.ptr()),
                           convert.length(), packet));
  }

  size_t packet_length = packet->length();
  size_t new_length = packet_length + conv_length + 1;

  if (new_length > packet->alloced_length() && packet->mem_realloc(new_length))
    return true;

  char *length_pos = packet->ptr() + packet_length;
  char *to = length_pos + 1;

  to += copy_and_convert(to, conv_length, to_cs, (const char *)from, length,
                         from_cs, &dummy_errors);

  net_store_length((uchar *)length_pos, to - length_pos - 1);
  packet->length((uint)(to - packet->ptr()));
  return false;
}

/**
  Send a error string to client.

  Design note:

  net_printf_error and net_send_error are low-level functions
  that shall be used only when a new connection is being
  established or at server startup.

  For SIGNAL/RESIGNAL and GET DIAGNOSTICS functionality it's
  critical that every error that can be intercepted is issued in one
  place only, my_message_sql.

  @param thd Thread handler
  @param sql_errno The error code to send
  @param err A pointer to the error message


  @retval false The message was sent to the client
  @retval true An error occurred and the message wasn't sent properly
*/

bool net_send_error(THD *thd, uint sql_errno, const char *err) {
  bool error;
  DBUG_TRACE;

  DBUG_ASSERT(!thd->sp_runtime_ctx);
  DBUG_ASSERT(sql_errno);
  DBUG_ASSERT(err);

  DBUG_PRINT("enter", ("sql_errno: %d  err: %s", sql_errno, err));

  /*
    It's one case when we can push an error even though there
    is an OK or EOF already.
  */
  thd->get_stmt_da()->set_overwrite_status(true);

  /* Abort multi-result sets */
  thd->server_status &= ~SERVER_MORE_RESULTS_EXISTS;

  error = net_send_error_packet(thd, sql_errno, err,
                                mysql_errno_to_sqlstate(sql_errno));

  thd->get_stmt_da()->set_overwrite_status(false);

  return error;
}

/**
  Send a error string to client using net struct.
  This is used initial connection handling code.

  @param net        Low-level net struct
  @param sql_errno  The error code to send
  @param err        A pointer to the error message

  @retval false The message was sent to the client
  @retval true  An error occurred and the message wasn't sent properly
*/

bool net_send_error(NET *net, uint sql_errno, const char *err) {
  DBUG_TRACE;

  DBUG_ASSERT(sql_errno && err);

  DBUG_PRINT("enter", ("sql_errno: %d  err: %s", sql_errno, err));

  bool error = net_send_error_packet(
      net, sql_errno, err, mysql_errno_to_sqlstate(sql_errno), false, 0,
      global_system_variables.character_set_results);

  return error;
}

/**
  @param thd Thread handler
  @param sql_errno The error code to send
  @param err A pointer to the error message
  @param sql_state The SQL state
   @param force_hash If '#' is always added
  @out buff The generated error packet (null-terminated)
  @out length The length of buff
   @return
   @retval FALSE The message was successfully sent
   @retval TRUE  An error occurred and the messages wasn't sent properly
*/
bool generate_error_packet(NET *net, uint sql_errno, const char *err,
                           const char *sql_state, bool force_hash,
                           bool bootstrap,
                           const CHARSET_INFO *character_set_results,
                           ulong client_capabilities, char *buff,
                           uint *length) {
  uint error;
  char converted_err[MYSQL_ERRMSG_SIZE], *pos;
  DBUG_ENTER("generate_error_packet");
  if (net->vio == 0) {
    if (bootstrap) {
      /* In bootstrap it's ok to print on stderr */
      my_message_local(ERROR_LEVEL, EE_NET_SEND_ERROR_IN_BOOTSTRAP, sql_errno,
                       err);
    }
    DBUG_RETURN(false);
  }

  int2store(buff, sql_errno);
  pos = buff + 2;
  if (force_hash || (client_capabilities & CLIENT_DEPRECATE_EOF)) {
    /* The first # is to make the protocol backward compatible */
    buff[2] = '#';
    pos = my_stpcpy(buff + 3, sql_state);
  }

  convert_error_message(converted_err, sizeof(converted_err),
                        character_set_results, err, strlen(err),
                        system_charset_info, &error);
  /* Converted error message is always null-terminated. */
  *length = (uint)(strmake(pos, converted_err, MYSQL_ERRMSG_SIZE - 1) - buff);

  DBUG_RETURN(true);
}

/* clang-format off */
/**
  @page page_protocol_basic_ok_packet OK_Packet

  An OK packet is sent from the server to the client to signal successful
  completion of a command. As of MySQL 5.7.5, OK packes are also used to
  indicate EOF, and EOF packets are deprecated.

  if ::CLIENT_PROTOCOL_41 is set, the packet contains a warning count.

  <table>
  <caption>The Payload of an OK Packet</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>header</td>
      <td>`0x00` or `0xFE` the OK packet header</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
      <td>affected_rows</td>
      <td>affected rows</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
      <td>last_insert_id</td>
      <td>last insert-id</td></tr>
  <tr><td colspan="3">if capabilities @& ::CLIENT_PROTOCOL_41 {</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>status_flags</td>
      <td>@ref SERVER_STATUS_flags_enum</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>warnings</td>
      <td>number of warnings</td></tr>
  <tr><td colspan="3">} else if capabilities @& ::CLIENT_TRANSACTIONS {</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>status_flags</td>
      <td>@ref SERVER_STATUS_flags_enum</td></tr>
  <tr><td colspan="3">}</td></tr>
  <tr><td colspan="3">if capabilities @& ::CLIENT_SESSION_TRACK</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>info</td>
      <td>human readable status information</td></tr>
  <tr><td colspan="3">  if status_flags @& ::SERVER_SESSION_STATE_CHANGED {</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>session state info</td>
      <td>@anchor a_protocol_basic_ok_packet_sessinfo
          @ref sect_protocol_basic_ok_packet_sessinfo</td></tr>
  <tr><td colspan="3">  }</td></tr>
  <tr><td colspan="3">} else {</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"</td>
      <td>info</td>
      <td>human readable status information</td></tr>
  <tr><td colspan="3">}</td></tr>
  </table>

  These rules distinguish whether the packet represents OK or EOF:
  - OK: header = 0 and length of packet > 7
  - EOF: header = 0xfe and length of packet < 9

  To ensure backward compatibility between old (prior to 5.7.5) and
  new (5.7.5 and up) versions of MySQL, new clients advertise
  the ::CLIENT_DEPRECATE_EOF flag:
  - Old clients do not know about this flag and do not advertise it.
    Consequently, the server does not send OK packets that represent EOF.
    (Old servers never do this, anyway. New servers recognize the absence
    of the flag to mean they should not.)
  - New clients advertise this flag. Old servers do not know this flag and
    do not send OK packets that represent EOF. New servers recognize the flag
    and can send OK packets that represent EOF.

  Example
  =======

  OK with ::CLIENT_PROTOCOL_41. 0 affected rows, last-insert-id was 0,
  AUTOCOMMIT enabled, 0 warnings. No further info.

  ~~~~~~~~~~~~~~~~~~~~~
  07 00 00 02 00 00 00 02    00 00 00
  ~~~~~~~~~~~~~~~~~~~~~

  @section sect_protocol_basic_ok_packet_sessinfo Session State Information

  State-change information is sent in the OK packet as a array of state-change
  blocks which are made up of:

  <table>
  <caption>Layout of Session State Information</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>type</td>
      <td>type of data. See enum_session_state_type</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>data</td>
      <td>data of the changed session info</td></tr>
  </table>

  Interpretation of the data field depends on the type value:

  @subsection sect_protocol_basic_ok_packet_sessinfo_SESSION_TRACK_SYSTEM_VARIABLES SESSION_TRACK_SYSTEM_VARIABLES

  <table>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>name</td>
      <td>name of the changed system variable</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>value</td>
      <td>value of the changed system variable</td></tr>
  </table>

  Example:

  After a SET autocommit = OFF statement:
  <table><tr>
  <td>
  ~~~~~~~~~~~~~~~~~~~~~
  00 0f1 0a 61 75 74 6f 63   6f 6d 6d 69 74 03 4f 46 46
  ~~~~~~~~~~~~~~~~~~~~~
  </td><td>
  ~~~~~~~~~~~~~~~~~~~~~
  ....autocommit.OFF
  ~~~~~~~~~~~~~~~~~~~~~
  </td></tr></table>

  @subsection sect_protocol_basic_ok_packet_sessinfo_SESSION_TRACK_SCHEMA SESSION_TRACK_SCHEMA

  <table>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>name</td>
      <td>name of the changed schema</td></tr>
  </table>

  Example:

  After a USE test statement:

  <table><tr>
  <td>
  ~~~~~~~~~~~~~~~~~~~~~
  01 05 04 74 65 73 74
  ~~~~~~~~~~~~~~~~~~~~~
  </td><td>
  ~~~~~~~~~~~~~~~~~~~~~
  ...test
  ~~~~~~~~~~~~~~~~~~~~~
  </td></tr></table>

  @subsection sect_protocol_basic_ok_packet_sessinfo_SESSION_TRACK_STATE_CHANGE SESSION_TRACK_STATE_CHANGE

  A flag byte that indicates whether session state changes occurred.
  This flag is represented as an ASCII value.

  <table>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
  <td>is_tracked</td>
  <td>`0x31` ("1") if state tracking got enabled.</td></tr>
  </table>

  Example:

  After a SET SESSION session_track_state_change = 1 statement:

  <table><tr>
  <td>
  ~~~~~~~~~~~~~~~~~~~~~
  03 02 01 31
  ~~~~~~~~~~~~~~~~~~~~~
  </td><td>
  ~~~~~~~~~~~~~~~~~~~~~
  ...1
  ~~~~~~~~~~~~~~~~~~~~~
  </td></tr></table>

  See also net_send_ok()
*/
/* clang-format on */

/**
  Return OK to the client.

  See @ref page_protocol_basic_ok_packet for the OK packet structure.

  @param thd                     Thread handler
  @param server_status           The server status
  @param statement_warn_count    Total number of warnings
  @param affected_rows           Number of rows changed by statement
  @param id                      Auto_increment id for first row (if used)
  @param message                 Message to send to the client
                                 (Used by mysql_status)
  @param eof_identifier          when true [FE] will be set in OK header
                                 else [00] will be used

  @retval false The message was successfully sent
  @retval true An error occurred and the messages wasn't sent properly
*/

static bool net_send_ok(THD *thd, uint server_status, uint statement_warn_count,
                        ulonglong affected_rows, ulonglong id,
                        const char *message, bool eof_identifier) {
  Protocol *protocol = thd->get_protocol();
  NET *net = thd->get_protocol_classic()->get_net();
  uchar buff[MYSQL_ERRMSG_SIZE + 10];
  uchar *pos, *start;

  /*
    To be used to manage the data storage in case session state change
    information is present.
  */
  String store;
  bool state_changed = false;

  bool error = false;
  DBUG_TRACE;

  if (!net->vio)  // hack for re-parsing queries
  {
    DBUG_PRINT("info", ("vio present: NO"));
    return false;
  }

  start = buff;

  /*
    Use 0xFE packet header if eof_identifier is true
    unless we are talking to old client
  */
  if (eof_identifier && (protocol->has_client_capability(CLIENT_DEPRECATE_EOF)))
    buff[0] = 254;
  else
    buff[0] = 0;

  /* affected rows */
  pos = net_store_length(buff + 1, affected_rows);

  /* last insert id */
  pos = net_store_length(pos, id);

  if (protocol->has_client_capability(CLIENT_SESSION_TRACK) &&
      thd->session_tracker.enabled_any() &&
      thd->session_tracker.changed_any()) {
    server_status |= SERVER_SESSION_STATE_CHANGED;
    state_changed = true;
  }

  if (protocol->has_client_capability(CLIENT_PROTOCOL_41)) {
    DBUG_PRINT("info",
               ("affected_rows: %lu  id: %lu  status: %u  warning_count: %u",
                (ulong)affected_rows, (ulong)id, (uint)(server_status & 0xffff),
                (uint)statement_warn_count));
    /* server status */
    int2store(pos, server_status);
    pos += 2;

    /* warning count: we can only return up to 65535 warnings in two bytes. */
    uint tmp = min(statement_warn_count, 65535U);
    int2store(pos, tmp);
    pos += 2;
  } else if (net->return_status)  // For 4.0 protocol
  {
    int2store(pos, server_status);
    pos += 2;
  }

  thd->get_stmt_da()->set_overwrite_status(true);

  if (protocol->has_client_capability(CLIENT_SESSION_TRACK)) {
    /* the info field */
    if (state_changed || (message && message[0]))
      pos = net_store_data(pos, pointer_cast<const uchar *>(message),
                           message ? strlen(message) : 0);
    /* session state change information */
    if (unlikely(state_changed)) {
      store.set_charset(thd->variables.collation_database);

      /*
        First append the fields collected so far. In case of malloc, memory
        for message is also allocated here.
      */
      store.append((const char *)start, (pos - start), MYSQL_ERRMSG_SIZE);

      /* .. and then the state change information. */
      thd->session_tracker.store(thd, store);

      start = (uchar *)store.ptr();
      pos = start + store.length();
    }
  } else if (message && message[0]) {
    /* the info field, if there is a message to store */
    pos = net_store_data(pos, pointer_cast<const uchar *>(message),
                         strlen(message));
  }

  /* OK packet length will be restricted to 16777215 bytes */
  if (((size_t)(pos - start)) > MAX_PACKET_LENGTH) {
    net->error = 1;
    net->last_errno = ER_NET_OK_PACKET_TOO_LARGE;
    my_error(ER_NET_OK_PACKET_TOO_LARGE, MYF(0));
    DBUG_PRINT("info", ("OK packet too large"));
    return true;
  }
  error = my_net_write(net, start, (size_t)(pos - start));
  if (!error) error = net_flush(net);

  thd->get_stmt_da()->set_overwrite_status(false);
  DBUG_PRINT("info", ("OK sent, so no more error sending allowed"));

  return error;
}

static uchar eof_buff[1] = {(uchar)254}; /* Marker for end of fields */

/* clang-format off */
/**
  @page page_protocol_basic_eof_packet EOF_Packet

  If ::CLIENT_PROTOCOL_41 is enabled, the EOF packet contains a
  warning count and status flags.

  @note
  In the MySQL client/server protocol, the
  @ref page_protocol_basic_eof_packet and
  @ref page_protocol_basic_ok_packet packets serve
  the same purpose, to mark the end of a query execution result.
  Due to changes in MySQL 5.7 in
  the @ref page_protocol_basic_ok_packet packets (such as session
  state tracking), and to avoid repeating the changes in
  the @ref page_protocol_basic_eof_packet packet, the
  @ref page_protocol_basic_ok_packet is deprecated as of MySQL 5.7.5.

  @warning
  The @ref page_protocol_basic_eof_packet packet may appear in places where
  a @ref sect_protocol_basic_dt_int_le "Protocol::LengthEncodedInteger"
  may appear. You must check whether the packet length is less than 9 to
  make sure that it is a @ref page_protocol_basic_eof_packet packet.

  <table>
  <caption>The Payload of an EOF Packet</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>header</td>
      <td>`0xFE` EOF packet header</td></tr>
  <tr><td colspan="3">if capabilities @& ::CLIENT_PROTOCOL_41 {</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>warnings</td>
      <td>number of warnings</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>status_flags</td>
      <td>@ref SERVER_STATUS_flags_enum</td></tr>
  </table>

  Example:

  A MySQL 4.1 EOF packet with: 0 warnings, AUTOCOMMIT enabled.

  <table><tr>
  <td>
  ~~~~~~~~~~~~~~~~~~~~~
  05 00 00 05 fe 00 00 02 00
  ~~~~~~~~~~~~~~~~~~~~~
  </td><td>
  ~~~~~~~~~~~~~~~~~~~~~
  ..........
  ~~~~~~~~~~~~~~~~~~~~~
  </td></tr></table>

  @sa net_send_eof().
*/
/* clang-format on */

/**
  Send eof (= end of result set) to the client.

  See @ref page_protocol_basic_eof_packet packet for the structure
  of the packet.

  note
  The warning count will not be sent if 'no_flush' is set as
  we don't want to report the warning count until all data is sent to the
  client.

  @param thd                    Thread handler
  @param server_status          The server status
  @param statement_warn_count   Total number of warnings

  @retval false The message was successfully sent
  @retval true An error occurred and the message wasn't sent properly
*/

static bool net_send_eof(THD *thd, uint server_status,
                         uint statement_warn_count) {
  NET *net = thd->get_protocol_classic()->get_net();
  bool error = false;
  DBUG_TRACE;
  /* Set to true if no active vio, to work well in case of --init-file */
  if (net->vio != nullptr) {
    thd->get_stmt_da()->set_overwrite_status(true);
    error = write_eof_packet(thd, net, server_status, statement_warn_count);
    if (!error) error = net_flush(net);
    thd->get_stmt_da()->set_overwrite_status(false);
    DBUG_PRINT("info", ("EOF sent, so no more error sending allowed"));
  }
  return error;
}

/**
  Format EOF packet according to the current protocol and
  write it to the network output buffer.

  See also @ref page_protocol_basic_err_packet

  @param thd The thread handler
  @param net The network handler
  @param server_status The server status
  @param statement_warn_count The number of warnings


  @retval false The message was sent successfully
  @retval true An error occurred and the messages wasn't sent properly
*/

static bool write_eof_packet(THD *thd, NET *net, uint server_status,
                             uint statement_warn_count) {
  bool error;
  Protocol *protocol = thd->get_protocol();
  if (protocol->has_client_capability(CLIENT_PROTOCOL_41)) {
    uchar buff[5];
    /*
      Don't send warn count during SP execution, as the warn_list
      is cleared between substatements, and mysqltest gets confused
    */
    uint tmp = min(statement_warn_count, 65535U);
    buff[0] = 254;
    int2store(buff + 1, tmp);
    /*
      The following test should never be true, but it's better to do it
      because if 'is_fatal_error' is set the server is not going to execute
      other queries (see the if test in dispatch_command / COM_QUERY)
    */
    if (thd->is_fatal_error()) server_status &= ~SERVER_MORE_RESULTS_EXISTS;
    int2store(buff + 3, server_status);
    error = my_net_write(net, buff, 5);
  } else
    error = my_net_write(net, eof_buff, 1);

  return error;
}

/* clang-format off */
/**
  @page page_protocol_basic_err_packet ERR_Packet

  This packet signals that an error occurred. It contains a SQL state value
  if ::CLIENT_PROTOCOL_41 is enabled.

  Error texts cannot exceed ::MYSQL_ERRMSG_SIZE

  <table>
  <caption>The Payload of an ERR Packet</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
  <td>header</td>
  <td>`0xFF` ERR packet header</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
  <td>error_code</td>
  <td>error-code</td></tr>
  <tr><td colspan="3">if capabilities @& ::CLIENT_PROTOCOL_41 {</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_fix "string[1]"</td>
  <td>sql_state_marker</td>
  <td># marker of the SQL state</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_fix "string[5]"</td>
  <td>sql_state</td>
  <td>SQL state</td></tr>
  <tr><td colspan="3">  }</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"</td>
  <td>error_message</td>
  <td>human readable error message</td></tr>
  </table>

  Example:

  <table><tr>
  <td>
  ~~~~~~~~~~~~~~~~~~~~~
  17 00 00 01 ff 48 04 23    48 59 30 30 30 4e 6f 20
  74 61 62 6c 65 73 20 75    73 65 64
  ~~~~~~~~~~~~~~~~~~~~~
  </td><td>
  ~~~~~~~~~~~~~~~~~~~~~
  .....H.#HY000No
  tables used
  ~~~~~~~~~~~~~~~~~~~~~
  </td></tr></table>

  @sa net_send_error_packet()
*/
/* clang-format on */

/**
  @param thd          Thread handler
  @param sql_errno    The error code to send
  @param err          A pointer to the error message
  @param sqlstate     SQL state

  @retval false The message was successfully sent
  @retval true  An error occurred and the messages wasn't sent properly

  See also @ref page_protocol_basic_err_packet
*/

static bool net_send_error_packet(THD *thd, uint sql_errno, const char *err,
                                  const char *sqlstate) {
  return net_send_error_packet(thd->get_protocol_classic()->get_net(),
                               sql_errno, err, sqlstate,
                               thd->is_bootstrap_system_thread(),
                               thd->get_protocol()->get_client_capabilities(),
                               thd->variables.character_set_results);
}

/**
  @param net                    Low-level NET struct
  @param sql_errno              The error code to send
  @param err                    A pointer to the error message
  @param sqlstate               SQL state
  @param bootstrap              Server is started in bootstrap mode
  @param client_capabilities    Client capabilities flag
  @param character_set_results  Char set info

  @retval false The message was successfully sent
  @retval true  An error occurred and the messages wasn't sent properly

  See also @ref page_protocol_basic_err_packet
*/

static bool net_send_error_packet(NET *net, uint sql_errno, const char *err,
                                  const char *sqlstate, bool bootstrap,
                                  ulong client_capabilities,
                                  const CHARSET_INFO *character_set_results) {
  uint length;
  /*
    buff[]: sql_errno:2 + ('#':1 + SQLSTATE_LENGTH:5) + MYSQL_ERRMSG_SIZE:512
  */
  char buff[2 + 1 + SQLSTATE_LENGTH + MYSQL_ERRMSG_SIZE];

  DBUG_TRACE;

  if (!generate_error_packet(net, sql_errno, err, sqlstate, false, bootstrap,
                             character_set_results, client_capabilities, buff,
                             &length)) {
    return false;
  }

  return net_write_command(net, uchar{255}, pointer_cast<const uchar *>(""), 0,
                           pointer_cast<uchar *>(buff), length);
}

/**
  Faster net_store_length when we know that length is less than 65536.
  We keep a separate version for that range because it's widely used in
  libmysql.

  uint is used as agrument type because of MySQL type conventions:
    - uint for 0..65536
    - ulong for 0..4294967296
    - ulonglong for bigger numbers.
*/

static uchar *net_store_length_fast(uchar *packet, size_t length) {
  if (length < 251) {
    *packet = (uchar)length;
    return packet + 1;
  }
  *packet++ = 252;
  int2store(packet, (uint)length);
  return packet + 2;
}

/****************************************************************************
  Functions used by the protocol functions (like net_send_ok) to store
  strings and numbers in the header result packet.
****************************************************************************/

/* The following will only be used for short strings < 65K */

uchar *net_store_data(uchar *to, const uchar *from, size_t length) {
  DBUG_ASSERT(length <= 65535);
  to = net_store_length_fast(to, length);
  if (length > 0) memcpy(to, from, length);
  return to + length;
}

/*****************************************************************************
  Protocol_classic functions
*****************************************************************************/

void Protocol_classic::init(THD *thd_arg) {
  m_thd = thd_arg;
  packet = &m_thd->packet;
#ifndef DBUG_OFF
  field_types = nullptr;
#endif
}

bool Protocol_classic::store_field(const Field *field) {
  return field->send_to_protocol(this);
}

/**
  A default implementation of "OK" packet response to the client.

  Currently this implementation is re-used by both network-oriented
  protocols -- the binary and text one. They do not differ
  in their OK packet format, which allows for a significant simplification
  on client side.
*/

bool Protocol_classic::send_ok(uint server_status, uint statement_warn_count,
                               ulonglong affected_rows,
                               ulonglong last_insert_id, const char *message) {
  DBUG_TRACE;
  record_checksum();
  const bool retval =
      net_send_ok(m_thd, server_status, statement_warn_count, affected_rows,
                  last_insert_id, message, false);
  // Reclaim some memory
  convert.shrink(m_thd->variables.net_buffer_length);
  checksum = 0;
  should_record_checksum = false;
  return retval;
}

/**
  A default implementation of "EOF" packet response to the client.

  Binary and text protocol do not differ in their EOF packet format.
*/

bool Protocol_classic::send_eof(uint server_status, uint statement_warn_count) {
  DBUG_TRACE;
  bool retval;
  /*
    Normally end of statement reply is signaled by OK packet, but in case
    of binlog dump request an EOF packet is sent instead. Also, old clients
    expect EOF packet instead of OK
  */
  if (has_client_capability(CLIENT_DEPRECATE_EOF) &&
      (m_thd->get_command() != COM_BINLOG_DUMP &&
       m_thd->get_command() != COM_BINLOG_DUMP_GTID)) {
    record_checksum();
    retval = net_send_ok(m_thd, server_status, statement_warn_count, 0, 0,
                         nullptr, true);
  } else
    retval = net_send_eof(m_thd, server_status, statement_warn_count);
  // Reclaim some memory
  convert.shrink(m_thd->variables.net_buffer_length);
  checksum = 0;
  should_record_checksum = false;
  return retval;
}

/**
  A default implementation of "ERROR" packet response to the client.

  Binary and text protocol do not differ in ERROR packet format.
*/

bool Protocol_classic::send_error(uint sql_errno, const char *err_msg,
                                  const char *sql_state) {
  DBUG_TRACE;
  const bool retval =
      net_send_error_packet(m_thd, sql_errno, err_msg, sql_state);
  // Reclaim some memory
  convert.shrink(m_thd->variables.net_buffer_length);
  checksum = 0;
  should_record_checksum = false;
  return retval;
}

void Protocol_classic::gen_conn_timeout_err(char *msg_buf) {
  /* The error code must be 2006 to ensure the compatiblity
     with client error 2006 MySQL server has gone away so
     the actual error code ER_CONNECTION_TIMEOUT will be
     ignored and only the error message will be used.
  */
  uint sql_errno = CR_SERVER_GONE_ERROR;
  char err[128];
  sprintf(err, ER_THD(m_thd, ER_CONNECTION_TIMEOUT),
          m_thd->variables.net_wait_timeout);
  // The default value is "HY000"
  const char *sql_state = mysql_errno_to_sqlstate(sql_errno);
  uint length;
  char buff[2 + 1 + SQLSTATE_LENGTH + MYSQL_ERRMSG_SIZE];
  /* Always add # to make the protocol backward compatible */
  if (generate_error_packet(&m_thd->net, sql_errno, err, sql_state, true, false,
                            global_system_variables.character_set_results,
                            get_client_capabilities(), buff, &length)) {
    memcpy(msg_buf, buff, length);
    msg_buf[length] = '\0';
  } else {
    msg_buf[0] = '\0';
  }
}

void Protocol_classic::set_read_timeout(ulong read_timeout) {
  my_net_set_read_timeout(&m_thd->net, timeout_from_seconds(read_timeout));
}

void Protocol_classic::set_write_timeout(ulong write_timeout) {
  my_net_set_write_timeout(&m_thd->net, timeout_from_seconds(write_timeout));
}

// NET interaction functions
bool Protocol_classic::init_net(Vio *vio) {
  return my_net_init(&m_thd->net, vio);
}

void Protocol_classic::claim_memory_ownership() {
  net_claim_memory_ownership(&m_thd->net);
}

void Protocol_classic::end_net() {
  DBUG_ASSERT(m_thd->net.buff);
  net_end(&m_thd->net);
  m_thd->net.vio = nullptr;
}

bool Protocol_classic::write(const uchar *ptr, size_t len) {
  return my_net_write(&m_thd->net, ptr, len);
}

uchar Protocol_classic::get_error() { return m_thd->net.error; }

void Protocol_classic::wipe_net() {
  memset(&m_thd->net, 0, sizeof(m_thd->net));
}

void Protocol_classic::set_max_packet_size(ulong max_packet_size) {
  m_thd->net.max_packet_size = max_packet_size;
}

NET *Protocol_classic::get_net() { return &m_thd->net; }

Vio *Protocol_classic::get_vio() { return m_thd->net.vio; }

const Vio *Protocol_classic::get_vio() const { return m_thd->net.vio; }

void Protocol_classic::set_vio(Vio *vio) { m_thd->net.vio = vio; }

void Protocol_classic::set_output_pkt_nr(uint pkt_nr) {
  m_thd->net.pkt_nr = pkt_nr;
}

uint Protocol_classic::get_output_pkt_nr() { return m_thd->net.pkt_nr; }

String *Protocol_classic::get_output_packet() { return &m_thd->packet; }

int Protocol_classic::read_packet() {
  input_packet_length = my_net_read(&m_thd->net);
  if (input_packet_length != packet_error) {
    DBUG_ASSERT(!m_thd->net.error);
    bad_packet = false;
    input_raw_packet = m_thd->net.read_pos;
    return 0;
  }

  bad_packet = true;
  return m_thd->net.error == 3 ? 1 : -1;
}

/* clang-format off */
/**
  @page page_protocol_com_quit COM_QUIT

  Tells the server that the client wants it to close the connection.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x01: COM_QUIT</td></tr>
  </table>

  Server closes the connection or returns @ref page_protocol_basic_err_packet.
*/


/**
  @page page_protocol_com_init_db COM_INIT_DB

  Change the default schema of the connection

  @return
  - @ref page_protocol_basic_ok_packet on success
  - @ref page_protocol_basic_err_packet on error

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x02: COM_INIT_DB</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"</td>
      <td>schema name</td>
      <td>name of the schema to change to</td></tr>
  </table>

  @par Example
  ~~~~~~~~~
  05 00 00 00 02 74 65 73    74                         .....test
  ~~~~~~~~~
*/


/**
  @page page_protocol_com_query COM_QUERY

  Send a @ref page_protocol_command_phase_text based SQL query

  Execution starts immediately.

  @return
  - @subpage page_protocol_com_query_response

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x03: COM_QUERY</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"</td>
      <td>query</td>
      <td>the text of the SQL query to execute</td></tr>
  </table>

  @par Example
  ~~~~~~~~~
  21 00 00 00 03 73 65 6c    65 63 74 20 40 40 76 65    !....select @@ve
  72 73 69 6f 6e 5f 63 6f    6d 6d 65 6e 74 20 6c 69    rsion_comment li
  6d 69 74 20 31                                        mit 1
  ~~~~~~~~~

  @sa Protocol_classic::parse_packet, dispatch_command,
    mysql_parse, alloc_query, THD::set_query
*/


/**
  @page page_protocol_com_query_response COM_QUERY Response

  The query response packet is a meta packet which can be one of:

  - @ref page_protocol_basic_err_packet
  - @ref page_protocol_basic_ok_packet
  - @subpage page_protocol_com_query_response_local_infile_request
  - @subpage page_protocol_com_query_response_text_resultset

  @startuml
  COM_QUERY --> COM_QUERY_RESPONSE

  COM_QUERY_RESPONSE --> TextResultSet : length encoded integer

  state "Text Resultset" as TextResultSet {
  FIELD_COUNT --> FIELD
  FIELD --> FIELD
  FIELD --> EOF
  EOF --> ROW
  ROW --> ROW
  }

  TextResultSet --> FinalEOF
  TextResultSet --> Error

  state "Final EOF" as FinalEOF

  COM_QUERY_RESPONSE --> Error : 0xFF
  COM_QUERY_RESPONSE --> OK : 0x00
  COM_QUERY_RESPONSE --> MoreData : 0xFB

  state "More Data" as MoreData {
    GET_MORE_DATA --> SEND_MORE_DATA
  }

  MoreData --> Error
  MoreData --> OK
  @enduml

  @note if ::CLIENT_DEPRECATE_EOF is on,
  @ref page_protocol_basic_ok_packet is sent instead of an actual
  @ref page_protocol_basic_eof_packet packet.

  @sa cli_read_query_result, mysql_send_query, mysql_execute_command
*/


/**
  @page page_protocol_com_query_response_local_infile_request LOCAL INFILE Request

  If the client wants to `LOAD DATA` from a `LOCAL` file into the server it sends:

  ~~~~~
  LOAD DATA LOCAL INFILE '<filename>' INTO TABLE <table>;
  ~~~~~

  The `LOCAL` keyword triggers the server to set a `LOCAL INFILE` request packet
  which asks the client to send the file via a
  @subpage page_protocol_com_query_response_local_infile_data response

  @startuml
  Client -> Server: COM_QUERY
  Server -> Client: 0xFB + filename
  Client -> Server: content of filename
  Client -> Server: empty packet
  Server -> Client: OK
  @enduml

  The client has to send the ::CLIENT_LOCAL_FILES capability flag.

  @return @ref page_protocol_com_query_response_local_infile_data

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>packet type</td>
      <td>0xFB: LOCAL INFILE</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"</td>
      <td>filename</td>
      <td>the path to the file the client shall send</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  0c 00 00 01 fb 2f 65 74    63 2f 70 61 73 73 77 64    ...../etc/passwd
  ~~~~~~~~

  @sa handle_local_infile, mysql_set_local_infile_handler, net_request_file,
  Sql_cmd_load_table::execute_inner
*/


/**
  @page page_protocol_com_query_response_local_infile_data LOCAL INFILE Data

  If the client has data to send, it sends in one or more non-empty packets AS IS
  followed by a empty packet.

  If the file is empty or there is a error while reading the file only the empty
  packet is sent.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"</td>
      <td>file content</td>
      <td>raw file data</td></tr>
  </table>

  @sa handle_local_infile, mysql_set_local_infile_handler, net_request_file,
  Sql_cmd_load_table::execute_inner
*/


/**
  @page page_protocol_com_query_response_text_resultset Text Resultset

  A Text Resultset is a possible @ref page_protocol_com_query_response.

  It is made up of 2 parts:
    - the column definitions (a.k.a. the metadata)
    - the actual rows

  The column definitions part starts with a packet containing the column-count,
  followed by as many
  @subpage page_protocol_com_query_response_text_resultset_column_definition
  packets as there are columns and terminated by a
  @ref page_protocol_basic_eof_packet if the ::CLIENT_DEPRECATE_EOF is not set.

  Each row is a packet, too. The rows are terminated by another
  @ref page_protocol_basic_eof_packet. In case the query could generate the
  @ref page_protocol_com_query_response_text_resultset_column_definition set,
  but generating the rows afterwards failed, a @ref page_protocol_basic_err_packet
  may be sent instead of the last @ref page_protocol_basic_eof_packet.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td colspan="3">if capabilities @& ::CLIENT_OPTIONAL_RESULTSET_METADATA {</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
  <td>metadata_follows</td>
  <td>Flag specifying if metadata are skipped or not. See @ref enum_resultset_metadata</td></tr>
  <tr><td colspan="3">}</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
      <td>column_count</td>
      <td>Number of @ref page_protocol_com_query_response_text_resultset_column_definition to follow</td></tr>
  <tr><td colspan="3">if (not (capabilities @& ::CLIENT_OPTIONAL_RESULTSET_METADATA))
                          or `metadata_follows` == ::RESULTSET_METADATA_FULL {</td></tr>
  <tr><td>`column_count` x @ref page_protocol_com_query_response_text_resultset_column_definition</td>
      <td>Field metadata</td>
      <td>one @ref page_protocol_com_query_response_text_resultset_column_definition for each field up to `column_count`</td></tr>
  <tr><td colspan="3">}</td></tr>
  <tr><td colspan="3">if (not capabilities @& ::CLIENT_DEPRECATE_EOF) {</td></tr>
  <tr><td>@ref page_protocol_basic_eof_packet</td>
  <td>End of metadata</td>
  <td>Marker to set the end of metadata</td></tr>
  <tr><td colspan="3">}</td></tr>
  <tr><td>One or more @subpage page_protocol_com_query_response_text_resultset_row</td>
  <td>The row data</td>
  <td>each @ref page_protocol_com_query_response_text_resultset_row contains `column_count` values</td></tr>
  <tr><td colspan="3">if (error processing) {</td></tr>
  <tr><td>@ref page_protocol_basic_err_packet</td>
  <td>terminator</td>
  <td>Error details</td></tr>
  <tr><td colspan="3">} else if capabilities @& ::CLIENT_DEPRECATE_EOF {</td></tr>
  <tr><td>@ref page_protocol_basic_ok_packet</td>
  <td>terminator</td>
  <td>All the execution details</td></tr>
  <tr><td colspan="3">} else {</td></tr>
  <tr><td>@ref page_protocol_basic_eof_packet</td>
  <td>terminator</td>
  <td>end of resultset marker</td></tr>
  <tr><td colspan="3">}</td></tr>
  </table>

  If the ::SERVER_MORE_RESULTS_EXISTS flag is set in the last
  @ref page_protocol_basic_eof_packet/@ref page_protocol_basic_ok_packet,
  another @ref page_protocol_com_query_response_text_resultset will follow.
  See Multi-resultset.

  @todo Fill in the link for the Multi-resultset.

  @startuml
  :column count;
  repeat
    while (column available ?)
      :column definition;
    endwhile
    :EOF;
    while (row available?)
      :row;
    endwhile
    if (error?) then (yes)
      :ERR;
    else (no)
      :EOF;
    endif
  repeat while (SERVER_MORE_RESULTS_EXISTS?)
  end
  @enduml

  @note if ::CLIENT_OPTIONAL_RESULTSET_METADATA is on,
  there might be no column descriptions sent and one extra flag
  is sent before the column counts.

  @sa Protocol_classic::send_field_metadata, THD::send_result_metadata
*/

/**
  @page page_protocol_com_query_response_text_resultset_row Text Resultset Row

  ProtocolText::ResultsetRow:

  A row with data for each column.
  - NULL is sent as `0xFB`
  - everything else is converted to a string and is sent as
    @ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"

  @sa THD::send_result_set_row, Protocol_text
*/

/**
  @page page_protocol_com_field_list COM_FIELD_LIST

  @note As of MySQL 5.7.11, COM_FIELD_LIST is deprecated and will be removed in
  a future version of MySQL. Instead, use COM_QUERY to execute a SHOW COLUMNS
  statement.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x04: COM_FIELD_LIST</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_null "string&lt;NUL&gt;"</td>
      <td>table</td>
      <td>the name of the table to return column information for
      (in the current database for the connection)</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"</td>
      <td>wildcard</td>
      <td>field wildcard</td></tr>
  </table>

  @return @ref sect_protocol_com_field_list_response

  @sa mysql_list_fields, mysqld_list_fields

  @section sect_protocol_com_field_list_response COM_FIELD_LIST Response

  The response to @ref page_protocol_com_field_list can be one of:
   - @ref page_protocol_basic_err_packet
   - zero or more
     @ref page_protocol_com_query_response_text_resultset_column_definition
   - a closing @ref page_protocol_basic_eof_packet

  @warning if ::CLIENT_OPTIONAL_RESULTSET_METADATA is on and the server side
  variable ::Sys_resultset_metadata is not set to ::RESULTSET_METADATA_FULL
  no rows will be sent, just an empty resultset.

  ~~~~~~~~
  31 00 00 01 03 64 65 66    04 74 65 73 74 09 66 69    1....def.test.fi
  65 6c 64 6c 69 73 74 09    66 69 65 6c 64 6c 69 73    eldlist.fieldlis
  74 02 69 64 02 69 64 0c    3f 00 0b 00 00 00 03 00    t.id.id.?.......
  00 00 00 00 fb 05 00 00    02 fe 00 00 02 00          ..............
  ~~~~~~~~

  @sa mysql_list_fields, mysqld_list_fields, THD::send_result_metadata,
  dispatch_command, cli_list_fields

*/

/**
  @page page_protocol_com_refresh COM_REFRESH

  @warning As of MySQL 5.7.11, COM_REFRESH is deprecated and will be removed
  in a future version of MySQL. Instead, use COM_QUERY to execute a
  FLUSH statement.

  A low-level version of several FLUSH ... and RESET ... statements.

  Calls REFRESH or FLUSH statements.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x07: COM_REFRESH</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>sub_command</td>
      <td>A bitmask of sub-systems to refresh.
      A combination of the first 8 bits of
      @ref group_cs_com_refresh_flags</td></tr>
  </table>

  @return @ref page_protocol_basic_err_packet or
    @ref page_protocol_basic_ok_packet

  @sa dispatch_command, handle_reload_request, mysql_refresh
*/


/**
  @page page_protocol_com_statistics COM_STATISTICS

  Get a human readable string of some internal status vars.
  The statistics are refreshed at the time of executing this command.
  If the returned string is of zero length an error message is returned
  by ::mysql_stat to the client application instead
  of the actual empty statistics string.

  @return elther a @ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x08: COM_STATISTICS</td></tr>
  </table>

  @sa cli_read_statistics, mysql_stat, dispatch_command, calc_sum_of_all_status
*/


/**
  @page page_protocol_com_process_info COM_PROCESS_INFO

  @warning As of 5.7.11 ::COM_PROCESS_INFO is deprecated in favor of ::COM_QUERY
    with SHOW PROCESSLIST

  Get a list of active threads

  @return @ref page_protocol_com_query_response_text_resultset or a
  @ref page_protocol_basic_err_packet

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x0A: COM_PROCESS_INFO</td></tr>
  </table>

  @sa mysql_list_processes, dispatch_command, mysqld_list_processes
*/


/**
  @page page_protocol_com_process_kill COM_PROCESS_KILL

  Ask the server to terminate a connection

  @warning As of MySQL 5.7.11, COM_PROCESS_KILL is deprecated and will be
  removed in a future version of MySQL. Instead, use ::COM_QUERY and
  a KILL command.

  Same as the SQL command `KILL <id>`.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x0C: COM_PROCESS_KILL</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>connection_id</td>
      <td>The connection to kill</td></tr>
  </table>

  @return @ref page_protocol_basic_err_packet or
    @ref page_protocol_basic_ok_packet

  @sa dispatch_command, mysql_kill, sql_kill
*/

/**
  @page page_protocol_com_debug COM_DEBUG

  @brief Dump debug info to server's stdout

  COM_DEBUG triggers a dump on internal debug info to stdout of the mysql-server.

  The ::SUPER_ACL privilege is required for this operation.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x0D: COM_DEBUG</td></tr>
  </table>

  @return @ref page_protocol_basic_err_packet or
    @ref page_protocol_basic_ok_packet

  @sa mysql_dump_debug_info, dispatch_command
*/

/**
  @page page_protocol_com_ping COM_PING

  @brief Check if the server is alive

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x0E: COM_PING</td></tr>
  </table>

  @return @ref page_protocol_basic_ok_packet

  @sa mysql_ping, dispatch_command
*/

/**
  @page page_protocol_com_reset_connection COM_RESET_CONNECTION

  @brief Resets the session state

  A more lightweightt version of ::COM_CHANGE_USER that does
  about the same to clean up the session state, but:
  - it does not re-authenticate (and do the extra client/server
    exchange for that)
  - it does not close the connection

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x1F: COM_RESET_CONNECTION</td></tr>
  </table>

  @return @ref page_protocol_basic_ok_packet

  @sa ::mysql_reset_connection, THD::cleanup_connection,
  ::dispatch_command
*/

/**
  @page page_protocol_com_stmt_prepare COM_STMT_PREPARE

  @brief Creates a prepared statement for the passed query string.

  The server returns a @ref sect_protocol_com_stmt_prepare_response which
  contains a `statement-id` which is ised to identify the prepared statement.

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>command</td>
      <td>0x16: COM_STMT_PREPARE</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_eof "string&lt;EOF&gt;"</td>
      <td>query</td>
      <td>The query to prepare</td></tr>
  </table>

  @par Example
  ~~~~~~~
  1c 00 00 00 16 53 45 4c    45 43 54 20 43 4f 4e 43    .....SELECT CONC
  41 54 28 3f 2c 20 3f 29    20 41 53 20 63 6f 6c 31    AT(?, ?) AS col1
  ~~~~~~~

  @return @ref sect_protocol_com_stmt_prepare_response_ok on success,
     @ref page_protocol_basic_err_packet otherwise

  @sa ::mysqld_stmt_prepare, ::mysql_stmt_precheck, ::Prepared_statement,
  ::mysql_stmt_prepare, ::mysql_stmt_init

  @note As LOAD DATA isn't supported by ::COM_STMT_PREPARE yet, no
  @ref page_protocol_com_query_response_local_infile_request is expected here.
  This is unlike @ref page_protocol_com_query_response.


  @section sect_protocol_com_stmt_prepare_response COM_STMT_PREPARE Response

  If ::COM_STMT_PREPARE succeeded, it sends a
  @ref sect_protocol_com_stmt_prepare_response_ok


  @subsection sect_protocol_com_stmt_prepare_response_ok COM_STMT_PREPARE_OK

  <table>
  <caption>Payload of the first packet</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>status</td>
      <td>0x00: OK: Ignored by ::cli_read_prepare_result</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>statement_id</td>
      <td>statement ID</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>num_columns</td>
      <td>Number of columns</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>num_params</td>
      <td>Number of parameters</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>reserved_1</td>
      <td>[00] filler</td></tr>
  <tr><td colspan="3">if (packet_lenght > 12) {</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>warning_count</td>
      <td>Number of warnings</td></tr>
  <tr><td colspan="3">if capabilities @& ::CLIENT_OPTIONAL_RESULTSET_METADATA {</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>metadata_follows</td>
      <td>Flag specifying if metadata are skipped or not.
      See @ref enum_resultset_metadata</td></tr>
  <tr><td colspan="3">} -- ::CLIENT_OPTIONAL_RESULTSET_METADATA {</td></tr>
  <tr><td colspan="3">} -- packet_lenght > 12</td></tr>
  </table>

  if `num_params` > 0 and ::CLIENT_OPTIONAL_RESULTSET_METADATA is not set or
  if `medatdata_follows` is ::RESULTSET_METADATA_FULL `num_params` packets will
  follow. Then a @ref page_protocol_basic_eof_packet will be transmitted,
  provided that ::CLIENT_DEPRECATE_EOF is not set.

  <table>
  <caption>Parameter definition block</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td colspan="3">`num_params` *
    @ref page_protocol_com_query_response_text_resultset_column_definition</td></tr>
  <tr><td colspan="3">if (not capabilities @& ::CLIENT_DEPRECATE_EOF) {</td></tr>
  <tr><td colspan="3">@ref page_protocol_basic_eof_packet</td></tr>
  <tr><td colspan="3">} --::CLIENT_DEPRECATE_EOF</td></tr>
  </table>

  if `num_columns` > 0 and ::CLIENT_OPTIONAL_RESULTSET_METADATA is not set or
  if `medatdata_follows` is ::RESULTSET_METADATA_FULL `num_columns` packets will
  follow. Then a @ref page_protocol_basic_eof_packet will be transmitted,
  provided that ::CLIENT_DEPRECATE_EOF is not set.

  <table>
  <caption>Column definition block</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td colspan="3">`num_columns` *
  @ref page_protocol_com_query_response_text_resultset_column_definition</td></tr>
  <tr><td colspan="3">if (not capabilities @& ::CLIENT_DEPRECATE_EOF) {</td></tr>
  <tr><td colspan="3">@ref page_protocol_basic_eof_packet</td></tr>
  <tr><td colspan="3">} --::CLIENT_DEPRECATE_EOF</td></tr>
  </table>

  @par Example
  for a prepared query like  SELECT CONCAT(?, ?) AS col1 and no ::CLIENT_OPTIONAL_RESULTSET_METADATA
  ~~~~~~~~~~~
  0c 00 00 01 00 01 00 00    00 01 00 02 00 00 00 00|   ................
  17 00 00 02 03 64 65 66    00 00 00 01 3f 00 0c 3f    .....def....?..?
  00 00 00 00 00 fd 80 00    00 00 00|17 00 00 03 03    ................
  64 65 66 00 00 00 01 3f    00 0c 3f 00 00 00 00 00    def....?..?.....
  fd 80 00 00 00 00|05 00    00 04 fe 00 00 02 00|1a    ................
  00 00 05 03 64 65 66 00    00 00 04 63 6f 6c 31 00    ....def....col1.
  0c 3f 00 00 00 00 00 fd    80 00 1f 00 00|05 00 00    .?..............
  06 fe 00 00 02 00                                     ...
  ~~~~~~~~~~~

  @par Example
  for a a query without parameters and resultset like DO 1 and no ::CLIENT_OPTIONAL_RESULTSET_METADATA :
  ~~~~~~~~~~~
  0c 00 00 01 00 01 00 00    00 00 00 00 00 00 00 00
  ~~~~~~~~~~~

  @sa ::cli_read_prepare_result, mysql_stmt_prepare, ::send_statement,
  THD::send_result_metadata
*/

/**
  @page page_protocol_com_stmt_send_long_data COM_STMT_SEND_LONG_DATA

  @brief Sends the data for a parameter.

  Repeating to send it, appends the data to the parameter.

  No response is sent back to the client

  @startuml
  Client -> Server : COM_STMT_SEND_LONG_DATA
  @enduml

  @return None

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>status</td>
      <td>[0x18] COM_STMT_SEND_LONG_DATA</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;4&gt;"</td>
      <td>statement_id</td>
      <td>ID of the statement</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>param_id</td>
      <td>The parameter to supply data to</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_var "binary&lt;var&gt;"</td>
      <td>data</td>
      <td>The actual payload to send</td></tr>
  </table>

  @note ::COM_STMT_SEND_LONG_DATA has to be sent before ::COM_STMT_EXECUTE
*/

/**
  @page page_protocol_com_stmt_execute COM_STMT_EXECUTE

  ::COM_STMT_EXECUTE asks the server to execute a prepared statement as
  identified by `statement_id`.

  It sends the values for the placeholders of the prepared statement
  (if it contained any) in @ref sect_protocol_binary_resultset_row_value
  form. The type of each parameter is made up of two bytes:
    - the type as in @ref enum_field_types
    - a flag byte which has the highest bit set if the type is unsigned [80]

  The `num_params` used for this packet has to match the `num_params` of the
  @ref sect_protocol_com_stmt_prepare_response_ok of the corresponsing prepared
  statement.

  @return @subpage page_protocol_com_stmt_execute_response

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>status</td>
      <td>[0x17] COM_STMT_EXECUTE</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>statement_id</td>
      <td>ID of the prepared statement to execute</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>flags</td>
      <td>Flags. See ::enum_cursor_type</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>iteration_count</td>
      <td>Number of times to execute the statement. Currently always 1.</td></tr>
  <tr><td colspan="3">if num_params > 0 {</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_var "binary&lt;var&gt;"</td>
      <td>null_bitmap</td>
      <td>NULL bitmap, length= (num_params + 7) / 8</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>new_params_bind_flag</td>
      <td>Flag if parameters must be re-bound</td></tr>
  <tr><td colspan="3">if new_params_bind_flag {</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_var "binary&lt;var&gt;"</td>
      <td>parameter_types</td>
      <td>Type of each parameter, length: num_params * 2</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_var "binary&lt;var&gt;"</td>
      <td>parameter_values</td>
      <td>value of each parameter</td></tr>
  </table>

  @par Example
  ~~~~~~~~~
  12 00 00 00 17 01 00 00    00 00 01 00 00 00 00 01    ................
  0f 00 03 66 6f 6f                                     ...foo
  ~~~~~~~~~

  `null_bitmap` is like the NULL-bitmap for the
  @ref sect_protocol_binary_resultset_row just that it has a bit_offset of 0.

  @sa ::mysql_stmt_execute, ::cli_stmt_execute, ::mysql_stmt_precheck,
  ::mysqld_stmt_execute
*/

/**
  @page page_protocol_com_stmt_execute_response COM_STMT_EXECUTE Response

  Similar to the @ref page_protocol_com_query_response a ::COM_STMT_EXECUTE
  returns either:
    - a @ref page_protocol_basic_ok_packet
    - a @ref page_protocol_basic_err_packet
    - @subpage page_protocol_binary_resultset
*/

/**
  @page page_protocol_binary_resultset Binary Protocol Resultset

  Binary Protocol Resultset is similar to the
  @ref page_protocol_com_query_response_text_resultset.
  It just contains the rows in @ref sect_protocol_binary_resultset_row
  format.

  <table>
  <caption>ProtocolBinary::Resultset:</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
    <td>column_count</td>
    <td>always grater than 0</td></tr>
  <tr><td colspan="3">`column_count` *
    @ref page_protocol_com_query_response_text_resultset_column_definition</td></tr>
  <tr><td colspan="3">None or many
    @ref sect_protocol_binary_resultset_row</td></tr>
  <tr><td colspan="3">@ref page_protocol_basic_eof_packet</td></tr>
  </table>

  @note if ::CLIENT_DEPRECATE_EOF client capability flag is set,
  @ref page_protocol_basic_ok_packet is sent, else
  @ref page_protocol_basic_eof_packet is sent.

  @par Example
  ~~~~~~~
  01 00 00 01 01|1a 00 00    02 03 64 65 66 00 00 00    ..........def...
  04 63 6f 6c 31 00 0c 08    00 06 00 00 00 fd 00 00    .col1...........
  1f 00 00|05 00 00 03 fe    00 00 02 00|09 00 00 04    ................
  00 00 06 66 6f 6f 62 61    72|05 00 00 05 fe 00 00    ...foobar.......
  02 00
  ~~~~~~~


  @section sect_protocol_binary_resultset_row Binary Protocol Resultset Row

  A Binary Protocol Resultset Row is made up of a `NULL bitmap` containing as
  many bits as we have columns in the resultset + 2 and the `values` for
  columns that are not NULL in the @ref sect_protocol_binary_resultset_row_value
  format.

  <table>
  <caption>ProtocolBinary::ResultsetRow:</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>packet_header</td>
      <td>[0x00] packer header</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_var "binary&lt;var&gt;"</td>
    <td>null_bitmap</td>
    <td>NULL bitmap, length= (column_count + 7 + 2) / 8</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_var "binary&lt;var&gt;"</td>
    <td>values</td>
    <td>values for non-null columns</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  09 00 00 04 00 00 06 66 6f 6f 62 61 72
  ~~~~~~~~


  @subsection sect_protocol_binary_resultset_row_null_bitmap NULL-Bitmap

  The binary protocol sends NULL values as bits inside a bitmap instead of a
  full byte as the @ref page_protocol_com_query_response_text_resultset_row
  does. If many NULL values are sent, it is more efficient than the old way.

  @par Caution
  For the @ref sect_protocol_binary_resultset_row the num_fields
  and the field_pos need to add a offset of 2. For ::COM_STMT_EXECUTE
  this offset is 0.

  The NULL-bitmap needs enough space to store a possible NULL bit for each
  column that is sent. Its space is calculated with:
  ~~~~~
  NULL-bitmap-bytes = (num_fields + 7 + offset) / 8
  ~~~~~

  resulting in:

  <table>
  <tr><th>num_fields+offset</th><th>NULL_bitmap bytes</th></tr>
  <tr><td>0</td><td>0</td></tr>
  <tr><td>1</td><td>1</td></tr>
  <tr><td>[...]</td><td>[...]</td></tr>
  <tr><td>8</td><td>1</td></tr>
  <tr><td>9</td><td>2</td></tr>
  <tr><td>[...]</td><td>[...]</td></tr>
  </table>

  To store a NULL bit in the bitmap, you need to calculate the bitmap-byte
  (starting with 0) and the bitpos (starting with 0) in that byte from the
  index_field (starting with 0):

  ~~~~~~~~~~
  NULL-bitmap-byte = ((field-pos + offset) / 8)
  NULL-bitmap-bit  = ((field-pos + offset) % 8)
  ~~~~~~~~~~

  @par Example
  ~~~~~~~~~~
  Resultset Row, 9 fields, 9th field is a NULL (9th field -> field-index == 8, offset == 2)

  nulls -> [00] [00]

  byte_pos = (10 / 8) = 1
  bit_pos  = (10 % 8) = 2

  nulls[byte_pos] |= 1 << bit_pos
  nulls[1] |= 1 << 2;

  nulls -> [00] [04]
  ~~~~~~~~~~

  @section sect_protocol_binary_resultset_row_value Binary Protocol Value

  @subsection sect_protocol_binary_resultset_row_value_string ProtocolBinary::MYSQL_TYPE_STRING, ProtocolBinary::MYSQL_TYPE_VARCHAR, ProtocolBinary::MYSQL_TYPE_VAR_STRING, ProtocolBinary::MYSQL_TYPE_ENUM, ProtocolBinary::MYSQL_TYPE_SET, ProtocolBinary::MYSQL_TYPE_LONG_BLOB, ProtocolBinary::MYSQL_TYPE_MEDIUM_BLOB, ProtocolBinary::MYSQL_TYPE_BLOB, ProtocolBinary::MYSQL_TYPE_TINY_BLOB, ProtocolBinary::MYSQL_TYPE_GEOMETRY, ProtocolBinary::MYSQL_TYPE_BIT, ProtocolBinary::MYSQL_TYPE_DECIMAL, ProtocolBinary::MYSQL_TYPE_NEWDECIMAL:

  <table>
  <caption>::MYSQL_TYPE_STRING</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
    <td>value</td>
    <td>String</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  03 66 6f 6f -- string = "foo"
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_longlong ProtocolBinary::MYSQL_TYPE_LONGLONG

  <table>
  <caption>::MYSQL_TYPE_LONGLONG</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int8 "int&lt;8&gt;"</td>
      <td>value</td>
      <td>integer</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  01 00 00 00 00 00 00 00 -- int64 = 1
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_long ProtocolBinary::MYSQL_TYPE_LONG, ProtocolBinary::MYSQL_TYPE_INT24

  <table>
  <caption>::MYSQL_TYPE_LONG, ::MYSQL_TYPE_INT24</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
    <td>value</td>
    <td>integer</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  01 00 00 00 -- int32 = 1
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_short ProtocolBinary::MYSQL_TYPE_SHORT, ProtocolBinary::MYSQL_TYPE_YEAR

  <table>
  <caption>::MYSQL_TYPE_SHORT, ::MYSQL_TYPE_YEAR</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
    <td>value</td>
    <td>integer</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  01 00 -- int16 = 1
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_tiny ProtocolBinary::MYSQL_TYPE_TINY

  <table>
  <caption>::MYSQL_TYPE_TINY</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
    <td>value</td>
    <td>integer</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  01 -- int8 = 1
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_double ProtocolBinary::MYSQL_TYPE_DOUBLE

  MYSQL_TYPE_DOUBLE stores a floating point in IEEE 754 double precision format.

  First byte is the last byte of the significant as stored in C.

  <table>
  <caption>::MYSQL_TYPE_DOUBLE</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_fix "string[8]"</td>
    <td>value</td>
    <td>a IEEE 754 double precision format (8 bytes) double</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  66 66 66 66 66 66 24 40 -- double = 10.2
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_float ProtocolBinary::MYSQL_TYPE_FLOAT

  MYSQL_TYPE_FLOAT stores a floating point in IEEE 754 single precision format.

  <table>
  <caption>::MYSQL_TYPE_FLOAT</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_fix "string[4]"</td>
    <td>value</td>
    <td>a IEEE 754 single precision format (4 bytes) float</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  33 33 23 41 -- float = 10.2
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_date ProtocolBinary::MYSQL_TYPE_DATE, ProtocolBinary::MYSQL_TYPE_DATETIME, ProtocolBinary::MYSQL_TYPE_TIMESTAMP:

  Type to store a ::MYSQL_TYPE_DATE, ::MYSQL_TYPE_DATETIME and
  ::MYSQL_TYPE_TIMESTAMP fields in the binary protocol.

  To save space the packet can be compressed:
    - if year, month, day, hour, minutes, seconds and microseconds are all 0,
      length is 0 and no other field is sent.
    - if hour, seconds and microseconds are all 0, length is 4 and no other
      field is sent.
    - if microseconds is 0, length is 7 and micro_seconds is not sent.
    - otherwise the length is 11


  <table>
  <caption>::MYSQL_TYPE_DATE, ::MYSQL_TYPE_DATETIME and ::MYSQL_TYPE_TIMESTAMP</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
    <td>length</td>
    <td>number of bytes following (valid values: 0, 4, 7, 11)</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
    <td>year</td>
    <td>year</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
    <td>month</td>
    <td>month</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
    <td>day</td>
    <td>day</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
    <td>hour</td>
    <td>hour</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
    <td>minute</td>
    <td>minute</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
    <td>second</td>
    <td>second</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
    <td>microsecond</td>
    <td>micro seconds</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  0b da 07 0a 11 13 1b 1e 01 00 00 00 -- datetime 2010-10-17 19:27:30.000 001
  04 da 07 0a 11                      -- date = 2010-10-17
  0b da 07 0a 11 13 1b 1e 01 00 00 00 -- timestamp
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_time ProtocolBinary::MYSQL_TYPE_TIME

  Type to store a ::MYSQL_TYPE_TIME field in the binary protocol.

  To save space the packet can be compressed:
  - if day, hour, minutes, seconds and microseconds are all 0,
  length is 0 and no other field is sent.
  - if microseconds is 0, length is 8 and micro_seconds is not sent.
  - otherwise the length is 12


  <table>
  <caption>::MYSQL_TYPE_DATE, ::MYSQL_TYPE_DATETIME and ::MYSQL_TYPE_TIMESTAMP</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
  <td>length</td>
  <td>number of bytes following (valid values: 0, 8, 12)</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
  <td>is_negative</td>
  <td>1 if minus, 0 for plus</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
  <td>days</td>
  <td>days</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
  <td>hour</td>
  <td>hour</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
  <td>minute</td>
  <td>minute</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
  <td>second</td>
  <td>second</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
  <td>microsecond</td>
  <td>micro seconds</td></tr>
  </table>

  @par Example
  ~~~~~~~~
  0c 01 78 00 00 00 13 1b 1e 01 00 00 00 -- time  -120d 19:27:30.000 001
  08 01 78 00 00 00 13 1b 1e             -- time  -120d 19:27:30
  01                                     -- time     0d 00:00:00
  ~~~~~~~~

  @subsection sect_protocol_binary_resultset_row_value_null ProtocolBinary::MYSQL_TYPE_NULL

  stored in the @ref sect_protocol_binary_resultset_row_null_bitmap only
 */

/* clang-format on */

/**
  @page page_protocol_com_stmt_reset COM_STMT_RESET

  ::COM_STMT_RESET resets the data of a prepared statement which was
  accumulated with ::COM_STMT_SEND_LONG_DATA commands and closes the cursor if
  it was opened with ::COM_STMT_EXECUTE.

  The server will send a @ref page_protocol_basic_ok_packet if the
  statement could be reset, a @ref page_protocol_basic_err_packet if not.

  @return @ref page_protocol_basic_ok_packet or a
  @ref page_protocol_basic_err_packet

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>status</td>
      <td>[0x1A] COM_STMT_RESET</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>statement_id</td>
      <td>ID of the prepared statement to reset</td></tr>
  </table>

  @par Example
  ~~~~~~~~~~~~
  05 00 00 00 1a 01 00 00    00                         .........
  ~~~~~~~~~~~~

  @sa ::mysql_stmt_reset, ::mysqld_stmt_reset, ::mysql_stmt_precheck
*/

/**
  @page page_protocol_com_stmt_close COM_STMT_CLOSE

  ::COM_STMT_CLOSE deallocates a prepared statement.

  No response packet is sent back to the client.

  @return None

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>status</td>
      <td>[0x19] COM_STMT_CLOSE</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>statement_id</td>
      <td>ID of the prepared statement to close</td></tr>
  </table>

  @par Example
  ~~~~~~~~~~~~
  05 00 00 00 19 01 00 00    00                         .........
  ~~~~~~~~~~~~

  @sa ::mysql_stmt_close, ::mysql_stmt_prepare,
  ::mysqld_stmt_close, ::mysql_stmt_precheck
*/

/**
  @page page_protocol_com_stmt_fetch COM_STMT_FETCH

  Fetches the requested amount of rows from
  a resultset produced by ::COM_STMT_EXECUTE

  @return @ref sect_protocol_com_stmt_fetch_response
  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>status</td>
      <td>[0x19] COM_STMT_CLOSE</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>statement_id</td>
      <td>ID of the prepared statement to close</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>num_rows</td>
      <td>max number of rows to return</td></tr>
  </table>

  @sa ::mysqld_stmt_fetch, ::mysql_stmt_fetch

  @section sect_protocol_com_stmt_fetch_response COM_STMT_FETCH Response

  ::COM_STMT_FETCH may return one of:
    - @ref sect_protocol_command_phase_sp_multi_resultset
    - @ref page_protocol_basic_err_packet
*/

bool Protocol_classic::parse_packet(union COM_DATA *data,
                                    enum_server_command cmd) {
  DBUG_TRACE;
  switch (cmd) {
    case COM_INIT_DB: {
      data->com_init_db.db_name =
          reinterpret_cast<const char *>(input_raw_packet);
      data->com_init_db.length = input_packet_length;
      break;
    }
    case COM_REFRESH: {
      if (input_packet_length < 1) goto malformed;
      data->com_refresh.options = input_raw_packet[0];
      break;
    }
    case COM_PROCESS_KILL: {
      if (input_packet_length < 4) goto malformed;
      data->com_kill.id = (ulong)uint4korr(input_raw_packet);
      break;
    }
    case COM_SET_OPTION: {
      if (input_packet_length < 2) goto malformed;
      data->com_set_option.opt_command = uint2korr(input_raw_packet);
      break;
    }
    case COM_STMT_EXECUTE: {
      if (input_packet_length < 9) goto malformed;
      uchar *read_pos = input_raw_packet;
      size_t packet_left = input_packet_length;

      // Get the statement id
      data->com_stmt_execute.stmt_id = uint4korr(read_pos);
      read_pos += 4;
      packet_left -= 4;
      // Get execution flags
      data->com_stmt_execute.open_cursor = static_cast<bool>(*read_pos);
      read_pos += 5;
      packet_left -= 5;
      DBUG_PRINT("info", ("stmt %lu", data->com_stmt_execute.stmt_id));
      DBUG_PRINT("info", ("Flags %lu", data->com_stmt_execute.open_cursor));

      // Get the statement by id
      Prepared_statement *stmt =
          m_thd->stmt_map.find(data->com_stmt_execute.stmt_id);
      data->com_stmt_execute.parameter_count = 0;

      /*
        If no statement found there's no need to generate error.
        It will be generated in sql_parse.cc which will check again for the id.
      */
      if (!stmt || stmt->param_count < 1) break;

      uint param_count = stmt->param_count;
      data->com_stmt_execute.parameters =
          static_cast<PS_PARAM *>(m_thd->alloc(param_count * sizeof(PS_PARAM)));
      if (!data->com_stmt_execute.parameters)
        goto malformed; /* purecov: inspected */

      /* Then comes the null bits */
      const uint null_bits_packet_len = (param_count + 7) / 8;
      if (packet_left < null_bits_packet_len) goto malformed;
      unsigned char *null_bits = read_pos;
      read_pos += null_bits_packet_len;
      packet_left -= null_bits_packet_len;

      PS_PARAM *params = data->com_stmt_execute.parameters;

      /* Then comes the types byte. If set, new types are provided */
      if (!packet_left) goto malformed;
      bool has_new_types = static_cast<bool>(*read_pos++);
      --packet_left;
      data->com_stmt_execute.has_new_types = has_new_types;
      if (has_new_types) {
        DBUG_PRINT("info", ("Types provided"));
        for (uint i = 0; i < param_count; ++i) {
          if (packet_left < 2) goto malformed;

          ushort type_code = sint2korr(read_pos);
          read_pos += 2;
          packet_left -= 2;

          const uint signed_bit = 1 << 15;
          params[i].type =
              static_cast<enum enum_field_types>(type_code & ~signed_bit);
          params[i].unsigned_type = static_cast<bool>(type_code & signed_bit);
          DBUG_PRINT("info", ("type=%u", (uint)params[i].type));
          DBUG_PRINT("info", ("flags=%u", (uint)params[i].unsigned_type));
        }
      }
      /*
        No check for packet_left here or in case of only long data
        we will return malformed, although the packet will be correct
      */

      /* Here comes the real data */
      for (uint i = 0; i < param_count; ++i) {
        params[i].null_bit =
            static_cast<bool>(null_bits[i / 8] & (1 << (i & 7)));
        // Check if parameter is null
        if (params[i].null_bit) {
          DBUG_PRINT("info", ("null param"));
          params[i].value = nullptr;
          params[i].length = 0;
          data->com_stmt_execute.parameter_count++;
          continue;
        }
        enum enum_field_types type =
            has_new_types ? params[i].type : stmt->param_array[i]->data_type();
        if (stmt->param_array[i]->state == Item_param::LONG_DATA_VALUE) {
          DBUG_PRINT("info", ("long data"));
          if (!((type >= MYSQL_TYPE_TINY_BLOB) && (type <= MYSQL_TYPE_STRING)))
            goto malformed;
          data->com_stmt_execute.parameter_count++;

          continue;
        }

        bool buffer_underrun = false;
        ulong header_len;

        // Set parameter length.
        params[i].length = get_ps_param_len(type, read_pos, packet_left,
                                            &header_len, &buffer_underrun);
        if (buffer_underrun) goto malformed;

        read_pos += header_len;
        packet_left -= header_len;

        // Set parameter value
        params[i].value = read_pos;
        read_pos += params[i].length;
        packet_left -= params[i].length;
        data->com_stmt_execute.parameter_count++;
        DBUG_PRINT("info", ("param len %ul", (uint)params[i].length));
      }
      DBUG_PRINT("info", ("param count %ul",
                          (uint)data->com_stmt_execute.parameter_count));
      break;
    }
    case COM_STMT_FETCH: {
      if (input_packet_length < 8) goto malformed;
      data->com_stmt_fetch.stmt_id = uint4korr(input_raw_packet);
      data->com_stmt_fetch.num_rows = uint4korr(input_raw_packet + 4);
      break;
    }
    case COM_STMT_SEND_LONG_DATA: {
      if (input_packet_length < MYSQL_LONG_DATA_HEADER) goto malformed;
      data->com_stmt_send_long_data.stmt_id = uint4korr(input_raw_packet);
      data->com_stmt_send_long_data.param_number =
          uint2korr(input_raw_packet + 4);
      data->com_stmt_send_long_data.longdata = input_raw_packet + 6;
      data->com_stmt_send_long_data.length = input_packet_length - 6;
      break;
    }
    case COM_STMT_PREPARE: {
      data->com_stmt_prepare.query =
          reinterpret_cast<const char *>(input_raw_packet);
      data->com_stmt_prepare.length = input_packet_length;
      break;
    }
    case COM_STMT_CLOSE: {
      if (input_packet_length < 4) goto malformed;

      data->com_stmt_close.stmt_id = uint4korr(input_raw_packet);
      break;
    }
    case COM_STMT_RESET: {
      if (input_packet_length < 4) goto malformed;

      data->com_stmt_reset.stmt_id = uint4korr(input_raw_packet);
      break;
    }
    case COM_QUERY_ATTRS: {
      uchar *pos = input_raw_packet;
      data->com_query.query_attrs_length = net_field_length_ll((uchar **)&pos);
      data->com_query.query_attrs = reinterpret_cast<const char *>(pos);
      pos += data->com_query.query_attrs_length;
      data->com_query.query = reinterpret_cast<const char *>(pos);
      data->com_query.length = input_raw_packet + input_packet_length - pos;
      break;
    }
    case COM_QUERY: {
      data->com_query.query_attrs_length = 0;
      data->com_query.query_attrs = nullptr;
      data->com_query.query = reinterpret_cast<const char *>(input_raw_packet);
      data->com_query.length = input_packet_length;
      break;
    }
    case COM_FIELD_LIST: {
      /*
        We have name + wildcard in packet, separated by endzero
      */
      ulong len = strend((char *)input_raw_packet) - (char *)input_raw_packet;

      if (len >= input_packet_length || len > NAME_LEN) goto malformed;

      data->com_field_list.table_name = input_raw_packet;
      data->com_field_list.table_name_length = len;

      data->com_field_list.query = input_raw_packet + len + 1;
      data->com_field_list.query_length = input_packet_length - len;
      break;
    }
    default:
      break;
  }

  return false;

malformed:
  my_error(ER_MALFORMED_PACKET, MYF(0));
  bad_packet = true;
  return true;
}

bool Protocol_classic::create_command(COM_DATA *com_data,
                                      enum_server_command cmd, uchar *pkt,
                                      size_t length) {
  input_raw_packet = pkt;
  input_packet_length = length;

  return parse_packet(com_data, cmd);
}

int Protocol_classic::get_command(COM_DATA *com_data,
                                  enum_server_command *cmd) {
  // read packet from the network
  if (int rc = read_packet()) return rc;

  /*
    'input_packet_length' contains length of data, as it was stored in packet
    header. In case of malformed header, my_net_read returns zero.
    If input_packet_length is not zero, my_net_read ensures that the returned
    number of bytes was actually read from network.
    There is also an extra safety measure in my_net_read:
    it sets packet[input_packet_length]= 0, but only for non-zero packets.
  */
  if (input_packet_length == 0) /* safety */
  {
    /* Initialize with COM_SLEEP packet */
    input_raw_packet[0] = (uchar)COM_SLEEP;
    input_packet_length = 1;
  }
  /* Do not rely on my_net_read, extra safety against programming errors. */
  input_raw_packet[input_packet_length] = '\0'; /* safety */

  *cmd = (enum enum_server_command)(uchar)input_raw_packet[0];

  if (*cmd >= COM_END && *cmd <= COM_TOP_BEGIN)
    *cmd = COM_END;  // Wrong command

  DBUG_ASSERT(input_packet_length);
  // Skip 'command'
  input_packet_length--;
  input_raw_packet++;

  int rc = parse_packet(com_data, *cmd);
  // Here we pretend that we just received a COM_QUERY requested. This will
  // make tracking in perfschema easier, since it won't split our workload
  // according to whether query attributes were sent or not. Ideally, if this
  // were upstreamed, we would be able to change the COM_QUERY packet
  // structure via client capabilities.
  if (*cmd == COM_QUERY_ATTRS) {
    *cmd = COM_QUERY;
  }
  return rc;
}

uint Protocol_classic::get_rw_status() { return m_thd->net.reading_or_writing; }

/**
  Finish the result set with EOF packet, as is expected by the client,
  if there is an error evaluating the next row and a continue handler
  for the error.
*/

void Protocol_classic::end_partial_result_set() {
  net_send_eof(m_thd, m_thd->server_status,
               0 /* no warnings, we're inside SP */);
}

bool Protocol_classic::flush() { return net_flush(&m_thd->net); }

bool Protocol_classic::store_ps_status(ulong stmt_id, uint column_count,
                                       uint param_count, ulong cond_count) {
  DBUG_TRACE;

  uchar buff[13];
  buff[0] = 0; /* OK packet indicator */
  int4store(buff + 1, stmt_id);
  int2store(buff + 5, column_count);
  int2store(buff + 7, param_count);
  buff[9] = 0;  // Guard against a 4.1 client
  uint16 tmp =
      min(static_cast<uint16>(cond_count), std::numeric_limits<uint16>::max());
  int2store(buff + 10, tmp);
  if (has_client_capability(CLIENT_OPTIONAL_RESULTSET_METADATA)) {
    /* Store resultset metadata flag. */
    buff[12] = static_cast<uchar>(m_thd->variables.resultset_metadata);

    return my_net_write(&m_thd->net, buff, sizeof(buff));
  }
  return my_net_write(&m_thd->net, buff, sizeof(buff) - 1);
}

bool Protocol_classic::get_compression() { return m_thd->net.compress; }

char *Protocol_classic::get_compression_algorithm() {
  if (get_compression()) {
    NET_SERVER *ext = static_cast<NET_SERVER *>(m_thd->net.extension);
    return ext->compression.compress_algorithm;
  }
  return nullptr;
}

uint Protocol_classic::get_compression_level() {
  if (get_compression()) {
    NET_SERVER *ext = static_cast<NET_SERVER *>(m_thd->net.extension);
    return ext->compression.compress_level;
  }
  return 0;
}

bool Protocol_classic::start_result_metadata(uint num_cols_arg, uint flags,
                                             const CHARSET_INFO *cs) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("num_cols %u, flags %u", num_cols_arg, flags));
  uint num_cols = num_cols_arg;
  result_cs = cs;
  send_metadata = true;
  field_count = num_cols;
  sending_flags = flags;

  should_record_checksum = false;
  for (const auto &p : m_thd->query_attrs_list) {
    if (enable_resultset_checksum && p.first == CHECKSUM) {
      should_record_checksum = true;
      checksum = 0;
      break;
    }
  }

  DBUG_EXECUTE_IF("send_large_column_count_in_metadata", num_cols = 50397184;);
  /*
    We don't send number of column for PS, as it's sent in a preceding packet.
  */
  if (flags & Protocol::SEND_NUM_ROWS) {
    uchar tmp[sizeof(ulonglong) + 1];
    uchar *pos = net_store_length((uchar *)&tmp, num_cols);

    if (has_client_capability(CLIENT_OPTIONAL_RESULTSET_METADATA)) {
      /* Store resultset metadata flag. */
      *pos = static_cast<uchar>(m_thd->variables.resultset_metadata);
      pos++;
    }

    my_net_write(&m_thd->net, (uchar *)&tmp, (size_t)(pos - (uchar *)&tmp));
  }
  DBUG_EXECUTE_IF("send_large_column_count_in_metadata",
                  num_cols = num_cols_arg;);
#ifndef DBUG_OFF
  /*
    field_types will be filled only if we send metadata.
    Set it to NULL if we skip resultset metadata to avoid
    ::storeXXX() method's asserts failures.
  */
  if (m_thd->variables.resultset_metadata == RESULTSET_METADATA_FULL)
    field_types =
        (enum_field_types *)m_thd->alloc(sizeof(field_types) * num_cols);
  else
    field_types = nullptr;
  count = 0;
#endif

  return false;
}

bool Protocol_classic::end_result_metadata() {
  DBUG_TRACE;
  DBUG_PRINT("info", ("num_cols %u, flags %u", field_count, sending_flags));
  send_metadata = false;
  if (sending_flags & SEND_EOF) {
    /* if it is new client do not send EOF packet */
    if (!(has_client_capability(CLIENT_DEPRECATE_EOF))) {
      /*
        Mark the end of meta-data result set, and store m_thd->server_status,
        to show that there is no cursor.
        Send no warning information, as it will be sent at statement end.
      */
      if (write_eof_packet(
              m_thd, &m_thd->net, m_thd->server_status,
              m_thd->get_stmt_da()->current_statement_cond_count())) {
        return true;
      }
    }
  }
  return false;
}

/* clang-format off */
/**
  @page page_protocol_com_query_response_text_resultset_column_definition Column Definition

  if ::CLIENT_PROTOCOL_41 is set @ref sect_protocol_com_query_response_text_resultset_column_definition_41
  is used, @ref sec_protocol_com_query_response_text_resultset_column_definition_320

  @section sect_protocol_com_query_response_text_resultset_column_definition_41 Protocol::ColumnDefinition41:

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>catalog</td>
      <td>The catalog used. Currently always "def"</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>schema</td>
      <td>schema name</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>table</td>
      <td>virtual table name</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>org_table</td>
      <td>physical table name</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>name</td>
      <td>virtual column name</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>org_name</td>
      <td>physical column name</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
      <td>length of fixed length fields</td>
      <td>[0x0c]</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>character_set</td>
      <td>the column character set as defined in @ref page_protocol_basic_character_set</td></tr>
  <tr><td>@ref a_protocol_type_int4 "int&lt;4&gt;"</td>
      <td>column_length</td>
      <td>maximum length of the field</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>type</td>
      <td>type of the column as defined in ::enum_field_types</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;2&gt;"</td>
      <td>flags</td>
      <td>Flags as defined in @ref group_cs_column_definition_flags</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>decimals</td>
      <td>max shown decimal digits:
        <ul>
        <li>0x00 for integers and static strings</li>
        <li>0x1f for dynamic strings, double, float</li>
        <li>0x00 to 0x51 for decimals</li>
        </ul></td></tr>
  </table>

  @note `decimals` and `column_length` can be used for text output formatting


  @section sec_protocol_com_query_response_text_resultset_column_definition_320 Protocol::ColumnDefinition320:

  <table>
  <caption>Payload</caption>
  <tr><th>Type</th><th>Name</th><th>Description</th></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>table</td>
      <td>Table name</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>name</td>
      <td>Column name</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
      <td>lenth of type field</td>
      <td>[01]</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>type</td>
      <td>type of the column as defined in ::enum_field_types</td></tr>
  <tr><td colspan="3">if capabilities @& ::CLIENT_LONG_FLAG {</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
      <td>length of flags + decimals fields</td>
      <td>[03]</td></tr>
  <tr><td>@ref a_protocol_type_int2 "int&lt;2&gt;"</td>
      <td>flags</td>
      <td>Flags as defined in @ref group_cs_column_definition_flags</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>decimals</td>
      <td>number of decimal digits</td></tr>
  <tr><td colspan="3">} else {</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
      <td>length of flags + decimals fields</td>
      <td>[02]</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;2&gt;"</td>
      <td>flags</td>
      <td>Flags as defined in @ref group_cs_column_definition_flags</td></tr>
  <tr><td>@ref a_protocol_type_int1 "int&lt;1&gt;"</td>
      <td>decimals</td>
      <td>number of decimal digits</td></tr>
  <tr><td colspan="3">}</td></tr>
  <tr><td colspan="3">if command was COM_FIELD_LIST {</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_int_le "int&lt;lenenc&gt;"</td>
      <td>length of default values</td>
      <td>[02]</td></tr>
  <tr><td>@ref sect_protocol_basic_dt_string_le "string&lt;lenenc&gt;"</td>
      <td>default_values</td>
      <td></td></tr>
  <tr><td colspan="3">}</td></tr>
  </table>

  @sa Protocol_classic::send_field_metadata
*/
/* clang-format on */

/**
  Store schema object names (database, table, and column names)
  in the result set metadata.

  @param field  Field to be sent.
  @retval false success
  @retval true  error
*/
bool Protocol_classic::store_result_set_metadata_object_names(
    Send_field *field) {
  bool res = false;
  const char **metadata;
  const CHARSET_INFO *cs = system_charset_info;

  /* Database, table, and column names are included by default. */
  const char *standard_metadata[] = {field->db_name, field->table_name,
                                     field->org_table_name, field->col_name,
                                     field->org_col_name};

  /* Minimal metadata includes only the column name. */
  const char *minimal_metadata[] = {empty_c_string, empty_c_string,
                                    empty_c_string, field->col_name,
                                    empty_c_string};

  /*
    Use a minimal set of object (database, table, and column) names
    in the result set metadata if the appropriate protocol mode is
    set. Otherwise, use standard metadata.
  */
  if (m_thd->variables.protocol_mode == PROTO_MODE_MINIMAL_OBJECT_NAMES_IN_RSMD)
    metadata = minimal_metadata;
  else
    metadata = standard_metadata;

  res |= store_string(STRING_WITH_LEN("def"), cs);

  for (uint i = 0; i < array_elements(standard_metadata); i++)
    res |= store_string(metadata[i], strlen(metadata[i]), cs);

  return res;
}

/**
  Sends a single column metadata

  @param field Field description
  @param item_charset Character set to use
  @retval false success
  @retval true  error

  See @ref page_protocol_com_query_response_text_resultset_column_definition for
  the format
*/

bool Protocol_classic::send_field_metadata(Send_field *field,
                                           const CHARSET_INFO *item_charset) {
  DBUG_TRACE;
  char *pos;
  const CHARSET_INFO *cs = system_charset_info;
  const CHARSET_INFO *thd_charset = m_thd->variables.character_set_results;

  /* Keep things compatible for old clients */
  if (field->type == MYSQL_TYPE_VARCHAR) field->type = MYSQL_TYPE_VAR_STRING;

  send_metadata = true;
  if (has_client_capability(CLIENT_PROTOCOL_41)) {
    if (store_result_set_metadata_object_names(field) ||
        packet->mem_realloc(packet->length() + 12)) {
      send_metadata = false;
      return true;
    }
    /* Store fixed length fields */
    pos = packet->ptr() + packet->length();
    *pos++ = 12;  // Length of packed fields
    /* inject a NULL to test the client */
    DBUG_EXECUTE_IF("poison_rs_fields", pos[-1] = (char)0xfb;);
    if (item_charset == &my_charset_bin || thd_charset == nullptr) {
      /* No conversion */
      int2store(pos, item_charset->number);
      int4store(pos + 2, field->length);
    } else {
      /* With conversion */
      uint32 field_length, max_length;
      int2store(pos, thd_charset->number);
      /*
        For TEXT/BLOB columns, field_length describes the maximum data
        length in bytes. There is no limit to the number of characters
        that a TEXT column can store, as long as the data fits into
        the designated space.
        For the rest of textual columns, field_length is evaluated as
        char_count * mbmaxlen, where character count is taken from the
        definition of the column. In other words, the maximum number
        of characters here is limited by the column definition.

        When one has a LONG TEXT column with a single-byte
        character set, and the connection character set is multi-byte, the
        client may get fields longer than UINT_MAX32, due to
        <character set column> -> <character set connection> conversion.
        In that case column max length does not fit into the 4 bytes
        reserved for it in the protocol.
      */
      max_length = (field->type >= MYSQL_TYPE_TINY_BLOB &&
                    field->type <= MYSQL_TYPE_BLOB)
                       ? field->length / item_charset->mbminlen
                       : field->length / item_charset->mbmaxlen;
      field_length =
          char_to_byte_length_safe(max_length, thd_charset->mbmaxlen);
      int4store(pos + 2, field_length);
    }
    pos[6] = field->type;
    int2store(pos + 7, field->flags);
    pos[9] = (char)field->decimals;
    pos[10] = 0;  // For the future
    pos[11] = 0;  // For the future
    pos += 12;
  } else {
    if (store_string(field->table_name, strlen(field->table_name), cs) ||
        store_string(field->col_name, strlen(field->col_name), cs) ||
        packet->mem_realloc(packet->length() + 10)) {
      send_metadata = false;
      return true;
    }
    pos = packet->ptr() + packet->length();
    pos[0] = 3;
    int3store(pos + 1, field->length);
    pos[4] = 1;
    pos[5] = field->type;
    pos[6] = 3;
    int2store(pos + 7, field->flags);
    pos[9] = (char)field->decimals;
    pos += 10;
  }
  packet->length((uint)(pos - packet->ptr()));

#ifndef DBUG_OFF
  field_types[count++] = field->type;
#endif
  return false;
}

bool Protocol_classic::end_row() {
  DBUG_TRACE;
  // Only checksum the data rows
  if (should_record_checksum)
    checksum = crc32(checksum, (uchar *)packet->ptr(), packet->length());

  return my_net_write(&m_thd->net, pointer_cast<uchar *>(packet->ptr()),
                      packet->length());
}

/**
  Send a set of strings as one long string with ',' in between.
*/

bool store(Protocol *prot, I_List<i_string> *str_list) {
  char buf[256];
  String tmp(buf, sizeof(buf), &my_charset_bin);
  size_t len;
  I_List_iterator<i_string> it(*str_list);
  i_string *s;

  tmp.length(0);
  while ((s = it++)) {
    tmp.append(s->ptr);
    tmp.append(',');
  }
  if ((len = tmp.length())) len--;  // Remove last ','
  return prot->store_string(tmp.ptr(), len, tmp.charset());
}

/****************************************************************************
  Functions to handle the simple (default) protocol where everything is
  This protocol is the one that is used by default between the MySQL server
  and client when you are not using prepared statements.

  All data are sent as 'packed-string-length' followed by 'string-data'
****************************************************************************/

bool Protocol_classic::connection_alive() const {
  return m_thd->net.vio != nullptr;
}

void Protocol_classic::record_checksum() {
  if (should_record_checksum) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lu", checksum);
    auto &tracker = m_thd->session_tracker;
    auto sess_tracker = tracker.get_tracker(SESSION_RESP_ATTR_TRACKER);
    if (sess_tracker->is_enabled()) {
      LEX_CSTRING key = {CHECKSUM, strlen(CHECKSUM)};
      LEX_CSTRING value = {buf, strlen(buf)};
      sess_tracker->mark_as_changed(m_thd, &key, &value);
    }
  }
}

void Protocol_text::start_row() {
  field_pos = 0;
  packet->length(0);
}

bool Protocol_text::store_null() {
  field_pos++;
  char buff[1];
  buff[0] = (char)251;
  return packet->append(buff, sizeof(buff), PACKET_BUFFER_EXTRA_ALLOC);
}

int Protocol_classic::shutdown(bool) {
  return m_thd->net.vio ? vio_shutdown(m_thd->net.vio) : 0;
}

bool Protocol_classic::store_string(const char *from, size_t length,
                                    const CHARSET_INFO *fromcs) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_DECIMAL ||
              field_types[field_pos] == MYSQL_TYPE_BIT ||
              field_types[field_pos] == MYSQL_TYPE_NEWDECIMAL ||
              field_types[field_pos] == MYSQL_TYPE_NEWDATE ||
              field_types[field_pos] == MYSQL_TYPE_JSON ||
              (field_types[field_pos] >= MYSQL_TYPE_ENUM &&
               field_types[field_pos] <= MYSQL_TYPE_GEOMETRY));
  field_pos++;
  // result_cs is nullptr when client issues SET character_set_results=NULL
  if (result_cs != nullptr && !my_charset_same(fromcs, result_cs) &&
      fromcs != &my_charset_bin && result_cs != &my_charset_bin) {
    // Store with conversion.
    return net_store_data_with_conversion(pointer_cast<const uchar *>(from),
                                          length, fromcs, result_cs);
  }
  // Store without conversion.
  return net_store_data(pointer_cast<const uchar *>(from), length, packet);
}

/**
  Stores an integer in the protocol buffer for the text protocol.

  @param value          the integer value to convert to a string
  @param unsigned_flag  true if the integer is unsigned
  @param zerofill       the length up to which the value should be zero-padded
  @param packet         the destination buffer
  @return false on success, true on error
*/
static bool store_integer(int64 value, bool unsigned_flag, uint32 zerofill,
                          String *packet) {
  if (zerofill != 0) {
    char buff[MY_INT64_NUM_DECIMAL_DIGITS + 1];
    const char *end = longlong10_to_str(value, buff, unsigned_flag ? 10 : -10);
    const size_t int_length = end - buff;
    return net_store_zero_padded_data(buff, int_length, zerofill, packet);
  }

  // Make sure the packet has space for a length byte, the digits and a
  // terminating zero character.
  char *pos = packet->prep_append(MY_INT64_NUM_DECIMAL_DIGITS + 2,
                                  PACKET_BUFFER_EXTRA_ALLOC);
  if (pos == nullptr) return true;
  const char *end = longlong10_to_str(value, pos + 1, unsigned_flag ? 10 : -10);
  *pos = end - (pos + 1);  // Set the length byte.
  packet->length(end - packet->ptr());
  return false;
}

bool Protocol_text::store_tiny(longlong from, uint32 zerofill) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_TINY);
  field_pos++;
  return store_integer(from, false, zerofill, packet);
}

bool Protocol_text::store_short(longlong from, uint32 zerofill) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_YEAR ||
              field_types[field_pos] == MYSQL_TYPE_SHORT);
  field_pos++;
  return store_integer(from, false, zerofill, packet);
}

bool Protocol_text::store_long(longlong from, uint32 zerofill) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_INT24 ||
              field_types[field_pos] == MYSQL_TYPE_LONG);
  field_pos++;
  return store_integer(from, false, zerofill, packet);
}

bool Protocol_text::store_longlong(longlong from, bool unsigned_flag,
                                   uint32 zerofill) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_LONGLONG);
  field_pos++;
  return store_integer(from, unsigned_flag, zerofill, packet);
}

bool Protocol_text::store_decimal(const my_decimal *d, uint prec, uint dec) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_NEWDECIMAL);
  field_pos++;

  // Lengths less than 251 bytes are encoded in a single byte. See
  // net_store_length(). Assert that we can fit all DECIMALs in that space.
  static_assert(DECIMAL_MAX_STR_LENGTH < 251,
                "Length needs more than one byte");

  // Reserve space for the maximum string length of a DECIMAL, plus one byte for
  // the terminating '\0' written by decimal2string(), plus one byte to encode
  // the length of the string.
  char *pos = packet->prep_append(DECIMAL_MAX_STR_LENGTH + 2,
                                  PACKET_BUFFER_EXTRA_ALLOC);
  if (pos == nullptr) return true;

  int string_length = DECIMAL_MAX_STR_LENGTH + 1;
  int error MY_ATTRIBUTE((unused)) =
      decimal2string(d, pos + 1, &string_length, prec, dec);

  // decimal2string() can only fail with E_DEC_TRUNCATED or E_DEC_OVERFLOW.
  // Since it was given a buffer with the maximum length of a DECIMAL,
  // truncation and overflow should never happen.
  DBUG_ASSERT(error == E_DEC_OK);

  // Store the actual length, and update the length of packet.
  *pos = string_length;
  packet->length((pos + 1 + string_length) - packet->ptr());

  return false;
}

/**
  Converts a floating-point value to text for the text protocol.

  @param value          the floating point value
  @param decimals       the precision of the value
  @param gcvt_arg_type  the type of the floating-point value
  @param buffer         a buffer large enough to hold FLOATING_POINT_BUFFER
                        characters plus a terminating zero character
  @return the length of the text representation of the value
*/
static size_t floating_point_to_text(double value, uint32 decimals,
                                     my_gcvt_arg_type gcvt_arg_type,
                                     char *buffer) {
  if (decimals < DECIMAL_NOT_SPECIFIED)
    return my_fcvt(value, decimals, buffer, nullptr);
  return my_gcvt(value, gcvt_arg_type, FLOATING_POINT_BUFFER, buffer, nullptr);
}

/**
  Stores a floating-point value in the text protocol.

  @param value          the floating point value
  @param decimals       the precision of the value
  @param zerofill       the length up to which the value should be zero-padded,
                        or 0 if no zero-padding should be used
  @param gcvt_arg_type  the type of the floating-point value
  @param packet         the destination buffer
  @return false on success, true on error
*/
static bool store_floating_point(double value, uint32 decimals, uint32 zerofill,
                                 my_gcvt_arg_type gcvt_arg_type,
                                 String *packet) {
  char buffer[FLOATING_POINT_BUFFER + 1];
  size_t length =
      floating_point_to_text(value, decimals, gcvt_arg_type, buffer);
  if (zerofill != 0)
    return net_store_zero_padded_data(buffer, length, zerofill, packet);
  return net_store_data(pointer_cast<const uchar *>(buffer), length, packet);
}

bool Protocol_text::store_float(float from, uint32 decimals, uint32 zerofill) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_FLOAT);
  field_pos++;
  return store_floating_point(from, decimals, zerofill, MY_GCVT_ARG_FLOAT,
                              packet);
}

bool Protocol_text::store_double(double from, uint32 decimals,
                                 uint32 zerofill) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_DOUBLE);
  field_pos++;
  return store_floating_point(from, decimals, zerofill, MY_GCVT_ARG_DOUBLE,
                              packet);
}

/**
  Stores a temporal value in the protocol buffer for the text protocol.

  @param to_string the function that converts the temporal value to a string
  @param packet    the destination buffer
  @return false on success, true on error
*/
template <typename ToString>
static bool store_temporal(ToString to_string, String *packet) {
  const size_t packet_length = packet->length();
  // Allocate space for the temporal value, plus one byte for the length.
  char *pos = packet->prep_append(MAX_DATE_STRING_REP_LENGTH + 1,
                                  PACKET_BUFFER_EXTRA_ALLOC);
  if (pos == nullptr) return true;
  const int length = to_string(pos + 1);
  *pos = length;
  packet->length(packet_length + length + 1);
  return false;
}

bool Protocol_text::store_datetime(const MYSQL_TIME &tm, uint decimals) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              is_temporal_type_with_date_and_time(field_types[field_pos]));
  field_pos++;
  return store_temporal(
      [&tm, decimals](char *to) {
        return my_datetime_to_str(tm, to, decimals);
      },
      packet);
}

bool Protocol_text::store_date(const MYSQL_TIME &tm) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_DATE);
  field_pos++;
  return store_temporal([&tm](char *to) { return my_date_to_str(tm, to); },
                        packet);
}

bool Protocol_text::store_time(const MYSQL_TIME &tm, uint decimals) {
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(send_metadata || field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_TIME);
  field_pos++;
  return store_temporal(
      [&tm, decimals](char *to) { return my_time_to_str(tm, to, decimals); },
      packet);
}

/**
  Sends OUT-parameters by writing the values to the protocol.


  @param parameters       List of PS/SP parameters (both input and output).
  @param is_sql_prepare  If it's an sql prepare then
                         text protocol wil be used.

  @return Error status.
    @retval false Success.
    @retval true  Error.
*/
bool Protocol_binary::send_parameters(List<Item_param> *parameters,
                                      bool is_sql_prepare) {
  if (is_sql_prepare)
    return Protocol_text::send_parameters(parameters, is_sql_prepare);

  List_iterator_fast<Item_param> item_param_it(*parameters);

  if (!has_client_capability(CLIENT_PS_MULTI_RESULTS))
    // The client does not support OUT-parameters.
    return false;

  List<Item> out_param_lst;
  Item_param *item_param;
  while ((item_param = item_param_it++)) {
    // Skip it as it's just an IN-parameter.
    if (!item_param->get_out_param_info()) continue;

    if (out_param_lst.push_back(item_param))
      return true; /* purecov: inspected */
  }

  // Empty list
  if (!out_param_lst.elements) return false;

  /*
    We have to set SERVER_PS_OUT_PARAMS in THD::server_status, because it
    is used in send_result_metadata().
  */
  m_thd->server_status |= SERVER_PS_OUT_PARAMS | SERVER_MORE_RESULTS_EXISTS;

  // Send meta-data.
  if (m_thd->send_result_metadata(&out_param_lst,
                                  Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  // Send data.
  start_row();
  if (m_thd->send_result_set_row(&out_param_lst)) return true;
  if (end_row()) return true;

  // Restore THD::server_status.
  m_thd->server_status &= ~SERVER_PS_OUT_PARAMS;
  m_thd->server_status &= ~SERVER_MORE_RESULTS_EXISTS;

  if (has_client_capability(CLIENT_DEPRECATE_EOF))
    return net_send_ok(m_thd,
                       (m_thd->server_status | SERVER_PS_OUT_PARAMS |
                        SERVER_MORE_RESULTS_EXISTS),
                       m_thd->get_stmt_da()->current_statement_cond_count(), 0,
                       0, nullptr, true);
  else
    /*
      In case of old clients send EOF packet.
      @ref page_protocol_basic_eof_packet is deprecated as of MySQL 5.7.5.
    */
    return send_eof(m_thd->server_status, 0);
}

/**
  Sets OUT-parameters to user variables.

  @param parameters  List of PS/SP parameters (both input and output).

  @return Error status.
    @retval false Success.
    @retval true  Error.
*/
bool Protocol_text::send_parameters(List<Item_param> *parameters, bool) {
  List_iterator_fast<Item_param> item_param_it(*parameters);
  List_iterator_fast<LEX_STRING> user_var_name_it(
      m_thd->lex->prepared_stmt_params);

  Item_param *item_param;
  LEX_STRING *user_var_name;
  while ((item_param = item_param_it++) &&
         (user_var_name = user_var_name_it++)) {
    // Skip if it as it's just an IN-parameter.
    if (!item_param->get_out_param_info()) continue;

    Item_func_set_user_var *suv =
        new Item_func_set_user_var(*user_var_name, item_param, false);
    /*
      Item_func_set_user_var is not fixed after construction,
      call fix_fields().
    */
    if (suv->fix_fields(m_thd, nullptr)) return true;

    if (suv->check(false)) return true;

    if (suv->update()) return true;
  }

  return false;
}

/****************************************************************************
  Functions to handle the binary protocol used with prepared statements

  Data format:

    [ok:1]                            reserved ok packet
    [null_field:(field_count+7+2)/8]  reserved to send null data. The size is
                                      calculated using:
                                      bit_fields= (field_count+7+2)/8;
                                      2 bits are reserved for identifying type
                                      of package.
    [[length]data]                    data field (the length applies only for
                                      string/binary/time/timestamp fields and
                                      rest of them are not sent as they have
                                      the default length that client understands
                                      based on the field type
    [..]..[[length]data]              data
****************************************************************************/
bool Protocol_binary::start_result_metadata(uint num_cols, uint flags,
                                            const CHARSET_INFO *result_cs_arg) {
  bit_fields = (num_cols + 9) / 8;
  packet->alloc(bit_fields + 1);
  return Protocol_classic::start_result_metadata(num_cols, flags,
                                                 result_cs_arg);
}

void Protocol_binary::start_row() {
  if (send_metadata) return Protocol_text::start_row();
  packet->length(bit_fields + 1);
  memset(packet->ptr(), 0, 1 + bit_fields);
  field_pos = 0;
}

bool Protocol_binary::store_null() {
  if (send_metadata) return Protocol_text::store_null();
  uint offset = (field_pos + 2) / 8 + 1, bit = (1 << ((field_pos + 2) & 7));
  /* Room for this as it's allocated in prepare_for_send */
  char *to = packet->ptr() + offset;
  *to = (char)((uchar)*to | (uchar)bit);
  field_pos++;
  return false;
}

bool Protocol_binary::store_tiny(longlong from, uint32 zerofill) {
  if (send_metadata) return Protocol_text::store_tiny(from, zerofill);
  char buff[1];
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_TINY);
  field_pos++;
  buff[0] = (uchar)from;
  return packet->append(buff, sizeof(buff), PACKET_BUFFER_EXTRA_ALLOC);
}

bool Protocol_binary::store_short(longlong from, uint32 zerofill) {
  if (send_metadata) return Protocol_text::store_short(from, zerofill);
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_YEAR ||
              field_types[field_pos] == MYSQL_TYPE_SHORT);
  field_pos++;
  char *to = packet->prep_append(2, PACKET_BUFFER_EXTRA_ALLOC);
  if (!to) return true;
  int2store(to, (int)from);
  return false;
}

bool Protocol_binary::store_long(longlong from, uint32 zerofill) {
  if (send_metadata) return Protocol_text::store_long(from, zerofill);
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_INT24 ||
              field_types[field_pos] == MYSQL_TYPE_LONG);
  field_pos++;
  char *to = packet->prep_append(4, PACKET_BUFFER_EXTRA_ALLOC);
  if (!to) return true;
  int4store(to, static_cast<uint32>(from));
  return false;
}

bool Protocol_binary::store_longlong(longlong from, bool unsigned_flag,
                                     uint32 zerofill) {
  if (send_metadata)
    return Protocol_text::store_longlong(from, unsigned_flag, zerofill);
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_LONGLONG);
  field_pos++;
  char *to = packet->prep_append(8, PACKET_BUFFER_EXTRA_ALLOC);
  if (!to) return true;
  int8store(to, from);
  return false;
}

bool Protocol_binary::store_float(float from, uint32 decimals,
                                  uint32 zerofill) {
  if (send_metadata)
    return Protocol_text::store_float(from, decimals, zerofill);
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_FLOAT);
  field_pos++;
  char *to = packet->prep_append(4, PACKET_BUFFER_EXTRA_ALLOC);
  if (!to) return true;
  float4store(to, from);
  return false;
}

bool Protocol_binary::store_double(double from, uint32 decimals,
                                   uint32 zerofill) {
  if (send_metadata)
    return Protocol_text::store_double(from, decimals, zerofill);
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_DOUBLE);
  field_pos++;
  char *to = packet->prep_append(8, PACKET_BUFFER_EXTRA_ALLOC);
  if (!to) return true;
  float8store(to, from);
  return false;
}

bool Protocol_binary::store_datetime(const MYSQL_TIME &tm, uint precision) {
  if (send_metadata) return Protocol_text::store_datetime(tm, precision);

  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              is_temporal_type_with_date_and_time(field_types[field_pos]));
  field_pos++;

  size_t length;
  if (tm.second_part)
    length = 11;
  else if (tm.hour || tm.minute || tm.second)
    length = 7;
  else if (tm.year || tm.month || tm.day)
    length = 4;
  else
    length = 0;

  char *pos = packet->prep_append(length + 1, PACKET_BUFFER_EXTRA_ALLOC);
  if (pos == nullptr) return true;

  *pos++ = char(length);

  const char *const end = pos + length;
  if (pos == end) return false;  // Only zero parts.

  int2store(pos, tm.year);
  pos += 2;
  *pos++ = char(tm.month);
  *pos++ = char(tm.day);

  if (pos == end) return false;  // Only date parts.

  *pos++ = char(tm.hour);
  *pos++ = char(tm.minute);
  *pos++ = char(tm.second);

  if (pos == end) return false;  // No microseconds.

  int4store(pos, tm.second_part);
  DBUG_ASSERT(pos + 4 == end);
  return false;
}

bool Protocol_binary::store_date(const MYSQL_TIME &tm) {
  if (send_metadata) return Protocol_text::store_date(tm);
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_DATE);
  field_pos++;

  if (tm.year == 0 && tm.month == 0 && tm.day == 0) {
    // Nothing to send, except a single byte to indicate length = 0.
    return packet->append(char{0});
  }

  char *pos = packet->prep_append(5, PACKET_BUFFER_EXTRA_ALLOC);
  if (pos == nullptr) return true;
  pos[0] = char{4};  // length
  int2store(pos + 1, tm.year);
  pos[3] = char(tm.month);
  pos[4] = char(tm.day);
  return false;
}

bool Protocol_binary::store_time(const MYSQL_TIME &tm, uint precision) {
  if (send_metadata) return Protocol_text::store_time(tm, precision);
  // field_types check is needed because of the embedded protocol
  DBUG_ASSERT(field_types == nullptr ||
              field_types[field_pos] == MYSQL_TYPE_TIME);
  field_pos++;

  size_t length;
  if (tm.second_part)
    length = 12;
  else if (tm.hour || tm.minute || tm.second || tm.day)
    length = 8;
  else
    length = 0;

  char *pos = packet->prep_append(length + 1, PACKET_BUFFER_EXTRA_ALLOC);
  if (pos == nullptr) return false;
  *pos++ = char(length);

  const char *const end = pos + length;
  if (pos == end) return false;  // zero date

  // Move hours to days if we have 24 hours or more.
  const unsigned days = tm.day + tm.hour / 24;
  const unsigned hours = tm.hour % 24;

  *pos++ = tm.neg ? 1 : 0;
  int4store(pos, days);
  pos += 4;
  *pos++ = char(hours);
  *pos++ = char(tm.minute);
  *pos++ = char(tm.second);

  if (pos == end) return false;  // no second part

  int4store(pos, tm.second_part);
  DBUG_ASSERT(pos + 4 == end);
  return false;
}

/**
  @returns: the file descriptor of the socket.
*/

my_socket Protocol_classic::get_socket() { return get_vio()->mysql_socket.fd; }

/**
  Read the length of the parameter data and return it back to
  the caller.

  @param packet             a pointer to the data
  @param packet_left_len    remaining packet length
  @param header_len         size of the header stored at the beginning of the
                            packet and used to specify the length of the data.

  @return
    Length of data piece.
*/

static ulong get_param_length(uchar *packet, ulong packet_left_len,
                              ulong *header_len) {
  if (packet_left_len < 1) {
    *header_len = 0;
    return 0;
  }

  switch (*packet) {
    case (252): {
      if (packet_left_len < 3) {
        *header_len = 0;
        return 0;
      }
      *header_len = 3;
      return static_cast<ulong>(uint2korr(packet + 1));
    }

    case (253): {
      if (packet_left_len < 4) {
        *header_len = 0;
        return 0;
      }
      *header_len = 4;
      return static_cast<ulong>(uint3korr(packet + 1));
    }

    case (254): {
      /*
       In our client-server protocol all numbers bigger than 2^24
       stored as 8 bytes with uint8korr. Here we always know that
       parameter length is less than 2^4 so we don't look at the second
       4 bytes. But still we need to obey the protocol hence 9 in the
       assignment below.
      */
      if (packet_left_len < 9) {
        *header_len = 0;
        return 0;
      }
      *header_len = 9;
      return static_cast<ulong>(uint4korr(packet + 1));
    }

    // 0xff as the first byte of a length-encoded integer is undefined.
    case (255): {
      *header_len = 0;
      return 0;
    }

    // (*packet < 251)
    default: {
      *header_len = 1;
      return static_cast<ulong>(*packet);
    }
  }
}

/**
  Returns the length of the encoded data

   @param[in]  type            parameter data type
   @param[in]  packet          network buffer
   @param[in]  packet_left_len number of bytes left in packet
   @param[out] header_len      the size of the header(bytes to be skiped)
   @param[out] err             boolean to store if an error occurred
*/
static ulong get_ps_param_len(enum enum_field_types type, uchar *packet,
                              ulong packet_left_len, ulong *header_len,
                              bool *err) {
  DBUG_TRACE;
  *header_len = 0;

  switch (type) {
    case MYSQL_TYPE_TINY:
      *err = (packet_left_len < 1);
      return 1;
    case MYSQL_TYPE_SHORT:
      *err = (packet_left_len < 2);
      return 2;
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_LONG:
      *err = (packet_left_len < 4);
      return 4;
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_LONGLONG:
      *err = (packet_left_len < 8);
      return 8;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP: {
      ulong param_length =
          get_param_length(packet, packet_left_len, header_len);
      /* in case of error ret is 0 and header size is 0 */
      *err = ((param_length == 0 && *header_len == 0) ||
              (packet_left_len < *header_len + param_length));
      DBUG_PRINT("info", ("ret=%lu ", param_length));
      return param_length;
    }
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    default: {
      ulong param_length =
          get_param_length(packet, packet_left_len, header_len);
      /* in case of error ret is 0 and header size is 0 */
      *err = (param_length == 0 && *header_len == 0);
      if (param_length > packet_left_len - *header_len)
        param_length = packet_left_len - *header_len;
      DBUG_PRINT("info", ("ret=%lu", param_length));
      return param_length;
    }
  }
}
