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

#include "hphp/runtime/ext/extension.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(ldap_connect,
                      const Variant& hostname = null_variant,
                      int port = 389);
Variant HHVM_FUNCTION(ldap_explode_dn,
                      const String& dn,
                      int with_attrib);
Variant HHVM_FUNCTION(ldap_dn2ufn,
                      const String& db);
String HHVM_FUNCTION(ldap_err2str,
                     int errnum);
bool HHVM_FUNCTION(ldap_add,
                   const Resource& link,
                   const String& dn,
                   const Array& entry);
bool HHVM_FUNCTION(ldap_mod_add,
                   const Resource& link,
                   const String& dn,
                   const Array& entry);
bool HHVM_FUNCTION(ldap_mod_del,
                   const Resource& link,
                   const String& dn,
                   const Array& entry);
bool HHVM_FUNCTION(ldap_mod_replace,
                   const Resource& link,
                   const String& dn,
                   const Array& entry);
bool HHVM_FUNCTION(ldap_modify,
                   const Resource& link,
                   const String& dn,
                   const Array& entry);
bool HHVM_FUNCTION(ldap_bind,
                   const Resource& link,
                   const Variant& bind_rdn = null_variant,
                   const Variant& bind_password = null_variant);
bool HHVM_FUNCTION(ldap_set_rebind_proc,
                   const Resource& link,
                   const Variant& callback);
bool HHVM_FUNCTION(ldap_sort,
                   const Resource& link,
                   const Resource& result,
                   const String& sortfilter);
bool HHVM_FUNCTION(ldap_start_tls,
                   const Resource& link);
bool HHVM_FUNCTION(ldap_unbind,
                   const Resource& link);
bool HHVM_FUNCTION(ldap_get_option,
                   const Resource& link,
                   int option,
                   VRefParam retval);
bool HHVM_FUNCTION(ldap_set_option,
                   const Variant& link,
                   int option,
                   const Variant& newval);
bool HHVM_FUNCTION(ldap_close,
                   const Resource& link);
Variant HHVM_FUNCTION(ldap_list,
                      const Variant& link,
                      const Variant& base_dn,
                      const Variant& filter,
                      const Variant& attributes = null_variant,
                      int attrsonly = 0,
                      int sizelimit = -1,
                      int timelimit = -1,
                      int deref = -1);
Variant HHVM_FUNCTION(ldap_read,
                      const Variant& link,
                      const Variant& base_dn,
                      const Variant& filter,
                      const Variant& attributes = null_variant,
                      int attrsonly = 0,
                      int sizelimit = -1,
                      int timelimit = -1,
                      int deref = -1);
Variant HHVM_FUNCTION(ldap_search,
                      const Variant& link,
                      const Variant& base_dn,
                      const Variant& filter,
                      const Variant& attributes = null_variant,
                      int attrsonly = 0,
                      int sizelimit = -1,
                      int timelimit = -1,
                      int deref = -1);
bool HHVM_FUNCTION(ldap_rename,
                   const Resource& link,
                   const String& dn,
                   const String& newrdn,
                   const String& newparent,
                   bool deleteoldrdn);
bool HHVM_FUNCTION(ldap_delete,
                   const Resource& link,
                   const String& dn);
Variant HHVM_FUNCTION(ldap_compare,
                      const Resource& link,
                      const String& dn,
                      const String& attribute,
                      const String& value);
int64_t HHVM_FUNCTION(ldap_errno,
                      const Resource& link);
String HHVM_FUNCTION(ldap_error,
                     const Resource& link);
Variant HHVM_FUNCTION(ldap_get_dn,
                      const Resource& link,
                      const Resource& result_entry);
int64_t HHVM_FUNCTION(ldap_count_entries,
                      const Resource& link,
                      const Resource& result);
Variant HHVM_FUNCTION(ldap_get_entries,
                      const Resource& link,
                      const Resource& result);
Variant HHVM_FUNCTION(ldap_first_entry,
                      const Resource& link,
                      const Resource& result);
Variant HHVM_FUNCTION(ldap_next_entry,
                      const Resource& link,
                      const Resource& result_entry);
Array HHVM_FUNCTION(ldap_get_attributes,
                    const Resource& link,
                    const Resource& result_entry);
Variant HHVM_FUNCTION(ldap_first_attribute,
                      const Resource& link,
                      const Resource& result_entry);
Variant HHVM_FUNCTION(ldap_next_attribute,
                      const Resource& link,
                      const Resource& result_entry);
Variant HHVM_FUNCTION(ldap_first_reference,
                      const Resource& link,
                      const Resource& result);
Variant HHVM_FUNCTION(ldap_next_reference,
                      const Resource& link,
                      const Resource& result_entry);
bool HHVM_FUNCTION(ldap_parse_reference,
                   const Resource& link,
                   const Resource& result_entry,
                   VRefParam referrals);
bool HHVM_FUNCTION(ldap_parse_result,
                   const Resource& link,
                   const Resource& result,
                   VRefParam errcode,
                   VRefParam matcheddn = uninit_null(),
                   VRefParam errmsg = uninit_null(),
                   VRefParam referrals = uninit_null());
bool HHVM_FUNCTION(ldap_free_result,
                   const Resource& result);
Variant HHVM_FUNCTION(ldap_get_values_len,
                      const Resource& link,
                      const Resource& result_entry,
                      const String& attribute);
Variant HHVM_FUNCTION(ldap_get_values,
                      const Resource& link,
                      const Resource& result_entry,
                      const String& attribute);
bool HHVM_FUNCTION(ldap_control_paged_result,
                   const Resource& link,
                   int pagesize,
                   bool iscritical = false,
                   const String& cookie = empty_string_ref);
bool HHVM_FUNCTION(ldap_control_paged_result_response,
                   const Resource& link,
                   const Resource& result,
                   VRefParam cookie = uninit_null(),
                   VRefParam estimated = uninit_null());
String HHVM_FUNCTION(ldap_escape,
                     const String& value,
                     const String& ignores = empty_string(),
                     int flags = 0);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_LDAP_H_
