<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

namespace HH\Lib\_Private\_OS;

use namespace HH\Lib\{C, Dict, OS, Str, Vec};
use namespace HH\Lib\_Private\_OS;

type TForkAndExecve = (function(
  string,
  vec<string>,
  vec<string>,
  dict<int, OS\FileDescriptor>,
  _OS\ForkAndExecveOptions,
): int);

const keyset<int> STD_FILENOS =
  keyset[_OS\STDIN_FILENO, _OS\STDOUT_FILENO, _OS\STDERR_FILENO];

// `fork_and_execve_default` will call `fork_and_execve` but redirect
// standard I/O to /dev/null by default.
function fork_and_execve_default(
  string $path,
  vec<string> $argv,
  vec<string> $envp,
  dict<int, OS\FileDescriptor> $fds,
  _OS\ForkAndExecveOptions $options,
): int {
  if (C\every(STD_FILENOS, $std_fileno ==> C\contains_key($fds, $std_fileno))) {
    // Don't open /dev/null because all standard file numbers are specified
    return _OS\fork_and_execve($path, $argv, $envp, $fds, $options);
  }

  $dev_null_fd = _OS\open('/dev/null', O_RDWR);
  try {
    return _OS\fork_and_execve(
      $path,
      $argv,
      $envp,
      Dict\merge(Dict\fill_keys(STD_FILENOS, $dev_null_fd), $fds),
      $options,
    );
  } finally {
    _OS\close($dev_null_fd);
  }
}

abstract class PosixSpawnFileActionDecorator {

  // Each file action is interpreted as a decorator, adding more options and
  // flags onto `$fork_and_execve`.
  public static function forkAndExecveFromFileActions(
    OS\posix_spawn_file_actions_t $file_actions,
  ): _OS\TForkAndExecve {
    return C\reduce(
      Vec\reverse($file_actions),
      ($fork_and_execve, $file_action) ==>
        ($file_action as _OS\PosixSpawnFileActionDecorator)
          ->forkAndExecveDecorator($fork_and_execve),
      _OS\fork_and_execve_default<>,
    );
  }

  // Interpret this file action as a decorator, adding more options and flags
  // onto `$fork_and_execve`.
  abstract protected function forkAndExecveDecorator(
    _OS\TForkAndExecve $fork_and_execve,
  ): _OS\TForkAndExecve;

}


// Convert a posix_spawnattr_t to an _OS\ForkAndExecveOptions
function posix_spawnattr_t_to_options(
  _OS\ForkAndExecveOptions $default_options,
  OS\posix_spawnattr_t $attributes,
): _OS\ForkAndExecveOptions {
  $flags = Shapes::idx($attributes, 'posix_spawnattr_setflags');
  $options = $default_options;
  if ($flags is nonnull) {
    $options['setsid'] = ($flags & OS\POSIX_SPAWN_SETSID) !== 0;
    if (($flags & OS\POSIX_SPAWN_SETPGROUP) !== 0) {
      $options['setpgid'] =
        Shapes::at($attributes, 'posix_spawnattr_setpgroup');
    }
  }
  return $options;
}
