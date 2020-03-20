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
