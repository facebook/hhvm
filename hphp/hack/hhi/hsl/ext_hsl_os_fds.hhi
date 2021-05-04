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

const int E2BIG = 0;
const int EACCES = 0;
const int EADDRINUSE = 0;
const int EADDRNOTAVAIL = 0;
const int EAFNOSUPPORT = 0;
const int EAGAIN = 0;
const int EALREADY = 0;
const int EBADF = 0;
const int EBADMSG = 0;
const int EBUSY = 0;
const int ECANCELED = 0;
const int ECHILD = 0;
const int ECONNABORTED = 0;
const int ECONNREFUSED = 0;
const int ECONNRESET = 0;
const int EDEADLK = 0;
const int EDESTADDRREQ = 0;
const int EDOM = 0;
const int EDQUOT = 0;
const int EEXIST = 0;
const int EFAULT = 0;
const int EFBIG = 0;
const int EHOSTDOWN = 0;
const int EHOSTUNREACH = 0;
const int EIDRM = 0;
const int EILSEQ = 0;
const int EINPROGRESS = 0;
const int EINTR = 0;
const int EINVAL = 0;
const int EIO = 0;
const int EISCONN = 0;
const int EISDIR = 0;
const int ELOOP = 0;
const int EMFILE = 0;
const int EMLINK = 0;
const int EMSGSIZE = 0;
const int EMULTIHOP = 0;
const int ENAMETOOLONG = 0;
const int ENETDOWN = 0;
const int ENETRESET = 0;
const int ENETUNREACH = 0;
const int ENFILE = 0;
const int ENOBUFS = 0;
const int ENODATA = 0;
const int ENODEV = 0;
const int ENOENT = 0;
const int ENOEXEC = 0;
const int ENOLCK = 0;
const int ENOLINK = 0;
const int ENOMEM = 0;
const int ENOMSG = 0;
const int ENOPROTOOPT = 0;
const int ENOSPC = 0;
const int ENOSR = 0;
const int ENOSTR = 0;
const int ENOSYS = 0;
const int ENOTBLK = 0;
const int ENOTCONN = 0;
const int ENOTDIR = 0;
const int ENOTEMPTY = 0;
const int ENOTSOCK = 0;
const int ENOTSUP = 0;
const int ENOTTY = 0;
const int ENXIO = 0;
const int EOPNOTSUPP = 0;
const int EOVERFLOW = 0;
const int EPERM = 0;
const int EPFNOSUPPORT = 0;
const int EPIPE = 0;
const int EPROTO = 0;
const int EPROTONOSUPPORT = 0;
const int EPROTOTYPE = 0;
const int ERANGE = 0;
const int EROFS = 0;
const int ESHUTDOWN = 0;
const int ESOCKTNOSUPPORT = 0;
const int ESPIPE = 0;
const int ESRCH = 0;
const int ESTALE = 0;
const int ETIME = 0;
const int ETIMEDOUT = 0;
const int ETXTBSY = 0;
const int EUSERS = 0;
const int EXDEV = 0;

const int O_RDONLY = 0;
const int O_WRONLY = 0;
const int O_RDWR= 0;
const int O_NONBLOCK = 0;
const int O_APPEND = 0;
const int O_CREAT/*E < you dropped this*/ = 0;
const int O_TRUNC = 0;
const int O_EXCL = 0;
const int O_NOFOLLOW = 0;
const int O_CLOEXEC = 0;

const int SEEK_SET = 0;
const int SEEK_CUR = 0;
const int SEEK_END = 0;
const int SEEK_HOLE = 0;
const int SEEK_DATA = 0;

const int STDIN_FILENO = 0;
const int STDOUT_FILENO = 0;
const int STDERR_FILENO = 0;

final class ErrnoException extends \Exception {}

function poll_async(
  FileDescriptor $fd,
  int $events,
  int $timeout_ns
): Awaitable<int>;

function open(string $path, int $flags, int $mode = 0): FileDescriptor;

function request_stdio_fd(int $stdio_fileno): FileDescriptor;

function mkdtemp(
  string $template,
): string;

function mkostemps(
  string $template,
  int $suffixlen,
  int $flags
): (FileDescriptor, string);

function read(
  FileDescriptor $fd,
  int $max_to_read,
): string;

function write(FileDescriptor $fd, string $data): int;

function close(FileDescriptor $fd): void;

function pipe(): (FileDescriptor, FileDescriptor);

const int AF_UNSPEC = 0;
const int AF_UNIX = 0;
const int AF_INET = 0;
const int AF_INET6 = 0;
const int AF_MAX = 0;

// Same as AF_
const int PF_UNSPEC = 0;
const int PF_UNIX = 0;
const int PF_INET = 0;
const int PF_INET6 = 0;
const int PF_MAX = 0;

const int SUN_PATH_LEN = 0;
const int INADDR_ANY = 0;
const int INADDR_LOOPBACK = 0;
const int INADDR_BROADCAST = 0;
const int INADDR_NONE = 0;

const int SOCK_STREAM = 0;
const int SOCK_DGRAM = 0;
const int SOCK_RAW = 0;

const int F_GETFD = 0;
const int F_GETFL = 0;
const int F_GETOWN = 0;
const int F_SETFD = 0;
const int F_SETFL = 0;
const int F_SETOWN = 0;

const int FD_CLOEXEC = 0;

const int LOCK_SH = 0;
const int LOCK_EX = 0;
const int LOCK_NB = 0;
const int LOCK_UN = 0;

const int SOL_SOCKET = 0;
// the constant formerly known as SOL_TCP
const int IPPROTO_TCP = 0;

const int SO_BROADCAST = 0;
const int SO_DEBUG = 0;
const int SO_DONTROUTE = 0;
const int SO_ERROR = 0;
const int SO_KEEPALIVE = 0;
const int SO_LINGER = 0;
const int SO_OOBINLINE = 0;
const int SO_RCVBUF = 0;
const int SO_RCVLOWAT = 0;
const int SO_REUSEADDR = 0;
const int SO_REUSEPORT = 0;
const int SO_SNDBUF = 0;
const int SO_SNDLOWAT = 0;
const int SO_TYPE = 0;

const int TCP_FASTOPEN = 0;
const int TCP_KEEPCNT = 0;
const int TCP_KEEPINTVL = 0;
const int TCP_MAXSEG = 0;
const int TCP_NODELAY = 0;
const int TCP_NOTSENT_LOWAT = 0;

type uint32_t = int;
type sa_family_t = int;
type in_port_t = int;

class sockaddr {
  public function __construct(
    public sa_family_t $sa_family,
  ) {}
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
function socketpair(int $domain, int $type, int $protocol): (FileDescriptor, FileDescriptor);

function fcntl(FileDescriptor $fd, int $cmd, mixed $arg = null): mixed;
function getsockopt_int(FileDescriptor $fd, int $level, int $option): int;
function setsockopt_int(FileDescriptor $fd, int $level, int $option, int $value): void;

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
