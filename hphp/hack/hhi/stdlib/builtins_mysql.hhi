<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int MYSQL_ASSOC = 0;
const int MYSQL_BOTH = 0;
const int MYSQL_NUM = 0;
const int MYSQL_CLIENT_COMPRESS = 32;
const int MYSQL_CLIENT_IGNORE_SPACE = 256;
const int MYSQL_CLIENT_INTERACTIVE = 1024;
const int MYSQL_CLIENT_SSL = 2048;

// Note: mysql_connect() and mysql_pconnect() are upstream functions and should
// not have been modified.  However, the addition of $connect_timeout_ms and
// $query_timeout_ms happened sometime in the past, so the damage is done.
// Since no further damage is possible, I added the $conn_attrs parameter
// instead of creating new functions - jkedgar@fb.com
<<__PHPStdLib>>
function mysql_connect($server = null, $username = null, $password = null, $new_link = false, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1, $conn_attrs = []);
<<__PHPStdLib>>
function mysql_pconnect($server = null, $username = null, $password = null, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1, $conn_attrs = []);
<<__PHPStdLib>>
function mysql_connect_with_db($server = null, $username = null, $password = null, $database = null, $new_link = false, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1, $conn_attrs = []);
<<__PHPStdLib>>
function mysql_connect_with_ssl($server = null, $username = null, $password = null, $database = null, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1, ?MySSLContextProvider $ssl_context = null, $conn_attrs = []);
<<__PHPStdLib>>
function mysql_pconnect_with_db($server = null, $username = null, $password = null, $database = null, $client_flags = 0, $connect_timeout_ms = -1, $query_timeout_ms = -1, $conn_attrs = []);
<<__PHPStdLib>>
function mysql_set_charset($charset, $link_identifier = null);
<<__PHPStdLib>>
function mysql_ping($link_identifier = null);
<<__PHPStdLib>>
function mysql_escape_string($unescaped_string);
<<__PHPStdLib>>
function mysql_real_escape_string($unescaped_string, $link_identifier = null);
<<__PHPStdLib>>
function mysql_client_encoding($link_identifier = null);
<<__PHPStdLib>>
function mysql_close($link_identifier = null);
<<__PHPStdLib>>
function mysql_errno($link_identifier = null);
<<__PHPStdLib>>
function mysql_error($link_identifier = null);
<<__PHPStdLib>>
function mysql_warning_count($link_identifier = null);
<<__PHPStdLib>>
function mysql_get_client_info();
<<__PHPStdLib>>
function mysql_get_host_info($link_identifier = null);
<<__PHPStdLib>>
function mysql_get_proto_info($link_identifier = null);
<<__PHPStdLib>>
function mysql_get_server_info($link_identifier = null);
<<__PHPStdLib>>
function mysql_info($link_identifier = null);
<<__PHPStdLib>>
function mysql_insert_id($link_identifier = null);
<<__PHPStdLib>>
function mysql_stat($link_identifier = null);
<<__PHPStdLib>>
function mysql_thread_id($link_identifier = null);
<<__PHPStdLib>>
function mysql_create_db($db, $link_identifier = null);
<<__PHPStdLib>>
function mysql_select_db($db, $link_identifier = null);
<<__PHPStdLib>>
function mysql_drop_db($db, $link_identifier = null);
<<__PHPStdLib>>
function mysql_affected_rows($link_identifier = null);
<<__PHPStdLib>>
function mysql_set_timeout($query_timeout_ms = -1, $link_identifier = null);
<<__PHPStdLib>>
function mysql_query($query, $link_identifier = null);
<<__PHPStdLib>>
function mysql_multi_query($query, $link_identifier = null);
<<__PHPStdLib>>
function mysql_next_result($link_identifier = null);
<<__PHPStdLib>>
function mysql_more_results($link_identifier = null);
<<__PHPStdLib>>
function mysql_fetch_result($link_identifier = null);
<<__PHPStdLib>>
function mysql_unbuffered_query($query, $link_identifier = null);
<<__PHPStdLib>>
function mysql_db_query($database, $query, $link_identifier = null);
<<__PHPStdLib>>
function mysql_list_dbs($link_identifier = null);
<<__PHPStdLib>>
function mysql_list_tables($database, $link_identifier = null);
<<__PHPStdLib>>
function mysql_list_fields($database_name, $table_name, $link_identifier = null);
<<__PHPStdLib>>
function mysql_list_processes($link_identifier = null);
<<__PHPStdLib>>
function mysql_db_name($result, $row, $field = null);
<<__PHPStdLib>>
function mysql_tablename($result, $i);
<<__PHPStdLib>>
function mysql_num_fields($result);
<<__PHPStdLib>>
function mysql_num_rows($result);
<<__PHPStdLib>>
function mysql_free_result($result);
<<__PHPStdLib>>
function mysql_data_seek($result, $row);
<<__PHPStdLib>>
function mysql_fetch_row($result);
<<__PHPStdLib>>
function mysql_fetch_assoc($result);
<<__PHPStdLib>>
function mysql_fetch_array($result, $result_type = 3);
<<__PHPStdLib>>
function mysql_fetch_lengths($result);
<<__PHPStdLib>>
function mysql_fetch_object($result, $class_name = "stdClass", $params = null);
<<__PHPStdLib>>
function mysql_result($result, $row, $field = null);
<<__PHPStdLib>>
function mysql_fetch_field($result, $field = -1);
<<__PHPStdLib>>
function mysql_field_seek($result, $field = 0);
<<__PHPStdLib>>
function mysql_field_name($result, $field = 0);
<<__PHPStdLib>>
function mysql_field_table($result, $field = 0);
<<__PHPStdLib>>
function mysql_field_len($result, $field = 0);
<<__PHPStdLib>>
function mysql_field_type($result, $field = 0);
<<__PHPStdLib>>
function mysql_field_flags($result, $field = 0);
