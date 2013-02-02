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
HPHP::Variant HPHP::f_mysql_connect(HPHP::String const&, HPHP::String const&, HPHP::String const&, bool, int, int, int)
_ZN4HPHP15f_mysql_connectERKNS_6StringES2_S2_biii

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
new_link => r8
client_flags => r9
connect_timeout_ms => st0
query_timeout_ms => st8
*/

TypedValue* fh_mysql_connect(TypedValue* _rv, Value* server, Value* username, Value* password, bool new_link, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP15f_mysql_connectERKNS_6StringES2_S2_biii");

/*
HPHP::Variant HPHP::f_mysql_pconnect(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP16f_mysql_pconnectERKNS_6StringES2_S2_iii

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
client_flags => r8
connect_timeout_ms => r9
query_timeout_ms => st0
*/

TypedValue* fh_mysql_pconnect(TypedValue* _rv, Value* server, Value* username, Value* password, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP16f_mysql_pconnectERKNS_6StringES2_S2_iii");

/*
HPHP::Variant HPHP::f_mysql_connect_with_db(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, bool, int, int, int)
_ZN4HPHP23f_mysql_connect_with_dbERKNS_6StringES2_S2_S2_biii

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
database => r8
new_link => r9
client_flags => st0
connect_timeout_ms => st8
query_timeout_ms => st16
*/

TypedValue* fh_mysql_connect_with_db(TypedValue* _rv, Value* server, Value* username, Value* password, Value* database, bool new_link, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP23f_mysql_connect_with_dbERKNS_6StringES2_S2_S2_biii");

/*
HPHP::Variant HPHP::f_mysql_pconnect_with_db(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP24f_mysql_pconnect_with_dbERKNS_6StringES2_S2_S2_iii

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
database => r8
client_flags => r9
connect_timeout_ms => st0
query_timeout_ms => st8
*/

TypedValue* fh_mysql_pconnect_with_db(TypedValue* _rv, Value* server, Value* username, Value* password, Value* database, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP24f_mysql_pconnect_with_dbERKNS_6StringES2_S2_S2_iii");

/*
HPHP::String HPHP::f_mysql_escape_string(HPHP::String const&)
_ZN4HPHP21f_mysql_escape_stringERKNS_6StringE

(return value) => rax
_rv => rdi
unescaped_string => rsi
*/

Value* fh_mysql_escape_string(Value* _rv, Value* unescaped_string) asm("_ZN4HPHP21f_mysql_escape_stringERKNS_6StringE");

/*
HPHP::Variant HPHP::f_mysql_real_escape_string(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP26f_mysql_real_escape_stringERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
unescaped_string => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_real_escape_string(TypedValue* _rv, Value* unescaped_string, TypedValue* link_identifier) asm("_ZN4HPHP26f_mysql_real_escape_stringERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_errno(HPHP::Variant const&)
_ZN4HPHP13f_mysql_errnoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_errno(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_errnoERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_error(HPHP::Variant const&)
_ZN4HPHP13f_mysql_errorERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_error(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_errorERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_warning_count(HPHP::Variant const&)
_ZN4HPHP21f_mysql_warning_countERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_warning_count(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP21f_mysql_warning_countERKNS_7VariantE");

/*
bool HPHP::f_mysql_set_timeout(int, HPHP::Variant const&)
_ZN4HPHP19f_mysql_set_timeoutEiRKNS_7VariantE

(return value) => rax
query_timeout_ms => rdi
link_identifier => rsi
*/

bool fh_mysql_set_timeout(int query_timeout_ms, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_set_timeoutEiRKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_query(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP13f_mysql_queryERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
query => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_queryERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_multi_query(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_mysql_multi_queryERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
query => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_multi_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_multi_queryERKNS_6StringERKNS_7VariantE");

/*
bool HPHP::f_mysql_next_result(HPHP::Variant const&)
_ZN4HPHP19f_mysql_next_resultERKNS_7VariantE

(return value) => rax
link_identifier => rdi
*/

bool fh_mysql_next_result(TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_next_resultERKNS_7VariantE");

/*
bool HPHP::f_mysql_more_results(HPHP::Variant const&)
_ZN4HPHP20f_mysql_more_resultsERKNS_7VariantE

(return value) => rax
link_identifier => rdi
*/

bool fh_mysql_more_results(TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_more_resultsERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_fetch_result(HPHP::Variant const&)
_ZN4HPHP20f_mysql_fetch_resultERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_fetch_result(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_fetch_resultERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_unbuffered_query(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP24f_mysql_unbuffered_queryERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
query => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_unbuffered_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP24f_mysql_unbuffered_queryERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_list_dbs(HPHP::Variant const&)
_ZN4HPHP16f_mysql_list_dbsERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_list_dbs(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP16f_mysql_list_dbsERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_list_tables(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_mysql_list_tablesERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
database => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_list_tables(TypedValue* _rv, Value* database, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_list_tablesERKNS_6StringERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_list_processes(HPHP::Variant const&)
_ZN4HPHP22f_mysql_list_processesERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_list_processes(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP22f_mysql_list_processesERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_num_fields(HPHP::Variant const&)
_ZN4HPHP18f_mysql_num_fieldsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_num_fields(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP18f_mysql_num_fieldsERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_num_rows(HPHP::Variant const&)
_ZN4HPHP16f_mysql_num_rowsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_num_rows(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP16f_mysql_num_rowsERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_free_result(HPHP::Variant const&)
_ZN4HPHP19f_mysql_free_resultERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_free_result(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP19f_mysql_free_resultERKNS_7VariantE");

/*
bool HPHP::f_mysql_data_seek(HPHP::Variant const&, int)
_ZN4HPHP17f_mysql_data_seekERKNS_7VariantEi

(return value) => rax
result => rdi
row => rsi
*/

bool fh_mysql_data_seek(TypedValue* result, int row) asm("_ZN4HPHP17f_mysql_data_seekERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_mysql_fetch_row(HPHP::Variant const&)
_ZN4HPHP17f_mysql_fetch_rowERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_row(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP17f_mysql_fetch_rowERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_fetch_assoc(HPHP::Variant const&)
_ZN4HPHP19f_mysql_fetch_assocERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_assoc(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP19f_mysql_fetch_assocERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_fetch_array(HPHP::Variant const&, int)
_ZN4HPHP19f_mysql_fetch_arrayERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
result_type => rdx
*/

TypedValue* fh_mysql_fetch_array(TypedValue* _rv, TypedValue* result, int result_type) asm("_ZN4HPHP19f_mysql_fetch_arrayERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_mysql_fetch_lengths(HPHP::Variant const&)
_ZN4HPHP21f_mysql_fetch_lengthsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_lengths(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP21f_mysql_fetch_lengthsERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_mysql_fetch_object(HPHP::Variant const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP20f_mysql_fetch_objectERKNS_7VariantERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
result => rsi
class_name => rdx
params => rcx
*/

TypedValue* fh_mysql_fetch_object(TypedValue* _rv, TypedValue* result, Value* class_name, Value* params) asm("_ZN4HPHP20f_mysql_fetch_objectERKNS_7VariantERKNS_6StringERKNS_5ArrayE");

/*
HPHP::Variant HPHP::f_mysql_result(HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP14f_mysql_resultERKNS_7VariantEiS2_

(return value) => rax
_rv => rdi
result => rsi
row => rdx
field => rcx
*/

TypedValue* fh_mysql_result(TypedValue* _rv, TypedValue* result, int row, TypedValue* field) asm("_ZN4HPHP14f_mysql_resultERKNS_7VariantEiS2_");

/*
HPHP::Variant HPHP::f_mysql_fetch_field(HPHP::Variant const&, int)
_ZN4HPHP19f_mysql_fetch_fieldERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_fetch_field(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_fetch_fieldERKNS_7VariantEi");

/*
bool HPHP::f_mysql_field_seek(HPHP::Variant const&, int)
_ZN4HPHP18f_mysql_field_seekERKNS_7VariantEi

(return value) => rax
result => rdi
field => rsi
*/

bool fh_mysql_field_seek(TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_seekERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_mysql_field_name(HPHP::Variant const&, int)
_ZN4HPHP18f_mysql_field_nameERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_name(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_nameERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_mysql_field_table(HPHP::Variant const&, int)
_ZN4HPHP19f_mysql_field_tableERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_table(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_field_tableERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_mysql_field_len(HPHP::Variant const&, int)
_ZN4HPHP17f_mysql_field_lenERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_len(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP17f_mysql_field_lenERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_mysql_field_type(HPHP::Variant const&, int)
_ZN4HPHP18f_mysql_field_typeERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_type(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_typeERKNS_7VariantEi");

/*
HPHP::Variant HPHP::f_mysql_field_flags(HPHP::Variant const&, int)
_ZN4HPHP19f_mysql_field_flagsERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_flags(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_field_flagsERKNS_7VariantEi");


} // !HPHP

