<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int MYSQL_ASSOC;
const int MYSQL_BOTH;
const int MYSQL_NUM;
const int MYSQL_CLIENT_COMPRESS;
const int MYSQL_CLIENT_IGNORE_SPACE;
const int MYSQL_CLIENT_INTERACTIVE;
const int MYSQL_CLIENT_SSL;

// These constants are defined in the MySQL extension. They're some int, but
// their values may change with the version of the MySQL library. It seems a
// little strange to make it 0 here, but this file is *only used for type
// checking*, so the value doesn't matter - just that it's some Hack int.
const int MYSQL_CLIENT_CR_UNKNOWN_ERROR;
const int MYSQL_CLIENT_CR_SOCKET_CREATE_ERROR;
const int MYSQL_CLIENT_CR_CONNECTION_ERROR;
const int MYSQL_CLIENT_CR_CONN_HOST_ERROR;
const int MYSQL_CLIENT_CR_IPSOCK_ERROR;
const int MYSQL_CLIENT_CR_UNKNOWN_HOST;
const int MYSQL_CLIENT_CR_SERVER_GONE_ERROR;
const int MYSQL_CLIENT_CR_VERSION_ERROR;
const int MYSQL_CLIENT_CR_OUT_OF_MEMORY;
const int MYSQL_CLIENT_CR_WRONG_HOST_INFO;
const int MYSQL_CLIENT_CR_LOCALHOST_CONNECTION;
const int MYSQL_CLIENT_CR_TCP_CONNECTION;
const int MYSQL_CLIENT_CR_SERVER_HANDSHAKE_ERR;
const int MYSQL_CLIENT_CR_SERVER_LOST;
const int MYSQL_CLIENT_CR_COMMANDS_OUT_OF_SYNC;
const int MYSQL_CLIENT_CR_NAMEDPIPE_CONNECTION;
const int MYSQL_CLIENT_CR_NAMEDPIPEWAIT_ERROR;
const int MYSQL_CLIENT_CR_NAMEDPIPEOPEN_ERROR;
const int MYSQL_CLIENT_CR_NAMEDPIPESETSTATE_ERROR;
const int MYSQL_CLIENT_CR_CANT_READ_CHARSET;
const int MYSQL_CLIENT_CR_NET_PACKET_TOO_LARGE;
const int MYSQL_CLIENT_CR_EMBEDDED_CONNECTION;
const int MYSQL_CLIENT_CR_PROBE_SLAVE_STATUS;
const int MYSQL_CLIENT_CR_PROBE_SLAVE_HOSTS;
const int MYSQL_CLIENT_CR_PROBE_SLAVE_CONNECT;
const int MYSQL_CLIENT_CR_PROBE_MASTER_CONNECT;
const int MYSQL_CLIENT_CR_SSL_CONNECTION_ERROR;
const int MYSQL_CLIENT_CR_MALFORMED_PACKET;
const int MYSQL_CLIENT_CR_WRONG_LICENSE;
const int MYSQL_CLIENT_CR_NULL_POINTER;
const int MYSQL_CLIENT_CR_NO_PREPARE_STMT;
const int MYSQL_CLIENT_CR_PARAMS_NOT_BOUND;
const int MYSQL_CLIENT_CR_DATA_TRUNCATED;
const int MYSQL_CLIENT_CR_NO_PARAMETERS_EXISTS;
const int MYSQL_CLIENT_CR_INVALID_PARAMETER_NO;
const int MYSQL_CLIENT_CR_INVALID_BUFFER_USE;
const int MYSQL_CLIENT_CR_UNSUPPORTED_PARAM_TYPE;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECTION;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_MAP_ERROR;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_FILE_MAP_ERROR;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_MAP_ERROR;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_EVENT_ERROR;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR;
const int MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_SET_ERROR;
const int MYSQL_CLIENT_CR_CONN_UNKNOW_PROTOCOL;
const int MYSQL_CLIENT_CR_INVALID_CONN_HANDLE;
const int MYSQL_CLIENT_CR_UNUSED_1;
const int MYSQL_CLIENT_CR_FETCH_CANCELED;
const int MYSQL_CLIENT_CR_NO_DATA;
const int MYSQL_CLIENT_CR_NO_STMT_METADATA;
const int MYSQL_CLIENT_CR_NO_RESULT_SET;
const int MYSQL_CLIENT_CR_NOT_IMPLEMENTED;
const int MYSQL_CLIENT_CR_SERVER_LOST_EXTENDED;
const int MYSQL_CLIENT_CR_STMT_CLOSED;
const int MYSQL_CLIENT_CR_NEW_STMT_METADATA;
const int MYSQL_CLIENT_CR_ALREADY_CONNECTED;
const int MYSQL_CLIENT_CR_AUTH_PLUGIN_CANNOT_LOAD;
const int MYSQL_CLIENT_CR_DUPLICATE_CONNECTION_ATTR;
const int MYSQL_CLIENT_CR_AUTH_PLUGIN_ERR;
const int MYSQL_CLIENT_CR_INSECURE_API_ERR;
const int MYSQL_CLIENT_CR_FILE_NAME_TOO_LONG;
const int MYSQL_CLIENT_CR_SSL_FIPS_MODE_ERR;
const int MYSQL_CLIENT_CR_COMPRESSION_NOT_SUPPORTED;

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
function mysql_connect(
  string $server = "",
  string $username = "",
  string $password = "",
  bool $new_link = false,
  int $client_flags = 0,
  int $connect_timeout_ms = -1,
  int $query_timeout_ms = -1,
  darray<string, string> $conn_attrs = dict[],
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_pconnect(
  string $server = "",
  string $username = "",
  string $password = "",
  int $client_flags = 0,
  int $connect_timeout_ms = -1,
  int $query_timeout_ms = -1,
  darray<string, string> $conn_attrs = dict[],
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_connect_with_db(
  string $server = "",
  string $username = "",
  string $password = "",
  string $database = "",
  bool $new_link = false,
  int $client_flags = 0,
  int $connect_timeout_ms = -1,
  int $query_timeout_ms = -1,
  darray<string, string> $conn_attrs = dict[],
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_connect_with_ssl(
  string $server = "",
  string $username = "",
  string $password = "",
  string $database = "",
  int $client_flags = 0,
  int $connect_timeout_ms = -1,
  int $query_timeout_ms = -1,
  ?MySSLContextProvider $ssl_context = null,
  darray<string, string> $conn_attrs = dict[],
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_pconnect_with_db(
  string $server = "",
  string $username = "",
  string $password = "",
  string $database = "",
  int $client_flags = 0,
  int $connect_timeout_ms = -1,
  int $query_timeout_ms = -1,
  darray<string, string> $conn_attrs = dict[],
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_set_charset(
  string $charset,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_ping(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_escape_string(
  string $unescaped_string,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_real_escape_string(
  string $unescaped_string,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_client_encoding(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_close(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_errno(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_error(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_warning_count(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_get_client_info(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_get_host_info(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_get_proto_info(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_get_server_info(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_info(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_insert_id(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_stat(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_thread_id(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_create_db(
  string $db,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_select_db(
  string $db,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_drop_db(
  string $db,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_affected_rows(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_set_timeout(
  int $query_timeout_ms = -1,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_query(
  string $query,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_multi_query(
  string $query,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_next_result(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_more_results(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_fetch_result(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_unbuffered_query(
  string $query,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_db_query(
  string $database,
  string $query,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_list_dbs(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_list_tables(
  string $database,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_list_fields(
  string $database_name,
  string $table_name,
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_list_processes(
  HH\FIXME\MISSING_PARAM_TYPE $link_identifier = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_db_name(
  resource $result,
  HH\FIXME\MISSING_PARAM_TYPE $row,
  HH\FIXME\MISSING_PARAM_TYPE $field = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_tablename(
  resource $result,
  int $i,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_num_fields(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_num_rows(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_free_result(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_data_seek(
  resource $result,
  int $row,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_fetch_row(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_fetch_assoc(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_fetch_array(
  resource $result,
  int $result_type = 3,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_fetch_lengths(resource $result): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_fetch_object(
  HH\FIXME\MISSING_PARAM_TYPE $result,
  string $class_name = "stdClass",
  HH\FIXME\MISSING_PARAM_TYPE $params = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_result(
  resource $result,
  int $row,
  HH\FIXME\MISSING_PARAM_TYPE $field = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_fetch_field(
  resource $result,
  int $field = -1,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_field_seek(
  resource $result,
  int $field = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_field_name(
  resource $result,
  int $field = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_field_table(
  resource $result,
  int $field = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_field_len(
  resource $result,
  int $field = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_field_type(
  resource $result,
  int $field = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mysql_field_flags(
  resource $result,
  int $field = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
