<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */
use namespace HH\Lib\{File, IO, OS};
use namespace \HH\Lib\_Private\_OS;
use function HH\__Private\MiniTest\expect;
use type HH\__Private\MiniTest\HackTest;

final class SpawnTest extends HackTest {
  public async function testBasicUsage(): Awaitable<void> {
    list($read_pipe, $write_pipe) = IO\pipe();
    using $read_pipe->closeWhenDisposed();
    using ($write_pipe->closeWhenDisposed()) {
      $pid = OS\posix_spawnp(
        'ls',
        vec[
          new OS\posix_spawn_file_actions_addchdir_np(__DIR__),
          new OS\posix_spawn_file_actions_adddup2(
            $write_pipe->getFileDescriptor(),
            _OS\STDOUT_FILENO,
          ),
        ],
        shape(),
        vec['ls', '.'],
        vec[],
      );
    }
    try {
      $output = await $read_pipe->readAllAsync();
    } finally {
      $status = null;
      \pcntl_waitpid($pid, inout $status);
    }
    expect(\pcntl_wexitstatus($status as nonnull))->toEqual(0);
    expect($output)->toContainSubstring(\basename(__FILE__));
  }

  public async function testChdirThenOpen(): Awaitable<void> {
    $tmp_file_path = \tempnam('/tmp', __FUNCTION__);
    $pid = OS\posix_spawnp(
      'echo',
      vec[
        new OS\posix_spawn_file_actions_addchdir_np('/tmp'),
        new OS\posix_spawn_file_actions_addopen(
          _OS\STDOUT_FILENO,
          \basename($tmp_file_path),
          OS\O_WRONLY,
          OS\O_CREAT,
        ),
      ],
      shape(),
      vec['echo', 'test output'],
      vec[],
    );
    $status = null;
    \pcntl_waitpid($pid, inout $status);
    expect(\pcntl_wexitstatus($status as nonnull))->toEqual(0);
    $handle = File\open_read_only($tmp_file_path);
    using $handle->closeWhenDisposed();
    $output = await $handle->readAllAsync();
    expect($output)->toContainSubstring('test output');
  }

  public async function testPipeUsage(): Awaitable<void> {
    list($ls_read_pipe, $find_write_pipe) = IO\pipe();
    try {
      using ($find_write_pipe->closeWhenDisposed()) {
        $find_process = OS\posix_spawnp(
          'find',
          vec[
            new OS\posix_spawn_file_actions_adddup2(
              $find_write_pipe->getFileDescriptor(),
              _OS\STDOUT_FILENO,
            ),
          ],
          shape(),
          vec['find', __DIR__, '-type', 'f', '-print0'],
          vec[],
        );
      }
      list($final_read_pipe, $ls_write_pipe) = IO\pipe();
    } catch (\Throwable $e) {
      $ls_read_pipe->close();
      throw $e;
    }
    using $final_read_pipe->closeWhenDisposed();
    try {
      using (
        $ls_read_pipe->closeWhenDisposed(),
        $ls_write_pipe->closeWhenDisposed()
      ) {
        $cat_process = OS\posix_spawnp(
          'xargs',
          vec[
            new OS\posix_spawn_file_actions_adddup2(
              $ls_write_pipe->getFileDescriptor(),
              _OS\STDOUT_FILENO,
            ),
            new OS\posix_spawn_file_actions_adddup2(
              $ls_read_pipe->getFileDescriptor(),
              _OS\STDIN_FILENO,
            ),
          ],
          shape(),
          vec['xargs', '-0', 'cat'],
          vec[],
        );
      }
      try {
        $output = await $final_read_pipe->readAllAsync();
      } finally {
        $cat_status = null;
        \pcntl_waitpid($cat_process, inout $cat_status);
      }
    } finally {
      $find_status = null;
      \pcntl_waitpid($find_process, inout $find_status);
    }
    expect(\pcntl_wexitstatus($find_status as nonnull))->toEqual(0);
    expect(\pcntl_wexitstatus($cat_status as nonnull))->toEqual(0);
    expect($output)->toContainSubstring(__FUNCTION__);
  }

  public async function testPosixSpawn(): Awaitable<void> {
    expect(() ==> OS\posix_spawn('true', vec[], shape(), vec['true'], vec[]))
      // Clang compiled hhvm throws an IsNotADirectoryException,
      // while gcc compiled hhvm throws a NotFoundException.
      ->toThrow(OS\ErrnoException::class)
      |> expect($$->getMessage())->toContainSubstring(' execve() failed');

    $pid = OS\posix_spawn(
      '/usr/bin/env',
      vec[],
      shape(),
      vec['/usr/bin/env', 'true'],
      vec[],
    );
    $status = null;
    \pcntl_waitpid($pid, inout $status);
    expect(\pcntl_wexitstatus($status as nonnull))->toEqual(0);
  }

}
