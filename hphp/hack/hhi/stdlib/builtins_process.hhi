<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const int WNOHANG   = 0b001;
const int WUNTRACED = 0b010;

function pcntl_alarm($seconds);
function pcntl_exec($path, $args = null, $envs = null);
function pcntl_fork();
function pcntl_getpriority($pid = 0, $process_identifier = 0);
function pcntl_setpriority($priority, $pid = 0, $process_identifier = 0);
function pcntl_signal($signo, $handler, $restart_syscalls = true);

function pcntl_wait(&$status, $options = 0);
function pcntl_waitpid($pid, &$status, $options = 0);
function pcntl_wexitstatus($status);
function pcntl_wifexited($status);
function pcntl_wifsignaled($status);
function pcntl_wifstopped($status);
function pcntl_wstopsig($status);
function pcntl_wtermsig($status);
function pcntl_signal_dispatch();
function shell_exec($cmd);
function exec($command, &$output = null, &$return_var = null);
function passthru($command, &$return_var = null);
function system($command, &$return_var = null);
function proc_open($cmd, $descriptorspec, &$pipes, $cwd = null, $env = null, $other_options = null);
function proc_terminate($process, $signal = 0);
function proc_close($process);
function proc_get_status($process);
function proc_nice($increment);
function escapeshellarg($arg);
function escapeshellcmd($command);

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
