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

#include <cpp/ext/ext_ldap.h>
#include "crutch.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Object f_ldap_connect(CStrRef hostname /* = null_string */, int port /* = 389 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(hostname), NEW(ArrayElement)(port), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_connect", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Array f_ldap_explode_dn(CStrRef dn, int with_attrib) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(dn), NEW(ArrayElement)(with_attrib), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_explode_dn", _schema, _params);
  return (Variant)_ret[0];
}

String f_ldap_dn2ufn(CStrRef db) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(db), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_dn2ufn", _schema, _params);
  return (Variant)_ret[0];
}

String f_ldap_err2str(int errnum) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(errnum), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_err2str", _schema, _params);
  return (Variant)_ret[0];
}

String f_ldap_8859_to_t61(CStrRef value) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(value), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_8859_to_t61", _schema, _params);
  return (Variant)_ret[0];
}

String f_ldap_t61_to_8859(CStrRef value) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(value), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_t61_to_8859", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_add(CObjRef link_identifier, CStrRef dn, CArrRef entry) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(dn), NEW(ArrayElement)(entry), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_add", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_mod_add(CObjRef link_identifier, CStrRef dn, CArrRef entry) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(dn), NEW(ArrayElement)(entry), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_mod_add", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_mod_del(CObjRef link_identifier, CStrRef dn, CArrRef entry) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(dn), NEW(ArrayElement)(entry), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_mod_del", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_mod_replace(CObjRef link_identifier, CStrRef dn, CArrRef entry) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(dn), NEW(ArrayElement)(entry), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_mod_replace", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_modify(CObjRef link_identifier, CStrRef dn, CArrRef entry) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(dn), NEW(ArrayElement)(entry), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_modify", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_bind(CObjRef link_identifier, CStrRef bind_rdn /* = null_string */, CStrRef bind_password /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(bind_rdn), NEW(ArrayElement)(bind_password), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_bind", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_sasl_bind(CObjRef link, CStrRef binddn /* = null_string */, CStrRef password /* = null_string */, CStrRef sasl_mech /* = null_string */, CStrRef sasl_realm /* = null_string */, CStrRef sasl_authz_id /* = null_string */, CStrRef props /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link)), NEW(ArrayElement)(binddn), NEW(ArrayElement)(password), NEW(ArrayElement)(sasl_mech), NEW(ArrayElement)(sasl_realm), NEW(ArrayElement)(sasl_authz_id), NEW(ArrayElement)(props), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_sasl_bind", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_set_rebind_proc(CObjRef link, CStrRef callback) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link)), NEW(ArrayElement)(callback), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_set_rebind_proc", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_sort(CObjRef link, CObjRef result, CStrRef sortfilter) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link)), NEW(ArrayElement)(OpaqueObject::GetIndex(result)), NEW(ArrayElement)(sortfilter), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_sort", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_start_tls(CObjRef link) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_start_tls", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_unbind(CObjRef link_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_unbind", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_get_option(CObjRef link_identifier, int option, Variant retval) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(2, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(option), NEW(ArrayElement)(retval), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_get_option", _schema, _params);
  retval = ((Variant)_ret[1])[2];
  return (Variant)_ret[0];
}

bool f_ldap_set_option(CObjRef link_identifier, int option, CVarRef newval) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(option), NEW(ArrayElement)(newval), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_set_option", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_close(CObjRef link_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_close", _schema, _params);
  return (Variant)_ret[0];
}

Object f_ldap_list(CObjRef link_identifier, CStrRef base_dn, CStrRef filter, CArrRef attributes /* = null */, int attrsonly /* = 0 */, int sizelimit /* = 0 */, int timelimit /* = 0 */, int deref /* = 0 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(base_dn), NEW(ArrayElement)(filter), NEW(ArrayElement)(attributes), NEW(ArrayElement)(attrsonly), NEW(ArrayElement)(sizelimit), NEW(ArrayElement)(timelimit), NEW(ArrayElement)(deref), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_list", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_ldap_read(CObjRef link_identifier, CStrRef base_dn, CStrRef filter, CArrRef attributes /* = null */, int attrsonly /* = 0 */, int sizelimit /* = 0 */, int timelimit /* = 0 */, int deref /* = 0 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(base_dn), NEW(ArrayElement)(filter), NEW(ArrayElement)(attributes), NEW(ArrayElement)(attrsonly), NEW(ArrayElement)(sizelimit), NEW(ArrayElement)(timelimit), NEW(ArrayElement)(deref), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_read", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_ldap_search(CObjRef link_identifier, CStrRef base_dn, CStrRef filter, CArrRef attributes /* = null */, int attrsonly /* = 0 */, int sizelimit /* = 0 */, int timelimit /* = 0 */, int deref /* = 0 */) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(base_dn), NEW(ArrayElement)(filter), NEW(ArrayElement)(attributes), NEW(ArrayElement)(attrsonly), NEW(ArrayElement)(sizelimit), NEW(ArrayElement)(timelimit), NEW(ArrayElement)(deref), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_search", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_ldap_rename(CObjRef link_identifier, CStrRef dn, CStrRef newrdn, CStrRef newparent, bool deleteoldrdn) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(dn), NEW(ArrayElement)(newrdn), NEW(ArrayElement)(newparent), NEW(ArrayElement)(deleteoldrdn), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_rename", _schema, _params);
  return (Variant)_ret[0];
}

bool f_ldap_delete(CObjRef link_identifier, CStrRef dn) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(dn), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_delete", _schema, _params);
  return (Variant)_ret[0];
}

Variant f_ldap_compare(CObjRef link_identifier, CStrRef dn, CStrRef attribute, CStrRef value) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(dn), NEW(ArrayElement)(attribute), NEW(ArrayElement)(value), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_compare", _schema, _params);
  return (Variant)_ret[0];
}

int f_ldap_errno(CObjRef link_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_errno", _schema, _params);
  return (Variant)_ret[0];
}

String f_ldap_error(CObjRef link_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_error", _schema, _params);
  return (Variant)_ret[0];
}

String f_ldap_get_dn(CObjRef link_identifier, CObjRef result_entry_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_entry_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_get_dn", _schema, _params);
  return (Variant)_ret[0];
}

int f_ldap_count_entries(CObjRef link_identifier, CObjRef result_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_count_entries", _schema, _params);
  return (Variant)_ret[0];
}

Array f_ldap_get_entries(CObjRef link_identifier, CObjRef result_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_get_entries", _schema, _params);
  return (Variant)_ret[0];
}

Object f_ldap_first_entry(CObjRef link_identifier, CObjRef result_identifier) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_first_entry", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_ldap_next_entry(CObjRef link_identifier, CObjRef result_identifier) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_next_entry", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Array f_ldap_get_attributes(CObjRef link_identifier, CObjRef result_entry_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_entry_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_get_attributes", _schema, _params);
  return (Variant)_ret[0];
}

String f_ldap_first_attribute(CObjRef link_identifier, CObjRef result_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_first_attribute", _schema, _params);
  return (Variant)_ret[0];
}

String f_ldap_next_attribute(CObjRef link_identifier, CObjRef result_entry_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_entry_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_next_attribute", _schema, _params);
  return (Variant)_ret[0];
}

Object f_ldap_first_reference(CObjRef link_identifier, CObjRef result_identifier) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_first_reference", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Object f_ldap_next_reference(CObjRef link_identifier, CObjRef result_identifier) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_next_reference", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_ldap_parse_reference(CObjRef link_identifier, CObjRef result_identifier, Variant referrals) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), NEW(ArrayElement)(2, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), NEW(ArrayElement)(referrals), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_parse_reference", _schema, _params);
  referrals = ((Variant)_ret[1])[2];
  return (Variant)_ret[0];
}

bool f_ldap_parse_result(CObjRef link_identifier, CObjRef result_entry_identifier, Variant errcode, Variant matcheddn /* = null */, Variant errmsg /* = null */, Variant referrals /* = null */) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), NEW(ArrayElement)(2, "R"), NEW(ArrayElement)(3, "R"), NEW(ArrayElement)(4, "R"), NEW(ArrayElement)(5, "R"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_entry_identifier)), NEW(ArrayElement)(errcode), NEW(ArrayElement)(matcheddn), NEW(ArrayElement)(errmsg), NEW(ArrayElement)(referrals), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_parse_result", _schema, _params);
  errcode = ((Variant)_ret[1])[2];
  matcheddn = ((Variant)_ret[1])[3];
  errmsg = ((Variant)_ret[1])[4];
  referrals = ((Variant)_ret[1])[5];
  return (Variant)_ret[0];
}

bool f_ldap_free_result(CObjRef result_identifier) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(result_identifier)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_free_result", _schema, _params);
  return (Variant)_ret[0];
}

Array f_ldap_get_values_len(CObjRef link_identifier, CObjRef result_entry_identifier, CStrRef attribute) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_entry_identifier)), NEW(ArrayElement)(attribute), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_get_values_len", _schema, _params);
  return (Variant)_ret[0];
}

Array f_ldap_get_values(CObjRef link_identifier, CObjRef result_entry_identifier, CStrRef attribute) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(link_identifier)), NEW(ArrayElement)(OpaqueObject::GetIndex(result_entry_identifier)), NEW(ArrayElement)(attribute), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("ldap_get_values", _schema, _params);
  return (Variant)_ret[0];
}

///////////////////////////////////////////////////////////////////////////////
}
