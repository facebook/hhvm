<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int WNOHANG;
const int WUNTRACED;

<<__PHPStdLib>>
function pcntl_alarm(int $seconds): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_exec(
  string $path,
  HH\FIXME\MISSING_PARAM_TYPE $args = null,
  HH\FIXME\MISSING_PARAM_TYPE $envs = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_fork(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_getpriority(
  int $pid = 0,
  int $process_identifier = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_setpriority(
  int $priority,
  int $pid = 0,
  int $process_identifier = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_signal(
  int $signo,
  HH\FIXME\MISSING_PARAM_TYPE $handler,
  bool $restart_syscalls = true,
): HH\FIXME\MISSING_RETURN_TYPE;

<<__PHPStdLib>>
function pcntl_wait(
  inout $status,
  int $options = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_waitpid(
  int $pid,
  inout ?int $status,
  int $options = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_wexitstatus(int $status): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_wifexited(int $status): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_wifsignaled(int $status): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_wifstopped(int $status): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_wstopsig(int $status): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_wtermsig(int $status): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pcntl_signal_dispatch(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function shell_exec(string $cmd): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function exec(
  string $command,
  inout $output,
  inout $return_var,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function passthru(
  string $command,
  inout $return_var,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function system(
  string $command,
  inout $return_var,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function proc_open(
  string $cmd,
  darray<int, mixed> $descriptorspec,
  inout $pipes,
  HH\FIXME\MISSING_PARAM_TYPE $cwd = null,
  HH\FIXME\MISSING_PARAM_TYPE $env = null,
  HH\FIXME\MISSING_PARAM_TYPE $other_options = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function proc_terminate(
  resource $process,
  int $signal = 0,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function proc_close(resource $process): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function proc_get_status(resource $process): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function proc_nice(int $increment): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function escapeshellarg(string $arg)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function escapeshellcmd(string $command): HH\FIXME\MISSING_RETURN_TYPE;

const int SIGABRT;
const int SIGALRM;
const int SIGBABY;
const int SIGBUS;
const int SIGCHLD;
const int SIGCLD;
const int SIGCONT;
const int SIGFPE;
const int SIGHUP;
const int SIGILL;
const int SIGINT;
const int SIGIO;
const int SIGIOT;
const int SIGKILL;
const int SIGPIPE;
const int SIGPOLL;
const int SIGPROF;
const int SIGPWR;
const int SIGQUIT;
const int SIGSEGV;
const int SIGSTKFLT;
const int SIGSTOP;
const int SIGSYS;
const int SIGTERM;
const int SIGTRAP;
const int SIGTSTP;
const int SIGTTIN;
const int SIGTTOU;
const int SIGURG;
const int SIGUSR1;
const int SIGUSR2;
const int SIGVTALRM;
const int SIGWINCH;
const int SIGXCPU;
const int SIGXFSZ;
const int SIG_DFL;
const int SIG_ERR;
const int SIG_IGN;
const int SIG_BLOCK;
const int SIG_UNBLOCK;
const int SIG_SETMASK;
