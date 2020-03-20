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

const int O_RDONLY = 0;
const int O_WRONLY = 0;
const int O_RDWR= 0;
const int O_NONBLOCK = 0;
const int O_APPEND = 0;
const int O_CREAT/*E < you dropped this*/ = 0;
const int O_TRUNC = 0;
const int O_EXCL = 0;
const int O_SHLOCK = 0;
const int O_EXLOCK = 0;
const int O_NOFOLLOW = 0;
const int O_SYMLINK = 0;
const int O_CLOEXEC = 0;

final class ErrnoException extends \Exception {}

function poll_async(FileDescriptor $fd, int $events, int $timeout_ns): Awaitable<int>;

function open(string $path, int $flags, int $mode = 0): FileDescriptor;

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

const int SOCK_STREAM = 0;
const int SOCK_DGRAM = 0;
const int SOCK_RAW = 0;

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
  public function __construct(public string $pathname) {
    parent::__construct();
  }
}

final class sockaddr_un_unnamed extends sockaddr_un {
}

function getpeername(FileDescriptor $fd): sockaddr;
function getsockname(FileDescriptor $fd): sockaddr;
function socketpair(int $domain, int $type, int $protocol): (FileDescriptor, FileDescriptor);
