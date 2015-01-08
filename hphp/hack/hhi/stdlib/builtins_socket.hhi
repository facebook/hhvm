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
define('AF_UNIX', 0);
define('AF_INET', 0);
define('AF_INET6', 0);
define('SOCK_STREAM', 0);
define('SOCK_DGRAM', 0);
define('SOCK_RAW', 0);
define('SOCK_SEQPACKET', 0);
define('SOCK_RDM', 0);
define('MSG_OOB', 0);
define('MSG_WAITALL', 0);
define('MSG_PEEK', 0);
define('MSG_DONTROUTE', 0);
define('MSG_EOR', 0);
define('MSG_EOF', 0);
define('SO_DEBUG', 0);
define('SO_REUSEADDR', 0);
define('SO_REUSEPORT', 0);
define('SO_KEEPALIVE', 0);
define('SO_DONTROUTE', 0);
define('SO_LINGER', 0);
define('SO_BROADCAST', 0);
define('SO_OOBINLINE', 0);
define('SO_SNDBUF', 0);
define('SO_RCVBUF', 0);
define('SO_SNDLOWAT', 0);
define('SO_RCVLOWAT', 0);
define('SO_SNDTIMEO', 0);
define('SO_RCVTIMEO', 0);
define('SO_TYPE', 0);
define('SO_ERROR', 0);
define('TCP_NODELAY', 0);
define('SOL_SOCKET', 0);
define('PHP_NORMAL_READ', 0);
define('PHP_BINARY_READ', 0);
define('SOL_TCP', 0);
define('SOL_UDP', 0);
define('SOCKET_EINTR', 0);
define('SOCKET_EBADF', 0);
define('SOCKET_EACCES', 0);
define('SOCKET_EFAULT', 0);
define('SOCKET_EINVAL', 0);
define('SOCKET_EMFILE', 0);
define('SOCKET_ENAMETOOLONG', 0);
define('SOCKET_ENOTEMPTY', 0);
define('SOCKET_ELOOP', 0);
define('SOCKET_EWOULDBLOCK', 0);
define('SOCKET_EREMOTE', 0);
define('SOCKET_EUSERS', 0);
define('SOCKET_ENOTSOCK', 0);
define('SOCKET_EDESTADDRREQ', 0);
define('SOCKET_EMSGSIZE', 0);
define('SOCKET_EPROTOTYPE', 0);
define('SOCKET_EPROTONOSUPPORT', 0);
define('SOCKET_ESOCKTNOSUPPORT', 0);
define('SOCKET_EOPNOTSUPP', 0);
define('SOCKET_EPFNOSUPPORT', 0);
define('SOCKET_EAFNOSUPPORT', 0);
define('SOCKET_EADDRNOTAVAIL', 0);
define('SOCKET_ENETDOWN', 0);
define('SOCKET_ENETUNREACH', 0);
define('SOCKET_ENETRESET', 0);
define('SOCKET_ECONNABORTED', 0);
define('SOCKET_ECONNRESET', 0);
define('SOCKET_ENOBUFS', 0);
define('SOCKET_EISCONN', 0);
define('SOCKET_ENOTCONN', 0);
define('SOCKET_ESHUTDOWN', 0);
define('SOCKET_ETIMEDOUT', 0);
define('SOCKET_ECONNREFUSED', 0);
define('SOCKET_EHOSTDOWN', 0);
define('SOCKET_EHOSTUNREACH', 0);
define('SOCKET_EALREADY', 0);
define('SOCKET_EINPROGRESS', 0);
define('SOCKET_ENOPROTOOPT', 0);
define('SOCKET_EADDRINUSE', 0);
define('SOCKET_ETOOMYREFS', 0);
define('SOCKET_EPROCLIM', 0);
define('SOCKET_EDUOT', 0);
define('SOCKET_ESTALE', 0);
define('SOCKET_EDISCON', 0);
define('SOCKET_SYSNOTREADY', 0);
define('SOCKET_VERNOTSUPPORTED', 0);
define('SOCKET_NOTINITIALISED', 0);
define('SOCKET_HOST_NOT_FOUND', 0);
define('SOCKET_TRY_AGAIN', 0);
define('SOCKET_NO_RECOVERY', 0);
define('SOCKET_NO_DATA', 0);
define('SOCKET_NO_ADDRESS', 0);
define('SOCKET_EPERM', 0);
define('SOCKET_ENOENT', 0);
define('SOCKET_EIO', 0);
define('SOCKET_ENXIO', 0);
define('SOCKET_E2BIG', 0);
define('SOCKET_EAGAIN', 0);
define('SOCKET_ENOMEM', 0);
define('SOCKET_ENOTBLK', 0);
define('SOCKET_EBUSY', 0);
define('SOCKET_EEXIST', 0);
define('SOCKET_EXDEV', 0);
define('SOCKET_ENODEV', 0);
define('SOCKET_ENOTDIR', 0);
define('SOCKET_EISDIR', 0);
define('SOCKET_ENFILE', 0);
define('SOCKET_ENOTTY', 0);
define('SOCKET_ENOSPC', 0);
define('SOCKET_ESPIPE', 0);
define('SOCKET_EROFS', 0);
define('SOCKET_EMLINK', 0);
define('SOCKET_EPIPE', 0);
define('SOCKET_ENOLCK', 0);
define('SOCKET_ENOSYS', 0);
define('SOCKET_ENOMSG', 0);
define('SOCKET_EIDRM', 0);
define('SOCKET_ECHRNG', 0);
define('SOCKET_EL2NSYNC', 0);
define('SOCKET_EL3HLT', 0);
define('SOCKET_EL3RST', 0);
define('SOCKET_ELNRNG', 0);
define('SOCKET_EUNATCH', 0);
define('SOCKET_ENOCSI', 0);
define('SOCKET_EL2HLT', 0);
define('SOCKET_EBADE', 0);
define('SOCKET_EBADR', 0);
define('SOCKET_EXFULL', 0);
define('SOCKET_ENOANO', 0);
define('SOCKET_EBADRQC', 0);
define('SOCKET_EBADSLT', 0);
define('SOCKET_ENOSTR', 0);
define('SOCKET_ENODATA', 0);
define('SOCKET_ETIME', 0);
define('SOCKET_ENOSR', 0);
define('SOCKET_ENONET', 0);
define('SOCKET_ENOLINK', 0);
define('SOCKET_EADV', 0);
define('SOCKET_ESRMNT', 0);
define('SOCKET_ECOMM', 0);
define('SOCKET_EPROTO', 0);
define('SOCKET_EMULTIHOP', 0);
define('SOCKET_EBADMSG', 0);
define('SOCKET_ENOTUNIQ', 0);
define('SOCKET_EBADFD', 0);
define('SOCKET_EREMCHG', 0);
define('SOCKET_ERESTART', 0);
define('SOCKET_ESTRPIPE', 0);
define('SOCKET_EPROTOOPT', 0);
define('SOCKET_ADDRINUSE', 0);
define('SOCKET_ETOOMANYREFS', 0);
define('SOCKET_EISNAM', 0);
define('SOCKET_EREMOTEIO', 0);
define('SOCKET_EDQUOT', 0);
define('SOCKET_ENOMEDIUM', 0);
define('SOCKET_EMEDIUMTYPE', 0);
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
function socket_last_error($socket = null) { }
function socket_clear_error($socket = null) { }
function getaddrinfo($host, $port, $family = 0, $socktype = 0, $protocol = 0, $flags = 0) { }
