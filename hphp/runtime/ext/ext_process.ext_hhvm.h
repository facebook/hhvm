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
namespace HPHP {

long fh_pcntl_alarm(int seconds) asm("_ZN4HPHP13f_pcntl_alarmEi");

void fh_pcntl_exec(Value* path, Value* args, Value* envs) asm("_ZN4HPHP12f_pcntl_execERKNS_6StringERKNS_5ArrayES5_");

long fh_pcntl_fork() asm("_ZN4HPHP12f_pcntl_forkEv");

TypedValue* fh_pcntl_getpriority(TypedValue* _rv, int pid, int process_identifier) asm("_ZN4HPHP19f_pcntl_getpriorityEii");

bool fh_pcntl_setpriority(int priority, int pid, int process_identifier) asm("_ZN4HPHP19f_pcntl_setpriorityEiii");

bool fh_pcntl_signal_dispatch() asm("_ZN4HPHP23f_pcntl_signal_dispatchEv");

bool fh_pcntl_signal(int signo, TypedValue* handler, bool restart_syscalls) asm("_ZN4HPHP14f_pcntl_signalEiRKNS_7VariantEb");

long fh_pcntl_wait(TypedValue* status, int options) asm("_ZN4HPHP12f_pcntl_waitERKNS_14VRefParamValueEi");

long fh_pcntl_waitpid(int pid, TypedValue* status, int options) asm("_ZN4HPHP15f_pcntl_waitpidEiRKNS_14VRefParamValueEi");

long fh_pcntl_wexitstatus(int status) asm("_ZN4HPHP19f_pcntl_wexitstatusEi");

bool fh_pcntl_wifexited(int status) asm("_ZN4HPHP17f_pcntl_wifexitedEi");

bool fh_pcntl_wifsignaled(int status) asm("_ZN4HPHP19f_pcntl_wifsignaledEi");

bool fh_pcntl_wifstopped(int status) asm("_ZN4HPHP18f_pcntl_wifstoppedEi");

long fh_pcntl_wstopsig(int status) asm("_ZN4HPHP16f_pcntl_wstopsigEi");

long fh_pcntl_wtermsig(int status) asm("_ZN4HPHP16f_pcntl_wtermsigEi");

Value* fh_shell_exec(Value* _rv, Value* cmd) asm("_ZN4HPHP12f_shell_execERKNS_6StringE");

Value* fh_exec(Value* _rv, Value* command, TypedValue* output, TypedValue* return_var) asm("_ZN4HPHP6f_execERKNS_6StringERKNS_14VRefParamValueES5_");

void fh_passthru(Value* command, TypedValue* return_var) asm("_ZN4HPHP10f_passthruERKNS_6StringERKNS_14VRefParamValueE");

Value* fh_system(Value* _rv, Value* command, TypedValue* return_var) asm("_ZN4HPHP8f_systemERKNS_6StringERKNS_14VRefParamValueE");

TypedValue* fh_proc_open(TypedValue* _rv, Value* cmd, Value* descriptorspec, TypedValue* pipes, Value* cwd, TypedValue* env, TypedValue* other_options) asm("_ZN4HPHP11f_proc_openERKNS_6StringERKNS_5ArrayERKNS_14VRefParamValueES2_RKNS_7VariantESB_");

bool fh_proc_terminate(Value* process, int signal) asm("_ZN4HPHP16f_proc_terminateERKNS_6ObjectEi");

long fh_proc_close(Value* process) asm("_ZN4HPHP12f_proc_closeERKNS_6ObjectE");

Value* fh_proc_get_status(Value* _rv, Value* process) asm("_ZN4HPHP17f_proc_get_statusERKNS_6ObjectE");

bool fh_proc_nice(int increment) asm("_ZN4HPHP11f_proc_niceEi");

Value* fh_escapeshellarg(Value* _rv, Value* arg) asm("_ZN4HPHP16f_escapeshellargERKNS_6StringE");

Value* fh_escapeshellcmd(Value* _rv, Value* command) asm("_ZN4HPHP16f_escapeshellcmdERKNS_6StringE");

} // namespace HPHP
