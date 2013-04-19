/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
namespace HPHP {

TypedValue* fh_ldap_connect(TypedValue* _rv, Value* hostname, int port) asm("_ZN4HPHP14f_ldap_connectERKNS_6StringEi");

TypedValue* fh_ldap_explode_dn(TypedValue* _rv, Value* dn, int with_attrib) asm("_ZN4HPHP17f_ldap_explode_dnERKNS_6StringEi");

TypedValue* fh_ldap_dn2ufn(TypedValue* _rv, Value* db) asm("_ZN4HPHP13f_ldap_dn2ufnERKNS_6StringE");

Value* fh_ldap_err2str(Value* _rv, int errnum) asm("_ZN4HPHP14f_ldap_err2strEi");

bool fh_ldap_add(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP10f_ldap_addERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

bool fh_ldap_mod_add(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP14f_ldap_mod_addERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

bool fh_ldap_mod_del(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP14f_ldap_mod_delERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

bool fh_ldap_mod_replace(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP18f_ldap_mod_replaceERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

bool fh_ldap_modify(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP13f_ldap_modifyERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

bool fh_ldap_bind(Value* link, Value* bind_rdn, Value* bind_password) asm("_ZN4HPHP11f_ldap_bindERKNS_6ObjectERKNS_6StringES5_");

bool fh_ldap_set_rebind_proc(Value* link, TypedValue* callback) asm("_ZN4HPHP22f_ldap_set_rebind_procERKNS_6ObjectERKNS_7VariantE");

bool fh_ldap_sort(Value* link, Value* result, Value* sortfilter) asm("_ZN4HPHP11f_ldap_sortERKNS_6ObjectES2_RKNS_6StringE");

bool fh_ldap_start_tls(Value* link) asm("_ZN4HPHP16f_ldap_start_tlsERKNS_6ObjectE");

bool fh_ldap_unbind(Value* link) asm("_ZN4HPHP13f_ldap_unbindERKNS_6ObjectE");

bool fh_ldap_get_option(Value* link, int option, TypedValue* retval) asm("_ZN4HPHP17f_ldap_get_optionERKNS_6ObjectEiRKNS_14VRefParamValueE");

bool fh_ldap_set_option(TypedValue* link, int option, TypedValue* newval) asm("_ZN4HPHP17f_ldap_set_optionERKNS_7VariantEiS2_");

bool fh_ldap_close(Value* link) asm("_ZN4HPHP12f_ldap_closeERKNS_6ObjectE");

TypedValue* fh_ldap_list(TypedValue* _rv, TypedValue* link, TypedValue* base_dn, TypedValue* filter, Value* attributes, int attrsonly, int sizelimit, int timelimit, int deref) asm("_ZN4HPHP11f_ldap_listERKNS_7VariantES2_S2_RKNS_5ArrayEiiii");

TypedValue* fh_ldap_read(TypedValue* _rv, TypedValue* link, TypedValue* base_dn, TypedValue* filter, Value* attributes, int attrsonly, int sizelimit, int timelimit, int deref) asm("_ZN4HPHP11f_ldap_readERKNS_7VariantES2_S2_RKNS_5ArrayEiiii");

TypedValue* fh_ldap_search(TypedValue* _rv, TypedValue* link, TypedValue* base_dn, TypedValue* filter, Value* attributes, int attrsonly, int sizelimit, int timelimit, int deref) asm("_ZN4HPHP13f_ldap_searchERKNS_7VariantES2_S2_RKNS_5ArrayEiiii");

bool fh_ldap_rename(Value* link, Value* dn, Value* newrdn, Value* newparent, bool deleteoldrdn) asm("_ZN4HPHP13f_ldap_renameERKNS_6ObjectERKNS_6StringES5_S5_b");

bool fh_ldap_delete(Value* link, Value* dn) asm("_ZN4HPHP13f_ldap_deleteERKNS_6ObjectERKNS_6StringE");

TypedValue* fh_ldap_compare(TypedValue* _rv, Value* link, Value* dn, Value* attribute, Value* value) asm("_ZN4HPHP14f_ldap_compareERKNS_6ObjectERKNS_6StringES5_S5_");

long fh_ldap_errno(Value* link) asm("_ZN4HPHP12f_ldap_errnoERKNS_6ObjectE");

Value* fh_ldap_error(Value* _rv, Value* link) asm("_ZN4HPHP12f_ldap_errorERKNS_6ObjectE");

TypedValue* fh_ldap_get_dn(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP13f_ldap_get_dnERKNS_6ObjectES2_");

long fh_ldap_count_entries(Value* link, Value* result) asm("_ZN4HPHP20f_ldap_count_entriesERKNS_6ObjectES2_");

TypedValue* fh_ldap_get_entries(TypedValue* _rv, Value* link, Value* result) asm("_ZN4HPHP18f_ldap_get_entriesERKNS_6ObjectES2_");

TypedValue* fh_ldap_first_entry(TypedValue* _rv, Value* link, Value* result) asm("_ZN4HPHP18f_ldap_first_entryERKNS_6ObjectES2_");

TypedValue* fh_ldap_next_entry(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP17f_ldap_next_entryERKNS_6ObjectES2_");

Value* fh_ldap_get_attributes(Value* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP21f_ldap_get_attributesERKNS_6ObjectES2_");

TypedValue* fh_ldap_first_attribute(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP22f_ldap_first_attributeERKNS_6ObjectES2_");

TypedValue* fh_ldap_next_attribute(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP21f_ldap_next_attributeERKNS_6ObjectES2_");

TypedValue* fh_ldap_first_reference(TypedValue* _rv, Value* link, Value* result) asm("_ZN4HPHP22f_ldap_first_referenceERKNS_6ObjectES2_");

TypedValue* fh_ldap_next_reference(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP21f_ldap_next_referenceERKNS_6ObjectES2_");

bool fh_ldap_parse_reference(Value* link, Value* result_entry, TypedValue* referrals) asm("_ZN4HPHP22f_ldap_parse_referenceERKNS_6ObjectES2_RKNS_14VRefParamValueE");

bool fh_ldap_parse_result(Value* link, Value* result, TypedValue* errcode, TypedValue* matcheddn, TypedValue* errmsg, TypedValue* referrals) asm("_ZN4HPHP19f_ldap_parse_resultERKNS_6ObjectES2_RKNS_14VRefParamValueES5_S5_S5_");

bool fh_ldap_free_result(Value* result) asm("_ZN4HPHP18f_ldap_free_resultERKNS_6ObjectE");

TypedValue* fh_ldap_get_values_len(TypedValue* _rv, Value* link, Value* result_entry, Value* attribute) asm("_ZN4HPHP21f_ldap_get_values_lenERKNS_6ObjectES2_RKNS_6StringE");

TypedValue* fh_ldap_get_values(TypedValue* _rv, Value* link, Value* result_entry, Value* attribute) asm("_ZN4HPHP17f_ldap_get_valuesERKNS_6ObjectES2_RKNS_6StringE");

} // namespace HPHP
