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

use namespace HH\Lib\_Private\{_File, _OS};
use namespace HH\Lib\{C, OS, Vec};

newtype pid_t as int = int;
newtype ExitCode as int = int;

<<__Sealed(
  posix_spawn_file_actions_addopen::class,
  posix_spawn_file_actions_adddup2::class,
  posix_spawn_file_actions_addchdir_np::class,
)>>
interface PosixSpawnFileActionsSetter {}

final class posix_spawn_file_actions_addopen
  extends _OS\PosixSpawnFileActionDecorator
  implements PosixSpawnFileActionsSetter {
    public function __construct(
    public int $filedes,
    public string $path,
    public int $oflag,
    public int $mode,
  ) {}

  // A decorator to open a file for the spawned process
  <<__Override>>
  protected function forkAndExecveDecorator(
    _OS\TForkAndExecve $fork_and_execve,
  ): _OS\TForkAndExecve {
    return ($path, $argv, $envp, $fds, $options) ==> {
      $fd = _OS\open(
        _File\relative_path($this->path, Shapes::idx($options, 'cwd')),
        $this->oflag,
        $this->mode,
      );
      try {
        $fds[$this->filedes] = $fd;
        return $fork_and_execve($path, $argv, $envp, $fds, $options);
      } finally {
        _OS\close($fd);
      }
    };
  }
}

final class posix_spawn_file_actions_adddup2
  extends _OS\PosixSpawnFileActionDecorator
  implements PosixSpawnFileActionsSetter {
  public function __construct(
    public FileDescriptor $filedes,
    public int $newfiledes,
  ) {}

  // A decorator to pass an FD to the spawned process
  <<__Override>>
  protected function forkAndExecveDecorator(
    _OS\TForkAndExecve $fork_and_execve,
  ): _OS\TForkAndExecve {
    return ($path, $argv, $envp, $fds, $options) ==> {
      $fds[$this->newfiledes] = $this->filedes;
      return $fork_and_execve($path, $argv, $envp, $fds, $options);
    };
  }
}

final class posix_spawn_file_actions_addchdir_np
  extends _OS\PosixSpawnFileActionDecorator
  implements PosixSpawnFileActionsSetter {
  public function __construct(public string $path) {}

  // A decorator to change directory when spawn the process
  <<__Override>>
  protected function forkAndExecveDecorator(
    _OS\TForkAndExecve $fork_and_execve,
  ): _OS\TForkAndExecve {
    return ($path, $argv, $envp, $fds, $options) ==> {
      $new_cwd =
        _File\relative_path($this->path, Shapes::idx($options, 'cwd'));
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
  $fork_and_execve =
    _OS\PosixSpawnFileActionDecorator::forkAndExecveFromFileActions(
      $file_actions
    );
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
  $fork_and_execve =
    _OS\PosixSpawnFileActionDecorator::forkAndExecveFromFileActions(
      $file_actions
    );
  $options =
    _OS\posix_spawnattr_t_to_options(shape('execvpe' => true), $attributes);
  return _OS\wrap_impl(
    () ==> $fork_and_execve($file, $argv, $envp, dict[], $options),
  );
}
