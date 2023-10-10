<?hh

/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
 */
namespace HH\Lib\OS {

<<__NativeData>>
final class FileDescriptor {
  private function __construct() {}

  <<__Native>>
  public function __debugInfo(): darray<string, mixed>;
}

} // namespace OS

namespace HH\Lib\_Private\_OS {


use type HH\Lib\OS\FileDescriptor;

final class ErrnoException extends \Exception {}

<<__Native>>
function open(string $path, int $flags, int $mode = 0): FileDescriptor;

<<__Native>>
function mkostemps(string $template, int $suffixlen, int $flags): varray<mixed> /* (FileDescriptor, string) */;

<<__Native>>
function mkdtemp(string $template): string;

<<__Native>>
function read(
  FileDescriptor $fd,
  int $max_to_read,
): string;

<<__Native>>
function write(FileDescriptor $fd, string $data): int;

<<__Native>>
function close(FileDescriptor $fd): void;

<<__Native>>
function dup(FileDescriptor $fd): FileDescriptor;

<<__Native>>
function pipe(): varray<FileDescriptor>;

<<__Native>>
function poll_async(FileDescriptor $fd, int $events, int $timeout_ns): Awaitable<int>;

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

/** Base class for unix sockets.
 *
 * Have subclasses instead of `?string $path`, as there are multiple types of
 * sockaddr_un - using Linux's naming:
 * - pathname: portable, what everyone expects. On filesystem.
 * - unnamed: what you get from `socketpair()`, or before `bind()`/`connect()`.
 *   - Linux: 0-length `sun_path`
 *   - MacOS: all-null 14-byte `sun_path`
 * - abstract: unix sockets with a meaningful name, which does not exist on the
 *   filesystem.
 *   - Linux: first byte of `sun_path` is null
 *   - does not exist on other platforms
 *
 * These are made portable here by providing specific subclasses for `pathname`
 * and `unnamed` - `abstract` are not supported at present, but may be in the
 * future by adding another subclass.
 */
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

<<__Native>>
function getpeername(FileDescriptor $fd): sockaddr;

<<__Native>>
function getsockname(FileDescriptor $fd): sockaddr;

<<__Native>>
function socketpair(int $domain, int $type, int $protocol): varray<FileDescriptor>;

<<__Native>>
function socket(int $domain, int $type, int $protocol): FileDescriptor;

<<__Native>>
function connect(FileDescriptor $socket, sockaddr $addr): void;

<<__Native>>
function bind(FileDescriptor $socket, sockaddr $addr): void;

<<__Native>>
function listen(FileDescriptor $socket, int $backlog): void;

<<__Native>>
function accept(FileDescriptor $socket): varray<mixed> /* (FileDescriptor, SockAddr) */;

<<__Native>>
function fcntl(FileDescriptor $fd, int $cmd, mixed $arg = null): mixed;

<<__Native>>
function isatty(FileDescriptor $fd): bool;

<<__Native>>
function ttyname(FileDescriptor $fd): string;

<<__Native>>
function getsockopt_int(FileDescriptor $fd, int $level, int $option): int;

<<__Native>>
function setsockopt_int(FileDescriptor $fd, int $level, int $option, int $value): void;

<<__Native>>
function lseek(FileDescriptor $fd, int $offset, int $whence): int;

<<__Native>>
function ftruncate(FileDescriptor $fd, int $length): void;

<<__Native>>
function flock(FileDescriptor $fd, int $operation): void;

<<__Native>>
function request_stdio_fd(int $fd): FileDescriptor;

type ForkAndExecveOptions = shape(
  ?'cwd' => string,
  ?'setsid' => bool,
  ?'execvpe' => bool,
  ?'setpgid' => int,
);

<<__Native>>
function fork_and_execve(
  string $path,
  vec<string> $argv,
  vec<string> $envp,
  dict<int, FileDescriptor> $fds,

  // Don't use ForkAndExecveOptions because native functions does not support type aliases.
  shape(...) $options,
): int;

} // namespace _OS
