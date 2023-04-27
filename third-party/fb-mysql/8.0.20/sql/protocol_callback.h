/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PROTOCOL_CALLBACK_INCLUDED
#define PROTOCOL_CALLBACK_INCLUDED

/**
@file
  Interface of the Protocol_callback class, which is used by the Command
  service as proxy protocol.
*/

#include <stddef.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_command.h"
#include "my_inttypes.h"
#include "mysql/service_command.h"
#include "mysql_time.h"
#include "sql/protocol.h"
#include "violite.h"

class Item_param;
class Send_field;
class String;
class my_decimal;
template <class T>
class List;
union COM_DATA;

class Protocol_callback final : public Protocol {
 public:
  Protocol_callback(const struct st_command_service_cbs *cbs,
                    enum cs_text_or_binary t_or_b, void *cbs_ctx)
      : callbacks_ctx(cbs_ctx),
        callbacks(*cbs),
        client_capabilities(0),
        client_capabilities_set(false),
        text_or_binary(t_or_b),
        in_meta_sending(false) {}

  /**
    Forces read of packet from the connection

    @return
      bytes read
      -1 failure
  */
  int read_packet() override;

  /**
    Reads from the line and parses the data into union COM_DATA

    @return
      bytes read
      -1 failure
  */
  int get_command(COM_DATA *com_data, enum_server_command *cmd) override;

  /**
    Returns the type of the protocol

    @return
      false  success
      true   failure
  */
  enum enum_protocol_type type() const override { return PROTOCOL_PLUGIN; }

  /**
    Returns the type of the connection

    @return
      enum enum_vio_type
  */
  enum enum_vio_type connection_type() const override;

  /**
    Sends null value

    @return
      false  success
      true   failure
  */
  bool store_null() override;

  /**
    Sends TINYINT value

    @param from value

    @return
      false  success
      true   failure
  */
  bool store_tiny(longlong from, uint32) override;

  /**
    Sends SMALLINT value

    @param from value

    @return
      false  success
      true   failure
  */
  bool store_short(longlong from, uint32) override;

  /**
    Sends INT/INTEGER value

    @param from value

    @return
      false  success
      true   failure
  */
  bool store_long(longlong from, uint32) override;

  /**
    Sends BIGINT value

    @param from         value
    @param is_unsigned  from is unsigned

    @return
      false  success
      true   failure
  */
  bool store_longlong(longlong from, bool is_unsigned, uint32) override;

  /**
    Sends DECIMAL value

    @param d    value

    @return
      false  success
      true   failure
  */
  bool store_decimal(const my_decimal *d, uint, uint) override;

  /**
    Sends string (CHAR/VARCHAR/TEXT/BLOB) value

    @return
      false  success
      true   failure
  */
  bool store_string(const char *from, size_t length,
                    const CHARSET_INFO *fromcs) override;

  /**
    Sends FLOAT value

    @param from      value
    @param decimals

    @return
      false  success
      true   failure
  */
  bool store_float(float from, uint32 decimals, uint32) override;

  /**
    Sends DOUBLE value

    @param from      value
    @param decimals

    @return
      false  success
      true   failure
  */
  bool store_double(double from, uint32 decimals, uint32) override;

  /**
    Sends DATETIME value

    @param time      value
    @param precision

    @return
      false  success
      true   failure
  */
  bool store_datetime(const MYSQL_TIME &time, uint precision) override;

  /**
    Sends DATE value

    @param time      value

    @return
      false  success
      true   failure
  */
  bool store_date(const MYSQL_TIME &time) override;

  /**
    Sends TIME value

    @param time      value
    @param precision

    @return
      false  success
      true   failure
  */
  bool store_time(const MYSQL_TIME &time, uint precision) override;

  /**
    Sends Field

    @param field

    @return
      false  success
      true   failure
  */
  bool store_field(const Field *field) override;

  /**
    Returns the capabilities supported by the protocol
  */
  ulong get_client_capabilities() override;

  /**
    Checks if the protocol supports a capability

    @param capability the capability

    @return
      true   supports
      false  does not support
  */
  bool has_client_capability(unsigned long capability) override;

  /**
    Called BEFORE sending data row or before field_metadata
  */
  void start_row() override;

  /**
    Called AFTER sending data row or before field_metadata
  */
  bool end_row() override;

  /**
    Called when a row is aborted
  */
  void abort_row() override;

  /**
    Called in case of error while sending data
  */
  void end_partial_result_set() override;

  /**
    Called when the server shuts down the connection (THD is being destroyed).
    In this regard, this is also called when the server shuts down. The callback
    implementor can differentiate between those 2 events by inspecting the
    shutdown_type parameter.

    @param server_shutdown  Whether this is a normal connection shutdown (false)
                            or a server shutdown (true).

    @return
    0   success
    !0  failure
  */
  int shutdown(bool server_shutdown = false) override;

  /**
    This function always returns true as in many places in the server this
    is a prerequisite for continuing operations.

    @return
      true   alive
  */
  bool connection_alive() const override;

  /**
    Should return protocol's reading/writing status. Returns 0 (idle) as it
    this is the best guess that can be made as there is no callback for
    get_rw_status().
  */
  uint get_rw_status() override;

  /**
    Checks if compression is enabled

    @return
      true  enabled
      false disabled
  */
  bool get_compression() override;

  /**
    Checks if compression is enabled and return compression method name

    @return
      algorithm name if compression is supported else null
  */
  char *get_compression_algorithm() override;

  /**
   Checks if compression is enabled and return compression level.

    @return
      compression level if compression is supported else 0
  */
  uint get_compression_level() override;

  /**
    Called BEFORE sending metadata

    @param num_cols Number of columns in the result set
    @param flags
    @param resultcs The character set of the results. Can be different from the
                    one in the field metadata.

    @return
      true  failure
     false success
  */
  bool start_result_metadata(uint num_cols, uint flags,
                             const CHARSET_INFO *resultcs) override;

  /**
    Sends metadata of one field. Called for every column in the result set.

    @param field  Field's metadata
    @param cs     Charset

    @return
      true  failure
      false success
  */
  bool send_field_metadata(Send_field *field, const CHARSET_INFO *cs) override;

  /**
    Called AFTER sending metadata

    @return
      true  failure
      false success
  */
  bool end_result_metadata() override;

  /**
    Sends OK

    @param server_status Bit field with different statuses. See SERVER_STATUS_*
    @param warn_count      Warning count from the execution
    @param affected_rows   Rows changed/deleted during the operation
    @param last_insert_id  ID of the last insert row, which has AUTO_INCROMENT
                           column
    @param message         Textual message from the execution. May be NULL.

    @return
      true  failure
      false success
  */
  bool send_ok(uint server_status, uint warn_count, ulonglong affected_rows,
               ulonglong last_insert_id, const char *message) override;

  /**
    Sends end of file.


    This will be called once all data has been sent.

    @param server_status Bit field with different statuses. See SERVER_STATUS_*
    @param warn_count    The warning count generated by the execution of the
                         statement.

    @return
      true  failure
      false success
  */
  bool send_eof(uint server_status, uint warn_count) override;

  /**
    Sends error

    @param sql_errno  Error number, beginning from 1000
    @param err_msg    The error message
    @param sql_state  The SQL state - 5 char string

    @return
      true  failure
      false success
  */
  bool send_error(uint sql_errno, const char *err_msg,
                  const char *sql_state) override;

  bool store_ps_status(ulong stmt_id, uint column_count, uint param_count,
                       ulong cond_count) override;

  bool send_parameters(List<Item_param> *parameters,
                       bool is_sql_prepare) override;
  bool flush() override;

  using Protocol::store_long;
  using Protocol::store_short;

 private:
  /**
    Set output parameters to variables bound at PS execution.

    This method handles the case when preparing and executing was done
    through SQL (not by COM_STMT_PREPARE/COM_STMT_EXECUTE) in which
    output parameters are not going to be send to client (or
    'st_command_service_cbs'), instead they will set concrete session
    variables.

    @param parameters  List of PS/SP parameters (both input and output).

    @return
      false  success
      true   failure
  */
  bool set_variables_from_parameters(List<Item_param> *parameters);

  void *callbacks_ctx;
  struct st_command_service_cbs callbacks;
  unsigned long client_capabilities;
  bool client_capabilities_set;
  enum cs_text_or_binary text_or_binary;
  bool in_meta_sending;
};

#endif /* PROTOCOL_CALLBACK_INCLUDED */
