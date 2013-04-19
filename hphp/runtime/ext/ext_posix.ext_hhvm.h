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

bool fh_posix_access(Value* file, int mode) asm("_ZN4HPHP14f_posix_accessERKNS_6StringEi");

Value* fh_posix_ctermid(Value* _rv) asm("_ZN4HPHP15f_posix_ctermidEv");

long fh_posix_get_last_error() asm("_ZN4HPHP22f_posix_get_last_errorEv");

Value* fh_posix_getcwd(Value* _rv) asm("_ZN4HPHP14f_posix_getcwdEv");

long fh_posix_getegid() asm("_ZN4HPHP15f_posix_getegidEv");

long fh_posix_geteuid() asm("_ZN4HPHP15f_posix_geteuidEv");

long fh_posix_getgid() asm("_ZN4HPHP14f_posix_getgidEv");

TypedValue* fh_posix_getgrgid(TypedValue* _rv, int gid) asm("_ZN4HPHP16f_posix_getgrgidEi");

TypedValue* fh_posix_getgrnam(TypedValue* _rv, Value* name) asm("_ZN4HPHP16f_posix_getgrnamERKNS_6StringE");

TypedValue* fh_posix_getgroups(TypedValue* _rv) asm("_ZN4HPHP17f_posix_getgroupsEv");

TypedValue* fh_posix_getlogin(TypedValue* _rv) asm("_ZN4HPHP16f_posix_getloginEv");

TypedValue* fh_posix_getpgid(TypedValue* _rv, int pid) asm("_ZN4HPHP15f_posix_getpgidEi");

long fh_posix_getpgrp() asm("_ZN4HPHP15f_posix_getpgrpEv");

long fh_posix_getpid() asm("_ZN4HPHP14f_posix_getpidEv");

long fh_posix_getppid() asm("_ZN4HPHP15f_posix_getppidEv");

TypedValue* fh_posix_getpwnam(TypedValue* _rv, Value* username) asm("_ZN4HPHP16f_posix_getpwnamERKNS_6StringE");

TypedValue* fh_posix_getpwuid(TypedValue* _rv, int uid) asm("_ZN4HPHP16f_posix_getpwuidEi");

TypedValue* fh_posix_getrlimit(TypedValue* _rv) asm("_ZN4HPHP17f_posix_getrlimitEv");

TypedValue* fh_posix_getsid(TypedValue* _rv, int pid) asm("_ZN4HPHP14f_posix_getsidEi");

long fh_posix_getuid() asm("_ZN4HPHP14f_posix_getuidEv");

bool fh_posix_initgroups(Value* name, int base_group_id) asm("_ZN4HPHP18f_posix_initgroupsERKNS_6StringEi");

bool fh_posix_isatty(TypedValue* fd) asm("_ZN4HPHP14f_posix_isattyERKNS_7VariantE");

bool fh_posix_kill(int pid, int sig) asm("_ZN4HPHP12f_posix_killEii");

bool fh_posix_mkfifo(Value* pathname, int mode) asm("_ZN4HPHP14f_posix_mkfifoERKNS_6StringEi");

bool fh_posix_mknod(Value* pathname, int mode, int major, int minor) asm("_ZN4HPHP13f_posix_mknodERKNS_6StringEiii");

bool fh_posix_setegid(int gid) asm("_ZN4HPHP15f_posix_setegidEi");

bool fh_posix_seteuid(int uid) asm("_ZN4HPHP15f_posix_seteuidEi");

bool fh_posix_setgid(int gid) asm("_ZN4HPHP14f_posix_setgidEi");

bool fh_posix_setpgid(int pid, int pgid) asm("_ZN4HPHP15f_posix_setpgidEii");

long fh_posix_setsid() asm("_ZN4HPHP14f_posix_setsidEv");

bool fh_posix_setuid(int uid) asm("_ZN4HPHP14f_posix_setuidEi");

Value* fh_posix_strerror(Value* _rv, int errnum) asm("_ZN4HPHP16f_posix_strerrorEi");

TypedValue* fh_posix_times(TypedValue* _rv) asm("_ZN4HPHP13f_posix_timesEv");

TypedValue* fh_posix_ttyname(TypedValue* _rv, TypedValue* fd) asm("_ZN4HPHP15f_posix_ttynameERKNS_7VariantE");

TypedValue* fh_posix_uname(TypedValue* _rv) asm("_ZN4HPHP13f_posix_unameEv");

} // namespace HPHP
