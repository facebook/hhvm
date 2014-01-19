/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
bool f_ldap_add(CResRef link, const String& dn, CArrRef entry);
bool f_ldap_mod_add(CResRef link, const String& dn, CArrRef entry);
bool f_ldap_mod_del(CResRef link, const String& dn, CArrRef entry);
bool f_ldap_mod_replace(CResRef link, const String& dn, CArrRef entry);
bool f_ldap_modify(CResRef link, const String& dn, CArrRef entry);
bool f_ldap_bind(
  CResRef link, const String& bind_rdn = null_string,
  const String& bind_password = null_string);
bool f_ldap_set_rebind_proc(CResRef link, CVarRef callback);
bool f_ldap_sort(CResRef link, CResRef result, const String& sortfilter);
bool f_ldap_start_tls(CResRef link);
bool f_ldap_unbind(CResRef link);
bool f_ldap_get_option(CResRef link, int option, VRefParam retval);
bool f_ldap_set_option(CVarRef link, int option, CVarRef newval);
bool f_ldap_close(CResRef link);
Variant f_ldap_list(
  CVarRef link, CVarRef base_dn, CVarRef filter,
  CArrRef attributes = null_array, int attrsonly = 0, int sizelimit = -1,
  int timelimit = -1, int deref = -1);
Variant f_ldap_read(
  CVarRef link, CVarRef base_dn, CVarRef filter,
  CArrRef attributes = null_array, int attrsonly = 0, int sizelimit = -1,
  int timelimit = -1, int deref = -1);
Variant f_ldap_search(
  CVarRef link, CVarRef base_dn, CVarRef filter,
  CArrRef attributes = null_array, int attrsonly = 0, int sizelimit = -1,
  int timelimit = -1, int deref = -1);
bool f_ldap_rename(
  CResRef link, const String& dn, const String& newrdn,
  const String& newparent, bool deleteoldrdn);
bool f_ldap_delete(CResRef link, const String& dn);
Variant f_ldap_compare(
  CResRef link, const String& dn, const String& attribute, const String& value);
int64_t f_ldap_errno(CResRef link);
String f_ldap_error(CResRef link);
Variant f_ldap_get_dn(CResRef link, CResRef result_entry);
int64_t f_ldap_count_entries(CResRef link, CResRef result);
Variant f_ldap_get_entries(CResRef link, CResRef result);
Variant f_ldap_first_entry(CResRef link, CResRef result);
Variant f_ldap_next_entry(CResRef link, CResRef result_entry);
Array f_ldap_get_attributes(CResRef link, CResRef result_entry);
Variant f_ldap_first_attribute(CResRef link, CResRef result_entry);
Variant f_ldap_next_attribute(CResRef link, CResRef result_entry);
Variant f_ldap_first_reference(CResRef link, CResRef result);
Variant f_ldap_next_reference(CResRef link, CResRef result_entry);
bool f_ldap_parse_reference(
  CResRef link, CResRef result_entry, VRefParam referrals);
bool f_ldap_parse_result(
  CResRef link, CResRef result, VRefParam errcode,
  VRefParam matcheddn = uninit_null(), VRefParam errmsg = uninit_null(),
  VRefParam referrals = uninit_null());
bool f_ldap_free_result(CResRef result);
Variant f_ldap_get_values_len
(CResRef link, CResRef result_entry, const String& attribute);
Variant f_ldap_get_values(
  CResRef link, CResRef result_entry, const String& attribute);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_LDAP_H_
