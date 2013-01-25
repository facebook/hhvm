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
#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <exception>

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

TypedValue * fg1_hphp_splfileinfo___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileinfo___construct((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_hphp_splfileinfo___construct((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo___construct(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo___construct", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileinfo_getatime(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getatimeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileinfo_getatime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getatimeERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getatime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getatime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getatime((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getatime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileinfo_getatime((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getatime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getatime", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileinfo_getbasename(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP30f_hphp_splfileinfo_getbasenameERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
suffix => rdx
*/

Value* fh_hphp_splfileinfo_getbasename(Value* _rv, Value* obj, Value* suffix) asm("_ZN4HPHP30f_hphp_splfileinfo_getbasenameERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_hphp_splfileinfo_getbasename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getbasename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileinfo_getbasename((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getbasename(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileinfo_getbasename((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getbasename(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getbasename", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileinfo_getctime(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getctimeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileinfo_getctime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getctimeERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getctime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getctime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getctime((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getctime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileinfo_getctime((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getctime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getctime", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_hphp_splfileinfo_getfileinfo(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP30f_hphp_splfileinfo_getfileinfoERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
class_name => rdx
*/

Value* fh_hphp_splfileinfo_getfileinfo(Value* _rv, Value* obj, Value* class_name) asm("_ZN4HPHP30f_hphp_splfileinfo_getfileinfoERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_hphp_splfileinfo_getfileinfo(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getfileinfo(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileinfo_getfileinfo((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getfileinfo(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_hphp_splfileinfo_getfileinfo((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getfileinfo(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getfileinfo", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileinfo_getfilename(HPHP::Object const&)
_ZN4HPHP30f_hphp_splfileinfo_getfilenameERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_getfilename(Value* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getfilenameERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getfilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getfilename(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileinfo_getfilename((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getfilename(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileinfo_getfilename((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getfilename(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getfilename", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileinfo_getgroup(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getgroupERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileinfo_getgroup(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getgroupERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getgroup(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getgroup(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getgroup((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getgroup(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileinfo_getgroup((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getgroup(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getgroup", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileinfo_getinode(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getinodeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileinfo_getinode(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getinodeERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getinode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getinode(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getinode((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getinode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileinfo_getinode((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getinode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getinode", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileinfo_getlinktarget(HPHP::Object const&)
_ZN4HPHP32f_hphp_splfileinfo_getlinktargetERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_getlinktarget(Value* _rv, Value* obj) asm("_ZN4HPHP32f_hphp_splfileinfo_getlinktargetERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getlinktarget(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getlinktarget(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileinfo_getlinktarget((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getlinktarget(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileinfo_getlinktarget((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getlinktarget(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getlinktarget", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileinfo_getmtime(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getmtimeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileinfo_getmtime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getmtimeERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getmtime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getmtime(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getmtime((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getmtime(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileinfo_getmtime((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getmtime(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getmtime", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileinfo_getowner(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getownerERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileinfo_getowner(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getownerERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getowner(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getowner(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getowner((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getowner(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileinfo_getowner((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getowner(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getowner", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileinfo_getpath(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileinfo_getpathERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_getpath(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_getpathERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getpath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getpath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileinfo_getpath((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getpath(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileinfo_getpath((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getpath(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getpath", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_hphp_splfileinfo_getpathinfo(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP30f_hphp_splfileinfo_getpathinfoERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
class_name => rdx
*/

Value* fh_hphp_splfileinfo_getpathinfo(Value* _rv, Value* obj, Value* class_name) asm("_ZN4HPHP30f_hphp_splfileinfo_getpathinfoERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_hphp_splfileinfo_getpathinfo(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getpathinfo(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileinfo_getpathinfo((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getpathinfo(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_hphp_splfileinfo_getpathinfo((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getpathinfo(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getpathinfo", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileinfo_getpathname(HPHP::Object const&)
_ZN4HPHP30f_hphp_splfileinfo_getpathnameERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_getpathname(Value* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getpathnameERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getpathname(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getpathname(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileinfo_getpathname((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getpathname(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileinfo_getpathname((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getpathname(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getpathname", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileinfo_getperms(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileinfo_getpermsERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileinfo_getperms(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getpermsERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getperms(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getperms(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getperms((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getperms(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileinfo_getperms((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getperms(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getperms", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_splfileinfo_getrealpath(HPHP::Object const&)
_ZN4HPHP30f_hphp_splfileinfo_getrealpathERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_splfileinfo_getrealpath(TypedValue* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getrealpathERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getrealpath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getrealpath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileinfo_getrealpath((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getrealpath(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_splfileinfo_getrealpath((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getrealpath(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getrealpath", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileinfo_getsize(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileinfo_getsizeERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileinfo_getsize(Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_getsizeERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_getsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_getsize(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getsize((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_getsize(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileinfo_getsize((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_getsize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_getsize", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileinfo_gettype(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileinfo_gettypeERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo_gettype(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_gettypeERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_gettype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_gettype(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileinfo_gettype((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_gettype(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileinfo_gettype((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_gettype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_gettype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileinfo_isdir(HPHP::Object const&)
_ZN4HPHP24f_hphp_splfileinfo_isdirERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_isdir(Value* obj) asm("_ZN4HPHP24f_hphp_splfileinfo_isdirERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_isdir(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_isdir(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileinfo_isdir((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_isdir(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileinfo_isdir((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_isdir(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_isdir", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileinfo_isexecutable(HPHP::Object const&)
_ZN4HPHP31f_hphp_splfileinfo_isexecutableERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_isexecutable(Value* obj) asm("_ZN4HPHP31f_hphp_splfileinfo_isexecutableERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_isexecutable(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_isexecutable(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileinfo_isexecutable((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_isexecutable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileinfo_isexecutable((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_isexecutable(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_isexecutable", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileinfo_isfile(HPHP::Object const&)
_ZN4HPHP25f_hphp_splfileinfo_isfileERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_isfile(Value* obj) asm("_ZN4HPHP25f_hphp_splfileinfo_isfileERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_isfile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_isfile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileinfo_isfile((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_isfile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileinfo_isfile((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_isfile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_isfile", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileinfo_islink(HPHP::Object const&)
_ZN4HPHP25f_hphp_splfileinfo_islinkERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_islink(Value* obj) asm("_ZN4HPHP25f_hphp_splfileinfo_islinkERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_islink(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_islink(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileinfo_islink((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_islink(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileinfo_islink((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_islink(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_islink", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileinfo_isreadable(HPHP::Object const&)
_ZN4HPHP29f_hphp_splfileinfo_isreadableERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_isreadable(Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo_isreadableERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_isreadable(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_isreadable(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileinfo_isreadable((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_isreadable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileinfo_isreadable((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_isreadable(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_isreadable", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileinfo_iswritable(HPHP::Object const&)
_ZN4HPHP29f_hphp_splfileinfo_iswritableERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileinfo_iswritable(Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo_iswritableERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo_iswritable(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_iswritable(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileinfo_iswritable((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_iswritable(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileinfo_iswritable((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_iswritable(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_iswritable", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



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

TypedValue * fg1_hphp_splfileinfo_openfile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_openfile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileinfo_openfile((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (bool)(args[-2].m_data.num), (args-3));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo_openfile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-2)->m_type == KindOfBoolean && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_hphp_splfileinfo_openfile((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (bool)(args[-2].m_data.num), (args-3));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_openfile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_openfile", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_splfileinfo_setfileclass(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP31f_hphp_splfileinfo_setfileclassERKNS_6ObjectERKNS_6StringE

obj => rdi
class_name => rsi
*/

void fh_hphp_splfileinfo_setfileclass(Value* obj, Value* class_name) asm("_ZN4HPHP31f_hphp_splfileinfo_setfileclassERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_hphp_splfileinfo_setfileclass(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_setfileclass(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileinfo_setfileclass((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_setfileclass(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_splfileinfo_setfileclass((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_setfileclass(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_setfileclass", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_splfileinfo_setinfoclass(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP31f_hphp_splfileinfo_setinfoclassERKNS_6ObjectERKNS_6StringE

obj => rdi
class_name => rsi
*/

void fh_hphp_splfileinfo_setinfoclass(Value* obj, Value* class_name) asm("_ZN4HPHP31f_hphp_splfileinfo_setinfoclassERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_hphp_splfileinfo_setinfoclass(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo_setinfoclass(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileinfo_setinfoclass((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_hphp_splfileinfo_setinfoclass(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_splfileinfo_setinfoclass((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo_setinfoclass(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo_setinfoclass", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileinfo___tostring(HPHP::Object const&)
_ZN4HPHP29f_hphp_splfileinfo___tostringERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileinfo___tostring(Value* _rv, Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo___tostringERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileinfo___tostring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileinfo___tostring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileinfo___tostring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileinfo___tostring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileinfo___tostring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileinfo___tostring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileinfo___tostring", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



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

TypedValue * fg1_hphp_splfileobject___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if ((args-3)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileobject___construct((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (bool)(args[-3].m_data.num), (args-4));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-3)->m_type == KindOfBoolean && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_hphp_splfileobject___construct((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (bool)(args[-3].m_data.num), (args-4));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject___construct(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject___construct", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_splfileobject_current(HPHP::Object const&)
_ZN4HPHP28f_hphp_splfileobject_currentERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_splfileobject_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP28f_hphp_splfileobject_currentERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_current(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_current(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_current((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_splfileobject_current((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_current(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_current", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileobject_eof(HPHP::Object const&)
_ZN4HPHP24f_hphp_splfileobject_eofERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileobject_eof(Value* obj) asm("_ZN4HPHP24f_hphp_splfileobject_eofERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_eof(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_eof(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileobject_eof((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileobject_eof(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileobject_eof((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_eof(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_eof", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileobject_fflush(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileobject_fflushERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileobject_fflush(Value* obj) asm("_ZN4HPHP27f_hphp_splfileobject_fflushERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_fflush(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fflush(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileobject_fflush((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileobject_fflush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileobject_fflush((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fflush(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fflush", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileobject_fgetc(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_fgetcERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileobject_fgetc(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fgetcERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_fgetc(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fgetc(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_fgetc((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject_fgetc(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileobject_fgetc((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fgetc(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fgetc", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



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

TypedValue * fg1_hphp_splfileobject_fgetcsv(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fgetcsv(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileobject_fgetcsv((rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject_fgetcsv(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_hphp_splfileobject_fgetcsv((&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fgetcsv(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fgetcsv", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileobject_fgets(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_fgetsERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_splfileobject_fgets(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fgetsERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_fgets(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fgets(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_fgets((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject_fgets(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileobject_fgets((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fgets(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fgets", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_hphp_splfileobject_fgetss(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP27f_hphp_splfileobject_fgetssERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
obj => rsi
allowable_tags => rdx
*/

Value* fh_hphp_splfileobject_fgetss(Value* _rv, Value* obj, Value* allowable_tags) asm("_ZN4HPHP27f_hphp_splfileobject_fgetssERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_hphp_splfileobject_fgetss(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fgetss(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileobject_fgetss((Value*)(rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject_fgetss(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_splfileobject_fgetss((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fgetss(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fgetss", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileobject_flock(HPHP::Object const&, HPHP::VRefParamValue const&)
_ZN4HPHP26f_hphp_splfileobject_flockERKNS_6ObjectERKNS_14VRefParamValueE

(return value) => rax
obj => rdi
wouldblock => rsi
*/

bool fh_hphp_splfileobject_flock(Value* obj, TypedValue* wouldblock) asm("_ZN4HPHP26f_hphp_splfileobject_flockERKNS_6ObjectERKNS_14VRefParamValueE");

TypedValue * fg1_hphp_splfileobject_flock(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_flock(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileobject_flock((Value*)(args-0), (args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileobject_flock(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileobject_flock((Value*)(args-0), (args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_flock(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_flock", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileobject_fpassthru(HPHP::Object const&)
_ZN4HPHP30f_hphp_splfileobject_fpassthruERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileobject_fpassthru(Value* obj) asm("_ZN4HPHP30f_hphp_splfileobject_fpassthruERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_fpassthru(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fpassthru(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_fpassthru((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileobject_fpassthru(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileobject_fpassthru((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fpassthru(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fpassthru", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_splfileobject_fscanf(long, HPHP::Object const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP27f_hphp_splfileobject_fscanfElRKNS_6ObjectERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
_argc => rsi
obj => rdx
format => rcx
_argv => r8
*/

TypedValue* fh_hphp_splfileobject_fscanf(TypedValue* _rv, long _argc, Value* obj, Value* format, TypedValue* _argv) asm("_ZN4HPHP27f_hphp_splfileobject_fscanfElRKNS_6ObjectERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_hphp_splfileobject_fscanf(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fscanf(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_hphp_splfileobject_fscanf((rv), (long)(args[-0].m_data.num), (Value*)(args-1), (Value*)(args-2), (args-3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject_fscanf(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfInt64) {
        fh_hphp_splfileobject_fscanf((&(rv)), (long)(args[-0].m_data.num), (Value*)(args-1), (Value*)(args-2), (args-3));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fscanf(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fscanf", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileobject_fseek(HPHP::Object const&, long, long)
_ZN4HPHP26f_hphp_splfileobject_fseekERKNS_6ObjectEll

(return value) => rax
obj => rdi
offset => rsi
whence => rdx
*/

long fh_hphp_splfileobject_fseek(Value* obj, long offset, long whence) asm("_ZN4HPHP26f_hphp_splfileobject_fseekERKNS_6ObjectEll");

TypedValue * fg1_hphp_splfileobject_fseek(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fseek(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_fseek((Value*)(args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
  return rv;
}

TypedValue* fg_hphp_splfileobject_fseek(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileobject_fseek((Value*)(args-0), (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fseek(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fseek", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_splfileobject_fstat(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_fstatERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_splfileobject_fstat(TypedValue* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fstatERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_fstat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fstat(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_fstat((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject_fstat(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_splfileobject_fstat((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fstat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fstat", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileobject_ftell(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_ftellERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileobject_ftell(Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_ftellERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_ftell(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_ftell(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_ftell((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileobject_ftell(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileobject_ftell((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_ftell(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_ftell", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileobject_ftruncate(HPHP::Object const&, long)
_ZN4HPHP30f_hphp_splfileobject_ftruncateERKNS_6ObjectEl

(return value) => rax
obj => rdi
size => rsi
*/

bool fh_hphp_splfileobject_ftruncate(Value* obj, long size) asm("_ZN4HPHP30f_hphp_splfileobject_ftruncateERKNS_6ObjectEl");

TypedValue * fg1_hphp_splfileobject_ftruncate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_ftruncate(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_hphp_splfileobject_ftruncate((Value*)(args-0), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileobject_ftruncate(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileobject_ftruncate((Value*)(args-0), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_ftruncate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_ftruncate", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileobject_fwrite(HPHP::Object const&, HPHP::String const&, long)
_ZN4HPHP27f_hphp_splfileobject_fwriteERKNS_6ObjectERKNS_6StringEl

(return value) => rax
obj => rdi
str => rsi
length => rdx
*/

long fh_hphp_splfileobject_fwrite(Value* obj, Value* str, long length) asm("_ZN4HPHP27f_hphp_splfileobject_fwriteERKNS_6ObjectERKNS_6StringEl");

TypedValue * fg1_hphp_splfileobject_fwrite(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_fwrite(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_fwrite((Value*)(args-0), (Value*)(args-1), (long)(args[-2].m_data.num));
  return rv;
}

TypedValue* fg_hphp_splfileobject_fwrite(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileobject_fwrite((Value*)(args-0), (Value*)(args-1), (long)(args[-2].m_data.num));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_fwrite(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_fwrite", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphp_splfileobject_getcvscontrol(HPHP::Object const&)
_ZN4HPHP34f_hphp_splfileobject_getcvscontrolERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_splfileobject_getcvscontrol(TypedValue* _rv, Value* obj) asm("_ZN4HPHP34f_hphp_splfileobject_getcvscontrolERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_getcvscontrol(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_getcvscontrol(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_getcvscontrol((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_splfileobject_getcvscontrol(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_splfileobject_getcvscontrol((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_getcvscontrol(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_getcvscontrol", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileobject_getflags(HPHP::Object const&)
_ZN4HPHP29f_hphp_splfileobject_getflagsERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileobject_getflags(Value* obj) asm("_ZN4HPHP29f_hphp_splfileobject_getflagsERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_getflags(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_getflags(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_getflags((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileobject_getflags(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileobject_getflags((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_getflags(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_getflags", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileobject_getmaxlinelen(HPHP::Object const&)
_ZN4HPHP34f_hphp_splfileobject_getmaxlinelenERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileobject_getmaxlinelen(Value* obj) asm("_ZN4HPHP34f_hphp_splfileobject_getmaxlinelenERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_getmaxlinelen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_getmaxlinelen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_getmaxlinelen((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileobject_getmaxlinelen(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileobject_getmaxlinelen((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_getmaxlinelen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_getmaxlinelen", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_hphp_splfileobject_key(HPHP::Object const&)
_ZN4HPHP24f_hphp_splfileobject_keyERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

long fh_hphp_splfileobject_key(Value* obj) asm("_ZN4HPHP24f_hphp_splfileobject_keyERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_key(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_key(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_key((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileobject_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_hphp_splfileobject_key((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_key(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_key", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_splfileobject_next(HPHP::Object const&)
_ZN4HPHP25f_hphp_splfileobject_nextERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_splfileobject_next(Value* obj) asm("_ZN4HPHP25f_hphp_splfileobject_nextERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_next(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_next(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_next((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileobject_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_splfileobject_next((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_next(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_next", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_splfileobject_rewind(HPHP::Object const&)
_ZN4HPHP27f_hphp_splfileobject_rewindERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_splfileobject_rewind(Value* obj) asm("_ZN4HPHP27f_hphp_splfileobject_rewindERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_rewind(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_rewind(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_rewind((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_splfileobject_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_splfileobject_rewind((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_rewind(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_rewind", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_hphp_splfileobject_valid(HPHP::Object const&)
_ZN4HPHP26f_hphp_splfileobject_validERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_splfileobject_valid(Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_validERKNS_6ObjectE");

TypedValue * fg1_hphp_splfileobject_valid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_valid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_splfileobject_valid((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_splfileobject_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_splfileobject_valid((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_valid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_valid", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_splfileobject_seek(HPHP::Object const&, long)
_ZN4HPHP25f_hphp_splfileobject_seekERKNS_6ObjectEl

obj => rdi
line_pos => rsi
*/

void fh_hphp_splfileobject_seek(Value* obj, long line_pos) asm("_ZN4HPHP25f_hphp_splfileobject_seekERKNS_6ObjectEl");

TypedValue * fg1_hphp_splfileobject_seek(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_seek(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileobject_seek((Value*)(args-0), (long)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_hphp_splfileobject_seek(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_splfileobject_seek((Value*)(args-0), (long)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_seek(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_seek", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_splfileobject_setcsvcontrol(HPHP::Object const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP34f_hphp_splfileobject_setcsvcontrolERKNS_6ObjectERKNS_6StringES5_S5_

obj => rdi
delimiter => rsi
enclosure => rdx
escape => rcx
*/

void fh_hphp_splfileobject_setcsvcontrol(Value* obj, Value* delimiter, Value* enclosure, Value* escape) asm("_ZN4HPHP34f_hphp_splfileobject_setcsvcontrolERKNS_6ObjectERKNS_6StringES5_S5_");

TypedValue * fg1_hphp_splfileobject_setcsvcontrol(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_setcsvcontrol(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileobject_setcsvcontrol((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3));
  return rv;
}

TypedValue* fg_hphp_splfileobject_setcsvcontrol(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_splfileobject_setcsvcontrol((Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (Value*)(args-3));
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_setcsvcontrol(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_setcsvcontrol", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_splfileobject_setflags(HPHP::Object const&, long)
_ZN4HPHP29f_hphp_splfileobject_setflagsERKNS_6ObjectEl

obj => rdi
flags => rsi
*/

void fh_hphp_splfileobject_setflags(Value* obj, long flags) asm("_ZN4HPHP29f_hphp_splfileobject_setflagsERKNS_6ObjectEl");

TypedValue * fg1_hphp_splfileobject_setflags(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_setflags(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileobject_setflags((Value*)(args-0), (long)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_hphp_splfileobject_setflags(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_splfileobject_setflags((Value*)(args-0), (long)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_setflags(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_setflags", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_splfileobject_setmaxlinelen(HPHP::Object const&, long)
_ZN4HPHP34f_hphp_splfileobject_setmaxlinelenERKNS_6ObjectEl

obj => rdi
max_len => rsi
*/

void fh_hphp_splfileobject_setmaxlinelen(Value* obj, long max_len) asm("_ZN4HPHP34f_hphp_splfileobject_setmaxlinelenERKNS_6ObjectEl");

TypedValue * fg1_hphp_splfileobject_setmaxlinelen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_splfileobject_setmaxlinelen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_splfileobject_setmaxlinelen((Value*)(args-0), (long)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_hphp_splfileobject_setmaxlinelen(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_splfileobject_setmaxlinelen((Value*)(args-0), (long)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_splfileobject_setmaxlinelen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_splfileobject_setmaxlinelen", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

