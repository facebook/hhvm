<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\OS;

use namespace HH\Lib\_Private\_OS;
use namespace HH\Lib\{C, Vec};

newtype pid_t as int = int;
newtype ExitCode as int = int;

<<__Sealed(
  posix_spawn_file_actions_addopen::class,
  posix_spawn_file_actions_adddup2::class,
  posix_spawn_file_actions_addchdir_np::class,
)>>
interface PosixSpawnFileActionsSetter {}

final class posix_spawn_file_actions_addopen
  implements PosixSpawnFileActionsSetter {
  public function __construct(
    public int $filedes,
    public string $path,
    public int $oflag,
    public int $mode,
  ) {}
}

final class posix_spawn_file_actions_adddup2
  implements PosixSpawnFileActionsSetter {
  public function __construct(
    public FileDescriptor $filedes,
    public int $newfiledes,
  ) {}
}

final class posix_spawn_file_actions_addchdir_np
  implements PosixSpawnFileActionsSetter {
  public function __construct(public string $path) {}
}

type posix_spawn_file_actions_t = vec<PosixSpawnFileActionsSetter>;

const int POSIX_SPAWN_SETPGROUP = 2;
const int POSIX_SPAWN_SETSID = _OS\IS_MACOS ? 0x0400 : 0x80;

type PosixSpawnFlags = int;

type posix_spawnattr_t = shape(
  ?'posix_spawnattr_setpgroup' => pid_t,
  ?'posix_spawnattr_setflags' => PosixSpawnFlags,
);

function posix_spawn(
  string $file,
  posix_spawn_file_actions_t $file_actions,
  posix_spawnattr_t $attributes,
  vec<string> $argv,
  vec<string> $envp,
): pid_t {
  $fork_and_execve = _OS\fork_and_execve_with_file_actions(_OS\fork_and_execve_default<>, $file_actions);
  $options =
    _OS\posix_spawnattr_t_to_options(shape('execvpe' => false), $attributes);
  return _OS\wrap_impl(
    () ==> $fork_and_execve($file, $argv, $envp, dict[], $options),
  );
}


function posix_spawnp(
  string $file,
  posix_spawn_file_actions_t $file_actions,
  posix_spawnattr_t $attributes,
  vec<string> $argv,
  vec<string> $envp,
): pid_t {
  $fork_and_execve = _OS\fork_and_execve_with_file_actions(_OS\fork_and_execve_default<>, $file_actions);
  $options =
    _OS\posix_spawnattr_t_to_options(shape('execvpe' => true), $attributes);
  return _OS\wrap_impl(
    () ==> $fork_and_execve($file, $argv, $envp, dict[], $options),
  );
}
