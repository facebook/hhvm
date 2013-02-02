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
bool HPHP::f_posix_isatty(HPHP::Variant const&)
_ZN4HPHP14f_posix_isattyERKNS_7VariantE

(return value) => rax
fd => rdi
*/

bool fh_posix_isatty(TypedValue* fd) asm("_ZN4HPHP14f_posix_isattyERKNS_7VariantE");

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

