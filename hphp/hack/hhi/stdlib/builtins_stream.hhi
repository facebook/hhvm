<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

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

function stream_context_create($options = null, $params = null);
function stream_context_get_default($options = null);
function stream_context_get_options($stream_or_context);
function stream_context_set_option($stream_or_context, $wrapper, $option = null, $value = null);
function stream_context_set_param($stream_or_context, $params);
function stream_copy_to_stream($source, $dest, $maxlength = -1, $offset = 0);
function stream_encoding($stream, $encoding = null);
function stream_bucket_append($brigade, $bucket);
function stream_bucket_prepend($brigade, $bucket);
function stream_bucket_make_writeable($brigade);
function stream_bucket_new($stream, $buffer);
function stream_filter_register($filtername, $classname);
function stream_filter_remove($stream_filter);
function stream_filter_append($stream, $filtername, $read_write = 0, $params = null);
function stream_filter_prepend($stream, $filtername, $read_write = 0, $params = null);
function stream_get_contents($handle, $maxlen = 0, $offset = 0);
function stream_get_filters();
function stream_get_line($handle, $length = 0, $ending = null);
function stream_get_meta_data($stream);
function stream_get_transports();
function stream_get_wrappers();
function stream_register_wrapper($protocol, $classname);
function stream_wrapper_register($protocol, $classname);
function stream_wrapper_restore($protocol);
function stream_wrapper_unregister($protocol);
function stream_resolve_include_path($filename, $context = null);
function stream_select(&$read, &$write, &$except, $vtv_sec, $tv_usec = 0);
function stream_set_blocking($stream, $mode);
function stream_set_timeout($stream, $seconds, $microseconds = 0);
function stream_set_write_buffer($stream, $buffer);
function set_file_buffer($stream, $buffer);
function stream_socket_accept($server_socket, $timeout = 0.0, &$peername = null);
function stream_socket_server($local_socket, &$errnum = null, &$errstr = null, $flags = 0, $context = null);
function stream_socket_client($remote_socket, &$errnum = null, &$errstr = null, $timeout = 0.0, $flags = 0, $context = null);
function stream_socket_enable_crypto($stream, $enable, $crypto_type = 0, $session_stream = null);
function stream_socket_get_name($handle, $want_peer);
function stream_socket_pair($domain, $type, $protocol);
function stream_socket_recvfrom($socket, $length, $flags = 0, $address = null);
function stream_socket_sendto($socket, $data, $flags = 0, $address = null);
function stream_socket_shutdown($stream, $how);
