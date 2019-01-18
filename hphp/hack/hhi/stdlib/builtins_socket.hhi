<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
<<__PHPStdLib>>
function socket_create(int $domain, int $type, int $protocol) { }
<<__PHPStdLib>>
function socket_create_listen(int $port, int $backlog = 128) { }
<<__PHPStdLib>>
function socket_create_pair(int $domain, int $type, int $protocol, &$fd) { }
<<__PHPStdLib>>
function socket_get_option(resource $socket, int $level, int $optname) { }
<<__PHPStdLib>>
function socket_getpeername(resource $socket, &$address, &$port = null) { }
<<__PHPStdLib>>
function socket_getsockname(resource $socket, &$address, &$port = null) { }
<<__PHPStdLib>>
function socket_set_block(resource $socket) { }
<<__PHPStdLib>>
function socket_set_nonblock(resource $socket) { }
<<__PHPStdLib>>
function socket_set_option(resource $socket, int $level, int $optname, $optval) { }
<<__PHPStdLib>>
function socket_connect(resource $socket, string $address, int $port = 0) { }
<<__PHPStdLib>>
function socket_bind(resource $socket, string $address, int $port = 0) { }
<<__PHPStdLib>>
function socket_listen(resource $socket, int $backlog = 0) { }
<<__PHPStdLib>>
function socket_select(&$read, &$write, &$except, $vtv_sec, int $tv_usec = 0) { }
<<__PHPStdLib>>
function socket_server(string $hostname, int $port = -1, &$errnum = null, &$errstr = null) { }
<<__PHPStdLib>>
function socket_accept(resource $socket) { }
<<__PHPStdLib>>
function socket_read(resource $socket, int $length, int $type = 0) { }
<<__PHPStdLib>>
function socket_write(resource $socket, string $buffer, int $length = 0) { }
<<__PHPStdLib>>
function socket_send(resource $socket, string $buf, int $len, int $flags) { }
<<__PHPStdLib>>
function socket_sendto(resource $socket, string $buf, int $len, int $flags, string $addr, int $port = 0) { }
<<__PHPStdLib>>
function socket_recv(resource $socket, &$buf, int $len, int $flags) { }
<<__PHPStdLib>>
function socket_recvfrom(resource $socket, &$buf, int $len, int $flags, &$name, &$port = 0) { }
<<__PHPStdLib>>
function socket_shutdown(resource $socket, int $how = 0) { }
<<__PHPStdLib>>
function socket_close(resource $socket) { }
<<__PHPStdLib>>
function socket_strerror(int $errnum) { }
<<__PHPStdLib>>
function socket_last_error($socket = null) { }
<<__PHPStdLib>>
function socket_clear_error($socket = null) { }
<<__PHPStdLib>>
function getaddrinfo(string $host, string $port, int $family = 0, int $socktype = 0, int $protocol = 0, int $flags = 0);
