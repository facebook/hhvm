/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_LDAP_H_
#define incl_HPHP_EXT_LDAP_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_ldap_connect(const String& hostname = null_string, int port = 389);
Variant f_ldap_explode_dn(const String& dn, int with_attrib);
Variant f_ldap_dn2ufn(const String& db);
String f_ldap_err2str(int errnum);
bool f_ldap_add(const Resource& link, const String& dn, const Array& entry);
bool f_ldap_mod_add(const Resource& link, const String& dn, const Array& entry);
bool f_ldap_mod_del(const Resource& link, const String& dn, const Array& entry);
bool f_ldap_mod_replace(const Resource& link, const String& dn, const Array& entry);
bool f_ldap_modify(const Resource& link, const String& dn, const Array& entry);
bool f_ldap_bind(
  const Resource& link, const String& bind_rdn = null_string,
  const String& bind_password = null_string);
bool f_ldap_set_rebind_proc(const Resource& link, const Variant& callback);
bool f_ldap_sort(const Resource& link, const Resource& result, const String& sortfilter);
bool f_ldap_start_tls(const Resource& link);
bool f_ldap_unbind(const Resource& link);
bool f_ldap_get_option(const Resource& link, int option, VRefParam retval);
bool f_ldap_set_option(const Variant& link, int option, const Variant& newval);
bool f_ldap_close(const Resource& link);
Variant f_ldap_list(
  const Variant& link, const Variant& base_dn, const Variant& filter,
  const Array& attributes = null_array, int attrsonly = 0, int sizelimit = -1,
  int timelimit = -1, int deref = -1);
Variant f_ldap_read(
  const Variant& link, const Variant& base_dn, const Variant& filter,
  const Array& attributes = null_array, int attrsonly = 0, int sizelimit = -1,
  int timelimit = -1, int deref = -1);
Variant f_ldap_search(
  const Variant& link, const Variant& base_dn, const Variant& filter,
  const Array& attributes = null_array, int attrsonly = 0, int sizelimit = -1,
  int timelimit = -1, int deref = -1);
bool f_ldap_rename(
  const Resource& link, const String& dn, const String& newrdn,
  const String& newparent, bool deleteoldrdn);
bool f_ldap_delete(const Resource& link, const String& dn);
Variant f_ldap_compare(
  const Resource& link, const String& dn, const String& attribute, const String& value);
int64_t f_ldap_errno(const Resource& link);
String f_ldap_error(const Resource& link);
Variant f_ldap_get_dn(const Resource& link, const Resource& result_entry);
int64_t f_ldap_count_entries(const Resource& link, const Resource& result);
Variant f_ldap_get_entries(const Resource& link, const Resource& result);
Variant f_ldap_first_entry(const Resource& link, const Resource& result);
Variant f_ldap_next_entry(const Resource& link, const Resource& result_entry);
Array f_ldap_get_attributes(const Resource& link, const Resource& result_entry);
Variant f_ldap_first_attribute(const Resource& link, const Resource& result_entry);
Variant f_ldap_next_attribute(const Resource& link, const Resource& result_entry);
Variant f_ldap_first_reference(const Resource& link, const Resource& result);
Variant f_ldap_next_reference(const Resource& link, const Resource& result_entry);
bool f_ldap_parse_reference(
  const Resource& link, const Resource& result_entry, VRefParam referrals);
bool f_ldap_parse_result(
  const Resource& link, const Resource& result, VRefParam errcode,
  VRefParam matcheddn = uninit_null(), VRefParam errmsg = uninit_null(),
  VRefParam referrals = uninit_null());
bool f_ldap_free_result(const Resource& result);
Variant f_ldap_get_values_len
(const Resource& link, const Resource& result_entry, const String& attribute);
Variant f_ldap_get_values(
  const Resource& link, const Resource& result_entry, const String& attribute);
bool f_ldap_control_paged_result(const Resource& link, int pagesize,
  bool iscritical = false, const String& cookie = empty_string);
bool f_ldap_control_paged_result_response(const Resource& link, const Resource& result,
  VRefParam cookie = uninit_null(), VRefParam estimated = uninit_null());

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_LDAP_H_
