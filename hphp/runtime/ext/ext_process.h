/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_EXT_PROCESS_H_
#define incl_HPHP_EXT_PROCESS_H_

#include "hphp/runtime/base/base-includes.h"
#include <sys/wait.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int64_t f_pcntl_alarm(int seconds);
void f_pcntl_exec(const String& path, CArrRef args = null_array, CArrRef envs = null_array);

int64_t f_pcntl_fork();
Variant f_pcntl_getpriority(int pid = 0, int process_identifier = 0);
bool f_pcntl_setpriority(int priority, int pid = 0,
                         int process_identifier = 0);

bool f_pcntl_signal(int signo, CVarRef handler, bool restart_syscalls = true);
int64_t f_pcntl_wait(VRefParam status, int options = 0);
int64_t f_pcntl_waitpid(int pid, VRefParam status, int options = 0);

int64_t f_pcntl_wexitstatus(int status);

/**
 * Process pending signals flagged earlier.
 */
bool f_pcntl_signal_dispatch();

// status querying
bool f_pcntl_wifexited(int status);
bool f_pcntl_wifsignaled(int status);
bool f_pcntl_wifstopped(int status);
int64_t f_pcntl_wstopsig(int status);
int64_t f_pcntl_wtermsig(int status);

///////////////////////////////////////////////////////////////////////////////

String f_shell_exec(const String& cmd);
String f_exec(const String& command, VRefParam output = uninit_null(),
              VRefParam return_var = uninit_null());
void f_passthru(const String& command, VRefParam return_var = uninit_null());
String f_system(const String& command, VRefParam return_var = uninit_null());

///////////////////////////////////////////////////////////////////////////////

Variant f_proc_open(const String& cmd, CArrRef descriptorspec, VRefParam pipes,
                    const String& cwd = null_string, CVarRef env = null_variant,
                    CVarRef other_options = null_variant);
bool f_proc_terminate(CResRef process, int signal = 0);
int64_t f_proc_close(CResRef process);
Array f_proc_get_status(CResRef process);
bool f_proc_nice(int increment);

///////////////////////////////////////////////////////////////////////////////

String f_escapeshellarg(const String& arg);
String f_escapeshellcmd(const String& command);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_PROCESS_H_
