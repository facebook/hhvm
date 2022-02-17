<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int AF_UNIX = 0;
const int AF_INET = 0;
const int AF_INET6 = 0;
const int SOCK_STREAM = 0;
const int SOCK_DGRAM = 0;
const int SOCK_RAW = 0;
const int SOCK_SEQPACKET = 0;
const int SOCK_RDM = 0;
const int MSG_OOB = 0;
const int MSG_WAITALL = 0;
const int MSG_PEEK = 0;
const int MSG_DONTROUTE = 0;
const int MSG_EOR = 0;
const int MSG_EOF = 0;
const int SO_DEBUG = 0;
const int SO_REUSEADDR = 0;
const int SO_REUSEPORT = 0;
const int SO_KEEPALIVE = 0;
const int SO_DONTROUTE = 0;
const int SO_LINGER = 0;
const int SO_BROADCAST = 0;
const int SO_OOBINLINE = 0;
const int SO_SNDBUF = 0;
const int SO_RCVBUF = 0;
const int SO_SNDLOWAT = 0;
const int SO_RCVLOWAT = 0;
const int SO_SNDTIMEO = 0;
const int SO_RCVTIMEO = 0;
const int SO_TYPE = 0;
const int SO_ERROR = 0;
const int TCP_NODELAY = 0;
const int SOL_SOCKET = 0;
const int PHP_NORMAL_READ = 0;
const int PHP_BINARY_READ = 0;
const int SOL_TCP = 0;
const int SOL_UDP = 0;
const int SOCKET_EINTR = 0;
const int SOCKET_EBADF = 0;
const int SOCKET_EACCES = 0;
const int SOCKET_EFAULT = 0;
const int SOCKET_EINVAL = 0;
const int SOCKET_EMFILE = 0;
const int SOCKET_ENAMETOOLONG = 0;
const int SOCKET_ENOTEMPTY = 0;
const int SOCKET_ELOOP = 0;
const int SOCKET_EWOULDBLOCK = 0;
const int SOCKET_EREMOTE = 0;
const int SOCKET_EUSERS = 0;
const int SOCKET_ENOTSOCK = 0;
const int SOCKET_EDESTADDRREQ = 0;
const int SOCKET_EMSGSIZE = 0;
const int SOCKET_EPROTOTYPE = 0;
const int SOCKET_EPROTONOSUPPORT = 0;
const int SOCKET_ESOCKTNOSUPPORT = 0;
const int SOCKET_EOPNOTSUPP = 0;
const int SOCKET_EPFNOSUPPORT = 0;
const int SOCKET_EAFNOSUPPORT = 0;
const int SOCKET_EADDRNOTAVAIL = 0;
const int SOCKET_ENETDOWN = 0;
const int SOCKET_ENETUNREACH = 0;
const int SOCKET_ENETRESET = 0;
const int SOCKET_ECONNABORTED = 0;
const int SOCKET_ECONNRESET = 0;
const int SOCKET_ENOBUFS = 0;
const int SOCKET_EISCONN = 0;
const int SOCKET_ENOTCONN = 0;
const int SOCKET_ESHUTDOWN = 0;
const int SOCKET_ETIMEDOUT = 0;
const int SOCKET_ECONNREFUSED = 0;
const int SOCKET_EHOSTDOWN = 0;
const int SOCKET_EHOSTUNREACH = 0;
const int SOCKET_EALREADY = 0;
const int SOCKET_EINPROGRESS = 0;
const int SOCKET_ENOPROTOOPT = 0;
const int SOCKET_EADDRINUSE = 0;
const int SOCKET_ETOOMYREFS = 0;
const int SOCKET_EPROCLIM = 0;
const int SOCKET_EDUOT = 0;
const int SOCKET_ESTALE = 0;
const int SOCKET_EDISCON = 0;
const int SOCKET_SYSNOTREADY = 0;
const int SOCKET_VERNOTSUPPORTED = 0;
const int SOCKET_NOTINITIALISED = 0;
const int SOCKET_HOST_NOT_FOUND = 0;
const int SOCKET_TRY_AGAIN = 0;
const int SOCKET_NO_RECOVERY = 0;
const int SOCKET_NO_DATA = 0;
const int SOCKET_NO_ADDRESS = 0;
const int SOCKET_EPERM = 0;
const int SOCKET_ENOENT = 0;
const int SOCKET_EIO = 0;
const int SOCKET_ENXIO = 0;
const int SOCKET_E2BIG = 0;
const int SOCKET_EAGAIN = 0;
const int SOCKET_ENOMEM = 0;
const int SOCKET_ENOTBLK = 0;
const int SOCKET_EBUSY = 0;
const int SOCKET_EEXIST = 0;
const int SOCKET_EXDEV = 0;
const int SOCKET_ENODEV = 0;
const int SOCKET_ENOTDIR = 0;
const int SOCKET_EISDIR = 0;
const int SOCKET_ENFILE = 0;
const int SOCKET_ENOTTY = 0;
const int SOCKET_ENOSPC = 0;
const int SOCKET_ESPIPE = 0;
const int SOCKET_EROFS = 0;
const int SOCKET_EMLINK = 0;
const int SOCKET_EPIPE = 0;
const int SOCKET_ENOLCK = 0;
const int SOCKET_ENOSYS = 0;
const int SOCKET_ENOMSG = 0;
const int SOCKET_EIDRM = 0;
const int SOCKET_ECHRNG = 0;
const int SOCKET_EL2NSYNC = 0;
const int SOCKET_EL3HLT = 0;
const int SOCKET_EL3RST = 0;
const int SOCKET_ELNRNG = 0;
const int SOCKET_EUNATCH = 0;
const int SOCKET_ENOCSI = 0;
const int SOCKET_EL2HLT = 0;
const int SOCKET_EBADE = 0;
const int SOCKET_EBADR = 0;
const int SOCKET_EXFULL = 0;
const int SOCKET_ENOANO = 0;
const int SOCKET_EBADRQC = 0;
const int SOCKET_EBADSLT = 0;
const int SOCKET_ENOSTR = 0;
const int SOCKET_ENODATA = 0;
const int SOCKET_ETIME = 0;
const int SOCKET_ENOSR = 0;
const int SOCKET_ENONET = 0;
const int SOCKET_ENOLINK = 0;
const int SOCKET_EADV = 0;
const int SOCKET_ESRMNT = 0;
const int SOCKET_ECOMM = 0;
const int SOCKET_EPROTO = 0;
const int SOCKET_EMULTIHOP = 0;
const int SOCKET_EBADMSG = 0;
const int SOCKET_ENOTUNIQ = 0;
const int SOCKET_EBADFD = 0;
const int SOCKET_EREMCHG = 0;
const int SOCKET_ERESTART = 0;
const int SOCKET_ESTRPIPE = 0;
const int SOCKET_EPROTOOPT = 0;
const int SOCKET_ADDRINUSE = 0;
const int SOCKET_ETOOMANYREFS = 0;
const int SOCKET_EISNAM = 0;
const int SOCKET_EREMOTEIO = 0;
const int SOCKET_EDQUOT = 0;
const int SOCKET_ENOMEDIUM = 0;
const int SOCKET_EMEDIUMTYPE = 0;
<<__PHPStdLib>>
function socket_create(int $domain, int $type, int $protocol)[leak_safe] { }
<<__PHPStdLib>>
function socket_create_listen(int $port, int $backlog = 128) { }
<<__PHPStdLib>>
function socket_create_pair(int $domain, int $type, int $protocol, inout $fd) { }
<<__PHPStdLib>>
function socket_get_option(resource $socket, int $level, int $optname)[leak_safe] { }
<<__PHPStdLib>>
function socket_getpeername(resource $socket, inout $address, inout $port)[leak_safe] { }
<<__PHPStdLib>>
function socket_getsockname(resource $socket, inout $address, inout $port)[] { }
<<__PHPStdLib>>
function socket_set_block(resource $socket) { }
<<__PHPStdLib>>
function socket_set_nonblock(resource $socket) { }
<<__PHPStdLib>>
function socket_set_option(resource $socket, int $level, int $optname, $optval)[leak_safe] { }
<<__PHPStdLib>>
function socket_connect(resource $socket, string $address, int $port = 0) { }
<<__PHPStdLib>>
function socket_bind(resource $socket, string $address, int $port = 0) { }
<<__PHPStdLib>>
function socket_listen(resource $socket, int $backlog = 0) { }
<<__PHPStdLib>>
function socket_select(inout $read, inout $write, inout $except, $vtv_sec, int $tv_usec = 0)[leak_safe] { }
<<__PHPStdLib>>
function socket_server(string $hostname, int $port, inout $errnum, inout $errstr) { }
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
function socket_recv(resource $socket, inout $buf, int $len, int $flags) { }
<<__PHPStdLib>>
function socket_recvfrom(resource $socket, inout $buf, int $len, int $flags, inout $name, inout $port) { }
<<__PHPStdLib>>
function socket_shutdown(resource $socket, int $how = 0) { }
<<__PHPStdLib>>
function socket_close(resource $socket)[write_props] { }
<<__PHPStdLib>>
function socket_strerror(int $errnum)[] { }
<<__PHPStdLib>>
function socket_last_error($socket = null)[read_globals] { }
<<__PHPStdLib>>
function socket_clear_error($socket = null) { }
<<__PHPStdLib>>
function getaddrinfo(string $host, string $port, int $family = 0, int $socktype = 0, int $protocol = 0, int $flags = 0);
