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
bool HPHP::f_posix_access(HPHP::String const&, int)
_ZN4HPHP14f_posix_accessERKNS_6StringEi

(return value) => rax
file => rdi
mode => rsi
*/

bool fh_posix_access(Value* file, int mode) asm("_ZN4HPHP14f_posix_accessERKNS_6StringEi");

/*
HPHP::String HPHP::f_posix_ctermid()
_ZN4HPHP15f_posix_ctermidEv

(return value) => rax
_rv => rdi
*/

Value* fh_posix_ctermid(Value* _rv) asm("_ZN4HPHP15f_posix_ctermidEv");

/*
long HPHP::f_posix_get_last_error()
_ZN4HPHP22f_posix_get_last_errorEv

(return value) => rax
*/

long fh_posix_get_last_error() asm("_ZN4HPHP22f_posix_get_last_errorEv");

/*
HPHP::String HPHP::f_posix_getcwd()
_ZN4HPHP14f_posix_getcwdEv

(return value) => rax
_rv => rdi
*/

Value* fh_posix_getcwd(Value* _rv) asm("_ZN4HPHP14f_posix_getcwdEv");

/*
long HPHP::f_posix_getegid()
_ZN4HPHP15f_posix_getegidEv

(return value) => rax
*/

long fh_posix_getegid() asm("_ZN4HPHP15f_posix_getegidEv");

/*
long HPHP::f_posix_geteuid()
_ZN4HPHP15f_posix_geteuidEv

(return value) => rax
*/

long fh_posix_geteuid() asm("_ZN4HPHP15f_posix_geteuidEv");

/*
long HPHP::f_posix_getgid()
_ZN4HPHP14f_posix_getgidEv

(return value) => rax
*/

long fh_posix_getgid() asm("_ZN4HPHP14f_posix_getgidEv");

/*
HPHP::Variant HPHP::f_posix_getgrgid(int)
_ZN4HPHP16f_posix_getgrgidEi

(return value) => rax
_rv => rdi
gid => rsi
*/

TypedValue* fh_posix_getgrgid(TypedValue* _rv, int gid) asm("_ZN4HPHP16f_posix_getgrgidEi");

/*
HPHP::Variant HPHP::f_posix_getgrnam(HPHP::String const&)
_ZN4HPHP16f_posix_getgrnamERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

TypedValue* fh_posix_getgrnam(TypedValue* _rv, Value* name) asm("_ZN4HPHP16f_posix_getgrnamERKNS_6StringE");

/*
HPHP::Variant HPHP::f_posix_getgroups()
_ZN4HPHP17f_posix_getgroupsEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_getgroups(TypedValue* _rv) asm("_ZN4HPHP17f_posix_getgroupsEv");

/*
HPHP::Variant HPHP::f_posix_getlogin()
_ZN4HPHP16f_posix_getloginEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_getlogin(TypedValue* _rv) asm("_ZN4HPHP16f_posix_getloginEv");

/*
HPHP::Variant HPHP::f_posix_getpgid(int)
_ZN4HPHP15f_posix_getpgidEi

(return value) => rax
_rv => rdi
pid => rsi
*/

TypedValue* fh_posix_getpgid(TypedValue* _rv, int pid) asm("_ZN4HPHP15f_posix_getpgidEi");

/*
long HPHP::f_posix_getpgrp()
_ZN4HPHP15f_posix_getpgrpEv

(return value) => rax
*/

long fh_posix_getpgrp() asm("_ZN4HPHP15f_posix_getpgrpEv");

/*
long HPHP::f_posix_getpid()
_ZN4HPHP14f_posix_getpidEv

(return value) => rax
*/

long fh_posix_getpid() asm("_ZN4HPHP14f_posix_getpidEv");

/*
long HPHP::f_posix_getppid()
_ZN4HPHP15f_posix_getppidEv

(return value) => rax
*/

long fh_posix_getppid() asm("_ZN4HPHP15f_posix_getppidEv");

/*
HPHP::Variant HPHP::f_posix_getpwnam(HPHP::String const&)
_ZN4HPHP16f_posix_getpwnamERKNS_6StringE

(return value) => rax
_rv => rdi
username => rsi
*/

TypedValue* fh_posix_getpwnam(TypedValue* _rv, Value* username) asm("_ZN4HPHP16f_posix_getpwnamERKNS_6StringE");

/*
HPHP::Variant HPHP::f_posix_getpwuid(int)
_ZN4HPHP16f_posix_getpwuidEi

(return value) => rax
_rv => rdi
uid => rsi
*/

TypedValue* fh_posix_getpwuid(TypedValue* _rv, int uid) asm("_ZN4HPHP16f_posix_getpwuidEi");

/*
HPHP::Variant HPHP::f_posix_getrlimit()
_ZN4HPHP17f_posix_getrlimitEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_getrlimit(TypedValue* _rv) asm("_ZN4HPHP17f_posix_getrlimitEv");

/*
HPHP::Variant HPHP::f_posix_getsid(int)
_ZN4HPHP14f_posix_getsidEi

(return value) => rax
_rv => rdi
pid => rsi
*/

TypedValue* fh_posix_getsid(TypedValue* _rv, int pid) asm("_ZN4HPHP14f_posix_getsidEi");

/*
long HPHP::f_posix_getuid()
_ZN4HPHP14f_posix_getuidEv

(return value) => rax
*/

long fh_posix_getuid() asm("_ZN4HPHP14f_posix_getuidEv");

/*
bool HPHP::f_posix_initgroups(HPHP::String const&, int)
_ZN4HPHP18f_posix_initgroupsERKNS_6StringEi

(return value) => rax
name => rdi
base_group_id => rsi
*/

bool fh_posix_initgroups(Value* name, int base_group_id) asm("_ZN4HPHP18f_posix_initgroupsERKNS_6StringEi");

/*
bool HPHP::f_posix_isatty(HPHP::Variant const&)
_ZN4HPHP14f_posix_isattyERKNS_7VariantE

(return value) => rax
fd => rdi
*/

bool fh_posix_isatty(TypedValue* fd) asm("_ZN4HPHP14f_posix_isattyERKNS_7VariantE");

/*
bool HPHP::f_posix_kill(int, int)
_ZN4HPHP12f_posix_killEii

(return value) => rax
pid => rdi
sig => rsi
*/

bool fh_posix_kill(int pid, int sig) asm("_ZN4HPHP12f_posix_killEii");

/*
bool HPHP::f_posix_mkfifo(HPHP::String const&, int)
_ZN4HPHP14f_posix_mkfifoERKNS_6StringEi

(return value) => rax
pathname => rdi
mode => rsi
*/

bool fh_posix_mkfifo(Value* pathname, int mode) asm("_ZN4HPHP14f_posix_mkfifoERKNS_6StringEi");

/*
bool HPHP::f_posix_mknod(HPHP::String const&, int, int, int)
_ZN4HPHP13f_posix_mknodERKNS_6StringEiii

(return value) => rax
pathname => rdi
mode => rsi
major => rdx
minor => rcx
*/

bool fh_posix_mknod(Value* pathname, int mode, int major, int minor) asm("_ZN4HPHP13f_posix_mknodERKNS_6StringEiii");

/*
bool HPHP::f_posix_setegid(int)
_ZN4HPHP15f_posix_setegidEi

(return value) => rax
gid => rdi
*/

bool fh_posix_setegid(int gid) asm("_ZN4HPHP15f_posix_setegidEi");

/*
bool HPHP::f_posix_seteuid(int)
_ZN4HPHP15f_posix_seteuidEi

(return value) => rax
uid => rdi
*/

bool fh_posix_seteuid(int uid) asm("_ZN4HPHP15f_posix_seteuidEi");

/*
bool HPHP::f_posix_setgid(int)
_ZN4HPHP14f_posix_setgidEi

(return value) => rax
gid => rdi
*/

bool fh_posix_setgid(int gid) asm("_ZN4HPHP14f_posix_setgidEi");

/*
bool HPHP::f_posix_setpgid(int, int)
_ZN4HPHP15f_posix_setpgidEii

(return value) => rax
pid => rdi
pgid => rsi
*/

bool fh_posix_setpgid(int pid, int pgid) asm("_ZN4HPHP15f_posix_setpgidEii");

/*
long HPHP::f_posix_setsid()
_ZN4HPHP14f_posix_setsidEv

(return value) => rax
*/

long fh_posix_setsid() asm("_ZN4HPHP14f_posix_setsidEv");

/*
bool HPHP::f_posix_setuid(int)
_ZN4HPHP14f_posix_setuidEi

(return value) => rax
uid => rdi
*/

bool fh_posix_setuid(int uid) asm("_ZN4HPHP14f_posix_setuidEi");

/*
HPHP::String HPHP::f_posix_strerror(int)
_ZN4HPHP16f_posix_strerrorEi

(return value) => rax
_rv => rdi
errnum => rsi
*/

Value* fh_posix_strerror(Value* _rv, int errnum) asm("_ZN4HPHP16f_posix_strerrorEi");

/*
HPHP::Variant HPHP::f_posix_times()
_ZN4HPHP13f_posix_timesEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_times(TypedValue* _rv) asm("_ZN4HPHP13f_posix_timesEv");

/*
HPHP::Variant HPHP::f_posix_ttyname(HPHP::Variant const&)
_ZN4HPHP15f_posix_ttynameERKNS_7VariantE

(return value) => rax
_rv => rdi
fd => rsi
*/

TypedValue* fh_posix_ttyname(TypedValue* _rv, TypedValue* fd) asm("_ZN4HPHP15f_posix_ttynameERKNS_7VariantE");

/*
HPHP::Variant HPHP::f_posix_uname()
_ZN4HPHP13f_posix_unameEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_posix_uname(TypedValue* _rv) asm("_ZN4HPHP13f_posix_unameEv");


} // !HPHP

