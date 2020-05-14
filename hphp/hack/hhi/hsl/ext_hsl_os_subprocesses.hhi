<?hh
/**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH\Lib\_Private\_OS;

use type HH\Lib\OS\FileDescriptor;

type ForkAndExecveOptions = shape(
  ?'cwd' => string,
  ?'setsid' => bool,
  ?'execvpe' => bool,
  ?'setpgid' => int,
);

function fork_and_execve(
  string $path,
  vec<string> $argv,
  vec<string> $envp,
  dict<int, FileDescriptor> $fds,
  ForkAndExecveOptions $options,
): int;
