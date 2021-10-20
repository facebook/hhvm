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

// These constants are defined in the MySQL extension. They're some int, but
// their values may change with the version of the MySQL library. It seems a
// little strange to make it 0 here, but this file is *only used for type
// checking*, so the value doesn't matter - just that it's some Hack int.
const int MYSQL_CLIENT_CR_UNKNOWN_ERROR = 0;
const int MYSQL_CLIENT_CR_SOCKET_CREATE_ERROR = 0;
const int MYSQL_CLIENT_CR_CONNECTION_ERROR = 0;
const int MYSQL_CLIENT_CR_CONN_HOST_ERROR = 0;
const int MYSQL_CLIENT_CR_IPSOCK_ERROR = 0;
const int MYSQL_CLIENT_CR_UNKNOWN_HOST = 0;
const int MYSQL_CLIENT_CR_SERVER_GONE_ERROR = 0;
const int MYSQL_CLIENT_CR_VERSION_ERROR = 0;
const int MYSQL_CLIENT_CR_OUT_OF_MEMORY = 0;
const int MYSQL_CLIENT_CR_WRONG_HOST_INFO = 0;
const int MYSQL_CLIENT_CR_LOCALHOST_CONNECTION = 0;
const int MYSQL_CLIENT_CR_TCP_CONNECTION = 0;
const int MYSQL_CLIENT_CR_SERVER_HANDSHAKE_ERR = 0;
const int MYSQL_CLIENT_CR_SERVER_LOST = 0;
const int MYSQL_CLIENT_CR_COMMANDS_OUT_OF_SYNC = 0;
const int MYSQL_CLIENT_CR_NAMEDPIPE_CONNECTION = 0;
const int MYSQL_CLIENT_CR_NAMEDPIPEWAIT_ERROR = 0;
const int MYSQL_CLIENT_CR_NAMEDPIPEOPEN_ERROR = 0;
const int MYSQL_CLIENT_CR_NAMEDPIPESETSTATE_ERROR = 0;
const int MYSQL_CLIENT_CR_CANT_READ_CHARSET = 0;
const int MYSQL_CLIENT_CR_NET_PACKET_TOO_LARGE = 0;
const int MYSQL_CLIENT_CR_EMBEDDED_CONNECTION = 0;
const int MYSQL_CLIENT_CR_PROBE_SLAVE_STATUS = 0;
const int MYSQL_CLIENT_CR_PROBE_SLAVE_HOSTS = 0;
const int MYSQL_CLIENT_CR_PROBE_SLAVE_CONNECT = 0;
const int MYSQL_CLIENT_CR_PROBE_MASTER_CONNECT = 0;
const int MYSQL_CLIENT_CR_SSL_CONNECTION_ERROR = 0;
const int MYSQL_CLIENT_CR_MALFORMED_PACKET = 0;
const int MYSQL_CLIENT_CR_WRONG_LICENSE = 0;
const int MYSQL_CLIENT_CR_NULL_POINTER = 0;
const int MYSQL_CLIENT_CR_NO_PREPARE_STMT = 0;
const int MYSQL_CLIENT_CR_PARAMS_NOT_BOUND = 0;
const int MYSQL_CLIENT_CR_DATA_TRUNCATED = 0;
const int MYSQL_CLIENT_CR_NO_PARAMETERS_EXISTS = 0;
const int MYSQL_CLIENT_CR_INVALID_PARAMETER_NO = 0;
const int MYSQL_CLIENT_CR_INVALID_BUFFER_USE = 0;
const int MYSQL_CLIENT_CR_UNSUPPORTED_PARAM_TYPE = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECTION = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_MAP_ERROR = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_FILE_MAP_ERROR = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_MAP_ERROR = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_EVENT_ERROR = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR = 0;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_SET_ERROR = 0;
const int MYSQL_CLIENT_CR_CONN_UNKNOW_PROTOCOL = 0;
const int MYSQL_CLIENT_CR_INVALID_CONN_HANDLE = 0;
const int MYSQL_CLIENT_CR_UNUSED_1 = 0;
const int MYSQL_CLIENT_CR_FETCH_CANCELED = 0;
const int MYSQL_CLIENT_CR_NO_DATA = 0;
const int MYSQL_CLIENT_CR_NO_STMT_METADATA = 0;
const int MYSQL_CLIENT_CR_NO_RESULT_SET = 0;
const int MYSQL_CLIENT_CR_NOT_IMPLEMENTED = 0;
const int MYSQL_CLIENT_CR_SERVER_LOST_EXTENDED = 0;
const int MYSQL_CLIENT_CR_STMT_CLOSED = 0;
const int MYSQL_CLIENT_CR_NEW_STMT_METADATA = 0;
const int MYSQL_CLIENT_CR_ALREADY_CONNECTED = 0;
const int MYSQL_CLIENT_CR_AUTH_PLUGIN_CANNOT_LOAD = 0;
const int MYSQL_CLIENT_CR_DUPLICATE_CONNECTION_ATTR = 0;
const int MYSQL_CLIENT_CR_AUTH_PLUGIN_ERR = 0;
const int MYSQL_CLIENT_CR_INSECURE_API_ERR = 0;
const int MYSQL_CLIENT_CR_FILE_NAME_TOO_LONG = 0;
const int MYSQL_CLIENT_CR_SSL_FIPS_MODE_ERR = 0;
const int MYSQL_CLIENT_CR_COMPRESSION_NOT_SUPPORTED = 0;

// We can't add the following yet as some builds are not using a new enough MySQL client

// Added in MySQL 8.0.18
// const int MYSQL_CLIENT_CR_COMPRESSION_WRONGLY_CONFIGURED = 0;
// Added in MySQL 8.0.20
// const int MYSQL_CLIENT_CR_KERBEROS_USER_NOT_FOUND = 0;;

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
