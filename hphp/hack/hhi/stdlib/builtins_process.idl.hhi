<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function pcntl_alarm($seconds) { }
function pcntl_exec($path, $args = null, $envs = null) { }
function pcntl_fork() { }
function pcntl_getpriority($pid = 0, $process_identifier = 0) { }
function pcntl_setpriority($priority, $pid = 0, $process_identifier = 0) { }
function pcntl_signal($signo, $handler, $restart_syscalls = true) { }
function pcntl_wait(&$status, $options = 0) { }
function pcntl_waitpid($pid, &$status, $options = 0) { }
function pcntl_wexitstatus($status) { }
function pcntl_wifexited($status) { }
function pcntl_wifsignaled($status) { }
function pcntl_wifstopped($status) { }
function pcntl_wstopsig($status) { }
function pcntl_wtermsig($status) { }
function pcntl_signal_dispatch() { }
function shell_exec($cmd) { }
function exec($command, &$output = null, &$return_var = null) { }
function passthru($command, &$return_var = null) { }
function system($command, &$return_var = null) { }
function proc_open($cmd, $descriptorspec, &$pipes, $cwd = null, $env = null_variant, $other_options = null_variant) { }
function proc_terminate($process, $signal = 0) { }
function proc_close($process) { }
function proc_get_status($process) { }
function proc_nice($increment) { }
function escapeshellarg($arg) { }
function escapeshellcmd($command) { }
