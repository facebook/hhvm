<?hh
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// we anticipate raising a new error, similar to __Deprecated, for calling
// 'private' builtins; despite the fact they shouldn't be called by anything
// except the HSL, we're defining an HHI so the HSL itself can be type-safe:
// suppressing "this should not be called" is better than making a completely
// untyped call.

namespace HH\Lib\_Private\_OS;

use type HH\Lib\OS\FileDescriptor;

// Actual values are platform-specific

const int E2BIG;
const int EACCES;
const int EADDRINUSE;
const int EADDRNOTAVAIL;
const int EAFNOSUPPORT;
const int EAGAIN;
const int EALREADY;
const int EBADF;
const int EBADMSG;
const int EBUSY;
const int ECANCELED;
const int ECHILD;
const int ECONNABORTED;
const int ECONNREFUSED;
const int ECONNRESET;
const int EDEADLK;
const int EDESTADDRREQ;
const int EDOM;
const int EDQUOT;
const int EEXIST;
const int EFAULT;
const int EFBIG;
const int EHOSTDOWN;
const int EHOSTUNREACH;
const int EIDRM;
const int EILSEQ;
const int EINPROGRESS;
const int EINTR;
const int EINVAL;
const int EIO;
const int EISCONN;
const int EISDIR;
const int ELOOP;
const int EMFILE;
const int EMLINK;
const int EMSGSIZE;
const int EMULTIHOP;
const int ENAMETOOLONG;
const int ENETDOWN;
const int ENETRESET;
const int ENETUNREACH;
const int ENFILE;
const int ENOBUFS;
const int ENODATA;
const int ENODEV;
const int ENOENT;
const int ENOEXEC;
const int ENOLCK;
const int ENOLINK;
const int ENOMEM;
const int ENOMSG;
const int ENOPROTOOPT;
const int ENOSPC;
const int ENOSR;
const int ENOSTR;
const int ENOSYS;
const int ENOTBLK;
const int ENOTCONN;
const int ENOTDIR;
const int ENOTEMPTY;
const int ENOTSOCK;
const int ENOTSUP;
const int ENOTTY;
const int ENXIO;
const int EOPNOTSUPP;
const int EOVERFLOW;
const int EPERM;
const int EPFNOSUPPORT;
const int EPIPE;
const int EPROTO;
const int EPROTONOSUPPORT;
const int EPROTOTYPE;
const int ERANGE;
const int EROFS;
const int ESHUTDOWN;
const int ESOCKTNOSUPPORT;
const int ESPIPE;
const int ESRCH;
const int ESTALE;
const int ETIME;
const int ETIMEDOUT;
const int ETXTBSY;
const int EUSERS;
const int EXDEV;

const int O_RDONLY;
const int O_WRONLY;
const int O_RDWR;
const int O_NONBLOCK;
const int O_APPEND;
const int O_CREAT/*E < you dropped this*/ ;
const int O_TRUNC;
const int O_EXCL;
const int O_NOFOLLOW;
const int O_CLOEXEC;

const int SEEK_SET;
const int SEEK_CUR;
const int SEEK_END;
const int SEEK_HOLE;
const int SEEK_DATA;

const int STDIN_FILENO;
const int STDOUT_FILENO;
const int STDERR_FILENO;

final class ErrnoException extends \Exception {}

function poll_async(
  FileDescriptor $fd,
  int $events,
  int $timeout_ns,
): Awaitable<int>;

function open(string $path, int $flags, int $mode = 0): FileDescriptor;

function request_stdio_fd(int $stdio_fileno): FileDescriptor;

function mkdtemp(string $template): string;

function mkostemps(
  string $template,
  int $suffixlen,
  int $flags,
): (FileDescriptor, string);

function read(FileDescriptor $fd, int $max_to_read): string;

function write(FileDescriptor $fd, string $data): int;

function close(FileDescriptor $fd): void;

function dup(FileDescriptor $fd): FileDescriptor;

function pipe(): (FileDescriptor, FileDescriptor);

const int AF_UNSPEC;
const int AF_UNIX;
const int AF_INET;
const int AF_INET6;
const int AF_MAX;

// Same as AF_
const int PF_UNSPEC;
const int PF_UNIX;
const int PF_INET;
const int PF_INET6;
const int PF_MAX;

const int SUN_PATH_LEN;
const int INADDR_ANY;
const int INADDR_LOOPBACK;
const int INADDR_BROADCAST;
const int INADDR_NONE;

const int SOCK_STREAM;
const int SOCK_DGRAM;
const int SOCK_RAW;

const int F_GETFD;
const int F_GETFL;
const int F_GETOWN;
const int F_SETFD;
const int F_SETFL;
const int F_SETOWN;

const int FD_CLOEXEC;

const int LOCK_SH;
const int LOCK_EX;
const int LOCK_NB;
const int LOCK_UN;

const int SOL_SOCKET;
// the constant formerly known as SOL_TCP
const int IPPROTO_TCP;

const int SO_BROADCAST;
const int SO_DEBUG;
const int SO_DONTROUTE;
const int SO_ERROR;
const int SO_KEEPALIVE;
const int SO_LINGER;
const int SO_OOBINLINE;
const int SO_RCVBUF;
const int SO_RCVLOWAT;
const int SO_REUSEADDR;
const int SO_REUSEPORT;
const int SO_SNDBUF;
const int SO_SNDLOWAT;
const int SO_TYPE;

const int TCP_FASTOPEN;
const int TCP_KEEPCNT;
const int TCP_KEEPINTVL;
const int TCP_MAXSEG;
const int TCP_NODELAY;
const int TCP_NOTSENT_LOWAT;

type uint32_t = int;
type sa_family_t = int;
type in_port_t = int;

class sockaddr {
  public function __construct(public sa_family_t $sa_family) {}
  // Intentionally not including "data":
  // - it's a placeholder
  // - it can be thought of as a union of the data of all the others
  // - may include pointers that can't be dealt with.
}

final class sockaddr_in extends sockaddr {
  public function __construct(
    public in_port_t $sin_port,
    public uint32_t $sin_addr,
  ) {
    parent::__construct(namespace\AF_INET);
  }
}

final class sockaddr_in6 extends sockaddr {
  public function __construct(
    public in_port_t $sin6_port,
    public uint32_t $sin6_flowinfo,
    public string $sin6_addr,
    public uint32_t $sin6_scope_id,
  ) {
    parent::__construct(namespace\AF_INET6);
  }
}

abstract class sockaddr_un extends sockaddr {
  public function __construct() {
    parent::__construct(namespace\AF_UNIX);
  }
}

final class sockaddr_un_pathname extends sockaddr_un {
  public function __construct(public string $sun_path) {
    parent::__construct();
  }
}

final class sockaddr_un_unnamed extends sockaddr_un {
}

function getpeername(FileDescriptor $fd): sockaddr;
function getsockname(FileDescriptor $fd): sockaddr;
function socketpair(
  int $domain,
  int $type,
  int $protocol,
): (FileDescriptor, FileDescriptor);

function fcntl(FileDescriptor $fd, int $cmd, mixed $arg = null): mixed;
function getsockopt_int(FileDescriptor $fd, int $level, int $option): int;
function setsockopt_int(
  FileDescriptor $fd,
  int $level,
  int $option,
  int $value,
): void;

function socket(int $domain, int $type, int $protocol): FileDescriptor;
function connect(FileDescriptor $socket, sockaddr $addr): void;
function bind(FileDescriptor $socket, sockaddr $addr): void;
function listen(FileDescriptor $socket, int $backlog): void;
function accept(FileDescriptor $socket): (FileDescriptor, sockaddr);
function lseek(FileDescriptor $fd, int $offset, int $whence): int;
function ftruncate(FileDescriptor $fd, int $length): void;
function flock(FileDescriptor $fd, int $operation): void;

function isatty(FileDescriptor $fd): bool;
function ttyname(FileDescriptor $fd): string;
