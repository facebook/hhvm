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
function mysql_connect(string $server = "", string $username = "", string $password = "", bool $new_link = false, int $client_flags = 0, int $connect_timeout_ms = -1, int $query_timeout_ms = -1, darray<string, string> $conn_attrs = darray[]);
<<__PHPStdLib>>
function mysql_pconnect(string $server = "", string $username = "", string $password = "", int $client_flags = 0, int $connect_timeout_ms = -1, int $query_timeout_ms = -1, darray<string, string> $conn_attrs = darray[]);
<<__PHPStdLib>>
function mysql_connect_with_db(string $server = "", string $username = "", string $password = "", string $database = "", bool $new_link = false, int $client_flags = 0, int $connect_timeout_ms = -1, int $query_timeout_ms = -1, darray<string, string> $conn_attrs = darray[]);
<<__PHPStdLib>>
function mysql_connect_with_ssl(string $server = "", string $username = "", string $password = "", string $database = "", int $client_flags = 0, int $connect_timeout_ms = -1, int $query_timeout_ms = -1, ?MySSLContextProvider $ssl_context = null, darray<string, string> $conn_attrs = darray[]);
<<__PHPStdLib>>
function mysql_pconnect_with_db(string $server = "", string $username = "", string $password = "", string $database = "", int $client_flags = 0, int $connect_timeout_ms = -1, int $query_timeout_ms = -1, darray<string, string> $conn_attrs = darray[]);
<<__PHPStdLib>>
function mysql_set_charset(string $charset, $link_identifier = null);
<<__PHPStdLib>>
function mysql_ping($link_identifier = null);
<<__PHPStdLib>>
function mysql_escape_string(string $unescaped_string);
<<__PHPStdLib>>
function mysql_real_escape_string(string $unescaped_string, $link_identifier = null);
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
function mysql_create_db(string $db, $link_identifier = null);
<<__PHPStdLib>>
function mysql_select_db(string $db, $link_identifier = null);
<<__PHPStdLib>>
function mysql_drop_db(string $db, $link_identifier = null);
<<__PHPStdLib>>
function mysql_affected_rows($link_identifier = null);
<<__PHPStdLib>>
function mysql_set_timeout(int $query_timeout_ms = -1, $link_identifier = null);
<<__PHPStdLib>>
function mysql_query(string $query, $link_identifier = null);
<<__PHPStdLib>>
function mysql_multi_query(string $query, $link_identifier = null);
<<__PHPStdLib>>
function mysql_next_result($link_identifier = null);
<<__PHPStdLib>>
function mysql_more_results($link_identifier = null);
<<__PHPStdLib>>
function mysql_fetch_result($link_identifier = null);
<<__PHPStdLib>>
function mysql_unbuffered_query(string $query, $link_identifier = null);
<<__PHPStdLib>>
function mysql_db_query(string $database, string $query, $link_identifier = null);
<<__PHPStdLib>>
function mysql_list_dbs($link_identifier = null);
<<__PHPStdLib>>
function mysql_list_tables(string $database, $link_identifier = null);
<<__PHPStdLib>>
function mysql_list_fields(string $database_name, string $table_name, $link_identifier = null);
<<__PHPStdLib>>
function mysql_list_processes($link_identifier = null);
<<__PHPStdLib>>
function mysql_db_name(resource $result, $row, $field = null);
<<__PHPStdLib>>
function mysql_tablename(resource $result, int $i);
<<__PHPStdLib>>
function mysql_num_fields(resource $result);
<<__PHPStdLib>>
function mysql_num_rows(resource $result);
<<__PHPStdLib>>
function mysql_free_result(resource $result);
<<__PHPStdLib>>
function mysql_data_seek(resource $result, int $row);
<<__PHPStdLib>>
function mysql_fetch_row(resource $result);
<<__PHPStdLib>>
function mysql_fetch_assoc(resource $result);
<<__PHPStdLib>>
function mysql_fetch_array(resource $result, int $result_type = 3);
<<__PHPStdLib>>
function mysql_fetch_lengths(resource $result);
<<__PHPStdLib>>
function mysql_fetch_object($result, string $class_name = "stdClass", $params = null);
<<__PHPStdLib>>
function mysql_result(resource $result, int $row, $field = null);
<<__PHPStdLib>>
function mysql_fetch_field(resource $result, int $field = -1);
<<__PHPStdLib>>
function mysql_field_seek(resource $result, int $field = 0);
<<__PHPStdLib>>
function mysql_field_name(resource $result, int $field = 0);
<<__PHPStdLib>>
function mysql_field_table(resource $result, int $field = 0);
<<__PHPStdLib>>
function mysql_field_len(resource $result, int $field = 0);
<<__PHPStdLib>>
function mysql_field_type(resource $result, int $field = 0);
<<__PHPStdLib>>
function mysql_field_flags(resource $result, int $field = 0);
