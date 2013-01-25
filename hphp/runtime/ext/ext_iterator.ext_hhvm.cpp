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
HPHP::Object HPHP::f_hphp_recursiveiteratoriterator___construct(HPHP::Object const&, HPHP::Object const&, long, long)
_ZN4HPHP44f_hphp_recursiveiteratoriterator___constructERKNS_6ObjectES2_ll

(return value) => rax
_rv => rdi
obj => rsi
iterator => rdx
mode => rcx
flags => r8
*/

Value* fh_hphp_recursiveiteratoriterator___construct(Value* _rv, Value* obj, Value* iterator, long mode, long flags) asm("_ZN4HPHP44f_hphp_recursiveiteratoriterator___constructERKNS_6ObjectES2_ll");

TypedValue * fg1_hphp_recursiveiteratoriterator___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursiveiteratoriterator___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_recursiveiteratoriterator___construct((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (long)(args[-2].m_data.num), (long)(args[-3].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursiveiteratoriterator___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_hphp_recursiveiteratoriterator___construct((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (long)(args[-2].m_data.num), (long)(args[-3].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursiveiteratoriterator___construct(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursiveiteratoriterator___construct", count, 4, 4, 1);
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
HPHP::Object HPHP::f_hphp_recursiveiteratoriterator_getinneriterator(HPHP::Object const&)
_ZN4HPHP49f_hphp_recursiveiteratoriterator_getinneriteratorERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_recursiveiteratoriterator_getinneriterator(Value* _rv, Value* obj) asm("_ZN4HPHP49f_hphp_recursiveiteratoriterator_getinneriteratorERKNS_6ObjectE");

TypedValue * fg1_hphp_recursiveiteratoriterator_getinneriterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursiveiteratoriterator_getinneriterator(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursiveiteratoriterator_getinneriterator((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursiveiteratoriterator_getinneriterator(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_hphp_recursiveiteratoriterator_getinneriterator((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursiveiteratoriterator_getinneriterator(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursiveiteratoriterator_getinneriterator", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_hphp_recursiveiteratoriterator_current(HPHP::Object const&)
_ZN4HPHP40f_hphp_recursiveiteratoriterator_currentERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_recursiveiteratoriterator_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP40f_hphp_recursiveiteratoriterator_currentERKNS_6ObjectE");

TypedValue * fg1_hphp_recursiveiteratoriterator_current(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursiveiteratoriterator_current(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursiveiteratoriterator_current((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursiveiteratoriterator_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_recursiveiteratoriterator_current((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursiveiteratoriterator_current(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursiveiteratoriterator_current", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_hphp_recursiveiteratoriterator_key(HPHP::Object const&)
_ZN4HPHP36f_hphp_recursiveiteratoriterator_keyERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_recursiveiteratoriterator_key(TypedValue* _rv, Value* obj) asm("_ZN4HPHP36f_hphp_recursiveiteratoriterator_keyERKNS_6ObjectE");

TypedValue * fg1_hphp_recursiveiteratoriterator_key(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursiveiteratoriterator_key(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursiveiteratoriterator_key((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursiveiteratoriterator_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_recursiveiteratoriterator_key((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursiveiteratoriterator_key(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursiveiteratoriterator_key", count, 1, 1, 1);
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
void HPHP::f_hphp_recursiveiteratoriterator_next(HPHP::Object const&)
_ZN4HPHP37f_hphp_recursiveiteratoriterator_nextERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_recursiveiteratoriterator_next(Value* obj) asm("_ZN4HPHP37f_hphp_recursiveiteratoriterator_nextERKNS_6ObjectE");

TypedValue * fg1_hphp_recursiveiteratoriterator_next(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursiveiteratoriterator_next(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursiveiteratoriterator_next((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_recursiveiteratoriterator_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_recursiveiteratoriterator_next((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursiveiteratoriterator_next(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursiveiteratoriterator_next", count, 1, 1, 1);
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
void HPHP::f_hphp_recursiveiteratoriterator_rewind(HPHP::Object const&)
_ZN4HPHP39f_hphp_recursiveiteratoriterator_rewindERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_recursiveiteratoriterator_rewind(Value* obj) asm("_ZN4HPHP39f_hphp_recursiveiteratoriterator_rewindERKNS_6ObjectE");

TypedValue * fg1_hphp_recursiveiteratoriterator_rewind(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursiveiteratoriterator_rewind(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursiveiteratoriterator_rewind((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_recursiveiteratoriterator_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_recursiveiteratoriterator_rewind((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursiveiteratoriterator_rewind(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursiveiteratoriterator_rewind", count, 1, 1, 1);
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
bool HPHP::f_hphp_recursiveiteratoriterator_valid(HPHP::Object const&)
_ZN4HPHP38f_hphp_recursiveiteratoriterator_validERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_recursiveiteratoriterator_valid(Value* obj) asm("_ZN4HPHP38f_hphp_recursiveiteratoriterator_validERKNS_6ObjectE");

TypedValue * fg1_hphp_recursiveiteratoriterator_valid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursiveiteratoriterator_valid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_recursiveiteratoriterator_valid((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_recursiveiteratoriterator_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_recursiveiteratoriterator_valid((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursiveiteratoriterator_valid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursiveiteratoriterator_valid", count, 1, 1, 1);
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
bool HPHP::f_hphp_directoryiterator___construct(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP36f_hphp_directoryiterator___constructERKNS_6ObjectERKNS_6StringE

(return value) => rax
obj => rdi
path => rsi
*/

bool fh_hphp_directoryiterator___construct(Value* obj, Value* path) asm("_ZN4HPHP36f_hphp_directoryiterator___constructERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_hphp_directoryiterator___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_hphp_directoryiterator___construct((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_directoryiterator___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_directoryiterator___construct((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator___construct(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator___construct", count, 2, 2, 1);
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
HPHP::Variant HPHP::f_hphp_directoryiterator_key(HPHP::Object const&)
_ZN4HPHP28f_hphp_directoryiterator_keyERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_directoryiterator_key(TypedValue* _rv, Value* obj) asm("_ZN4HPHP28f_hphp_directoryiterator_keyERKNS_6ObjectE");

TypedValue * fg1_hphp_directoryiterator_key(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator_key(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_directoryiterator_key((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_directoryiterator_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_directoryiterator_key((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator_key(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator_key", count, 1, 1, 1);
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
void HPHP::f_hphp_directoryiterator_next(HPHP::Object const&)
_ZN4HPHP29f_hphp_directoryiterator_nextERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_directoryiterator_next(Value* obj) asm("_ZN4HPHP29f_hphp_directoryiterator_nextERKNS_6ObjectE");

TypedValue * fg1_hphp_directoryiterator_next(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator_next(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_directoryiterator_next((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_directoryiterator_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_directoryiterator_next((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator_next(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator_next", count, 1, 1, 1);
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
void HPHP::f_hphp_directoryiterator_rewind(HPHP::Object const&)
_ZN4HPHP31f_hphp_directoryiterator_rewindERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_directoryiterator_rewind(Value* obj) asm("_ZN4HPHP31f_hphp_directoryiterator_rewindERKNS_6ObjectE");

TypedValue * fg1_hphp_directoryiterator_rewind(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator_rewind(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_directoryiterator_rewind((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_directoryiterator_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_directoryiterator_rewind((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator_rewind(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator_rewind", count, 1, 1, 1);
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
void HPHP::f_hphp_directoryiterator_seek(HPHP::Object const&, long)
_ZN4HPHP29f_hphp_directoryiterator_seekERKNS_6ObjectEl

obj => rdi
position => rsi
*/

void fh_hphp_directoryiterator_seek(Value* obj, long position) asm("_ZN4HPHP29f_hphp_directoryiterator_seekERKNS_6ObjectEl");

TypedValue * fg1_hphp_directoryiterator_seek(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator_seek(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_hphp_directoryiterator_seek((Value*)(args-0), (long)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_hphp_directoryiterator_seek(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_directoryiterator_seek((Value*)(args-0), (long)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator_seek(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator_seek", count, 2, 2, 1);
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
HPHP::Variant HPHP::f_hphp_directoryiterator_current(HPHP::Object const&)
_ZN4HPHP32f_hphp_directoryiterator_currentERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_directoryiterator_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP32f_hphp_directoryiterator_currentERKNS_6ObjectE");

TypedValue * fg1_hphp_directoryiterator_current(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator_current(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_directoryiterator_current((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_directoryiterator_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_directoryiterator_current((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator_current(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator_current", count, 1, 1, 1);
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
HPHP::String HPHP::f_hphp_directoryiterator___tostring(HPHP::Object const&)
_ZN4HPHP35f_hphp_directoryiterator___tostringERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_directoryiterator___tostring(Value* _rv, Value* obj) asm("_ZN4HPHP35f_hphp_directoryiterator___tostringERKNS_6ObjectE");

TypedValue * fg1_hphp_directoryiterator___tostring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator___tostring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_directoryiterator___tostring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_directoryiterator___tostring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_directoryiterator___tostring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator___tostring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator___tostring", count, 1, 1, 1);
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
bool HPHP::f_hphp_directoryiterator_valid(HPHP::Object const&)
_ZN4HPHP30f_hphp_directoryiterator_validERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_directoryiterator_valid(Value* obj) asm("_ZN4HPHP30f_hphp_directoryiterator_validERKNS_6ObjectE");

TypedValue * fg1_hphp_directoryiterator_valid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator_valid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_directoryiterator_valid((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_directoryiterator_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_directoryiterator_valid((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator_valid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator_valid", count, 1, 1, 1);
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
bool HPHP::f_hphp_directoryiterator_isdot(HPHP::Object const&)
_ZN4HPHP30f_hphp_directoryiterator_isdotERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_directoryiterator_isdot(Value* obj) asm("_ZN4HPHP30f_hphp_directoryiterator_isdotERKNS_6ObjectE");

TypedValue * fg1_hphp_directoryiterator_isdot(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_directoryiterator_isdot(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_directoryiterator_isdot((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_directoryiterator_isdot(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_directoryiterator_isdot((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_directoryiterator_isdot(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_directoryiterator_isdot", count, 1, 1, 1);
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
bool HPHP::f_hphp_recursivedirectoryiterator___construct(HPHP::Object const&, HPHP::String const&, long)
_ZN4HPHP45f_hphp_recursivedirectoryiterator___constructERKNS_6ObjectERKNS_6StringEl

(return value) => rax
obj => rdi
path => rsi
flags => rdx
*/

bool fh_hphp_recursivedirectoryiterator___construct(Value* obj, Value* path, long flags) asm("_ZN4HPHP45f_hphp_recursivedirectoryiterator___constructERKNS_6ObjectERKNS_6StringEl");

TypedValue * fg1_hphp_recursivedirectoryiterator___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_hphp_recursivedirectoryiterator___construct((Value*)(args-0), (Value*)(args-1), (long)(args[-2].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_recursivedirectoryiterator___construct((Value*)(args-0), (Value*)(args-1), (long)(args[-2].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator___construct(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator___construct", count, 3, 3, 1);
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
HPHP::Variant HPHP::f_hphp_recursivedirectoryiterator_key(HPHP::Object const&)
_ZN4HPHP37f_hphp_recursivedirectoryiterator_keyERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_recursivedirectoryiterator_key(TypedValue* _rv, Value* obj) asm("_ZN4HPHP37f_hphp_recursivedirectoryiterator_keyERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_key(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_key(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursivedirectoryiterator_key((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_recursivedirectoryiterator_key((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_key(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_key", count, 1, 1, 1);
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
void HPHP::f_hphp_recursivedirectoryiterator_next(HPHP::Object const&)
_ZN4HPHP38f_hphp_recursivedirectoryiterator_nextERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_recursivedirectoryiterator_next(Value* obj) asm("_ZN4HPHP38f_hphp_recursivedirectoryiterator_nextERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_next(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_next(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursivedirectoryiterator_next((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_recursivedirectoryiterator_next((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_next(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_next", count, 1, 1, 1);
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
void HPHP::f_hphp_recursivedirectoryiterator_rewind(HPHP::Object const&)
_ZN4HPHP40f_hphp_recursivedirectoryiterator_rewindERKNS_6ObjectE

obj => rdi
*/

void fh_hphp_recursivedirectoryiterator_rewind(Value* obj) asm("_ZN4HPHP40f_hphp_recursivedirectoryiterator_rewindERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_rewind(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_rewind(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursivedirectoryiterator_rewind((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_recursivedirectoryiterator_rewind((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_rewind(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_rewind", count, 1, 1, 1);
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
void HPHP::f_hphp_recursivedirectoryiterator_seek(HPHP::Object const&, long)
_ZN4HPHP38f_hphp_recursivedirectoryiterator_seekERKNS_6ObjectEl

obj => rdi
position => rsi
*/

void fh_hphp_recursivedirectoryiterator_seek(Value* obj, long position) asm("_ZN4HPHP38f_hphp_recursivedirectoryiterator_seekERKNS_6ObjectEl");

TypedValue * fg1_hphp_recursivedirectoryiterator_seek(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_seek(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_hphp_recursivedirectoryiterator_seek((Value*)(args-0), (long)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_seek(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphp_recursivedirectoryiterator_seek((Value*)(args-0), (long)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_seek(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_seek", count, 2, 2, 1);
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
HPHP::Variant HPHP::f_hphp_recursivedirectoryiterator_current(HPHP::Object const&)
_ZN4HPHP41f_hphp_recursivedirectoryiterator_currentERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

TypedValue* fh_hphp_recursivedirectoryiterator_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP41f_hphp_recursivedirectoryiterator_currentERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_current(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_current(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursivedirectoryiterator_current((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_hphp_recursivedirectoryiterator_current((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_current(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_current", count, 1, 1, 1);
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
HPHP::String HPHP::f_hphp_recursivedirectoryiterator___tostring(HPHP::Object const&)
_ZN4HPHP44f_hphp_recursivedirectoryiterator___tostringERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_recursivedirectoryiterator___tostring(Value* _rv, Value* obj) asm("_ZN4HPHP44f_hphp_recursivedirectoryiterator___tostringERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator___tostring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator___tostring(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursivedirectoryiterator___tostring((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator___tostring(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_recursivedirectoryiterator___tostring((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator___tostring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator___tostring", count, 1, 1, 1);
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
bool HPHP::f_hphp_recursivedirectoryiterator_valid(HPHP::Object const&)
_ZN4HPHP39f_hphp_recursivedirectoryiterator_validERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_recursivedirectoryiterator_valid(Value* obj) asm("_ZN4HPHP39f_hphp_recursivedirectoryiterator_validERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_valid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_valid(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_recursivedirectoryiterator_valid((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_recursivedirectoryiterator_valid((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_valid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_valid", count, 1, 1, 1);
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
bool HPHP::f_hphp_recursivedirectoryiterator_haschildren(HPHP::Object const&)
_ZN4HPHP45f_hphp_recursivedirectoryiterator_haschildrenERKNS_6ObjectE

(return value) => rax
obj => rdi
*/

bool fh_hphp_recursivedirectoryiterator_haschildren(Value* obj) asm("_ZN4HPHP45f_hphp_recursivedirectoryiterator_haschildrenERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_haschildren(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_haschildren(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_hphp_recursivedirectoryiterator_haschildren((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_haschildren(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphp_recursivedirectoryiterator_haschildren((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_haschildren(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_haschildren", count, 1, 1, 1);
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
HPHP::Object HPHP::f_hphp_recursivedirectoryiterator_getchildren(HPHP::Object const&)
_ZN4HPHP45f_hphp_recursivedirectoryiterator_getchildrenERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_recursivedirectoryiterator_getchildren(Value* _rv, Value* obj) asm("_ZN4HPHP45f_hphp_recursivedirectoryiterator_getchildrenERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_getchildren(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_getchildren(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursivedirectoryiterator_getchildren((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_getchildren(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfObject;
        fh_hphp_recursivedirectoryiterator_getchildren((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_getchildren(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_getchildren", count, 1, 1, 1);
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
HPHP::String HPHP::f_hphp_recursivedirectoryiterator_getsubpath(HPHP::Object const&)
_ZN4HPHP44f_hphp_recursivedirectoryiterator_getsubpathERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_recursivedirectoryiterator_getsubpath(Value* _rv, Value* obj) asm("_ZN4HPHP44f_hphp_recursivedirectoryiterator_getsubpathERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_getsubpath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_getsubpath(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursivedirectoryiterator_getsubpath((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_getsubpath(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_recursivedirectoryiterator_getsubpath((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_getsubpath(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_getsubpath", count, 1, 1, 1);
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
HPHP::String HPHP::f_hphp_recursivedirectoryiterator_getsubpathname(HPHP::Object const&)
_ZN4HPHP48f_hphp_recursivedirectoryiterator_getsubpathnameERKNS_6ObjectE

(return value) => rax
_rv => rdi
obj => rsi
*/

Value* fh_hphp_recursivedirectoryiterator_getsubpathname(Value* _rv, Value* obj) asm("_ZN4HPHP48f_hphp_recursivedirectoryiterator_getsubpathnameERKNS_6ObjectE");

TypedValue * fg1_hphp_recursivedirectoryiterator_getsubpathname(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_recursivedirectoryiterator_getsubpathname(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_hphp_recursivedirectoryiterator_getsubpathname((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_recursivedirectoryiterator_getsubpathname(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_hphp_recursivedirectoryiterator_getsubpathname((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_recursivedirectoryiterator_getsubpathname(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_recursivedirectoryiterator_getsubpathname", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



HPHP::VM::Instance* new_MutableArrayIterator_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_MutableArrayIterator) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_MutableArrayIterator(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_MutableArrayIterator::t___construct(HPHP::VRefParamValue const&)
_ZN4HPHP22c_MutableArrayIterator13t___constructERKNS_14VRefParamValueE

this_ => rdi
array => rsi
*/

void th_20MutableArrayIterator___construct(ObjectData* this_, TypedValue* array) asm("_ZN4HPHP22c_MutableArrayIterator13t___constructERKNS_14VRefParamValueE");

TypedValue* tg_20MutableArrayIterator___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_20MutableArrayIterator___construct((this_), (args-0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("MutableArrayIterator::__construct", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("MutableArrayIterator::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_MutableArrayIterator::t_currentref()
_ZN4HPHP22c_MutableArrayIterator12t_currentrefEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_20MutableArrayIterator_currentRef(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP22c_MutableArrayIterator12t_currentrefEv");

TypedValue* tg_20MutableArrayIterator_currentRef(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_20MutableArrayIterator_currentRef((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MutableArrayIterator::currentRef", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MutableArrayIterator::currentRef");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_MutableArrayIterator::t_current()
_ZN4HPHP22c_MutableArrayIterator9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_20MutableArrayIterator_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP22c_MutableArrayIterator9t_currentEv");

TypedValue* tg_20MutableArrayIterator_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_20MutableArrayIterator_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MutableArrayIterator::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MutableArrayIterator::current");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_MutableArrayIterator::t_key()
_ZN4HPHP22c_MutableArrayIterator5t_keyEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_20MutableArrayIterator_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP22c_MutableArrayIterator5t_keyEv");

TypedValue* tg_20MutableArrayIterator_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_20MutableArrayIterator_key((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MutableArrayIterator::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MutableArrayIterator::key");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_MutableArrayIterator::t_next()
_ZN4HPHP22c_MutableArrayIterator6t_nextEv

this_ => rdi
*/

void th_20MutableArrayIterator_next(ObjectData* this_) asm("_ZN4HPHP22c_MutableArrayIterator6t_nextEv");

TypedValue* tg_20MutableArrayIterator_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_20MutableArrayIterator_next((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MutableArrayIterator::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MutableArrayIterator::next");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_MutableArrayIterator::t_valid()
_ZN4HPHP22c_MutableArrayIterator7t_validEv

(return value) => rax
this_ => rdi
*/

bool th_20MutableArrayIterator_valid(ObjectData* this_) asm("_ZN4HPHP22c_MutableArrayIterator7t_validEv");

TypedValue* tg_20MutableArrayIterator_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_20MutableArrayIterator_valid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("MutableArrayIterator::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("MutableArrayIterator::valid");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

