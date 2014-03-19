<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
define('ASYNC_OP_INVALID', 0);
define('ASYNC_OP_UNSET', 0);
define('ASYNC_OP_CONNECT', 0);
define('ASYNC_OP_QUERY', 0);
define('ASYNC_OP_FETCH_ROW', 0);
function mysql_connect($server = null, $username = null, $password = null, $new_link = false, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1) { }
function mysql_async_connect_start($server = null, $username = null, $password = null, $database = null) { }
function mysql_async_connect_completed($link_identifier) { }
function mysql_async_query_start($query, $link_identifier) { }
function mysql_async_query_result($link_identifier) { }
function mysql_async_query_completed($result) { }
function mysql_async_fetch_array($result, $result_type = 1) { }
function mysql_async_wait_actionable($items, $timeout) { }
function mysql_async_status($link_identifier) { }
function mysql_pconnect($server = null, $username = null, $password = null, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1) { }
function mysql_connect_with_db($server = null, $username = null, $password = null, $database = null, $new_link = false, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1) { }
function mysql_pconnect_with_db($server = null, $username = null, $password = null, $database = null, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1) { }
function mysql_set_charset($charset, $link_identifier = null) { }
function mysql_ping($link_identifier = null) { }
function mysql_escape_string($unescaped_string) { }
function mysql_real_escape_string($unescaped_string, $link_identifier = null) { }
function mysql_client_encoding($link_identifier = null) { }
function mysql_close($link_identifier = null) { }
function mysql_errno($link_identifier = null) { }
function mysql_error($link_identifier = null) { }
function mysql_warning_count($link_identifier = null) { }
function mysql_get_client_info() { }
function mysql_get_host_info($link_identifier = null) { }
function mysql_get_proto_info($link_identifier = null) { }
function mysql_get_server_info($link_identifier = null) { }
function mysql_info($link_identifier = null) { }
function mysql_insert_id($link_identifier = null) { }
function mysql_stat($link_identifier = null) { }
function mysql_thread_id($link_identifier = null) { }
function mysql_create_db($db, $link_identifier = null) { }
function mysql_select_db($db, $link_identifier = null) { }
function mysql_drop_db($db, $link_identifier = null) { }
function mysql_affected_rows($link_identifier = null) { }
function mysql_set_timeout($query_timeout_ms = -1, $link_identifier = null) { }
function mysql_query($query, $link_identifier = null) { }
function mysql_multi_query($query, $link_identifier = null) { }
function mysql_next_result($link_identifier = null) { }
function mysql_more_results($link_identifier = null) { }
function mysql_fetch_result($link_identifier = null) { }
function mysql_unbuffered_query($query, $link_identifier = null) { }
function mysql_db_query($database, $query, $link_identifier = null) { }
function mysql_list_dbs($link_identifier = null) { }
function mysql_list_tables($database, $link_identifier = null) { }
function mysql_list_fields($database_name, $table_name, $link_identifier = null) { }
function mysql_list_processes($link_identifier = null) { }
function mysql_db_name($result, $row, $field = null_variant) { }
function mysql_tablename($result, $i) { }
function mysql_num_fields($result) { }
function mysql_num_rows($result) { }
function mysql_free_result($result) { }
function mysql_data_seek($result, $row) { }
function mysql_fetch_row($result) { }
function mysql_fetch_assoc($result) { }
function mysql_fetch_array($result, $result_type = 3) { }
function mysql_fetch_lengths($result) { }
function mysql_fetch_object($result, $class_name = "stdClass", $params = null) { }
function mysql_result($result, $row, $field = null_variant) { }
function mysql_fetch_field($result, $field = -1) { }
function mysql_field_seek($result, $field = 0) { }
function mysql_field_name($result, $field = 0) { }
function mysql_field_table($result, $field = 0) { }
function mysql_field_len($result, $field = 0) { }
function mysql_field_type($result, $field = 0) { }
function mysql_field_flags($result, $field = 0) { }
