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

/*
HPHP::Variant HPHP::f_ldap_connect(HPHP::String const&, int)
_ZN4HPHP14f_ldap_connectERKNS_6StringEi

(return value) => rax
_rv => rdi
hostname => rsi
port => rdx
*/

TypedValue* fh_ldap_connect(TypedValue* _rv, Value* hostname, int port) asm("_ZN4HPHP14f_ldap_connectERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_ldap_explode_dn(HPHP::String const&, int)
_ZN4HPHP17f_ldap_explode_dnERKNS_6StringEi

(return value) => rax
_rv => rdi
dn => rsi
with_attrib => rdx
*/

TypedValue* fh_ldap_explode_dn(TypedValue* _rv, Value* dn, int with_attrib) asm("_ZN4HPHP17f_ldap_explode_dnERKNS_6StringEi");

/*
HPHP::Variant HPHP::f_ldap_dn2ufn(HPHP::String const&)
_ZN4HPHP13f_ldap_dn2ufnERKNS_6StringE

(return value) => rax
_rv => rdi
db => rsi
*/

TypedValue* fh_ldap_dn2ufn(TypedValue* _rv, Value* db) asm("_ZN4HPHP13f_ldap_dn2ufnERKNS_6StringE");

/*
HPHP::String HPHP::f_ldap_err2str(int)
_ZN4HPHP14f_ldap_err2strEi

(return value) => rax
_rv => rdi
errnum => rsi
*/

Value* fh_ldap_err2str(Value* _rv, int errnum) asm("_ZN4HPHP14f_ldap_err2strEi");

/*
bool HPHP::f_ldap_add(HPHP::Object const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP10f_ldap_addERKNS_6ObjectERKNS_6StringERKNS_5ArrayE

(return value) => rax
link => rdi
dn => rsi
entry => rdx
*/

bool fh_ldap_add(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP10f_ldap_addERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

/*
bool HPHP::f_ldap_mod_add(HPHP::Object const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP14f_ldap_mod_addERKNS_6ObjectERKNS_6StringERKNS_5ArrayE

(return value) => rax
link => rdi
dn => rsi
entry => rdx
*/

bool fh_ldap_mod_add(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP14f_ldap_mod_addERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

/*
bool HPHP::f_ldap_mod_del(HPHP::Object const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP14f_ldap_mod_delERKNS_6ObjectERKNS_6StringERKNS_5ArrayE

(return value) => rax
link => rdi
dn => rsi
entry => rdx
*/

bool fh_ldap_mod_del(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP14f_ldap_mod_delERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

/*
bool HPHP::f_ldap_mod_replace(HPHP::Object const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP18f_ldap_mod_replaceERKNS_6ObjectERKNS_6StringERKNS_5ArrayE

(return value) => rax
link => rdi
dn => rsi
entry => rdx
*/

bool fh_ldap_mod_replace(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP18f_ldap_mod_replaceERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

/*
bool HPHP::f_ldap_modify(HPHP::Object const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13f_ldap_modifyERKNS_6ObjectERKNS_6StringERKNS_5ArrayE

(return value) => rax
link => rdi
dn => rsi
entry => rdx
*/

bool fh_ldap_modify(Value* link, Value* dn, Value* entry) asm("_ZN4HPHP13f_ldap_modifyERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

/*
bool HPHP::f_ldap_bind(HPHP::Object const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11f_ldap_bindERKNS_6ObjectERKNS_6StringES5_

(return value) => rax
link => rdi
bind_rdn => rsi
bind_password => rdx
*/

bool fh_ldap_bind(Value* link, Value* bind_rdn, Value* bind_password) asm("_ZN4HPHP11f_ldap_bindERKNS_6ObjectERKNS_6StringES5_");

/*
bool HPHP::f_ldap_set_rebind_proc(HPHP::Object const&, HPHP::Variant const&)
_ZN4HPHP22f_ldap_set_rebind_procERKNS_6ObjectERKNS_7VariantE

(return value) => rax
link => rdi
callback => rsi
*/

bool fh_ldap_set_rebind_proc(Value* link, TypedValue* callback) asm("_ZN4HPHP22f_ldap_set_rebind_procERKNS_6ObjectERKNS_7VariantE");

/*
bool HPHP::f_ldap_sort(HPHP::Object const&, HPHP::Object const&, HPHP::String const&)
_ZN4HPHP11f_ldap_sortERKNS_6ObjectES2_RKNS_6StringE

(return value) => rax
link => rdi
result => rsi
sortfilter => rdx
*/

bool fh_ldap_sort(Value* link, Value* result, Value* sortfilter) asm("_ZN4HPHP11f_ldap_sortERKNS_6ObjectES2_RKNS_6StringE");

/*
bool HPHP::f_ldap_start_tls(HPHP::Object const&)
_ZN4HPHP16f_ldap_start_tlsERKNS_6ObjectE

(return value) => rax
link => rdi
*/

bool fh_ldap_start_tls(Value* link) asm("_ZN4HPHP16f_ldap_start_tlsERKNS_6ObjectE");

/*
bool HPHP::f_ldap_unbind(HPHP::Object const&)
_ZN4HPHP13f_ldap_unbindERKNS_6ObjectE

(return value) => rax
link => rdi
*/

bool fh_ldap_unbind(Value* link) asm("_ZN4HPHP13f_ldap_unbindERKNS_6ObjectE");

/*
bool HPHP::f_ldap_get_option(HPHP::Object const&, int, HPHP::VRefParamValue const&)
_ZN4HPHP17f_ldap_get_optionERKNS_6ObjectEiRKNS_14VRefParamValueE

(return value) => rax
link => rdi
option => rsi
retval => rdx
*/

bool fh_ldap_get_option(Value* link, int option, TypedValue* retval) asm("_ZN4HPHP17f_ldap_get_optionERKNS_6ObjectEiRKNS_14VRefParamValueE");

/*
bool HPHP::f_ldap_set_option(HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP17f_ldap_set_optionERKNS_7VariantEiS2_

(return value) => rax
link => rdi
option => rsi
newval => rdx
*/

bool fh_ldap_set_option(TypedValue* link, int option, TypedValue* newval) asm("_ZN4HPHP17f_ldap_set_optionERKNS_7VariantEiS2_");

/*
bool HPHP::f_ldap_close(HPHP::Object const&)
_ZN4HPHP12f_ldap_closeERKNS_6ObjectE

(return value) => rax
link => rdi
*/

bool fh_ldap_close(Value* link) asm("_ZN4HPHP12f_ldap_closeERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_ldap_list(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&, int, int, int, int)
_ZN4HPHP11f_ldap_listERKNS_7VariantES2_S2_RKNS_5ArrayEiiii

(return value) => rax
_rv => rdi
link => rsi
base_dn => rdx
filter => rcx
attributes => r8
attrsonly => r9
sizelimit => st0
timelimit => st8
deref => st16
*/

TypedValue* fh_ldap_list(TypedValue* _rv, TypedValue* link, TypedValue* base_dn, TypedValue* filter, Value* attributes, int attrsonly, int sizelimit, int timelimit, int deref) asm("_ZN4HPHP11f_ldap_listERKNS_7VariantES2_S2_RKNS_5ArrayEiiii");

/*
HPHP::Variant HPHP::f_ldap_read(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&, int, int, int, int)
_ZN4HPHP11f_ldap_readERKNS_7VariantES2_S2_RKNS_5ArrayEiiii

(return value) => rax
_rv => rdi
link => rsi
base_dn => rdx
filter => rcx
attributes => r8
attrsonly => r9
sizelimit => st0
timelimit => st8
deref => st16
*/

TypedValue* fh_ldap_read(TypedValue* _rv, TypedValue* link, TypedValue* base_dn, TypedValue* filter, Value* attributes, int attrsonly, int sizelimit, int timelimit, int deref) asm("_ZN4HPHP11f_ldap_readERKNS_7VariantES2_S2_RKNS_5ArrayEiiii");

/*
HPHP::Variant HPHP::f_ldap_search(HPHP::Variant const&, HPHP::Variant const&, HPHP::Variant const&, HPHP::Array const&, int, int, int, int)
_ZN4HPHP13f_ldap_searchERKNS_7VariantES2_S2_RKNS_5ArrayEiiii

(return value) => rax
_rv => rdi
link => rsi
base_dn => rdx
filter => rcx
attributes => r8
attrsonly => r9
sizelimit => st0
timelimit => st8
deref => st16
*/

TypedValue* fh_ldap_search(TypedValue* _rv, TypedValue* link, TypedValue* base_dn, TypedValue* filter, Value* attributes, int attrsonly, int sizelimit, int timelimit, int deref) asm("_ZN4HPHP13f_ldap_searchERKNS_7VariantES2_S2_RKNS_5ArrayEiiii");

/*
bool HPHP::f_ldap_rename(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, bool)
_ZN4HPHP13f_ldap_renameERKNS_6ObjectERKNS_6StringES5_S5_b

(return value) => rax
link => rdi
dn => rsi
newrdn => rdx
newparent => rcx
deleteoldrdn => r8
*/

bool fh_ldap_rename(Value* link, Value* dn, Value* newrdn, Value* newparent, bool deleteoldrdn) asm("_ZN4HPHP13f_ldap_renameERKNS_6ObjectERKNS_6StringES5_S5_b");

/*
bool HPHP::f_ldap_delete(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP13f_ldap_deleteERKNS_6ObjectERKNS_6StringE

(return value) => rax
link => rdi
dn => rsi
*/

bool fh_ldap_delete(Value* link, Value* dn) asm("_ZN4HPHP13f_ldap_deleteERKNS_6ObjectERKNS_6StringE");

/*
HPHP::Variant HPHP::f_ldap_compare(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_ldap_compareERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
link => rsi
dn => rdx
attribute => rcx
value => r8
*/

TypedValue* fh_ldap_compare(TypedValue* _rv, Value* link, Value* dn, Value* attribute, Value* value) asm("_ZN4HPHP14f_ldap_compareERKNS_6ObjectERKNS_6StringES5_S5_");

/*
long long HPHP::f_ldap_errno(HPHP::Object const&)
_ZN4HPHP12f_ldap_errnoERKNS_6ObjectE

(return value) => rax
link => rdi
*/

long long fh_ldap_errno(Value* link) asm("_ZN4HPHP12f_ldap_errnoERKNS_6ObjectE");

/*
HPHP::String HPHP::f_ldap_error(HPHP::Object const&)
_ZN4HPHP12f_ldap_errorERKNS_6ObjectE

(return value) => rax
_rv => rdi
link => rsi
*/

Value* fh_ldap_error(Value* _rv, Value* link) asm("_ZN4HPHP12f_ldap_errorERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_ldap_get_dn(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP13f_ldap_get_dnERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result_entry => rdx
*/

TypedValue* fh_ldap_get_dn(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP13f_ldap_get_dnERKNS_6ObjectES2_");

/*
long long HPHP::f_ldap_count_entries(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP20f_ldap_count_entriesERKNS_6ObjectES2_

(return value) => rax
link => rdi
result => rsi
*/

long long fh_ldap_count_entries(Value* link, Value* result) asm("_ZN4HPHP20f_ldap_count_entriesERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_ldap_get_entries(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP18f_ldap_get_entriesERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result => rdx
*/

TypedValue* fh_ldap_get_entries(TypedValue* _rv, Value* link, Value* result) asm("_ZN4HPHP18f_ldap_get_entriesERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_ldap_first_entry(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP18f_ldap_first_entryERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result => rdx
*/

TypedValue* fh_ldap_first_entry(TypedValue* _rv, Value* link, Value* result) asm("_ZN4HPHP18f_ldap_first_entryERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_ldap_next_entry(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP17f_ldap_next_entryERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result_entry => rdx
*/

TypedValue* fh_ldap_next_entry(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP17f_ldap_next_entryERKNS_6ObjectES2_");

/*
HPHP::Array HPHP::f_ldap_get_attributes(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_ldap_get_attributesERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result_entry => rdx
*/

Value* fh_ldap_get_attributes(Value* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP21f_ldap_get_attributesERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_ldap_first_attribute(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP22f_ldap_first_attributeERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result_entry => rdx
*/

TypedValue* fh_ldap_first_attribute(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP22f_ldap_first_attributeERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_ldap_next_attribute(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_ldap_next_attributeERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result_entry => rdx
*/

TypedValue* fh_ldap_next_attribute(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP21f_ldap_next_attributeERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_ldap_first_reference(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP22f_ldap_first_referenceERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result => rdx
*/

TypedValue* fh_ldap_first_reference(TypedValue* _rv, Value* link, Value* result) asm("_ZN4HPHP22f_ldap_first_referenceERKNS_6ObjectES2_");

/*
HPHP::Variant HPHP::f_ldap_next_reference(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP21f_ldap_next_referenceERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
link => rsi
result_entry => rdx
*/

TypedValue* fh_ldap_next_reference(TypedValue* _rv, Value* link, Value* result_entry) asm("_ZN4HPHP21f_ldap_next_referenceERKNS_6ObjectES2_");

/*
bool HPHP::f_ldap_parse_reference(HPHP::Object const&, HPHP::Object const&, HPHP::VRefParamValue const&)
_ZN4HPHP22f_ldap_parse_referenceERKNS_6ObjectES2_RKNS_14VRefParamValueE

(return value) => rax
link => rdi
result_entry => rsi
referrals => rdx
*/

bool fh_ldap_parse_reference(Value* link, Value* result_entry, TypedValue* referrals) asm("_ZN4HPHP22f_ldap_parse_referenceERKNS_6ObjectES2_RKNS_14VRefParamValueE");

/*
bool HPHP::f_ldap_parse_result(HPHP::Object const&, HPHP::Object const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP19f_ldap_parse_resultERKNS_6ObjectES2_RKNS_14VRefParamValueES5_S5_S5_

(return value) => rax
link => rdi
result => rsi
errcode => rdx
matcheddn => rcx
errmsg => r8
referrals => r9
*/

bool fh_ldap_parse_result(Value* link, Value* result, TypedValue* errcode, TypedValue* matcheddn, TypedValue* errmsg, TypedValue* referrals) asm("_ZN4HPHP19f_ldap_parse_resultERKNS_6ObjectES2_RKNS_14VRefParamValueES5_S5_S5_");

/*
bool HPHP::f_ldap_free_result(HPHP::Object const&)
_ZN4HPHP18f_ldap_free_resultERKNS_6ObjectE

(return value) => rax
result => rdi
*/

bool fh_ldap_free_result(Value* result) asm("_ZN4HPHP18f_ldap_free_resultERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_ldap_get_values_len(HPHP::Object const&, HPHP::Object const&, HPHP::String const&)
_ZN4HPHP21f_ldap_get_values_lenERKNS_6ObjectES2_RKNS_6StringE

(return value) => rax
_rv => rdi
link => rsi
result_entry => rdx
attribute => rcx
*/

TypedValue* fh_ldap_get_values_len(TypedValue* _rv, Value* link, Value* result_entry, Value* attribute) asm("_ZN4HPHP21f_ldap_get_values_lenERKNS_6ObjectES2_RKNS_6StringE");

/*
HPHP::Variant HPHP::f_ldap_get_values(HPHP::Object const&, HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_ldap_get_valuesERKNS_6ObjectES2_RKNS_6StringE

(return value) => rax
_rv => rdi
link => rsi
result_entry => rdx
attribute => rcx
*/

TypedValue* fh_ldap_get_values(TypedValue* _rv, Value* link, Value* result_entry, Value* attribute) asm("_ZN4HPHP17f_ldap_get_valuesERKNS_6ObjectES2_RKNS_6StringE");


} // !HPHP

