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

function open(string $path, int $flags, int $mode = 0): FileDescriptor;

function read(
  FileDescriptor $fd,
  int $max_to_read,
): string;

function write(FileDescriptor $fd, string $data): int;

function close(FileDescriptor $fd): void;

function pipe(): (FileDescriptor, FileDescriptor);

function poll_async(FileDescriptor $fd, int $events, int $timeout_ns): Awaitable<int>;
