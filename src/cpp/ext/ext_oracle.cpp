/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <cpp/ext/ext_oracle.h>
#include "crutch.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Object f_oci_connect(CStrRef username, CStrRef password, CStrRef db /* = null_string */, CStrRef charset /* = null_string */, int session_mode /* = 0 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(username), NEW(ArrayElement)(password), NEW(ArrayElement)(db), NEW(ArrayElement)(charset), NEW(ArrayElement)(session_mode), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_connect", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_oci_new_connect(CStrRef username, CStrRef password, CStrRef db /* = null_string */, CStrRef charset /* = null_string */, int session_mode /* = 0 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(username), NEW(ArrayElement)(password), NEW(ArrayElement)(db), NEW(ArrayElement)(charset), NEW(ArrayElement)(session_mode), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_new_connect", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_oci_pconnect(CStrRef username, CStrRef password, CStrRef db /* = null_string */, CStrRef charset /* = null_string */, int session_mode /* = 0 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(username), NEW(ArrayElement)(password), NEW(ArrayElement)(db), NEW(ArrayElement)(charset), NEW(ArrayElement)(session_mode), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_pconnect", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

String f_oci_server_version(CObjRef connection) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(connection)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_server_version", _schema, _params);
  return (Variant)_ret[0];
}

Variant f_oci_password_change(CVarRef connection, CStrRef username, CStrRef old_password, CStrRef new_password) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(connection), NEW(ArrayElement)(username), NEW(ArrayElement)(old_password), NEW(ArrayElement)(new_password), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_password_change", _schema, _params);
  return (Variant)_ret[0];
}

Object f_oci_new_cursor(CObjRef connection) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(connection)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_new_cursor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_oci_new_descriptor(CObjRef connection, int type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(connection)), NEW(ArrayElement)(type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_new_descriptor", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_oci_close(CObjRef connection) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(connection)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_close", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_commit(CObjRef connection) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(connection)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_commit", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_rollback(CObjRef connection) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(connection)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_rollback", _schema, _params);
  return (Variant)_ret[0];
}

Array f_oci_error(CObjRef source /* = null */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(source)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_error", _schema, _params);
  return (Variant)_ret[0];
}

void f_oci_internal_debug(bool onoff) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(onoff), (ArrayElement*)NULL);
  Crutch::Invoke("oci_internal_debug", _schema, _params);
}

Object f_oci_parse(CObjRef connection, CStrRef query) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(connection)), NEW(ArrayElement)(query), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_parse", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

String f_oci_statement_type(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_statement_type", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_free_statement(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_free_statement", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_free_descriptor(CObjRef lob) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(lob)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_free_descriptor", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_bind_array_by_name(CObjRef statement, CStrRef name, Variant var_array, int max_table_length, int max_item_length /* = 0 */, int type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(2, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(name), NEW(ArrayElement)(var_array), NEW(ArrayElement)(max_table_length), NEW(ArrayElement)(max_item_length), NEW(ArrayElement)(type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_bind_array_by_name", _schema, _params);
  var_array = ((Variant)_ret[1])[2];
  return (Variant)_ret[0];
}

bool f_oci_bind_by_name(CObjRef statement, CStrRef ph_name, Variant variable, int max_length /* = 0 */, int type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(2, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(ph_name), NEW(ArrayElement)(variable), NEW(ArrayElement)(max_length), NEW(ArrayElement)(type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_bind_by_name", _schema, _params);
  variable = ((Variant)_ret[1])[2];
  return (Variant)_ret[0];
}

bool f_oci_cancel(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_cancel", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_define_by_name(CObjRef statement, CStrRef column_name, Variant variable, int type /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(2, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(column_name), NEW(ArrayElement)(variable), NEW(ArrayElement)(type), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_define_by_name", _schema, _params);
  variable = ((Variant)_ret[1])[2];
  return (Variant)_ret[0];
}

bool f_oci_execute(CObjRef statement, int mode /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(mode), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_execute", _schema, _params);
  return (Variant)_ret[0];
}

int f_oci_num_fields(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_num_fields", _schema, _params);
  return (Variant)_ret[0];
}

int f_oci_num_rows(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_num_rows", _schema, _params);
  return (Variant)_ret[0];
}

Variant f_oci_result(CObjRef statement, CVarRef field) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(field), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_result", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_set_prefetch(CObjRef statement, int rows) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(rows), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_set_prefetch", _schema, _params);
  return (Variant)_ret[0];
}

int f_oci_fetch_all(CObjRef statement, Variant output, int skip /* = 0 */, int maxrows /* = 0 */, int flags /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(output), NEW(ArrayElement)(skip), NEW(ArrayElement)(maxrows), NEW(ArrayElement)(flags), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_fetch_all", _schema, _params);
  output = ((Variant)_ret[1])[1];
  return (Variant)_ret[0];
}

Variant f_oci_fetch_array(CObjRef statement, int mode /* = 0 */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(mode), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_fetch_array", _schema, _params);
  return (Variant)_ret[0];
}

Variant f_oci_fetch_assoc(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_fetch_assoc", _schema, _params);
  return (Variant)_ret[0];
}

Variant f_oci_fetch_object(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_fetch_object", _schema, _params);
  return (Variant)_ret[0];
}

Variant f_oci_fetch_row(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_fetch_row", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_fetch(CObjRef statement) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_fetch", _schema, _params);
  return (Variant)_ret[0];
}

bool f_oci_field_is_null(CObjRef statement, CVarRef field) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(field), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_field_is_null", _schema, _params);
  return (Variant)_ret[0];
}

String f_oci_field_name(CObjRef statement, int field) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(field), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_field_name", _schema, _params);
  return (Variant)_ret[0];
}

int f_oci_field_precision(CObjRef statement, int field) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(field), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_field_precision", _schema, _params);
  return (Variant)_ret[0];
}

int f_oci_field_scale(CObjRef statement, int field) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(field), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_field_scale", _schema, _params);
  return (Variant)_ret[0];
}

int f_oci_field_size(CObjRef statement, CVarRef field) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(field), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_field_size", _schema, _params);
  return (Variant)_ret[0];
}

int f_oci_field_type_raw(CObjRef statement, int field) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(field), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_field_type_raw", _schema, _params);
  return (Variant)_ret[0];
}

Variant f_oci_field_type(CObjRef statement, int field) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(statement)), NEW(ArrayElement)(field), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("oci_field_type", _schema, _params);
  return (Variant)_ret[0];
}

///////////////////////////////////////////////////////////////////////////////
}
