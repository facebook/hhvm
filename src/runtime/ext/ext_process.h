/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EXT_PROCESS_H__
#define __EXT_PROCESS_H__

#include <runtime/base/base_includes.h>
#include <sys/wait.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

inline int f_pcntl_alarm(int seconds) {
  return alarm(seconds);
}
void f_pcntl_exec(CStrRef path, CArrRef args = null_array, CArrRef envs = null_array);

int f_pcntl_fork();
Variant f_pcntl_getpriority(int pid = 0, int process_identifier = 0);
bool f_pcntl_setpriority(int priority, int pid = 0,
                         int process_identifier = 0);

bool f_pcntl_signal(int signo, CVarRef handler, bool restart_syscalls = true);
int f_pcntl_wait(VRefParam status, int options = 0);
int f_pcntl_waitpid(int pid, VRefParam status, int options = 0);

inline int f_pcntl_wexitstatus(int status) {
  return WEXITSTATUS(status);
}

/**
 * Process pending signals flagged earlier.
 */
bool f_pcntl_signal_dispatch();

// status querying
inline bool f_pcntl_wifexited(int status) { return WIFEXITED(status);}
inline bool f_pcntl_wifsignaled(int status) { return WIFSIGNALED(status);}
inline bool f_pcntl_wifstopped(int status) { return WIFSTOPPED(status);}
inline int f_pcntl_wstopsig(int status) { return WSTOPSIG(status);}
inline int f_pcntl_wtermsig(int status) { return WTERMSIG(status);}

///////////////////////////////////////////////////////////////////////////////

String f_shell_exec(CStrRef cmd);
String f_exec(CStrRef command, VRefParam output = null,
              VRefParam return_var = null);
void f_passthru(CStrRef command, VRefParam return_var = null);
String f_system(CStrRef command, VRefParam return_var = null);

///////////////////////////////////////////////////////////////////////////////

Variant f_proc_open(CStrRef cmd, CArrRef descriptorspec, VRefParam pipes,
                    CStrRef cwd = null_string, CVarRef env = null_variant,
                    CVarRef other_options = null_variant);
bool f_proc_terminate(CObjRef process, int signal = 0);
int f_proc_close(CObjRef process);
Array f_proc_get_status(CObjRef process);
bool f_proc_nice(int increment);

///////////////////////////////////////////////////////////////////////////////

String f_escapeshellarg(CStrRef arg);
String f_escapeshellcmd(CStrRef command);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_PROCESS_H__
