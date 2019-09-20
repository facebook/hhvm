<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int WNOHANG   = 0b001;
const int WUNTRACED = 0b010;

<<__PHPStdLib>>
function pcntl_alarm(int $seconds);
<<__PHPStdLib>>
function pcntl_exec(string $path, $args = null, $envs = null);
<<__PHPStdLib>>
function pcntl_fork();
<<__PHPStdLib>>
function pcntl_getpriority(int $pid = 0, int $process_identifier = 0);
<<__PHPStdLib>>
function pcntl_setpriority(int $priority, int $pid = 0, int $process_identifier = 0);
<<__PHPStdLib>>
function pcntl_signal(int $signo, $handler, bool $restart_syscalls = true);

<<__PHPStdLib>>
function pcntl_wait(inout $status, int $options = 0);
<<__PHPStdLib>>
function pcntl_waitpid(int $pid, inout $status, int $options = 0);
<<__PHPStdLib>>
function pcntl_wexitstatus(int $status);
<<__PHPStdLib>>
function pcntl_wifexited(int $status);
<<__PHPStdLib>>
function pcntl_wifsignaled(int $status);
<<__PHPStdLib>>
function pcntl_wifstopped(int $status);
<<__PHPStdLib>>
function pcntl_wstopsig(int $status);
<<__PHPStdLib>>
function pcntl_wtermsig(int $status);
<<__PHPStdLib>>
function pcntl_signal_dispatch();
<<__PHPStdLib>>
function shell_exec(string $cmd);
<<__PHPStdLib>>
function exec(string $command, inout $output, inout $return_var);
<<__PHPStdLib>>
function passthru(string $command, inout $return_var);
<<__PHPStdLib>>
function system(string $command, inout $return_var);
<<__PHPStdLib>>
function proc_open(string $cmd, darray<int, mixed> $descriptorspec, inout $pipes, $cwd = null, $env = null, $other_options = null);
<<__PHPStdLib>>
function proc_terminate(resource $process, int $signal = 0);
<<__PHPStdLib>>
function proc_close(resource $process);
<<__PHPStdLib>>
function proc_get_status(resource $process);
<<__PHPStdLib>>
function proc_nice(int $increment);
<<__PHPStdLib>>
function escapeshellarg(string $arg);
<<__PHPStdLib>>
function escapeshellcmd(string $command);

const int SIGABRT = 6;
const int SIGALRM = 14;
const int SIGBABY = 31;
const int SIGBUS = 7;
const int SIGCHLD = 17;
const int SIGCLD = 17;
const int SIGCONT = 18;
const int SIGFPE = 8;
const int SIGHUP = 1;
const int SIGILL = 4;
const int SIGINT = 2;
const int SIGIO = 29;
const int SIGIOT = 6;
const int SIGKILL = 9;
const int SIGPIPE = 13;
const int SIGPOLL = 29;
const int SIGPROF = 27;
const int SIGPWR = 30;
const int SIGQUIT = 3;
const int SIGSEGV = 11;
const int SIGSTKFLT = 16;
const int SIGSTOP = 19;
const int SIGSYS = 31;
const int SIGTERM = 15;
const int SIGTRAP = 5;
const int SIGTSTP = 20;
const int SIGTTIN = 21;
const int SIGTTOU = 22;
const int SIGURG = 23;
const int SIGUSR1 = 10;
const int SIGUSR2 = 12;
const int SIGVTALRM = 26;
const int SIGWINCH = 28;
const int SIGXCPU = 24;
const int SIGXFSZ = 25;
const int SIG_DFL = 0;
const int SIG_ERR = -1;
const int SIG_IGN = 1;
const int SIG_BLOCK = 0;
const int SIG_UNBLOCK = 1;
const int SIG_SETMASK = 2;
