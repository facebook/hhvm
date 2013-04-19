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

TypedValue* fh_mysql_connect(TypedValue* _rv, Value* server, Value* username, Value* password, bool new_link, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP15f_mysql_connectERKNS_6StringES2_S2_biii");

TypedValue* fh_mysql_connect_with_db(TypedValue* _rv, Value* server, Value* username, Value* password, Value* database, bool new_link, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP23f_mysql_connect_with_dbERKNS_6StringES2_S2_S2_biii");

TypedValue* fh_mysql_pconnect(TypedValue* _rv, Value* server, Value* username, Value* password, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP16f_mysql_pconnectERKNS_6StringES2_S2_iii");

TypedValue* fh_mysql_pconnect_with_db(TypedValue* _rv, Value* server, Value* username, Value* password, Value* database, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP24f_mysql_pconnect_with_dbERKNS_6StringES2_S2_S2_iii");

bool fh_mysql_set_timeout(int query_timeout_ms, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_set_timeoutEiRKNS_7VariantE");

Value* fh_mysql_escape_string(Value* _rv, Value* unescaped_string) asm("_ZN4HPHP21f_mysql_escape_stringERKNS_6StringE");

TypedValue* fh_mysql_real_escape_string(TypedValue* _rv, Value* unescaped_string, TypedValue* link_identifier) asm("_ZN4HPHP26f_mysql_real_escape_stringERKNS_6StringERKNS_7VariantE");

Value* fh_mysql_get_client_info(Value* _rv) asm("_ZN4HPHP23f_mysql_get_client_infoEv");

TypedValue* fh_mysql_set_charset(TypedValue* _rv, Value* charset, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_set_charsetERKNS_6StringERKNS_7VariantE");

TypedValue* fh_mysql_ping(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP12f_mysql_pingERKNS_7VariantE");

TypedValue* fh_mysql_client_encoding(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP23f_mysql_client_encodingERKNS_7VariantE");

TypedValue* fh_mysql_close(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_closeERKNS_7VariantE");

TypedValue* fh_mysql_errno(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_errnoERKNS_7VariantE");

TypedValue* fh_mysql_error(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_errorERKNS_7VariantE");

TypedValue* fh_mysql_warning_count(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP21f_mysql_warning_countERKNS_7VariantE");

TypedValue* fh_mysql_get_host_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP21f_mysql_get_host_infoERKNS_7VariantE");

TypedValue* fh_mysql_get_proto_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP22f_mysql_get_proto_infoERKNS_7VariantE");

TypedValue* fh_mysql_get_server_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP23f_mysql_get_server_infoERKNS_7VariantE");

TypedValue* fh_mysql_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP12f_mysql_infoERKNS_7VariantE");

TypedValue* fh_mysql_insert_id(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP17f_mysql_insert_idERKNS_7VariantE");

TypedValue* fh_mysql_stat(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP12f_mysql_statERKNS_7VariantE");

TypedValue* fh_mysql_thread_id(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP17f_mysql_thread_idERKNS_7VariantE");

TypedValue* fh_mysql_create_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP17f_mysql_create_dbERKNS_6StringERKNS_7VariantE");

TypedValue* fh_mysql_select_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP17f_mysql_select_dbERKNS_6StringERKNS_7VariantE");

TypedValue* fh_mysql_drop_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP15f_mysql_drop_dbERKNS_6StringERKNS_7VariantE");

TypedValue* fh_mysql_affected_rows(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP21f_mysql_affected_rowsERKNS_7VariantE");

TypedValue* fh_mysql_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_queryERKNS_6StringERKNS_7VariantE");

TypedValue* fh_mysql_multi_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_multi_queryERKNS_6StringERKNS_7VariantE");

bool fh_mysql_next_result(TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_next_resultERKNS_7VariantE");

bool fh_mysql_more_results(TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_more_resultsERKNS_7VariantE");

TypedValue* fh_mysql_fetch_result(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_fetch_resultERKNS_7VariantE");

TypedValue* fh_mysql_unbuffered_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP24f_mysql_unbuffered_queryERKNS_6StringERKNS_7VariantE");

TypedValue* fh_mysql_db_query(TypedValue* _rv, Value* database, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP16f_mysql_db_queryERKNS_6StringES2_RKNS_7VariantE");

TypedValue* fh_mysql_list_dbs(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP16f_mysql_list_dbsERKNS_7VariantE");

TypedValue* fh_mysql_list_tables(TypedValue* _rv, Value* database, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_list_tablesERKNS_6StringERKNS_7VariantE");

TypedValue* fh_mysql_list_fields(TypedValue* _rv, Value* database_name, Value* table_name, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_list_fieldsERKNS_6StringES2_RKNS_7VariantE");

TypedValue* fh_mysql_list_processes(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP22f_mysql_list_processesERKNS_7VariantE");

bool fh_mysql_data_seek(TypedValue* result, int row) asm("_ZN4HPHP17f_mysql_data_seekERKNS_7VariantEi");

TypedValue* fh_mysql_async_connect_start(TypedValue* _rv, Value* server, Value* username, Value* password, Value* database) asm("_ZN4HPHP27f_mysql_async_connect_startERKNS_6StringES2_S2_S2_");

bool fh_mysql_async_connect_completed(TypedValue* link_identifier) asm("_ZN4HPHP31f_mysql_async_connect_completedERKNS_7VariantE");

bool fh_mysql_async_query_start(Value* query, TypedValue* link_identifier) asm("_ZN4HPHP25f_mysql_async_query_startERKNS_6StringERKNS_7VariantE");

TypedValue* fh_mysql_async_query_result(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP26f_mysql_async_query_resultERKNS_7VariantE");

bool fh_mysql_async_query_completed(TypedValue* result) asm("_ZN4HPHP29f_mysql_async_query_completedERKNS_7VariantE");

TypedValue* fh_mysql_async_fetch_array(TypedValue* _rv, TypedValue* result, int result_type) asm("_ZN4HPHP25f_mysql_async_fetch_arrayERKNS_7VariantEi");

TypedValue* fh_mysql_async_wait_actionable(TypedValue* _rv, TypedValue* items, double timeout) asm("_ZN4HPHP29f_mysql_async_wait_actionableERKNS_7VariantEd");

long fh_mysql_async_status(TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_async_statusERKNS_7VariantE");

TypedValue* fh_mysql_fetch_row(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP17f_mysql_fetch_rowERKNS_7VariantE");

TypedValue* fh_mysql_fetch_assoc(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP19f_mysql_fetch_assocERKNS_7VariantE");

TypedValue* fh_mysql_fetch_array(TypedValue* _rv, TypedValue* result, int result_type) asm("_ZN4HPHP19f_mysql_fetch_arrayERKNS_7VariantEi");

TypedValue* fh_mysql_fetch_object(TypedValue* _rv, TypedValue* result, Value* class_name, Value* params) asm("_ZN4HPHP20f_mysql_fetch_objectERKNS_7VariantERKNS_6StringERKNS_5ArrayE");

TypedValue* fh_mysql_fetch_lengths(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP21f_mysql_fetch_lengthsERKNS_7VariantE");

TypedValue* fh_mysql_result(TypedValue* _rv, TypedValue* result, int row, TypedValue* field) asm("_ZN4HPHP14f_mysql_resultERKNS_7VariantEiS2_");

TypedValue* fh_mysql_db_name(TypedValue* _rv, TypedValue* result, int row, TypedValue* field) asm("_ZN4HPHP15f_mysql_db_nameERKNS_7VariantEiS2_");

TypedValue* fh_mysql_tablename(TypedValue* _rv, TypedValue* result, int i) asm("_ZN4HPHP17f_mysql_tablenameERKNS_7VariantEi");

TypedValue* fh_mysql_num_fields(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP18f_mysql_num_fieldsERKNS_7VariantE");

TypedValue* fh_mysql_num_rows(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP16f_mysql_num_rowsERKNS_7VariantE");

TypedValue* fh_mysql_free_result(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP19f_mysql_free_resultERKNS_7VariantE");

TypedValue* fh_mysql_fetch_field(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_fetch_fieldERKNS_7VariantEi");

bool fh_mysql_field_seek(TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_seekERKNS_7VariantEi");

TypedValue* fh_mysql_field_name(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_nameERKNS_7VariantEi");

TypedValue* fh_mysql_field_table(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_field_tableERKNS_7VariantEi");

TypedValue* fh_mysql_field_len(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP17f_mysql_field_lenERKNS_7VariantEi");

TypedValue* fh_mysql_field_type(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_typeERKNS_7VariantEi");

TypedValue* fh_mysql_field_flags(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_field_flagsERKNS_7VariantEi");

} // namespace HPHP
