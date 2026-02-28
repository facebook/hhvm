<?hh

function test_error_code($value) :mixed{
  print($value."\n");
  invariant($value is int, 'Value is not int');
  invariant($value > 0, 'Value is not greater than 0');
}

<<__EntryPoint>>
function main() :mixed{
  // This test does NOT test the exact values of different MySQL error codes,
  // because these codes can change as we upgrade the MySQL library. However,
  // we do test that the error code constants exist and have non-zero values.

  test_error_code(MYSQL_CLIENT_CR_UNKNOWN_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SOCKET_CREATE_ERROR);
  test_error_code(MYSQL_CLIENT_CR_CONNECTION_ERROR);
  test_error_code(MYSQL_CLIENT_CR_CONN_HOST_ERROR);
  test_error_code(MYSQL_CLIENT_CR_IPSOCK_ERROR);
  test_error_code(MYSQL_CLIENT_CR_UNKNOWN_HOST);
  test_error_code(MYSQL_CLIENT_CR_SERVER_GONE_ERROR);
  test_error_code(MYSQL_CLIENT_CR_VERSION_ERROR);
  test_error_code(MYSQL_CLIENT_CR_OUT_OF_MEMORY);
  test_error_code(MYSQL_CLIENT_CR_WRONG_HOST_INFO);
  test_error_code(MYSQL_CLIENT_CR_LOCALHOST_CONNECTION);
  test_error_code(MYSQL_CLIENT_CR_TCP_CONNECTION);
  test_error_code(MYSQL_CLIENT_CR_SERVER_HANDSHAKE_ERR);
  test_error_code(MYSQL_CLIENT_CR_SERVER_LOST);
  test_error_code(MYSQL_CLIENT_CR_COMMANDS_OUT_OF_SYNC);
  test_error_code(MYSQL_CLIENT_CR_NAMEDPIPE_CONNECTION);
  test_error_code(MYSQL_CLIENT_CR_NAMEDPIPEWAIT_ERROR);
  test_error_code(MYSQL_CLIENT_CR_NAMEDPIPEOPEN_ERROR);
  test_error_code(MYSQL_CLIENT_CR_NAMEDPIPESETSTATE_ERROR);
  test_error_code(MYSQL_CLIENT_CR_CANT_READ_CHARSET);
  test_error_code(MYSQL_CLIENT_CR_NET_PACKET_TOO_LARGE);
  test_error_code(MYSQL_CLIENT_CR_EMBEDDED_CONNECTION);
  test_error_code(MYSQL_CLIENT_CR_PROBE_SLAVE_STATUS);
  test_error_code(MYSQL_CLIENT_CR_PROBE_SLAVE_HOSTS);
  test_error_code(MYSQL_CLIENT_CR_PROBE_SLAVE_CONNECT);
  test_error_code(MYSQL_CLIENT_CR_PROBE_MASTER_CONNECT);
  test_error_code(MYSQL_CLIENT_CR_SSL_CONNECTION_ERROR);
  test_error_code(MYSQL_CLIENT_CR_MALFORMED_PACKET);
  test_error_code(MYSQL_CLIENT_CR_WRONG_LICENSE);
  test_error_code(MYSQL_CLIENT_CR_NULL_POINTER);
  test_error_code(MYSQL_CLIENT_CR_NO_PREPARE_STMT);
  test_error_code(MYSQL_CLIENT_CR_PARAMS_NOT_BOUND);
  test_error_code(MYSQL_CLIENT_CR_DATA_TRUNCATED);
  test_error_code(MYSQL_CLIENT_CR_NO_PARAMETERS_EXISTS);
  test_error_code(MYSQL_CLIENT_CR_INVALID_PARAMETER_NO);
  test_error_code(MYSQL_CLIENT_CR_INVALID_BUFFER_USE);
  test_error_code(MYSQL_CLIENT_CR_UNSUPPORTED_PARAM_TYPE);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECTION);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_MAP_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_FILE_MAP_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_MAP_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_EVENT_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR);
  test_error_code(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_SET_ERROR);
  test_error_code(MYSQL_CLIENT_CR_CONN_UNKNOW_PROTOCOL);
  test_error_code(MYSQL_CLIENT_CR_INVALID_CONN_HANDLE);
  test_error_code(MYSQL_CLIENT_CR_UNUSED_1);
  test_error_code(MYSQL_CLIENT_CR_FETCH_CANCELED);
  test_error_code(MYSQL_CLIENT_CR_NO_DATA);
  test_error_code(MYSQL_CLIENT_CR_NO_STMT_METADATA);
  test_error_code(MYSQL_CLIENT_CR_NO_RESULT_SET);
  test_error_code(MYSQL_CLIENT_CR_NOT_IMPLEMENTED);
  test_error_code(MYSQL_CLIENT_CR_SERVER_LOST_EXTENDED);
  test_error_code(MYSQL_CLIENT_CR_STMT_CLOSED);
  test_error_code(MYSQL_CLIENT_CR_NEW_STMT_METADATA);
  test_error_code(MYSQL_CLIENT_CR_ALREADY_CONNECTED);
  test_error_code(MYSQL_CLIENT_CR_AUTH_PLUGIN_CANNOT_LOAD);
  test_error_code(MYSQL_CLIENT_CR_DUPLICATE_CONNECTION_ATTR);
  test_error_code(MYSQL_CLIENT_CR_AUTH_PLUGIN_ERR);
  test_error_code(MYSQL_CLIENT_CR_INSECURE_API_ERR);
  test_error_code(MYSQL_CLIENT_CR_FILE_NAME_TOO_LONG);
  test_error_code(MYSQL_CLIENT_CR_SSL_FIPS_MODE_ERR);
  test_error_code(MYSQL_CLIENT_CR_COMPRESSION_NOT_SUPPORTED);

  // We can't add the following yet as some builds are not using a new enough MySQL client

  // Added in MySQL 8.0.18
  // test_error_code(MYSQL_CLIENT_CR_COMPRESSION_WRONGLY_CONFIGURED);
  // Added in MySQL 8.0.20
  // test_error_code(MYSQL_CLIENT_CR_KERBEROS_USER_NOT_FOUND);

  print("Done!\n");
}
