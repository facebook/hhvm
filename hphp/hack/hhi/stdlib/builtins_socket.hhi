<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int AF_UNIX;
const int AF_INET;
const int AF_INET6;
const int SOCK_STREAM;
const int SOCK_DGRAM;
const int SOCK_RAW;
const int SOCK_SEQPACKET;
const int SOCK_RDM;
const int MSG_OOB;
const int MSG_WAITALL;
const int MSG_PEEK;
const int MSG_DONTROUTE;
const int MSG_EOR;
const int MSG_EOF;
const int SO_DEBUG;
const int SO_REUSEADDR;
const int SO_REUSEPORT;
const int SO_KEEPALIVE;
const int SO_DONTROUTE;
const int SO_LINGER;
const int SO_BROADCAST;
const int SO_OOBINLINE;
const int SO_SNDBUF;
const int SO_RCVBUF;
const int SO_SNDLOWAT;
const int SO_RCVLOWAT;
const int SO_SNDTIMEO;
const int SO_RCVTIMEO;
const int SO_TYPE;
const int SO_ERROR;
const int TCP_NODELAY;
const int SOL_SOCKET;
const int PHP_NORMAL_READ;
const int PHP_BINARY_READ;
const int SOL_TCP;
const int SOL_UDP;
const int SOCKET_EINTR;
const int SOCKET_EBADF;
const int SOCKET_EACCES;
const int SOCKET_EFAULT;
const int SOCKET_EINVAL;
const int SOCKET_EMFILE;
const int SOCKET_ENAMETOOLONG;
const int SOCKET_ENOTEMPTY;
const int SOCKET_ELOOP;
const int SOCKET_EWOULDBLOCK;
const int SOCKET_EREMOTE;
const int SOCKET_EUSERS;
const int SOCKET_ENOTSOCK;
const int SOCKET_EDESTADDRREQ;
const int SOCKET_EMSGSIZE;
const int SOCKET_EPROTOTYPE;
const int SOCKET_EPROTONOSUPPORT;
const int SOCKET_ESOCKTNOSUPPORT;
const int SOCKET_EOPNOTSUPP;
const int SOCKET_EPFNOSUPPORT;
const int SOCKET_EAFNOSUPPORT;
const int SOCKET_EADDRNOTAVAIL;
const int SOCKET_ENETDOWN;
const int SOCKET_ENETUNREACH;
const int SOCKET_ENETRESET;
const int SOCKET_ECONNABORTED;
const int SOCKET_ECONNRESET;
const int SOCKET_ENOBUFS;
const int SOCKET_EISCONN;
const int SOCKET_ENOTCONN;
const int SOCKET_ESHUTDOWN;
const int SOCKET_ETIMEDOUT;
const int SOCKET_ECONNREFUSED;
const int SOCKET_EHOSTDOWN;
const int SOCKET_EHOSTUNREACH;
const int SOCKET_EALREADY;
const int SOCKET_EINPROGRESS;
const int SOCKET_ENOPROTOOPT;
const int SOCKET_EADDRINUSE;
const int SOCKET_ETOOMYREFS;
const int SOCKET_EPROCLIM;
const int SOCKET_EDUOT;
const int SOCKET_ESTALE;
const int SOCKET_EDISCON;
const int SOCKET_SYSNOTREADY;
const int SOCKET_VERNOTSUPPORTED;
const int SOCKET_NOTINITIALISED;
const int SOCKET_HOST_NOT_FOUND;
const int SOCKET_TRY_AGAIN;
const int SOCKET_NO_RECOVERY;
const int SOCKET_NO_DATA;
const int SOCKET_NO_ADDRESS;
const int SOCKET_EPERM;
const int SOCKET_ENOENT;
const int SOCKET_EIO;
const int SOCKET_ENXIO;
const int SOCKET_E2BIG;
const int SOCKET_EAGAIN;
const int SOCKET_ENOMEM;
const int SOCKET_ENOTBLK;
const int SOCKET_EBUSY;
const int SOCKET_EEXIST;
const int SOCKET_EXDEV;
const int SOCKET_ENODEV;
const int SOCKET_ENOTDIR;
const int SOCKET_EISDIR;
const int SOCKET_ENFILE;
const int SOCKET_ENOTTY;
const int SOCKET_ENOSPC;
const int SOCKET_ESPIPE;
const int SOCKET_EROFS;
const int SOCKET_EMLINK;
const int SOCKET_EPIPE;
const int SOCKET_ENOLCK;
const int SOCKET_ENOSYS;
const int SOCKET_ENOMSG;
const int SOCKET_EIDRM;
const int SOCKET_ECHRNG;
const int SOCKET_EL2NSYNC;
const int SOCKET_EL3HLT;
const int SOCKET_EL3RST;
const int SOCKET_ELNRNG;
const int SOCKET_EUNATCH;
const int SOCKET_ENOCSI;
const int SOCKET_EL2HLT;
const int SOCKET_EBADE;
const int SOCKET_EBADR;
const int SOCKET_EXFULL;
const int SOCKET_ENOANO;
const int SOCKET_EBADRQC;
const int SOCKET_EBADSLT;
const int SOCKET_ENOSTR;
const int SOCKET_ENODATA;
const int SOCKET_ETIME;
const int SOCKET_ENOSR;
const int SOCKET_ENONET;
const int SOCKET_ENOLINK;
const int SOCKET_EADV;
const int SOCKET_ESRMNT;
const int SOCKET_ECOMM;
const int SOCKET_EPROTO;
const int SOCKET_EMULTIHOP;
const int SOCKET_EBADMSG;
const int SOCKET_ENOTUNIQ;
const int SOCKET_EBADFD;
const int SOCKET_EREMCHG;
const int SOCKET_ERESTART;
const int SOCKET_ESTRPIPE;
const int SOCKET_EPROTOOPT;
const int SOCKET_ADDRINUSE;
const int SOCKET_ETOOMANYREFS;
const int SOCKET_EISNAM;
const int SOCKET_EREMOTEIO;
const int SOCKET_EDQUOT;
const int SOCKET_ENOMEDIUM;
const int SOCKET_EMEDIUMTYPE;
<<__PHPStdLib>>
function socket_create(
  int $domain,
  int $type,
  int $protocol,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_create_listen(
  int $port,
  int $backlog = 128,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_create_pair(
  int $domain,
  int $type,
  int $protocol,
  inout $fd,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_get_option(
  resource $socket,
  int $level,
  int $optname,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_getpeername(
  resource $socket,
  inout $address,
  inout $port,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_getsockname(
  resource $socket,
  inout $address,
  inout $port,
)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_set_block(resource $socket): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_set_nonblock(resource $socket): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_set_option(
  resource $socket,
  int $level,
  int $optname,
  HH\FIXME\MISSING_PARAM_TYPE $optval,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_connect(
  resource $socket,
  string $address,
  int $port = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_bind(
  resource $socket,
  string $address,
  int $port = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_listen(
  resource $socket,
  int $backlog = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_select(
  inout $read,
  inout $write,
  inout $except,
  HH\FIXME\MISSING_PARAM_TYPE $vtv_sec,
  int $tv_usec = 0,
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_server(
  string $hostname,
  int $port,
  inout $errnum,
  inout $errstr,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_accept(resource $socket): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_read(
  resource $socket,
  int $length,
  int $type = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_write(
  resource $socket,
  string $buffer,
  int $length = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_send(
  resource $socket,
  string $buf,
  int $len,
  int $flags,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_sendto(
  resource $socket,
  string $buf,
  int $len,
  int $flags,
  string $addr,
  int $port = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_recv(
  resource $socket,
  inout $buf,
  int $len,
  int $flags,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_recvfrom(
  resource $socket,
  inout $buf,
  int $len,
  int $flags,
  inout $name,
  inout $port,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_shutdown(
  resource $socket,
  int $how = 0,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_close(
  resource $socket,
)[write_props]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_strerror(int $errnum)[]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_last_error(
  HH\FIXME\MISSING_PARAM_TYPE $socket = null,
)[read_globals]: HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function socket_clear_error(
  HH\FIXME\MISSING_PARAM_TYPE $socket = null,
): HH\FIXME\MISSING_RETURN_TYPE {}
<<__PHPStdLib>>
function getaddrinfo(
  string $host,
  string $port,
  int $family = 0,
  int $socktype = 0,
  int $protocol = 0,
  int $flags = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
