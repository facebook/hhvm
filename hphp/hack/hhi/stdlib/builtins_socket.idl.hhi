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
function socket_create($domain, $type, $protocol) { }
function socket_create_listen($port, $backlog = 128) { }
function socket_create_pair($domain, $type, $protocol, &$fd) { }
function socket_get_option($socket, $level, $optname) { }
function socket_getpeername($socket, &$address, &$port = null) { }
function socket_getsockname($socket, &$address, &$port = null) { }
function socket_set_block($socket) { }
function socket_set_nonblock($socket) { }
function socket_set_option($socket, $level, $optname, $optval) { }
function socket_connect($socket, $address, $port = 0) { }
function socket_bind($socket, $address, $port = 0) { }
function socket_listen($socket, $backlog = 0) { }
function socket_select(&$read, &$write, &$except, $vtv_sec, $tv_usec = 0) { }
function socket_server($hostname, $port = -1, &$errnum = null, &$errstr = null) { }
function socket_accept($socket) { }
function socket_read($socket, $length, $type = 0) { }
function socket_write($socket, $buffer, $length = 0) { }
function socket_send($socket, $buf, $len, $flags) { }
function socket_sendto($socket, $buf, $len, $flags, $addr, $port = 0) { }
function socket_recv($socket, &$buf, $len, $flags) { }
function socket_recvfrom($socket, &$buf, $len, $flags, &$name, &$port = 0) { }
function socket_shutdown($socket, $how = 0) { }
function socket_close($socket) { }
function socket_strerror($errnum) { }
function socket_last_error($socket = null_object) { }
function socket_clear_error($socket = null_object) { }
function getaddrinfo($host, $port, $family = 0, $socktype = 0, $protocol = 0, $flags = 0) { }
