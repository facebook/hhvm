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
function stream_context_create($options = null, $params = null) { }
function stream_context_get_default($options = null) { }
function stream_context_get_options($stream_or_context) { }
function stream_context_set_option($stream_or_context, $wrapper, $option = null, $value = null_variant) { }
function stream_context_set_param($stream_or_context, $params) { }
function stream_copy_to_stream($source, $dest, $maxlength = -1, $offset = 0) { }
function stream_encoding($stream, $encoding = null) { }
function stream_bucket_append($brigade, $bucket) { }
function stream_bucket_prepend($brigade, $bucket) { }
function stream_bucket_make_writeable($brigade) { }
function stream_bucket_new($stream, $buffer) { }
function stream_filter_register($filtername, $classname) { }
function stream_filter_remove($stream_filter) { }
function stream_filter_append($stream, $filtername, $read_write = 0, $params = null_variant) { }
function stream_filter_prepend($stream, $filtername, $read_write = 0, $params = null_variant) { }
function stream_get_contents($handle, $maxlen = 0, $offset = 0) { }
function stream_get_filters() { }
function stream_get_line($handle, $length = 0, $ending = null) { }
function stream_get_meta_data($stream) { }
function stream_get_transports() { }
function stream_get_wrappers() { }
function stream_register_wrapper($protocol, $classname) { }
function stream_wrapper_register($protocol, $classname) { }
function stream_wrapper_restore($protocol) { }
function stream_wrapper_unregister($protocol) { }
function stream_resolve_include_path($filename, $context = null_object) { }
function stream_select(&$read, &$write, &$except, $vtv_sec, $tv_usec = 0) { }
function stream_set_blocking($stream, $mode) { }
function stream_set_timeout($stream, $seconds, $microseconds = 0) { }
function stream_set_write_buffer($stream, $buffer) { }
function set_file_buffer($stream, $buffer) { }
function stream_socket_accept($server_socket, $timeout = 0.0, &$peername = null) { }
function stream_socket_server($local_socket, &$errnum = null, &$errstr = null, $flags = 0, $context = null_object) { }
function stream_socket_client($remote_socket, &$errnum = null, &$errstr = null, $timeout = 0.0, $flags = 0, $context = null_object) { }
function stream_socket_enable_crypto($stream, $enable, $crypto_type = 0, $session_stream = null_object) { }
function stream_socket_get_name($handle, $want_peer) { }
function stream_socket_pair($domain, $type, $protocol) { }
function stream_socket_recvfrom($socket, $length, $flags = 0, $address = null) { }
function stream_socket_sendto($socket, $data, $flags = 0, $address = null) { }
function stream_socket_shutdown($stream, $how) { }
