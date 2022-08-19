/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/protocol_callback.h"

#include <assert.h>
#include <stddef.h>
#include <algorithm>

#include "m_ctype.h"
#include "mysql_com.h"
#include "sql/current_thd.h"
#include "sql/field.h"
#include "sql/item.h"
#include "sql/item_func.h"
#include "sql/my_decimal.h"
#include "sql/sql_class.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql_string.h"

class String;
union COM_DATA;

/**
  @file sql/protocol_callback.cc
  Implementation of the Protocol_callback class, which is used by the Command
  service as proxy protocol.
*/

/**
  Practically does nothing.
  Returns -1, error, for the case this will be called. It should happen.
  read_packet() is called by get_command() which in turn is called by
  do_command() in sql_parse. After that COM_DATA is filled with proper info
  that in turn is passed to dispatch_command().
  The Command service doesn't use do_command() but dispatch_command() and
  passes COM_DATA directly from the user(plugin).

  @return
    -1 failure
*/
int Protocol_callback::read_packet() { return -1; }

/**
  Practically does nothing. See the comment of #read_packet().
  Always returns -1.

  @return
    -1
*/
int Protocol_callback::get_command(COM_DATA *, enum_server_command *) {
  return read_packet();
}

/**
  Returns the type of the connection.

  @return
    VIO_TYPE_PLUGIN
*/
enum enum_vio_type Protocol_callback::connection_type() const {
  return VIO_TYPE_PLUGIN;
}

bool Protocol_callback::store_null() {
  if (callbacks.get_null) return callbacks.get_null(callbacks_ctx);

  return false;
}

bool Protocol_callback::store_tiny(longlong from, uint32) {
  if (callbacks.get_integer) return callbacks.get_integer(callbacks_ctx, from);
  return false;
}

bool Protocol_callback::store_short(longlong from, uint32) {
  if (callbacks.get_integer) return callbacks.get_integer(callbacks_ctx, from);
  return false;
}

bool Protocol_callback::store_long(longlong from, uint32) {
  if (callbacks.get_integer) return callbacks.get_integer(callbacks_ctx, from);
  return false;
}

bool Protocol_callback::store_longlong(longlong from, bool is_unsigned,
                                       uint32) {
  if (callbacks.get_integer)
    return callbacks.get_longlong(callbacks_ctx, from, is_unsigned);
  return false;
}

/**
  Sends DECIMAL value

  @param d    value

  @return
    false  success
    true   failure
*/
bool Protocol_callback::store_decimal(const my_decimal *d, uint, uint) {
  if (callbacks.get_decimal) return callbacks.get_decimal(callbacks_ctx, d);
  return false;
}

bool Protocol_callback::store_string(const char *from, size_t length,
                                     const CHARSET_INFO *fromcs) {
  if (callbacks.get_string)
    return callbacks.get_string(callbacks_ctx, from, length, fromcs);
  return false;
}

bool Protocol_callback::store_float(float from, uint32 decimals, uint32) {
  if (callbacks.get_double)
    return callbacks.get_double(callbacks_ctx, from, decimals);
  return false;
}

bool Protocol_callback::store_double(double from, uint32 decimals, uint32) {
  if (callbacks.get_double)
    return callbacks.get_double(callbacks_ctx, from, decimals);
  return false;
}

bool Protocol_callback::store_datetime(const MYSQL_TIME &time, uint precision) {
  if (callbacks.get_datetime)
    return callbacks.get_datetime(callbacks_ctx, &time, precision);
  return false;
}

bool Protocol_callback::store_date(const MYSQL_TIME &time) {
  if (callbacks.get_datetime) return callbacks.get_date(callbacks_ctx, &time);
  return false;
}

bool Protocol_callback::store_time(const MYSQL_TIME &time, uint precision) {
  if (callbacks.get_time)
    return callbacks.get_time(callbacks_ctx, &time, precision);
  return false;
}

bool Protocol_callback::store_field(const Field *field) {
  switch (text_or_binary) {
    case CS_TEXT_REPRESENTATION: {
      if (field->is_null()) return store_null();
      StringBuffer<MAX_FIELD_WIDTH> buffer;
      return store(field->val_str(&buffer));
    }
    case CS_BINARY_REPRESENTATION:
      return field->send_to_protocol(this);
  }
  return true;
}

/**
  Returns the capabilities supported by the protocol
*/
ulong Protocol_callback::get_client_capabilities() {
  if (!client_capabilities_set && callbacks.get_client_capabilities) {
    client_capabilities_set = true;
    client_capabilities = callbacks.get_client_capabilities(callbacks_ctx);
  }
  return client_capabilities;
}

bool Protocol_callback::has_client_capability(unsigned long capability) {
  if (!client_capabilities_set) (void)get_client_capabilities();
  return client_capabilities & capability;
}

/**
  Called BEFORE sending data row or before field_metadata
*/
void Protocol_callback::start_row() {
  /*
    start_row() is called for metadata for convenience in Protocol_classic
    This is not wanted for protocol plugins as otherwise we get more calls
    and the plugin has to track state internally if it is in meta or not.
    start_meta(),
      start_row(), field_meta(), end_row(),
      ...
    end_meta()
    Calling start_row() is a left over from the era where metadata was
    sent with only one call send_metadata, and start_row() + end_row()
    were usable hooks in this send_metadata. Now it doesn't make any more
    sense as start_row() is called just before field_meta() and end_row()
    just after. The same code can go in to field_meta().
  */
  if (!in_meta_sending && callbacks.start_row)
    callbacks.start_row(callbacks_ctx);
}

/**
  Called AFTER sending all fields of a row, or after field_metadata().
  Please read the big comment in start_row() for explanation why
  in_meta_sending is used.

  @return
    true   ok
    false  not ok
*/
bool Protocol_callback::end_row() {
  /* See start_row() */
  if (!in_meta_sending && callbacks.end_row)
    return callbacks.end_row(callbacks_ctx);
  return false;
}

/**
  Called when a row is aborted
*/
void Protocol_callback::abort_row() {
  if (callbacks.end_row) callbacks.abort_row(callbacks_ctx);
}

/**
  Called in case of error while sending data
*/
void Protocol_callback::end_partial_result_set() {
  /* Protocol_callback shouldn't be used in this context */
  assert(0);
}

/**
  Called when the server shuts down the connection (THD is being destroyed).
  In this regard, this is also called when the server shuts down. The callback
  implementor can differentiate between those 2 events by inspecting the
  server_shutdown parameter.

  @param server_shutdown  Whether this is a normal connection shutdown (false)
                          or a server shutdown (true).

  @return
  0   success
  !0  failure
*/
int Protocol_callback::shutdown(bool server_shutdown) {
  if (callbacks.shutdown)
    callbacks.shutdown(callbacks_ctx, server_shutdown ? 1 : 0);
  return 0;
}

/**
  Returns if the connection is alive or dead.

  @note This function always returns true as in many places in the server this
  is a prerequisite for continuing operations.

  @return
    true  alive
*/
bool Protocol_callback::connection_alive() const { return true; }

/**
  Should return protocol's reading/writing status. Returns 0 (idle) as it this
  is the best guess that can be made as there is no callback for
  get_rw_status().

  @return
    0
*/
uint Protocol_callback::get_rw_status() { return 0; }

/**
  Should check if compression is enabled. Returns always false (no compression)

  @return
    false disabled
*/
bool Protocol_callback::get_compression() { return false; }

/**
  Always returns null.

  @return
     null compression not supported
*/
char *Protocol_callback::get_compression_algorithm() { return nullptr; }

/**
  Always returns 0.

  @return
   compression not supported
*/
uint Protocol_callback::get_compression_level() { return 0; }

/**
  Called BEFORE sending metadata

  @return
    true  failure
    false success
*/
bool Protocol_callback::start_result_metadata(uint num_cols, uint flags,
                                              const CHARSET_INFO *resultcs) {
  in_meta_sending = true;
  if (callbacks.start_result_metadata)
    return callbacks.start_result_metadata(callbacks_ctx, num_cols, flags,
                                           resultcs);
  return false;
}

/**
  Sends metadata of one field. Called for every column in the result set.

  @return
    true  failure
    false success
*/
bool Protocol_callback::send_field_metadata(Send_field *field,
                                            const CHARSET_INFO *cs) {
  if (callbacks.field_metadata) {
    struct st_send_field f;

    f.db_name = field->db_name;
    f.table_name = field->table_name;
    f.org_table_name = field->org_table_name;
    f.col_name = field->col_name;
    f.org_col_name = field->org_col_name;
    f.length = field->length;
    f.charsetnr = field->charsetnr;
    f.flags = field->flags;
    f.decimals = field->decimals;
    f.type = field->type;

    return callbacks.field_metadata(callbacks_ctx, &f, cs);
  }
  return true;
}

/**
  Called AFTER sending metadata

  @return
    true  failure
    false success
*/
bool Protocol_callback::end_result_metadata() {
  in_meta_sending = false;

  if (callbacks.end_result_metadata) {
    THD *t = current_thd;
    uint status = t->server_status;
    uint warn_count = t->get_stmt_da()->current_statement_cond_count();

    return callbacks.end_result_metadata(callbacks_ctx, status, warn_count);
  }
  return false;
}

/**
  Sends OK

  @return
    true  failure
    false success
*/
bool Protocol_callback::send_ok(uint server_status, uint warn_count,
                                ulonglong affected_rows,
                                ulonglong last_insert_id, const char *message) {
  if (callbacks.handle_ok)
    callbacks.handle_ok(callbacks_ctx, server_status, warn_count, affected_rows,
                        last_insert_id, message);
  return false;
}

bool Protocol_callback::flush() { return false; }

/**
  Sends end of file

  @return
    true  failure
    false success
*/
bool Protocol_callback::send_eof(uint server_status, uint warn_count) {
  if (callbacks.handle_ok)
    callbacks.handle_ok(callbacks_ctx, server_status, warn_count, 0, 0,
                        nullptr);
  return false;
}

/**
  Sends error

  @return
    true  failure
    false success
*/
bool Protocol_callback::send_error(uint sql_errno, const char *err_msg,
                                   const char *sql_state) {
  if (callbacks.handle_error)
    callbacks.handle_error(callbacks_ctx, sql_errno, err_msg, sql_state);
  return false;
}

bool Protocol_callback::store_ps_status(ulong stmt_id, uint column_count,
                                        uint param_count, ulong cond_count) {
  List<Item> field_list;
  field_list.push_back(new Item_empty_string("stmt_id", MY_CS_NAME_SIZE));
  field_list.push_back(new Item_empty_string("column_no", MY_CS_NAME_SIZE));
  field_list.push_back(new Item_empty_string("param_no", MY_CS_NAME_SIZE));
  field_list.push_back(new Item_empty_string("warning_no", MY_CS_NAME_SIZE));

  THD *thd = current_thd;
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  start_row();
  if (store_long(stmt_id) || store_short(column_count) ||
      store_short(param_count) || store_short(std::min(cond_count, 65535UL)) ||
      end_row()) {
    return true; /* purecov: inspected */
  }
  return false;
}

bool Protocol_callback::send_parameters(List<Item_param> *parameters,
                                        bool is_sql_prep) {
  if (is_sql_prep) return set_variables_from_parameters(parameters);

  if (!has_client_capability(CLIENT_PS_MULTI_RESULTS))
    // The client does not support OUT-parameters.
    return false;

  List_iterator_fast<Item_param> item_param_it(*parameters);

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

  THD *thd = current_thd;
  /*
    We have to set SERVER_PS_OUT_PARAMS in THD::server_status, because it
    is used in send_result_metadata().
  */
  thd->server_status |= SERVER_PS_OUT_PARAMS | SERVER_MORE_RESULTS_EXISTS;

  // Send meta-data.
  if (thd->send_result_metadata(&out_param_lst,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true; /* purecov: inspected */

  // Send data.
  start_row();
  if (thd->send_result_set_row(&out_param_lst) || end_row())
    return true; /* purecov: inspected */

  // Restore THD::server_status.
  thd->server_status &= ~SERVER_PS_OUT_PARAMS;
  thd->server_status &= ~SERVER_MORE_RESULTS_EXISTS;

  return send_ok(
      (thd->server_status | SERVER_PS_OUT_PARAMS | SERVER_MORE_RESULTS_EXISTS),
      thd->get_stmt_da()->current_statement_cond_count(), 0, 0, nullptr);
}

bool Protocol_callback::set_variables_from_parameters(
    List<Item_param> *parameters) {
  THD *thd = current_thd;
  List_iterator_fast<Item_param> item_param_it(*parameters);
  List_iterator_fast<LEX_STRING> user_var_name_it(
      thd->lex->prepared_stmt_params);

  Item_param *item_param;
  LEX_STRING *user_var_name;
  while ((item_param = item_param_it++) &&
         (user_var_name = user_var_name_it++)) {
    // Skip it as it's just an IN-parameter.
    if (!item_param->get_out_param_info()) continue;

    /*
      Delete call not needed, the object appends itself
      to THDs free_list.
    */
    Item_func_set_user_var *suv =
        new Item_func_set_user_var(*user_var_name, item_param, false);
    /*
      Item_func_set_user_var is not fixed after construction,
      call fix_fields().
    */
    if (suv->fix_fields(thd, nullptr)) return true;

    if (suv->check(false)) return true;

    if (suv->update()) return true;
  }

  return false;
}
