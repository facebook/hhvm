<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int STREAM_AWAIT_READ;
const int STREAM_AWAIT_WRITE;
const int STREAM_AWAIT_READ_WRITE;
const int STREAM_AWAIT_ERROR;
const int STREAM_AWAIT_TIMEOUT;
const int STREAM_AWAIT_READY;
const int STREAM_AWAIT_CLOSED;
const int STREAM_CLIENT_CONNECT;
const int STREAM_CLIENT_ASYNC_CONNECT;
const int STREAM_CLIENT_PERSISTENT;
const int STREAM_META_TOUCH;
const int STREAM_META_OWNER_NAME;
const int STREAM_META_OWNER;
const int STREAM_META_GROUP_NAME;
const int STREAM_META_GROUP;
const int STREAM_META_ACCESS;
const int STREAM_BUFFER_NONE; /* unbuffered */
const int STREAM_BUFFER_LINE; /* line buffered */
const int STREAM_BUFFER_FULL; /* fully buffered */
const int STREAM_SERVER_BIND;
const int STREAM_SERVER_LISTEN;
const int STREAM_CRYPTO_METHOD_SSLv23_CLIENT;
const int STREAM_CRYPTO_METHOD_SSLv23_SERVER;
const int STREAM_CRYPTO_METHOD_SSLv2_CLIENT;
const int STREAM_CRYPTO_METHOD_SSLv2_SERVER;
const int STREAM_CRYPTO_METHOD_SSLv3_CLIENT;
const int STREAM_CRYPTO_METHOD_SSLv3_SERVER;
const int STREAM_CRYPTO_METHOD_TLS_CLIENT;
const int STREAM_CRYPTO_METHOD_TLS_SERVER;
const int STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT;
const int STREAM_CRYPTO_METHOD_TLSv1_0_SERVER;
const int STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT;
const int STREAM_CRYPTO_METHOD_TLSv1_1_SERVER;
const int STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT;
const int STREAM_CRYPTO_METHOD_TLSv1_2_SERVER;
const int STREAM_CRYPTO_METHOD_ANY_CLIENT;
const int STREAM_CRYPTO_METHOD_ANY_SERVER;
const int STREAM_ENFORCE_SAFE_MODE;
const int STREAM_IGNORE_URL;
const int STREAM_IPPROTO_ICMP;
const int STREAM_IPPROTO_IP;
const int STREAM_IPPROTO_RAW;
const int STREAM_IPPROTO_TCP;
const int STREAM_IPPROTO_UDP;
const int STREAM_IS_URL;
const int STREAM_MKDIR_RECURSIVE;
const int STREAM_MUST_SEEK;
const int STREAM_NOTIFY_AUTH_REQUIRED;
const int STREAM_NOTIFY_AUTH_RESULT;
const int STREAM_NOTIFY_COMPLETED;
const int STREAM_NOTIFY_CONNECT;
const int STREAM_NOTIFY_FAILURE;
const int STREAM_NOTIFY_FILE_SIZE_IS;
const int STREAM_NOTIFY_MIME_TYPE_IS;
const int STREAM_NOTIFY_PROGRESS;
const int STREAM_NOTIFY_REDIRECTED;
const int STREAM_NOTIFY_RESOLVE;
const int STREAM_NOTIFY_SEVERITY_ERR;
const int STREAM_NOTIFY_SEVERITY_INFO;
const int STREAM_NOTIFY_SEVERITY_WARN;
const int STREAM_OOB;
const int STREAM_PEEK;
const int STREAM_PF_INET;
const int STREAM_PF_INET6;
const int STREAM_PF_UNIX;
const int STREAM_REPORT_ERRORS;
const int STREAM_SHUT_RD;
const int STREAM_SHUT_RDWR;
const int STREAM_SHUT_WR;
const int STREAM_SOCK_DGRAM;
const int STREAM_SOCK_RAW;
const int STREAM_SOCK_RDM;
const int STREAM_SOCK_SEQPACKET;
const int STREAM_SOCK_STREAM;
const int STREAM_USE_PATH;

<<__PHPStdLib>>
function stream_await(
  resource $fp,
  int $events,
  float $timeout = 0.0,
): Awaitable<int>;
<<__PHPStdLib>>
function stream_context_create(
  $options = null,
  $params = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_context_get_default(
  $options = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_context_get_options(
  resource $stream_or_context,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_context_set_option(
  $stream_or_context,
  $wrapper,
  $option = null,
  $value = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_context_set_param(
  $stream_or_context,
  $params,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_copy_to_stream(
  resource $source,
  resource $dest,
  int $maxlength = -1,
  int $offset = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_encoding(
  $stream,
  $encoding = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_get_contents(
  resource $handle,
  int $maxlen = 0,
  int $offset = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_get_line(
  resource $handle,
  int $length = 0,
  $ending = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_get_meta_data(resource $stream)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_get_transports(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_get_wrappers(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_resolve_include_path(
  string $filename,
  $context = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_select(
  inout $read,
  inout $write,
  inout $except,
  $vtv_sec,
  int $tv_usec = 0,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_set_blocking(
  resource $stream,
  bool $mode,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_set_timeout(
  resource $stream,
  int $seconds,
  int $microseconds = 0,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_set_write_buffer(
  resource $stream,
  int $buffer,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function set_file_buffer(
  resource $stream,
  int $buffer,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_accept(
  resource $server_socket,
  float $timeout,
  inout $peername,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_server(
  string $local_socket,
  inout $errnum,
  inout $errstr,
  int $flags = 0,
  $context = null,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_client(
  string $remote_socket,
  inout $errnum,
  inout $errstr,
  float $timeout = 0.0,
  int $flags = 0,
  $context = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_enable_crypto(
  resource $stream,
  bool $enable,
  int $crypto_type = 0,
  $session_stream = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_get_name(
  resource $handle,
  bool $want_peer,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_pair(
  int $domain,
  int $type,
  int $protocol,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_recvfrom(
  resource $socket,
  int $length,
  int $flags,
  inout $address,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_sendto(
  resource $socket,
  string $data,
  int $flags = 0,
  $address = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function stream_socket_shutdown(
  resource $stream,
  int $how,
): HH\FIXME\MISSING_RETURN_TYPE;
