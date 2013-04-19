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

Value* fh_hphp_splfileinfo___construct(Value* _rv, Value* obj, Value* file_name) asm("_ZN4HPHP30f_hphp_splfileinfo___constructERKNS_6ObjectERKNS_6StringE");

long fh_hphp_splfileinfo_getatime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getatimeERKNS_6ObjectE");

Value* fh_hphp_splfileinfo_getbasename(Value* _rv, Value* obj, Value* suffix) asm("_ZN4HPHP30f_hphp_splfileinfo_getbasenameERKNS_6ObjectERKNS_6StringE");

long fh_hphp_splfileinfo_getctime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getctimeERKNS_6ObjectE");

Value* fh_hphp_splfileinfo_getfileinfo(Value* _rv, Value* obj, Value* class_name) asm("_ZN4HPHP30f_hphp_splfileinfo_getfileinfoERKNS_6ObjectERKNS_6StringE");

Value* fh_hphp_splfileinfo_getfilename(Value* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getfilenameERKNS_6ObjectE");

long fh_hphp_splfileinfo_getgroup(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getgroupERKNS_6ObjectE");

long fh_hphp_splfileinfo_getinode(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getinodeERKNS_6ObjectE");

Value* fh_hphp_splfileinfo_getlinktarget(Value* _rv, Value* obj) asm("_ZN4HPHP32f_hphp_splfileinfo_getlinktargetERKNS_6ObjectE");

long fh_hphp_splfileinfo_getmtime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getmtimeERKNS_6ObjectE");

long fh_hphp_splfileinfo_getowner(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getownerERKNS_6ObjectE");

Value* fh_hphp_splfileinfo_getpath(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_getpathERKNS_6ObjectE");

Value* fh_hphp_splfileinfo_getpathinfo(Value* _rv, Value* obj, Value* class_name) asm("_ZN4HPHP30f_hphp_splfileinfo_getpathinfoERKNS_6ObjectERKNS_6StringE");

Value* fh_hphp_splfileinfo_getpathname(Value* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getpathnameERKNS_6ObjectE");

long fh_hphp_splfileinfo_getperms(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getpermsERKNS_6ObjectE");

TypedValue* fh_hphp_splfileinfo_getrealpath(TypedValue* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getrealpathERKNS_6ObjectE");

long fh_hphp_splfileinfo_getsize(Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_getsizeERKNS_6ObjectE");

Value* fh_hphp_splfileinfo_gettype(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_gettypeERKNS_6ObjectE");

bool fh_hphp_splfileinfo_isdir(Value* obj) asm("_ZN4HPHP24f_hphp_splfileinfo_isdirERKNS_6ObjectE");

bool fh_hphp_splfileinfo_isexecutable(Value* obj) asm("_ZN4HPHP31f_hphp_splfileinfo_isexecutableERKNS_6ObjectE");

bool fh_hphp_splfileinfo_isfile(Value* obj) asm("_ZN4HPHP25f_hphp_splfileinfo_isfileERKNS_6ObjectE");

bool fh_hphp_splfileinfo_islink(Value* obj) asm("_ZN4HPHP25f_hphp_splfileinfo_islinkERKNS_6ObjectE");

bool fh_hphp_splfileinfo_isreadable(Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo_isreadableERKNS_6ObjectE");

bool fh_hphp_splfileinfo_iswritable(Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo_iswritableERKNS_6ObjectE");

Value* fh_hphp_splfileinfo_openfile(Value* _rv, Value* obj, Value* open_mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP27f_hphp_splfileinfo_openfileERKNS_6ObjectERKNS_6StringEbRKNS_7VariantE");

void fh_hphp_splfileinfo_setfileclass(Value* obj, Value* class_name) asm("_ZN4HPHP31f_hphp_splfileinfo_setfileclassERKNS_6ObjectERKNS_6StringE");

void fh_hphp_splfileinfo_setinfoclass(Value* obj, Value* class_name) asm("_ZN4HPHP31f_hphp_splfileinfo_setinfoclassERKNS_6ObjectERKNS_6StringE");

Value* fh_hphp_splfileinfo___tostring(Value* _rv, Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo___tostringERKNS_6ObjectE");

Value* fh_hphp_splfileobject___construct(Value* _rv, Value* obj, Value* filename, Value* open_mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP32f_hphp_splfileobject___constructERKNS_6ObjectERKNS_6StringES5_bRKNS_7VariantE");

TypedValue* fh_hphp_splfileobject_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP28f_hphp_splfileobject_currentERKNS_6ObjectE");

bool fh_hphp_splfileobject_eof(Value* obj) asm("_ZN4HPHP24f_hphp_splfileobject_eofERKNS_6ObjectE");

bool fh_hphp_splfileobject_fflush(Value* obj) asm("_ZN4HPHP27f_hphp_splfileobject_fflushERKNS_6ObjectE");

Value* fh_hphp_splfileobject_fgetc(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fgetcERKNS_6ObjectE");

TypedValue* fh_hphp_splfileobject_fgetcsv(TypedValue* _rv, Value* obj, Value* delimiter, Value* enclosure, Value* escape) asm("_ZN4HPHP28f_hphp_splfileobject_fgetcsvERKNS_6ObjectERKNS_6StringES5_S5_");

Value* fh_hphp_splfileobject_fgets(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fgetsERKNS_6ObjectE");

Value* fh_hphp_splfileobject_fgetss(Value* _rv, Value* obj, Value* allowable_tags) asm("_ZN4HPHP27f_hphp_splfileobject_fgetssERKNS_6ObjectERKNS_6StringE");

bool fh_hphp_splfileobject_flock(Value* obj, TypedValue* wouldblock) asm("_ZN4HPHP26f_hphp_splfileobject_flockERKNS_6ObjectERKNS_14VRefParamValueE");

long fh_hphp_splfileobject_fpassthru(Value* obj) asm("_ZN4HPHP30f_hphp_splfileobject_fpassthruERKNS_6ObjectE");

TypedValue* fh_hphp_splfileobject_fscanf(TypedValue* _rv, long _argc, Value* obj, Value* format, TypedValue* _argv) asm("_ZN4HPHP27f_hphp_splfileobject_fscanfElRKNS_6ObjectERKNS_6StringERKNS_7VariantE");

long fh_hphp_splfileobject_fseek(Value* obj, long offset, long whence) asm("_ZN4HPHP26f_hphp_splfileobject_fseekERKNS_6ObjectEll");

TypedValue* fh_hphp_splfileobject_fstat(TypedValue* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fstatERKNS_6ObjectE");

long fh_hphp_splfileobject_ftell(Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_ftellERKNS_6ObjectE");

bool fh_hphp_splfileobject_ftruncate(Value* obj, long size) asm("_ZN4HPHP30f_hphp_splfileobject_ftruncateERKNS_6ObjectEl");

long fh_hphp_splfileobject_fwrite(Value* obj, Value* str, long length) asm("_ZN4HPHP27f_hphp_splfileobject_fwriteERKNS_6ObjectERKNS_6StringEl");

TypedValue* fh_hphp_splfileobject_getcvscontrol(TypedValue* _rv, Value* obj) asm("_ZN4HPHP34f_hphp_splfileobject_getcvscontrolERKNS_6ObjectE");

long fh_hphp_splfileobject_getflags(Value* obj) asm("_ZN4HPHP29f_hphp_splfileobject_getflagsERKNS_6ObjectE");

long fh_hphp_splfileobject_getmaxlinelen(Value* obj) asm("_ZN4HPHP34f_hphp_splfileobject_getmaxlinelenERKNS_6ObjectE");

long fh_hphp_splfileobject_key(Value* obj) asm("_ZN4HPHP24f_hphp_splfileobject_keyERKNS_6ObjectE");

void fh_hphp_splfileobject_next(Value* obj) asm("_ZN4HPHP25f_hphp_splfileobject_nextERKNS_6ObjectE");

void fh_hphp_splfileobject_rewind(Value* obj) asm("_ZN4HPHP27f_hphp_splfileobject_rewindERKNS_6ObjectE");

bool fh_hphp_splfileobject_valid(Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_validERKNS_6ObjectE");

void fh_hphp_splfileobject_seek(Value* obj, long line_pos) asm("_ZN4HPHP25f_hphp_splfileobject_seekERKNS_6ObjectEl");

void fh_hphp_splfileobject_setcsvcontrol(Value* obj, Value* delimiter, Value* enclosure, Value* escape) asm("_ZN4HPHP34f_hphp_splfileobject_setcsvcontrolERKNS_6ObjectERKNS_6StringES5_S5_");

void fh_hphp_splfileobject_setflags(Value* obj, long flags) asm("_ZN4HPHP29f_hphp_splfileobject_setflagsERKNS_6ObjectEl");

void fh_hphp_splfileobject_setmaxlinelen(Value* obj, long max_len) asm("_ZN4HPHP34f_hphp_splfileobject_setmaxlinelenERKNS_6ObjectEl");

} // namespace HPHP
