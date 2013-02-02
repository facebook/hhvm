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
HPHP::Object HPHP::f_hphp_splfileinfo___construct(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP30f_hphp_splfileinfo___constructERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
file_name => rdx
*/

Value* fh_hphp_splfileinfo___construct(Value* _rv, Value* obj, Value* file_name) asm("_ZN4HPHP30f_hphp_splfileinfo___constructERKNS_6ObjectERKNS_6StringE");

/*
long long HPHP::f_hphp_splfileinfo_getatime(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getatimeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileinfo_getatime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getatimeERKNS_6ObjectE");

/*
HPHP::String HPHP::f_hphp_splfileinfo_getbasename(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP30f_hphp_splfileinfo_getbasenameERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
suffix => rdx
*/

Value* fh_hphp_splfileinfo_getbasename(Value* _rv, Value* obj, Value* suffix) asm("_ZN4HPHP30f_hphp_splfileinfo_getbasenameERKNS_6ObjectERKNS_6StringE");

/*
long long HPHP::f_hphp_splfileinfo_getctime(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getctimeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileinfo_getctime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getctimeERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_hphp_splfileinfo_getfileinfo(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP30f_hphp_splfileinfo_getfileinfoERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
class_name => rdx
*/

Value* fh_hphp_splfileinfo_getfileinfo(Value* _rv, Value* obj, Value* class_name) asm("_ZN4HPHP30f_hphp_splfileinfo_getfileinfoERKNS_6ObjectERKNS_6StringE");

/*
HPHP::String HPHP::f_hphp_splfileinfo_getfilename(HPHP::Object const&)
_ZN4HPHP30f_hphp_splfileinfo_getfilenameERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_getfilename(Value* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getfilenameERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileinfo_getgroup(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getgroupERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileinfo_getgroup(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getgroupERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileinfo_getinode(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getinodeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileinfo_getinode(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getinodeERKNS_6ObjectE");

/*
HPHP::String HPHP::f_hphp_splfileinfo_getlinktarget(HPHP::Object const&)
_ZN4HPHP32f_hphp_splfileinfo_getlinktargetERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_getlinktarget(Value* _rv, Value* obj) asm("_ZN4HPHP32f_hphp_splfileinfo_getlinktargetERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileinfo_getmtime(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getmtimeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileinfo_getmtime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getmtimeERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileinfo_getowner(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getownerERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileinfo_getowner(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getownerERKNS_6ObjectE");

/*
HPHP::String HPHP::f_hphp_splfileinfo_getpath(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileinfo_getpathERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_getpath(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_getpathERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_hphp_splfileinfo_getpathinfo(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP30f_hphp_splfileinfo_getpathinfoERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
class_name => rdx
*/

Value* fh_hphp_splfileinfo_getpathinfo(Value* _rv, Value* obj, Value* class_name) asm("_ZN4HPHP30f_hphp_splfileinfo_getpathinfoERKNS_6ObjectERKNS_6StringE");

/*
HPHP::String HPHP::f_hphp_splfileinfo_getpathname(HPHP::Object const&)
_ZN4HPHP30f_hphp_splfileinfo_getpathnameERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_getpathname(Value* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getpathnameERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileinfo_getperms(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getpermsERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileinfo_getperms(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getpermsERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_hphp_splfileinfo_getrealpath(HPHP::Object const&)
_ZN4HPHP30f_hphp_splfileinfo_getrealpathERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_splfileinfo_getrealpath(TypedValue* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getrealpathERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileinfo_getsize(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileinfo_getsizeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileinfo_getsize(Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_getsizeERKNS_6ObjectE");

/*
HPHP::String HPHP::f_hphp_splfileinfo_gettype(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileinfo_gettypeERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_gettype(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_gettypeERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileinfo_isdir(HPHP::Object const&)
_ZN4HPHP24f_hphp_splfileinfo_isdirERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_isdir(Value* obj) asm("_ZN4HPHP24f_hphp_splfileinfo_isdirERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileinfo_isexecutable(HPHP::Object const&)
_ZN4HPHP31f_hphp_splfileinfo_isexecutableERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_isexecutable(Value* obj) asm("_ZN4HPHP31f_hphp_splfileinfo_isexecutableERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileinfo_isfile(HPHP::Object const&)
_ZN4HPHP25f_hphp_splfileinfo_isfileERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_isfile(Value* obj) asm("_ZN4HPHP25f_hphp_splfileinfo_isfileERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileinfo_islink(HPHP::Object const&)
_ZN4HPHP25f_hphp_splfileinfo_islinkERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_islink(Value* obj) asm("_ZN4HPHP25f_hphp_splfileinfo_islinkERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileinfo_isreadable(HPHP::Object const&)
_ZN4HPHP29f_hphp_splfileinfo_isreadableERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_isreadable(Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo_isreadableERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileinfo_iswritable(HPHP::Object const&)
_ZN4HPHP29f_hphp_splfileinfo_iswritableERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_iswritable(Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo_iswritableERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_hphp_splfileinfo_openfile(HPHP::Object const&, HPHP::String const&, bool, HPHP::Variant const&)
_ZN4HPHP27f_hphp_splfileinfo_openfileERKNS_6ObjectERKNS_6StringEbRKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
open_mode => rdx
use_include_path => rcx
context => r8
*/

Value* fh_hphp_splfileinfo_openfile(Value* _rv, Value* obj, Value* open_mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP27f_hphp_splfileinfo_openfileERKNS_6ObjectERKNS_6StringEbRKNS_7VariantE");

/*
void HPHP::f_hphp_splfileinfo_setfileclass(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP31f_hphp_splfileinfo_setfileclassERKNS_6ObjectERKNS_6StringE

obj => rdi
class_name => rsi
*/

void fh_hphp_splfileinfo_setfileclass(Value* obj, Value* class_name) asm("_ZN4HPHP31f_hphp_splfileinfo_setfileclassERKNS_6ObjectERKNS_6StringE");

/*
void HPHP::f_hphp_splfileinfo_setinfoclass(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP31f_hphp_splfileinfo_setinfoclassERKNS_6ObjectERKNS_6StringE

obj => rdi
class_name => rsi
*/

void fh_hphp_splfileinfo_setinfoclass(Value* obj, Value* class_name) asm("_ZN4HPHP31f_hphp_splfileinfo_setinfoclassERKNS_6ObjectERKNS_6StringE");

/*
HPHP::String HPHP::f_hphp_splfileinfo___tostring(HPHP::Object const&)
_ZN4HPHP29f_hphp_splfileinfo___tostringERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo___tostring(Value* _rv, Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo___tostringERKNS_6ObjectE");

/*
HPHP::Object HPHP::f_hphp_splfileobject___construct(HPHP::Object const&, HPHP::String const&, HPHP::String const&, bool, HPHP::Variant const&)
_ZN4HPHP32f_hphp_splfileobject___constructERKNS_6ObjectERKNS_6StringES5_bRKNS_7VariantE

(return value) => rax
_rv => rdi
obj => rsi
filename => rdx
open_mode => rcx
use_include_path => r8
context => r9
*/

Value* fh_hphp_splfileobject___construct(Value* _rv, Value* obj, Value* filename, Value* open_mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP32f_hphp_splfileobject___constructERKNS_6ObjectERKNS_6StringES5_bRKNS_7VariantE");

/*
HPHP::Variant HPHP::f_hphp_splfileobject_current(HPHP::Object const&)
_ZN4HPHP28f_hphp_splfileobject_currentERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_splfileobject_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP28f_hphp_splfileobject_currentERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileobject_eof(HPHP::Object const&)
_ZN4HPHP24f_hphp_splfileobject_eofERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileobject_eof(Value* obj) asm("_ZN4HPHP24f_hphp_splfileobject_eofERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileobject_fflush(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileobject_fflushERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileobject_fflush(Value* obj) asm("_ZN4HPHP27f_hphp_splfileobject_fflushERKNS_6ObjectE");

/*
HPHP::String HPHP::f_hphp_splfileobject_fgetc(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_fgetcERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileobject_fgetc(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fgetcERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_hphp_splfileobject_fgetcsv(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_hphp_splfileobject_fgetcsvERKNS_6ObjectERKNS_6StringES5_S5_

(return value) => rax
_rv => rdi
obj => rsi
delimiter => rdx
enclosure => rcx
escape => r8
*/

TypedValue* fh_hphp_splfileobject_fgetcsv(TypedValue* _rv, Value* obj, Value* delimiter, Value* enclosure, Value* escape) asm("_ZN4HPHP28f_hphp_splfileobject_fgetcsvERKNS_6ObjectERKNS_6StringES5_S5_");

/*
HPHP::String HPHP::f_hphp_splfileobject_fgets(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_fgetsERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileobject_fgets(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fgetsERKNS_6ObjectE");

/*
HPHP::String HPHP::f_hphp_splfileobject_fgetss(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP27f_hphp_splfileobject_fgetssERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
allowable_tags => rdx
*/

Value* fh_hphp_splfileobject_fgetss(Value* _rv, Value* obj, Value* allowable_tags) asm("_ZN4HPHP27f_hphp_splfileobject_fgetssERKNS_6ObjectERKNS_6StringE");

/*
bool HPHP::f_hphp_splfileobject_flock(HPHP::Object const&, HPHP::VRefParamValue const&)
_ZN4HPHP26f_hphp_splfileobject_flockERKNS_6ObjectERKNS_14VRefParamValueE

(return value) => rax
obj => rdi
wouldblock => rsi
*/

bool fh_hphp_splfileobject_flock(Value* obj, TypedValue* wouldblock) asm("_ZN4HPHP26f_hphp_splfileobject_flockERKNS_6ObjectERKNS_14VRefParamValueE");

/*
long long HPHP::f_hphp_splfileobject_fpassthru(HPHP::Object const&)
_ZN4HPHP30f_hphp_splfileobject_fpassthruERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileobject_fpassthru(Value* obj) asm("_ZN4HPHP30f_hphp_splfileobject_fpassthruERKNS_6ObjectE");

/*
HPHP::Variant HPHP::f_hphp_splfileobject_fscanf(long long, HPHP::Object const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP27f_hphp_splfileobject_fscanfExRKNS_6ObjectERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
_argc => rsi
obj => rdx
format => rcx
_argv => r8
*/

TypedValue* fh_hphp_splfileobject_fscanf(TypedValue* _rv, long long _argc, Value* obj, Value* format, TypedValue* _argv) asm("_ZN4HPHP27f_hphp_splfileobject_fscanfExRKNS_6ObjectERKNS_6StringERKNS_7VariantE");

/*
long long HPHP::f_hphp_splfileobject_fseek(HPHP::Object const&, long long, long long)
_ZN4HPHP26f_hphp_splfileobject_fseekERKNS_6ObjectExx

(return value) => rax
obj => rdi
offset => rsi
whence => rdx
*/

long long fh_hphp_splfileobject_fseek(Value* obj, long long offset, long long whence) asm("_ZN4HPHP26f_hphp_splfileobject_fseekERKNS_6ObjectExx");

/*
HPHP::Variant HPHP::f_hphp_splfileobject_fstat(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_fstatERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_splfileobject_fstat(TypedValue* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fstatERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileobject_ftell(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_ftellERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileobject_ftell(Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_ftellERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileobject_ftruncate(HPHP::Object const&, long long)
_ZN4HPHP30f_hphp_splfileobject_ftruncateERKNS_6ObjectEx

(return value) => rax
obj => rdi
size => rsi
*/

bool fh_hphp_splfileobject_ftruncate(Value* obj, long long size) asm("_ZN4HPHP30f_hphp_splfileobject_ftruncateERKNS_6ObjectEx");

/*
long long HPHP::f_hphp_splfileobject_fwrite(HPHP::Object const&, HPHP::String const&, long long)
_ZN4HPHP27f_hphp_splfileobject_fwriteERKNS_6ObjectERKNS_6StringEx

(return value) => rax
obj => rdi
str => rsi
length => rdx
*/

long long fh_hphp_splfileobject_fwrite(Value* obj, Value* str, long long length) asm("_ZN4HPHP27f_hphp_splfileobject_fwriteERKNS_6ObjectERKNS_6StringEx");

/*
HPHP::Variant HPHP::f_hphp_splfileobject_getcvscontrol(HPHP::Object const&)
_ZN4HPHP34f_hphp_splfileobject_getcvscontrolERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_splfileobject_getcvscontrol(TypedValue* _rv, Value* obj) asm("_ZN4HPHP34f_hphp_splfileobject_getcvscontrolERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileobject_getflags(HPHP::Object const&)
_ZN4HPHP29f_hphp_splfileobject_getflagsERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileobject_getflags(Value* obj) asm("_ZN4HPHP29f_hphp_splfileobject_getflagsERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileobject_getmaxlinelen(HPHP::Object const&)
_ZN4HPHP34f_hphp_splfileobject_getmaxlinelenERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileobject_getmaxlinelen(Value* obj) asm("_ZN4HPHP34f_hphp_splfileobject_getmaxlinelenERKNS_6ObjectE");

/*
long long HPHP::f_hphp_splfileobject_key(HPHP::Object const&)
_ZN4HPHP24f_hphp_splfileobject_keyERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long long fh_hphp_splfileobject_key(Value* obj) asm("_ZN4HPHP24f_hphp_splfileobject_keyERKNS_6ObjectE");

/*
void HPHP::f_hphp_splfileobject_next(HPHP::Object const&)
_ZN4HPHP25f_hphp_splfileobject_nextERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_splfileobject_next(Value* obj) asm("_ZN4HPHP25f_hphp_splfileobject_nextERKNS_6ObjectE");

/*
void HPHP::f_hphp_splfileobject_rewind(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileobject_rewindERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_splfileobject_rewind(Value* obj) asm("_ZN4HPHP27f_hphp_splfileobject_rewindERKNS_6ObjectE");

/*
bool HPHP::f_hphp_splfileobject_valid(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_validERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileobject_valid(Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_validERKNS_6ObjectE");

/*
void HPHP::f_hphp_splfileobject_seek(HPHP::Object const&, long long)
_ZN4HPHP25f_hphp_splfileobject_seekERKNS_6ObjectEx

obj => rdi
line_pos => rsi
*/

void fh_hphp_splfileobject_seek(Value* obj, long long line_pos) asm("_ZN4HPHP25f_hphp_splfileobject_seekERKNS_6ObjectEx");

/*
void HPHP::f_hphp_splfileobject_setcsvcontrol(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP34f_hphp_splfileobject_setcsvcontrolERKNS_6ObjectERKNS_6StringES5_S5_

obj => rdi
delimiter => rsi
enclosure => rdx
escape => rcx
*/

void fh_hphp_splfileobject_setcsvcontrol(Value* obj, Value* delimiter, Value* enclosure, Value* escape) asm("_ZN4HPHP34f_hphp_splfileobject_setcsvcontrolERKNS_6ObjectERKNS_6StringES5_S5_");

/*
void HPHP::f_hphp_splfileobject_setflags(HPHP::Object const&, long long)
_ZN4HPHP29f_hphp_splfileobject_setflagsERKNS_6ObjectEx

obj => rdi
flags => rsi
*/

void fh_hphp_splfileobject_setflags(Value* obj, long long flags) asm("_ZN4HPHP29f_hphp_splfileobject_setflagsERKNS_6ObjectEx");

/*
void HPHP::f_hphp_splfileobject_setmaxlinelen(HPHP::Object const&, long long)
_ZN4HPHP34f_hphp_splfileobject_setmaxlinelenERKNS_6ObjectEx

obj => rdi
max_len => rsi
*/

void fh_hphp_splfileobject_setmaxlinelen(Value* obj, long long max_len) asm("_ZN4HPHP34f_hphp_splfileobject_setmaxlinelenERKNS_6ObjectEx");


} // !HPHP

