<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int STREAM_AWAIT_READ = 1;
const int STREAM_AWAIT_WRITE = 2;
const int STREAM_AWAIT_READ_WRITE = 3;
const int STREAM_AWAIT_ERROR = -1;
const int STREAM_AWAIT_TIMEOUT = 0;
const int STREAM_AWAIT_READY = 1;
const int STREAM_AWAIT_CLOSED = 2;
const int STREAM_CLIENT_CONNECT = 4;
const int STREAM_CLIENT_ASYNC_CONNECT = 2;
const int STREAM_CLIENT_PERSISTENT = 1;
const int STREAM_META_TOUCH = 1;
const int STREAM_META_OWNER_NAME = 2;
const int STREAM_META_OWNER = 3;
const int STREAM_META_GROUP_NAME = 4;
const int STREAM_META_GROUP = 5;
const int STREAM_META_ACCESS = 6;
const int STREAM_BUFFER_NONE = 0; /* unbuffered */
const int STREAM_BUFFER_LINE = 1; /* line buffered */
const int STREAM_BUFFER_FULL = 2; /* fully buffered */
const int STREAM_SERVER_BIND = 4;
const int STREAM_SERVER_LISTEN = 8;
const int STREAM_CRYPTO_METHOD_SSLv23_CLIENT = 7;
const int STREAM_CRYPTO_METHOD_SSLv23_SERVER = 6;
const int STREAM_CRYPTO_METHOD_SSLv2_CLIENT = 3;
const int STREAM_CRYPTO_METHOD_SSLv2_SERVER = 2;
const int STREAM_CRYPTO_METHOD_SSLv3_CLIENT = 5;
const int STREAM_CRYPTO_METHOD_SSLv3_SERVER = 4;
const int STREAM_CRYPTO_METHOD_TLS_CLIENT = 57;
const int STREAM_CRYPTO_METHOD_TLS_SERVER = 56;
const int STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT = 9;
const int STREAM_CRYPTO_METHOD_TLSv1_0_SERVER = 8;
const int STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT = 17;
const int STREAM_CRYPTO_METHOD_TLSv1_1_SERVER = 16;
const int STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT = 33;
const int STREAM_CRYPTO_METHOD_TLSv1_2_SERVER = 32;
const int STREAM_CRYPTO_METHOD_ANY_CLIENT = 63;
const int STREAM_CRYPTO_METHOD_ANY_SERVER = 62;
const int STREAM_ENFORCE_SAFE_MODE = 4;
const int STREAM_IGNORE_URL = 2;
const int STREAM_IPPROTO_ICMP = 1;
const int STREAM_IPPROTO_IP = 0;
const int STREAM_IPPROTO_RAW = 255;
const int STREAM_IPPROTO_TCP = 6;
const int STREAM_IPPROTO_UDP = 17;
const int STREAM_IS_URL = 1;
const int STREAM_MKDIR_RECURSIVE = 1;
const int STREAM_MUST_SEEK = 16;
const int STREAM_NOTIFY_AUTH_REQUIRED = 3;
const int STREAM_NOTIFY_AUTH_RESULT = 10;
const int STREAM_NOTIFY_COMPLETED = 8;
const int STREAM_NOTIFY_CONNECT = 2;
const int STREAM_NOTIFY_FAILURE = 9;
const int STREAM_NOTIFY_FILE_SIZE_IS = 5;
const int STREAM_NOTIFY_MIME_TYPE_IS = 4;
const int STREAM_NOTIFY_PROGRESS = 7;
const int STREAM_NOTIFY_REDIRECTED = 6;
const int STREAM_NOTIFY_RESOLVE = 1;
const int STREAM_NOTIFY_SEVERITY_ERR = 2;
const int STREAM_NOTIFY_SEVERITY_INFO = 0;
const int STREAM_NOTIFY_SEVERITY_WARN = 1;
const int STREAM_OOB = 1;
const int STREAM_PEEK = 2;
const int STREAM_PF_INET = 2;
const int STREAM_PF_INET6 = 10;
const int STREAM_PF_UNIX = 1;
const int STREAM_REPORT_ERRORS = 8;
const int STREAM_SHUT_RD = 0;
const int STREAM_SHUT_RDWR = 2;
const int STREAM_SHUT_WR = 1;
const int STREAM_SOCK_DGRAM = 2;
const int STREAM_SOCK_RAW = 3;
const int STREAM_SOCK_RDM = 4;
const int STREAM_SOCK_SEQPACKET = 5;
const int STREAM_SOCK_STREAM = 1;
const int STREAM_USE_PATH = 1;

<<__PHPStdLib>>
function stream_await(resource $fp, int $events, float $timeout = 0.0): Awaitable<int>;
<<__PHPStdLib>>
function stream_context_create($options = null, $params = null);
<<__PHPStdLib>>
function stream_context_get_default($options = null);
<<__PHPStdLib>>
function stream_context_get_options(resource $stream_or_context);
<<__PHPStdLib>>
function stream_context_set_option($stream_or_context, $wrapper, $option = null, $value = null);
<<__PHPStdLib>>
function stream_context_set_param($stream_or_context, $params);
<<__PHPStdLib>>
function stream_copy_to_stream(resource $source, resource $dest, int $maxlength = -1, int $offset = 0);
<<__PHPStdLib>>
function stream_encoding($stream, $encoding = null);
<<__PHPStdLib>>
function stream_get_contents(resource $handle, int $maxlen = 0, int $offset = 0);
<<__PHPStdLib>>
function stream_get_line(resource $handle, int $length = 0, $ending = null);
<<__PHPStdLib>>
function stream_get_meta_data(resource $stream);
<<__PHPStdLib>>
function stream_get_transports();
<<__PHPStdLib>>
function stream_get_wrappers();
<<__PHPStdLib>>
function stream_resolve_include_path(string $filename, $context = null);
<<__PHPStdLib>>
function stream_select(inout $read, inout $write, inout $except, $vtv_sec, int $tv_usec = 0);
<<__PHPStdLib>>
function stream_set_blocking(resource $stream, bool $mode);
<<__PHPStdLib>>
function stream_set_timeout(resource $stream, int $seconds, int $microseconds = 0);
<<__PHPStdLib>>
function stream_set_write_buffer(resource $stream, int $buffer);
<<__PHPStdLib>>
function set_file_buffer(resource $stream, int $buffer);
<<__PHPStdLib>>
function stream_socket_accept(resource $server_socket, float $timeout, inout $peername);
<<__PHPStdLib>>
function stream_socket_server(string $local_socket, inout $errnum, inout $errstr, int $flags = 0, $context = null);
<<__PHPStdLib>>
function stream_socket_client(string $remote_socket, inout $errnum, inout $errstr, float $timeout = 0.0, int $flags = 0, $context = null);
<<__PHPStdLib>>
function stream_socket_enable_crypto(resource $stream, bool $enable, int $crypto_type = 0, $session_stream = null);
<<__PHPStdLib>>
function stream_socket_get_name(resource $handle, bool $want_peer);
<<__PHPStdLib>>
function stream_socket_pair(int $domain, int $type, int $protocol);
<<__PHPStdLib>>
function stream_socket_recvfrom(resource $socket, int $length, int $flags, inout $address);
<<__PHPStdLib>>
function stream_socket_sendto(resource $socket, string $data, int $flags = 0, $address = null);
<<__PHPStdLib>>
function stream_socket_shutdown(resource $stream, int $how);
