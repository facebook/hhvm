<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function posix_access(string $file, int $mode = 0);
<<__PHPStdLib>>
function posix_ctermid();
<<__PHPStdLib>>
function posix_get_last_error();
<<__PHPStdLib>>
function posix_getcwd();
<<__PHPStdLib>>
function posix_getegid();
<<__PHPStdLib>>
function posix_geteuid();
<<__PHPStdLib>>
function posix_getgid();
<<__PHPStdLib>>
function posix_getgrgid(int $gid);
<<__PHPStdLib>>
function posix_getgrnam(string $name);
<<__PHPStdLib>>
function posix_getgroups();
<<__PHPStdLib>>
function posix_getlogin();
<<__PHPStdLib>>
function posix_getpgid(int $pid);
<<__PHPStdLib>>
function posix_getpgrp();
<<__PHPStdLib>>
function posix_getpid();
<<__PHPStdLib>>
function posix_getppid();
<<__PHPStdLib>>
function posix_getpwnam(string $username);
<<__PHPStdLib>>
function posix_getpwuid(int $uid);
<<__PHPStdLib>>
function posix_getrlimit();
<<__PHPStdLib>>
function posix_getsid(int $pid);
<<__PHPStdLib>>
function posix_getuid();
<<__PHPStdLib>>
function posix_initgroups(string $name, int $base_group_id);
<<__PHPStdLib>>
function posix_isatty($fd);
<<__PHPStdLib>>
function posix_kill(int $pid, int $sig);
<<__PHPStdLib>>
function posix_mkfifo(string $pathname, int $mode);
<<__PHPStdLib>>
function posix_mknod(string $pathname, int $mode, int $major = 0, int $minor = 0);
<<__PHPStdLib>>
function posix_setegid(int $gid);
<<__PHPStdLib>>
function posix_seteuid(int $uid);
<<__PHPStdLib>>
function posix_setgid(int $gid);
<<__PHPStdLib>>
function posix_setpgid(int $pid, int $pgid);
<<__PHPStdLib>>
function posix_setsid();
<<__PHPStdLib>>
function posix_setuid(int $uid);
<<__PHPStdLib>>
function posix_strerror(int $errnum);
<<__PHPStdLib>>
function posix_times();
<<__PHPStdLib>>
function posix_ttyname($fd);
<<__PHPStdLib>>
function posix_uname();
