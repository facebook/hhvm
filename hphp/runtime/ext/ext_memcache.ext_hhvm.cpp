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
HPHP::Object HPHP::f_memcache_connect(HPHP::String const&, int, int, int)
_ZN4HPHP18f_memcache_connectERKNS_6StringEiii

(return value) => rax
_rv => rdi
host => rsi
port => rdx
timeout => rcx
timeoutms => r8
*/

Value* fh_memcache_connect(Value* _rv, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP18f_memcache_connectERKNS_6StringEiii");

TypedValue * fg1_memcache_connect(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_connect(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_memcache_connect((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_memcache_connect(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        fh_memcache_connect((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_connect(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_connect", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_memcache_pconnect(HPHP::String const&, int, int, int)
_ZN4HPHP19f_memcache_pconnectERKNS_6StringEiii

(return value) => rax
_rv => rdi
host => rsi
port => rdx
timeout => rcx
timeoutms => r8
*/

Value* fh_memcache_pconnect(Value* _rv, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP19f_memcache_pconnectERKNS_6StringEiii");

TypedValue * fg1_memcache_pconnect(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_pconnect(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_memcache_pconnect((Value*)(rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_memcache_pconnect(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        fh_memcache_pconnect((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_pconnect(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_pconnect", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_add(HPHP::Object const&, HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP14f_memcache_addERKNS_6ObjectERKNS_6StringERKNS_7VariantEii

(return value) => rax
memcache => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool fh_memcache_add(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP14f_memcache_addERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

TypedValue * fg1_memcache_add(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_add(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_add((Value*)(args-0), (Value*)(args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_add(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_add((Value*)(args-0), (Value*)(args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_add(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_add", count, 3, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_set(HPHP::Object const&, HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP14f_memcache_setERKNS_6ObjectERKNS_6StringERKNS_7VariantEii

(return value) => rax
memcache => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool fh_memcache_set(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP14f_memcache_setERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

TypedValue * fg1_memcache_set(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_set(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_set((Value*)(args-0), (Value*)(args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_set((Value*)(args-0), (Value*)(args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_set(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_set", count, 3, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_replace(HPHP::Object const&, HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP18f_memcache_replaceERKNS_6ObjectERKNS_6StringERKNS_7VariantEii

(return value) => rax
memcache => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool fh_memcache_replace(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP18f_memcache_replaceERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

TypedValue * fg1_memcache_replace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_replace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_replace((Value*)(args-0), (Value*)(args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_replace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_replace((Value*)(args-0), (Value*)(args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_replace(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_replace", count, 3, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_memcache_get(HPHP::Object const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_memcache_getERKNS_6ObjectERKNS_7VariantERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
memcache => rsi
key => rdx
flags => rcx
*/

TypedValue* fh_memcache_get(TypedValue* _rv, Value* memcache, TypedValue* key, TypedValue* flags) asm("_ZN4HPHP14f_memcache_getERKNS_6ObjectERKNS_7VariantERKNS_14VRefParamValueE");

TypedValue * fg1_memcache_get(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_get(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  VRefParamValue defVal2 = null;
  fh_memcache_get((rv), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_memcache_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-0)->m_type == KindOfObject) {
        VRefParamValue defVal2 = null;
        fh_memcache_get((&(rv)), (Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_get", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_delete(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP17f_memcache_deleteERKNS_6ObjectERKNS_6StringEi

(return value) => rax
memcache => rdi
key => rsi
expire => rdx
*/

bool fh_memcache_delete(Value* memcache, Value* key, int expire) asm("_ZN4HPHP17f_memcache_deleteERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_memcache_delete(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_delete(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_delete((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_delete(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_delete((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_delete(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_delete", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_memcache_increment(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP20f_memcache_incrementERKNS_6ObjectERKNS_6StringEi

(return value) => rax
memcache => rdi
key => rsi
offset => rdx
*/

long fh_memcache_increment(Value* memcache, Value* key, int offset) asm("_ZN4HPHP20f_memcache_incrementERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_memcache_increment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_increment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_memcache_increment((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(1));
  return rv;
}

TypedValue* fg_memcache_increment(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_memcache_increment((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(1));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_increment(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_increment", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_memcache_decrement(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP20f_memcache_decrementERKNS_6ObjectERKNS_6StringEi

(return value) => rax
memcache => rdi
key => rsi
offset => rdx
*/

long fh_memcache_decrement(Value* memcache, Value* key, int offset) asm("_ZN4HPHP20f_memcache_decrementERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_memcache_decrement(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_decrement(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_memcache_decrement((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(1));
  return rv;
}

TypedValue* fg_memcache_decrement(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_memcache_decrement((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(1));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_decrement(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_decrement", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_close(HPHP::Object const&)
_ZN4HPHP16f_memcache_closeERKNS_6ObjectE

(return value) => rax
memcache => rdi
*/

bool fh_memcache_close(Value* memcache) asm("_ZN4HPHP16f_memcache_closeERKNS_6ObjectE");

TypedValue * fg1_memcache_close(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_close(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_memcache_close((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_close(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_close((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_close(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_close", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_debug(bool)
_ZN4HPHP16f_memcache_debugEb

(return value) => rax
onoff => rdi
*/

bool fh_memcache_debug(bool onoff) asm("_ZN4HPHP16f_memcache_debugEb");

TypedValue * fg1_memcache_debug(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_debug(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToBooleanInPlace(args-0);
  rv->m_data.num = (fh_memcache_debug((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_debug(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfBoolean) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_debug((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_debug(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_debug", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_memcache_get_version(HPHP::Object const&)
_ZN4HPHP22f_memcache_get_versionERKNS_6ObjectE

(return value) => rax
_rv => rdi
memcache => rsi
*/

TypedValue* fh_memcache_get_version(TypedValue* _rv, Value* memcache) asm("_ZN4HPHP22f_memcache_get_versionERKNS_6ObjectE");

TypedValue * fg1_memcache_get_version(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_get_version(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_memcache_get_version((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_memcache_get_version(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_memcache_get_version((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_get_version(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_get_version", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_flush(HPHP::Object const&, int)
_ZN4HPHP16f_memcache_flushERKNS_6ObjectEi

(return value) => rax
memcache => rdi
timestamp => rsi
*/

bool fh_memcache_flush(Value* memcache, int timestamp) asm("_ZN4HPHP16f_memcache_flushERKNS_6ObjectEi");

TypedValue * fg1_memcache_flush(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_flush(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_flush((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_flush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_flush((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_flush(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_flush", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_setoptimeout(HPHP::Object const&, int)
_ZN4HPHP23f_memcache_setoptimeoutERKNS_6ObjectEi

(return value) => rax
memcache => rdi
timeoutms => rsi
*/

bool fh_memcache_setoptimeout(Value* memcache, int timeoutms) asm("_ZN4HPHP23f_memcache_setoptimeoutERKNS_6ObjectEi");

TypedValue * fg1_memcache_setoptimeout(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_setoptimeout(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_setoptimeout((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_setoptimeout(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_setoptimeout((Value*)(args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_setoptimeout(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_setoptimeout", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_memcache_get_server_status(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP28f_memcache_get_server_statusERKNS_6ObjectERKNS_6StringEi

(return value) => rax
memcache => rdi
host => rsi
port => rdx
*/

long fh_memcache_get_server_status(Value* memcache, Value* host, int port) asm("_ZN4HPHP28f_memcache_get_server_statusERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_memcache_get_server_status(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_get_server_status(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_memcache_get_server_status((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  return rv;
}

TypedValue* fg_memcache_get_server_status(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_memcache_get_server_status((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_get_server_status(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_get_server_status", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_set_compress_threshold(HPHP::Object const&, int, double)
_ZN4HPHP33f_memcache_set_compress_thresholdERKNS_6ObjectEid

(return value) => rax
memcache => rdi
threshold => rsi
min_savings => xmm0
*/

bool fh_memcache_set_compress_threshold(Value* memcache, int threshold, double min_savings) asm("_ZN4HPHP33f_memcache_set_compress_thresholdERKNS_6ObjectEid");

TypedValue * fg1_memcache_set_compress_threshold(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_set_compress_threshold(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_set_compress_threshold((Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_set_compress_threshold(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfDouble) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_set_compress_threshold((Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_set_compress_threshold(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_set_compress_threshold", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_memcache_get_stats(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP20f_memcache_get_statsERKNS_6ObjectERKNS_6StringEii

(return value) => rax
_rv => rdi
memcache => rsi
type => rdx
slabid => rcx
limit => r8
*/

Value* fh_memcache_get_stats(Value* _rv, Value* memcache, Value* type, int slabid, int limit) asm("_ZN4HPHP20f_memcache_get_statsERKNS_6ObjectERKNS_6StringEii");

TypedValue * fg1_memcache_get_stats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_get_stats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_memcache_get_stats((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(100));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_memcache_get_stats(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_memcache_get_stats((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(100));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_get_stats(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_get_stats", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_memcache_get_extended_stats(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP29f_memcache_get_extended_statsERKNS_6ObjectERKNS_6StringEii

(return value) => rax
_rv => rdi
memcache => rsi
type => rdx
slabid => rcx
limit => r8
*/

Value* fh_memcache_get_extended_stats(Value* _rv, Value* memcache, Value* type, int slabid, int limit) asm("_ZN4HPHP29f_memcache_get_extended_statsERKNS_6ObjectERKNS_6StringEii");

TypedValue * fg1_memcache_get_extended_stats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_get_extended_stats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_memcache_get_extended_stats((Value*)(rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(100));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_memcache_get_extended_stats(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_memcache_get_extended_stats((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(100));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_get_extended_stats(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_get_extended_stats", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_set_server_params(HPHP::Object const&, HPHP::String const&, int, int, int, bool, HPHP::Variant const&)
_ZN4HPHP28f_memcache_set_server_paramsERKNS_6ObjectERKNS_6StringEiiibRKNS_7VariantE

(return value) => rax
memcache => rdi
host => rsi
port => rdx
timeout => rcx
retry_interval => r8
status => r9
failure_callback => st0
*/

bool fh_memcache_set_server_params(Value* memcache, Value* host, int port, int timeout, int retry_interval, bool status, TypedValue* failure_callback) asm("_ZN4HPHP28f_memcache_set_server_paramsERKNS_6ObjectERKNS_6StringEiiibRKNS_7VariantE");

TypedValue * fg1_memcache_set_server_params(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_set_server_params(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 7
  case 6:
    if ((args-5)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_set_server_params((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(11211), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(true), (count > 6) ? (args-6) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_set_server_params(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 7LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfBoolean) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_set_server_params((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(11211), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(true), (count > 6) ? (args-6) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_set_server_params(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_set_server_params", count, 2, 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_memcache_add_server(HPHP::Object const&, HPHP::String const&, int, bool, int, int, int, bool, HPHP::Variant const&, int)
_ZN4HPHP21f_memcache_add_serverERKNS_6ObjectERKNS_6StringEibiiibRKNS_7VariantEi

(return value) => rax
memcache => rdi
host => rsi
port => rdx
persistent => rcx
weight => r8
timeout => r9
retry_interval => st0
status => st8
failure_callback => st16
timeoutms => st24
*/

bool fh_memcache_add_server(Value* memcache, Value* host, int port, bool persistent, int weight, int timeout, int retry_interval, bool status, TypedValue* failure_callback, int timeoutms) asm("_ZN4HPHP21f_memcache_add_serverERKNS_6ObjectERKNS_6StringEibiiibRKNS_7VariantEi");

TypedValue * fg1_memcache_add_server(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_memcache_add_server(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 10
    if ((args-9)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-9);
    }
  case 9:
  case 8:
    if ((args-7)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-7);
    }
  case 7:
    if ((args-6)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_memcache_add_server((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(11211), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (int)(args[-6].m_data.num) : (int)(0), (count > 7) ? (bool)(args[-7].m_data.num) : (bool)(true), (count > 8) ? (args-8) : (TypedValue*)(&null_variant), (count > 9) ? (int)(args[-9].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_memcache_add_server(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 10LL) {
      if ((count <= 9 || (args-9)->m_type == KindOfInt64) && (count <= 7 || (args-7)->m_type == KindOfBoolean) && (count <= 6 || (args-6)->m_type == KindOfInt64) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_memcache_add_server((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(11211), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (int)(args[-6].m_data.num) : (int)(0), (count > 7) ? (bool)(args[-7].m_data.num) : (bool)(true), (count > 8) ? (args-8) : (TypedValue*)(&null_variant), (count > 9) ? (int)(args[-9].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 10);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_memcache_add_server(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 10);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("memcache_add_server", count, 2, 10, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 10);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



HPHP::VM::Instance* new_Memcache_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Memcache) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Memcache(cls);
  return inst;
}

IMPLEMENT_CLASS(Memcache);
/*
void HPHP::c_Memcache::t___construct()
_ZN4HPHP10c_Memcache13t___constructEv

this_ => rdi
*/

void th_8Memcache___construct(ObjectData* this_) asm("_ZN4HPHP10c_Memcache13t___constructEv");

TypedValue* tg_8Memcache___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_8Memcache___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcache::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_connect(HPHP::String const&, int, int, int)
_ZN4HPHP10c_Memcache9t_connectERKNS_6StringEiii

(return value) => rax
this_ => rdi
host => rsi
port => rdx
timeout => rcx
timeoutms => r8
*/

bool th_8Memcache_connect(ObjectData* this_, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP10c_Memcache9t_connectERKNS_6StringEiii");

TypedValue* tg1_8Memcache_connect(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_connect(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_connect((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_connect(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_connect((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_connect(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::connect", count, 1, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::connect");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_pconnect(HPHP::String const&, int, int, int)
_ZN4HPHP10c_Memcache10t_pconnectERKNS_6StringEiii

(return value) => rax
this_ => rdi
host => rsi
port => rdx
timeout => rcx
timeoutms => r8
*/

bool th_8Memcache_pconnect(ObjectData* this_, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP10c_Memcache10t_pconnectERKNS_6StringEiii");

TypedValue* tg1_8Memcache_pconnect(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_pconnect(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_pconnect((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_pconnect(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_pconnect((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_pconnect(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::pconnect", count, 1, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::pconnect");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_add(HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP10c_Memcache5t_addERKNS_6StringERKNS_7VariantEii

(return value) => rax
this_ => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool th_8Memcache_add(ObjectData* this_, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP10c_Memcache5t_addERKNS_6StringERKNS_7VariantEii");

TypedValue* tg1_8Memcache_add(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_add(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_add((this_), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_add(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_add((this_), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_add(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::add", count, 2, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::add");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_set(HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP10c_Memcache5t_setERKNS_6StringERKNS_7VariantEii

(return value) => rax
this_ => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool th_8Memcache_set(ObjectData* this_, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP10c_Memcache5t_setERKNS_6StringERKNS_7VariantEii");

TypedValue* tg1_8Memcache_set(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_set(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_set((this_), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_set(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_set((this_), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_set(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::set", count, 2, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_replace(HPHP::String const&, HPHP::Variant const&, int, int)
_ZN4HPHP10c_Memcache9t_replaceERKNS_6StringERKNS_7VariantEii

(return value) => rax
this_ => rdi
key => rsi
var => rdx
flag => rcx
expire => r8
*/

bool th_8Memcache_replace(ObjectData* this_, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP10c_Memcache9t_replaceERKNS_6StringERKNS_7VariantEii");

TypedValue* tg1_8Memcache_replace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_replace(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_replace((this_), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_replace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_replace((this_), (Value*)(args-0), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_replace(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::replace", count, 2, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::replace");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcache::t_get(HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP10c_Memcache5t_getERKNS_7VariantERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
flags => rcx
*/

TypedValue* th_8Memcache_get(TypedValue* _rv, ObjectData* this_, TypedValue* key, TypedValue* flags) asm("_ZN4HPHP10c_Memcache5t_getERKNS_7VariantERKNS_14VRefParamValueE");

TypedValue* tg_8Memcache_get(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        VRefParamValue defVal1 = null;
        th_8Memcache_get((&(rv)), (this_), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Memcache::get", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_delete(HPHP::String const&, int)
_ZN4HPHP10c_Memcache8t_deleteERKNS_6StringEi

(return value) => rax
this_ => rdi
key => rsi
expire => rdx
*/

bool th_8Memcache_delete(ObjectData* this_, Value* key, int expire) asm("_ZN4HPHP10c_Memcache8t_deleteERKNS_6StringEi");

TypedValue* tg1_8Memcache_delete(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_delete(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_delete((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_delete(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_delete((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_delete(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::delete", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::delete");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Memcache::t_increment(HPHP::String const&, int)
_ZN4HPHP10c_Memcache11t_incrementERKNS_6StringEi

(return value) => rax
this_ => rdi
key => rsi
offset => rdx
*/

long th_8Memcache_increment(ObjectData* this_, Value* key, int offset) asm("_ZN4HPHP10c_Memcache11t_incrementERKNS_6StringEi");

TypedValue* tg1_8Memcache_increment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_increment(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (int64_t)th_8Memcache_increment((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
  return rv;
}

TypedValue* tg_8Memcache_increment(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfInt64;
          rv.m_data.num = (int64_t)th_8Memcache_increment((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_increment(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::increment", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::increment");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Memcache::t_decrement(HPHP::String const&, int)
_ZN4HPHP10c_Memcache11t_decrementERKNS_6StringEi

(return value) => rax
this_ => rdi
key => rsi
offset => rdx
*/

long th_8Memcache_decrement(ObjectData* this_, Value* key, int offset) asm("_ZN4HPHP10c_Memcache11t_decrementERKNS_6StringEi");

TypedValue* tg1_8Memcache_decrement(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_decrement(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (int64_t)th_8Memcache_decrement((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
  return rv;
}

TypedValue* tg_8Memcache_decrement(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfInt64;
          rv.m_data.num = (int64_t)th_8Memcache_decrement((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_decrement(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::decrement", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::decrement");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcache::t_getversion()
_ZN4HPHP10c_Memcache12t_getversionEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_8Memcache_getversion(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP10c_Memcache12t_getversionEv");

TypedValue* tg_8Memcache_getversion(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_8Memcache_getversion((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcache::getversion", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::getversion");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_flush(int)
_ZN4HPHP10c_Memcache7t_flushEi

(return value) => rax
this_ => rdi
expire => rsi
*/

bool th_8Memcache_flush(ObjectData* this_, int expire) asm("_ZN4HPHP10c_Memcache7t_flushEi");

TypedValue* tg1_8Memcache_flush(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_flush(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (th_8Memcache_flush((this_), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_flush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_flush((this_), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_flush(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("Memcache::flush", 1, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::flush");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_setoptimeout(long)
_ZN4HPHP10c_Memcache14t_setoptimeoutEl

(return value) => rax
this_ => rdi
timeoutms => rsi
*/

bool th_8Memcache_setoptimeout(ObjectData* this_, long timeoutms) asm("_ZN4HPHP10c_Memcache14t_setoptimeoutEl");

TypedValue* tg1_8Memcache_setoptimeout(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_setoptimeout(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (th_8Memcache_setoptimeout((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_setoptimeout(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_setoptimeout((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_setoptimeout(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::setoptimeout", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::setoptimeout");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_close()
_ZN4HPHP10c_Memcache7t_closeEv

(return value) => rax
this_ => rdi
*/

bool th_8Memcache_close(ObjectData* this_) asm("_ZN4HPHP10c_Memcache7t_closeEv");

TypedValue* tg_8Memcache_close(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_8Memcache_close((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcache::close", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::close");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Memcache::t_getserverstatus(HPHP::String const&, int)
_ZN4HPHP10c_Memcache17t_getserverstatusERKNS_6StringEi

(return value) => rax
this_ => rdi
host => rsi
port => rdx
*/

long th_8Memcache_getserverstatus(ObjectData* this_, Value* host, int port) asm("_ZN4HPHP10c_Memcache17t_getserverstatusERKNS_6StringEi");

TypedValue* tg1_8Memcache_getserverstatus(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_getserverstatus(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (int64_t)th_8Memcache_getserverstatus((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  return rv;
}

TypedValue* tg_8Memcache_getserverstatus(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfInt64;
          rv.m_data.num = (int64_t)th_8Memcache_getserverstatus((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_getserverstatus(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::getserverstatus", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::getserverstatus");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_setcompressthreshold(int, double)
_ZN4HPHP10c_Memcache22t_setcompressthresholdEid

(return value) => rax
this_ => rdi
threshold => rsi
min_savings => xmm0
*/

bool th_8Memcache_setcompressthreshold(ObjectData* this_, int threshold, double min_savings) asm("_ZN4HPHP10c_Memcache22t_setcompressthresholdEid");

TypedValue* tg1_8Memcache_setcompressthreshold(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_setcompressthreshold(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_setcompressthreshold((this_), (int)(args[-0].m_data.num), (count > 1) ? (args[-1].m_data.dbl) : (double)(0.2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_setcompressthreshold(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfDouble) && (args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_setcompressthreshold((this_), (int)(args[-0].m_data.num), (count > 1) ? (args[-1].m_data.dbl) : (double)(0.2))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_setcompressthreshold(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::setcompressthreshold", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::setcompressthreshold");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Memcache::t_getstats(HPHP::String const&, int, int)
_ZN4HPHP10c_Memcache10t_getstatsERKNS_6StringEii

(return value) => rax
_rv => rdi
this_ => rsi
type => rdx
slabid => rcx
limit => r8
*/

Value* th_8Memcache_getstats(Value* _rv, ObjectData* this_, Value* type, int slabid, int limit) asm("_ZN4HPHP10c_Memcache10t_getstatsERKNS_6StringEii");

TypedValue* tg1_8Memcache_getstats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_getstats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  th_8Memcache_getstats((Value*)(rv), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(100));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8Memcache_getstats(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_type = KindOfArray;
          th_8Memcache_getstats((Value*)(&(rv)), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(100));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_getstats(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("Memcache::getstats", 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::getstats");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Memcache::t_getextendedstats(HPHP::String const&, int, int)
_ZN4HPHP10c_Memcache18t_getextendedstatsERKNS_6StringEii

(return value) => rax
_rv => rdi
this_ => rsi
type => rdx
slabid => rcx
limit => r8
*/

Value* th_8Memcache_getextendedstats(Value* _rv, ObjectData* this_, Value* type, int slabid, int limit) asm("_ZN4HPHP10c_Memcache18t_getextendedstatsERKNS_6StringEii");

TypedValue* tg1_8Memcache_getextendedstats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_getextendedstats(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  th_8Memcache_getextendedstats((Value*)(rv), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(100));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8Memcache_getextendedstats(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_type = KindOfArray;
          th_8Memcache_getextendedstats((Value*)(&(rv)), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(100));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_getextendedstats(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("Memcache::getextendedstats", 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::getextendedstats");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_setserverparams(HPHP::String const&, int, int, int, bool, HPHP::Variant const&)
_ZN4HPHP10c_Memcache17t_setserverparamsERKNS_6StringEiiibRKNS_7VariantE

(return value) => rax
this_ => rdi
host => rsi
port => rdx
timeout => rcx
retry_interval => r8
status => r9
failure_callback => st0
*/

bool th_8Memcache_setserverparams(ObjectData* this_, Value* host, int port, int timeout, int retry_interval, bool status, TypedValue* failure_callback) asm("_ZN4HPHP10c_Memcache17t_setserverparamsERKNS_6StringEiiibRKNS_7VariantE");

TypedValue* tg1_8Memcache_setserverparams(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_setserverparams(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 6
  case 5:
    if ((args-4)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_setserverparams((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(11211), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(true), (count > 5) ? (args-5) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_setserverparams(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 6LL) {
        if ((count <= 4 || (args-4)->m_type == KindOfBoolean) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_setserverparams((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(11211), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(true), (count > 5) ? (args-5) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 6);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_setserverparams(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 6);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::setserverparams", count, 1, 6, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::setserverparams");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcache::t_addserver(HPHP::String const&, int, bool, int, int, int, bool, HPHP::Variant const&, int)
_ZN4HPHP10c_Memcache11t_addserverERKNS_6StringEibiiibRKNS_7VariantEi

(return value) => rax
this_ => rdi
host => rsi
port => rdx
persistent => rcx
weight => r8
timeout => r9
retry_interval => st0
status => st8
failure_callback => st16
timeoutms => st24
*/

bool th_8Memcache_addserver(ObjectData* this_, Value* host, int port, bool persistent, int weight, int timeout, int retry_interval, bool status, TypedValue* failure_callback, int timeoutms) asm("_ZN4HPHP10c_Memcache11t_addserverERKNS_6StringEibiiibRKNS_7VariantEi");

TypedValue* tg1_8Memcache_addserver(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8Memcache_addserver(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 9
    if ((args-8)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-8);
    }
  case 8:
  case 7:
    if ((args-6)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_8Memcache_addserver((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(11211), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(true), (count > 7) ? (args-7) : (TypedValue*)(&null_variant), (count > 8) ? (int)(args[-8].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_8Memcache_addserver(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 9LL) {
        if ((count <= 8 || (args-8)->m_type == KindOfInt64) && (count <= 6 || (args-6)->m_type == KindOfBoolean) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_8Memcache_addserver((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(11211), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(true), (count > 7) ? (args-7) : (TypedValue*)(&null_variant), (count > 8) ? (int)(args[-8].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 9);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8Memcache_addserver(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 9);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcache::addserver", count, 1, 9, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::addserver");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 9);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcache::t___destruct()
_ZN4HPHP10c_Memcache12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_8Memcache___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP10c_Memcache12t___destructEv");

TypedValue* tg_8Memcache___destruct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_8Memcache___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcache::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcache::__destruct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

