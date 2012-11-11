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

/*
void HPHP::f_pcntl_exec(HPHP::String const&, HPHP::Array const&, HPHP::Array const&)
_ZN4HPHP12f_pcntl_execERKNS_6StringERKNS_5ArrayES5_

path => rdi
args => rsi
envs => rdx
*/

void fh_pcntl_exec(Value* path, Value* args, Value* envs) asm("_ZN4HPHP12f_pcntl_execERKNS_6StringERKNS_5ArrayES5_");

/*
long long HPHP::f_pcntl_fork()
_ZN4HPHP12f_pcntl_forkEv

(return value) => rax
*/

long long fh_pcntl_fork() asm("_ZN4HPHP12f_pcntl_forkEv");

/*
HPHP::Variant HPHP::f_pcntl_getpriority(int, int)
_ZN4HPHP19f_pcntl_getpriorityEii

(return value) => rax
_rv => rdi
pid => rsi
process_identifier => rdx
*/

TypedValue* fh_pcntl_getpriority(TypedValue* _rv, int pid, int process_identifier) asm("_ZN4HPHP19f_pcntl_getpriorityEii");

/*
bool HPHP::f_pcntl_setpriority(int, int, int)
_ZN4HPHP19f_pcntl_setpriorityEiii

(return value) => rax
priority => rdi
pid => rsi
process_identifier => rdx
*/

bool fh_pcntl_setpriority(int priority, int pid, int process_identifier) asm("_ZN4HPHP19f_pcntl_setpriorityEiii");

/*
bool HPHP::f_pcntl_signal(int, HPHP::Variant const&, bool)
_ZN4HPHP14f_pcntl_signalEiRKNS_7VariantEb

(return value) => rax
signo => rdi
handler => rsi
restart_syscalls => rdx
*/

bool fh_pcntl_signal(int signo, TypedValue* handler, bool restart_syscalls) asm("_ZN4HPHP14f_pcntl_signalEiRKNS_7VariantEb");

/*
long long HPHP::f_pcntl_wait(HPHP::VRefParamValue const&, int)
_ZN4HPHP12f_pcntl_waitERKNS_14VRefParamValueEi

(return value) => rax
status => rdi
options => rsi
*/

long long fh_pcntl_wait(TypedValue* status, int options) asm("_ZN4HPHP12f_pcntl_waitERKNS_14VRefParamValueEi");

/*
long long HPHP::f_pcntl_waitpid(int, HPHP::VRefParamValue const&, int)
_ZN4HPHP15f_pcntl_waitpidEiRKNS_14VRefParamValueEi

(return value) => rax
pid => rdi
status => rsi
options => rdx
*/

long long fh_pcntl_waitpid(int pid, TypedValue* status, int options) asm("_ZN4HPHP15f_pcntl_waitpidEiRKNS_14VRefParamValueEi");

/*
bool HPHP::f_pcntl_signal_dispatch()
_ZN4HPHP23f_pcntl_signal_dispatchEv

(return value) => rax
*/

bool fh_pcntl_signal_dispatch() asm("_ZN4HPHP23f_pcntl_signal_dispatchEv");

/*
HPHP::String HPHP::f_shell_exec(HPHP::String const&)
_ZN4HPHP12f_shell_execERKNS_6StringE

(return value) => rax
_rv => rdi
cmd => rsi
*/

Value* fh_shell_exec(Value* _rv, Value* cmd) asm("_ZN4HPHP12f_shell_execERKNS_6StringE");

/*
HPHP::String HPHP::f_exec(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP6f_execERKNS_6StringERKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
command => rsi
output => rdx
return_var => rcx
*/

Value* fh_exec(Value* _rv, Value* command, TypedValue* output, TypedValue* return_var) asm("_ZN4HPHP6f_execERKNS_6StringERKNS_14VRefParamValueES5_");

/*
void HPHP::f_passthru(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP10f_passthruERKNS_6StringERKNS_14VRefParamValueE

command => rdi
return_var => rsi
*/

void fh_passthru(Value* command, TypedValue* return_var) asm("_ZN4HPHP10f_passthruERKNS_6StringERKNS_14VRefParamValueE");

/*
HPHP::String HPHP::f_system(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP8f_systemERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
command => rsi
return_var => rdx
*/

Value* fh_system(Value* _rv, Value* command, TypedValue* return_var) asm("_ZN4HPHP8f_systemERKNS_6StringERKNS_14VRefParamValueE");

/*
HPHP::Variant HPHP::f_proc_open(HPHP::String const&, HPHP::Array const&, HPHP::VRefParamValue const&, HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP11f_proc_openERKNS_6StringERKNS_5ArrayERKNS_14VRefParamValueES2_RKNS_7VariantESB_

(return value) => rax
_rv => rdi
cmd => rsi
descriptorspec => rdx
pipes => rcx
cwd => r8
env => r9
other_options => st0
*/

TypedValue* fh_proc_open(TypedValue* _rv, Value* cmd, Value* descriptorspec, TypedValue* pipes, Value* cwd, TypedValue* env, TypedValue* other_options) asm("_ZN4HPHP11f_proc_openERKNS_6StringERKNS_5ArrayERKNS_14VRefParamValueES2_RKNS_7VariantESB_");

/*
bool HPHP::f_proc_terminate(HPHP::Object const&, int)
_ZN4HPHP16f_proc_terminateERKNS_6ObjectEi

(return value) => rax
process => rdi
signal => rsi
*/

bool fh_proc_terminate(Value* process, int signal) asm("_ZN4HPHP16f_proc_terminateERKNS_6ObjectEi");

/*
long long HPHP::f_proc_close(HPHP::Object const&)
_ZN4HPHP12f_proc_closeERKNS_6ObjectE

(return value) => rax
process => rdi
*/

long long fh_proc_close(Value* process) asm("_ZN4HPHP12f_proc_closeERKNS_6ObjectE");

/*
HPHP::Array HPHP::f_proc_get_status(HPHP::Object const&)
_ZN4HPHP17f_proc_get_statusERKNS_6ObjectE

(return value) => rax
_rv => rdi
process => rsi
*/

Value* fh_proc_get_status(Value* _rv, Value* process) asm("_ZN4HPHP17f_proc_get_statusERKNS_6ObjectE");

/*
bool HPHP::f_proc_nice(int)
_ZN4HPHP11f_proc_niceEi

(return value) => rax
increment => rdi
*/

bool fh_proc_nice(int increment) asm("_ZN4HPHP11f_proc_niceEi");

/*
HPHP::String HPHP::f_escapeshellarg(HPHP::String const&)
_ZN4HPHP16f_escapeshellargERKNS_6StringE

(return value) => rax
_rv => rdi
arg => rsi
*/

Value* fh_escapeshellarg(Value* _rv, Value* arg) asm("_ZN4HPHP16f_escapeshellargERKNS_6StringE");

/*
HPHP::String HPHP::f_escapeshellcmd(HPHP::String const&)
_ZN4HPHP16f_escapeshellcmdERKNS_6StringE

(return value) => rax
_rv => rdi
command => rsi
*/

Value* fh_escapeshellcmd(Value* _rv, Value* command) asm("_ZN4HPHP16f_escapeshellcmdERKNS_6StringE");


} // !HPHP

