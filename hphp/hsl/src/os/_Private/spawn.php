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
use namespace HH\Lib\_Private\{_File, _OS};

type TForkAndExecveOptions = shape(
  ?'cwd' => string,
  ?'setsid' => bool,
  ?'execvpe' => bool,
  ?'setpgid' => int,
);
type TForkAndExecve = (function(
  string,
  vec<string>,
  vec<string>,
  dict<int, OS\FileDescriptor>,
  _OS\TForkAndExecveOptions,
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
  _OS\TForkAndExecveOptions $options,
): int {
  if (($options['execvpe'] ?? false) && !Str\contains($path, '/')) {
    $resolved_path = $path;
  } else {
    $resolved_path = _File\relative_path($path, Shapes::idx($options, 'cwd'));
  }
  if (C\every(STD_FILENOS, $std_fileno ==> C\contains_key($fds, $std_fileno))) {
    // Don't open /dev/null because all standard file numbers are specified
    return _OS\fork_and_execve($resolved_path, $argv, $envp, $fds, $options);
  }

  $dev_null_fd = _OS\open('/dev/null', O_RDWR);
  try {
    return _OS\fork_and_execve(
      $resolved_path,
      $argv,
      $envp,
      Dict\merge(Dict\fill_keys(STD_FILENOS, $dev_null_fd), $fds),
      $options,
    );
  } finally {
    _OS\close($dev_null_fd);
  }
}

// A decorator to change directory when spawn the process
function fork_and_execve_with_addchdir(
  _OS\TForkAndExecve $fork_and_execve,
  OS\posix_spawn_file_actions_addchdir_np $file_action,
): _OS\TForkAndExecve {
  return ($path, $argv, $envp, $fds, $options) ==> {
    $new_cwd =
      _File\relative_path($file_action->path, Shapes::idx($options, 'cwd'));
    $canonicalized_new_cwd = \realpath($new_cwd);
    if ($canonicalized_new_cwd === false) {
      _OS\throw_errno(OS\Errno::ENOENT, '%s does not exist', $new_cwd);
    } else {
      $canonicalized_new_cwd as string;
      if (\is_dir($canonicalized_new_cwd)) {
        $options['cwd'] = $canonicalized_new_cwd;
      } else {
        _OS\throw_errno(
          OS\Errno::ENOTDIR,
          '%s is not a directory',
          $canonicalized_new_cwd,
        );
      }
    }
    return $fork_and_execve($path, $argv, $envp, $fds, $options);
  };
}

// A decorator to pass an FD to the spawned process
function fork_and_execve_with_adddup2(
  _OS\TForkAndExecve $fork_and_execve,
  OS\posix_spawn_file_actions_adddup2 $file_action,
): TForkAndExecve {
  return ($path, $argv, $envp, $fds, $options) ==> {
    $fds[$file_action->newfiledes] = $file_action->filedes;
    return $fork_and_execve($path, $argv, $envp, $fds, $options);
  };
}

// A decorator to open a file for the spawned process
function fork_and_execve_with_addopen(
  _OS\TForkAndExecve $fork_and_execve,
  OS\posix_spawn_file_actions_addopen $file_action,
): TForkAndExecve {
  return ($path, $argv, $envp, $fds, $options) ==> {
    $fd = _OS\open(
      _File\relative_path($file_action->path, Shapes::idx($options, 'cwd')),
      $file_action->oflag,
      $file_action->mode,
    );
    try {
      $fds[$file_action->filedes] = $fd;
      return $fork_and_execve($path, $argv, $envp, $fds, $options);
    } finally {
      _OS\close($fd);
    }
  };
}

// Each file action is interpreted as a decorator, adding more options and
// flags onto `$fork_and_execve`.
function fork_and_execve_with_file_actions(
  _OS\TForkAndExecve $fork_and_execve,
  OS\posix_spawn_file_actions_t $file_actions,
): _OS\TForkAndExecve {
  return C\reduce(
    Vec\reverse($file_actions),
    ($fork_and_execve, $file_action) ==> {
      if ($file_action is OS\posix_spawn_file_actions_addchdir_np)
        return _OS\fork_and_execve_with_addchdir($fork_and_execve, $file_action);

      if ($file_action is OS\posix_spawn_file_actions_adddup2)
        return _OS\fork_and_execve_with_adddup2($fork_and_execve, $file_action);

      $file_action as OS\posix_spawn_file_actions_addopen;
      return _OS\fork_and_execve_with_addopen($fork_and_execve, $file_action);
    },
    $fork_and_execve,
  );
}

// Convert a posix_spawnattr_t to an _OS\TForkAndExecveOptions
function posix_spawnattr_t_to_options(
  _OS\TForkAndExecveOptions $default_options,
  OS\posix_spawnattr_t $attributes,
): _OS\TForkAndExecveOptions {
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
