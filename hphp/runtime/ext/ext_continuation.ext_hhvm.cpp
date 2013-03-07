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
HPHP::Object HPHP::f_hphp_create_continuation(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP26f_hphp_create_continuationERKNS_6StringES2_S2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
clsname => rsi
funcname => rdx
origFuncName => rcx
args => r8
*/

Value* fh_hphp_create_continuation(Value* _rv, Value* clsname, Value* funcname, Value* origFuncName, Value* args) asm("_ZN4HPHP26f_hphp_create_continuationERKNS_6StringES2_S2_RKNS_5ArrayE");

TypedValue * fg1_hphp_create_continuation(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_create_continuation(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_hphp_create_continuation((Value*)(rv), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&null_array));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphp_create_continuation(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfArray) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        fh_hphp_create_continuation((Value*)(&(rv)), (Value*)(args-0), (Value*)(args-1), (Value*)(args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&null_array));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_create_continuation(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_create_continuation", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_pack_continuation(HPHP::Object const&, long, HPHP::Variant const&)
_ZN4HPHP24f_hphp_pack_continuationERKNS_6ObjectElRKNS_7VariantE

continuation => rdi
label => rsi
value => rdx
*/

void fh_hphp_pack_continuation(Value* continuation, long label, TypedValue* value) asm("_ZN4HPHP24f_hphp_pack_continuationERKNS_6ObjectElRKNS_7VariantE");

TypedValue * fg1_hphp_pack_continuation(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_pack_continuation(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_hphp_pack_continuation((Value*)(args-0), (long)(args[-1].m_data.num), (args-2));
  return rv;
}

TypedValue* fg_hphp_pack_continuation(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_hphp_pack_continuation((Value*)(args-0), (long)(args[-1].m_data.num), (args-2));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_pack_continuation(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_pack_continuation", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphp_unpack_continuation(HPHP::Object const&)
_ZN4HPHP26f_hphp_unpack_continuationERKNS_6ObjectE

continuation => rdi
*/

void fh_hphp_unpack_continuation(Value* continuation) asm("_ZN4HPHP26f_hphp_unpack_continuationERKNS_6ObjectE");

TypedValue * fg1_hphp_unpack_continuation(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphp_unpack_continuation(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_hphp_unpack_continuation((Value*)(args-0));
  return rv;
}

TypedValue* fg_hphp_unpack_continuation(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_hphp_unpack_continuation((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphp_unpack_continuation(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphp_unpack_continuation", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



HPHP::VM::Instance* new_Continuation_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Continuation) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Continuation(cls);
  return inst;
}

IMPLEMENT_CLASS(Continuation);
/*
void HPHP::c_Continuation::t___construct(long, long, bool, HPHP::String const&, HPHP::Variant const&, HPHP::Array const&)
_ZN4HPHP14c_Continuation13t___constructEllbRKNS_6StringERKNS_7VariantERKNS_5ArrayE

this_ => rdi
func => rsi
extra => rdx
isMethod => rcx
origFuncName => r8
obj => r9
args => st0
*/

void th_12Continuation___construct(ObjectData* this_, long func, long extra, bool isMethod, Value* origFuncName, TypedValue* obj, Value* args) asm("_ZN4HPHP14c_Continuation13t___constructEllbRKNS_6StringERKNS_7VariantERKNS_5ArrayE");

TypedValue* tg1_12Continuation___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12Continuation___construct(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-5);
    }
  case 5:
  case 4:
    break;
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  Variant defVal4;
  th_12Continuation___construct((this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (bool)(args[-2].m_data.num), (Value*)(args-3), (count > 4) ? (args-4) : (TypedValue*)(&defVal4), (count > 5) ? (Value*)(args-5) : (Value*)(&null_array));
  return rv;
}

TypedValue* tg_12Continuation___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 4LL && count <= 6LL) {
        if ((count <= 5 || (args-5)->m_type == KindOfArray) && IS_STRING_TYPE((args-3)->m_type) && (args-2)->m_type == KindOfBoolean && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          Variant defVal4;
          th_12Continuation___construct((this_), (long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (bool)(args[-2].m_data.num), (Value*)(args-3), (count > 4) ? (args-4) : (TypedValue*)(&defVal4), (count > 5) ? (Value*)(args-5) : (Value*)(&null_array));
          frame_free_locals_inl(ar, 6);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12Continuation___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 6);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Continuation::__construct", count, 4, 6, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Continuation::t_update(long, HPHP::Variant const&)
_ZN4HPHP14c_Continuation8t_updateElRKNS_7VariantE

this_ => rdi
label => rsi
value => rdx
*/

void th_12Continuation_update(ObjectData* this_, long label, TypedValue* value) asm("_ZN4HPHP14c_Continuation8t_updateElRKNS_7VariantE");

TypedValue* tg1_12Continuation_update(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12Continuation_update(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToInt64InPlace(args-0);
  th_12Continuation_update((this_), (long)(args[-0].m_data.num), (args-1));
  return rv;
}

TypedValue* tg_12Continuation_update(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_12Continuation_update((this_), (long)(args[-0].m_data.num), (args-1));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12Continuation_update(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Continuation::update", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::update");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Continuation::t_done()
_ZN4HPHP14c_Continuation6t_doneEv

this_ => rdi
*/

void th_12Continuation_done(ObjectData* this_) asm("_ZN4HPHP14c_Continuation6t_doneEv");

TypedValue* tg_12Continuation_done(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12Continuation_done((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::done", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::done");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Continuation::t_getlabel()
_ZN4HPHP14c_Continuation10t_getlabelEv

(return value) => rax
this_ => rdi
*/

long th_12Continuation_getLabel(ObjectData* this_) asm("_ZN4HPHP14c_Continuation10t_getlabelEv");

TypedValue* tg_12Continuation_getLabel(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_12Continuation_getLabel((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::getLabel", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::getLabel");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Continuation::t_num_args()
_ZN4HPHP14c_Continuation10t_num_argsEv

(return value) => rax
this_ => rdi
*/

long th_12Continuation_num_args(ObjectData* this_) asm("_ZN4HPHP14c_Continuation10t_num_argsEv");

TypedValue* tg_12Continuation_num_args(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_12Continuation_num_args((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::num_args", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::num_args");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Continuation::t_get_args()
_ZN4HPHP14c_Continuation10t_get_argsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_12Continuation_get_args(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_Continuation10t_get_argsEv");

TypedValue* tg_12Continuation_get_args(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_12Continuation_get_args((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::get_args", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::get_args");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Continuation::t_get_arg(long)
_ZN4HPHP14c_Continuation9t_get_argEl

(return value) => rax
_rv => rdi
this_ => rsi
id => rdx
*/

TypedValue* th_12Continuation_get_arg(TypedValue* _rv, ObjectData* this_, long id) asm("_ZN4HPHP14c_Continuation9t_get_argEl");

TypedValue* tg1_12Continuation_get_arg(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12Continuation_get_arg(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_12Continuation_get_arg((rv), (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_12Continuation_get_arg(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_12Continuation_get_arg((&(rv)), (this_), (long)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12Continuation_get_arg(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Continuation::get_arg", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::get_arg");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Continuation::t_current()
_ZN4HPHP14c_Continuation9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12Continuation_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_Continuation9t_currentEv");

TypedValue* tg_12Continuation_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12Continuation_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::current");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Continuation::t_key()
_ZN4HPHP14c_Continuation5t_keyEv

(return value) => rax
this_ => rdi
*/

long th_12Continuation_key(ObjectData* this_) asm("_ZN4HPHP14c_Continuation5t_keyEv");

TypedValue* tg_12Continuation_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_12Continuation_key((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::key");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Continuation::t_next()
_ZN4HPHP14c_Continuation6t_nextEv

this_ => rdi
*/

void th_12Continuation_next(ObjectData* this_) asm("_ZN4HPHP14c_Continuation6t_nextEv");

TypedValue* tg_12Continuation_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12Continuation_next((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::next");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Continuation::t_rewind()
_ZN4HPHP14c_Continuation8t_rewindEv

this_ => rdi
*/

void th_12Continuation_rewind(ObjectData* this_) asm("_ZN4HPHP14c_Continuation8t_rewindEv");

TypedValue* tg_12Continuation_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12Continuation_rewind((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::rewind");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Continuation::t_valid()
_ZN4HPHP14c_Continuation7t_validEv

(return value) => rax
this_ => rdi
*/

bool th_12Continuation_valid(ObjectData* this_) asm("_ZN4HPHP14c_Continuation7t_validEv");

TypedValue* tg_12Continuation_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_12Continuation_valid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::valid");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Continuation::t_send(HPHP::Variant const&)
_ZN4HPHP14c_Continuation6t_sendERKNS_7VariantE

this_ => rdi
v => rsi
*/

void th_12Continuation_send(ObjectData* this_, TypedValue* v) asm("_ZN4HPHP14c_Continuation6t_sendERKNS_7VariantE");

TypedValue* tg_12Continuation_send(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12Continuation_send((this_), (args-0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Continuation::send", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::send");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Continuation::t_raise(HPHP::Variant const&)
_ZN4HPHP14c_Continuation7t_raiseERKNS_7VariantE

this_ => rdi
v => rsi
*/

void th_12Continuation_raise(ObjectData* this_, TypedValue* v) asm("_ZN4HPHP14c_Continuation7t_raiseERKNS_7VariantE");

TypedValue* tg_12Continuation_raise(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12Continuation_raise((this_), (args-0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("Continuation::raise", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::raise");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_Continuation::t_raised()
_ZN4HPHP14c_Continuation8t_raisedEv

this_ => rdi
*/

void th_12Continuation_raised(ObjectData* this_) asm("_ZN4HPHP14c_Continuation8t_raisedEv");

TypedValue* tg_12Continuation_raised(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_12Continuation_raised((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::raised", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::raised");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Continuation::t_receive()
_ZN4HPHP14c_Continuation9t_receiveEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12Continuation_receive(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_Continuation9t_receiveEv");

TypedValue* tg_12Continuation_receive(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12Continuation_receive((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::receive", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::receive");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_Continuation::t_getorigfuncname()
_ZN4HPHP14c_Continuation17t_getorigfuncnameEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_12Continuation_getOrigFuncName(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_Continuation17t_getorigfuncnameEv");

TypedValue* tg_12Continuation_getOrigFuncName(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_12Continuation_getOrigFuncName((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::getOrigFuncName", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::getOrigFuncName");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Continuation::t___clone()
_ZN4HPHP14c_Continuation9t___cloneEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12Continuation___clone(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_Continuation9t___cloneEv");

TypedValue* tg_12Continuation___clone(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12Continuation___clone((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Continuation::__clone", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Continuation::__clone");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DummyContinuation_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DummyContinuation) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DummyContinuation(cls);
  return inst;
}

IMPLEMENT_CLASS(DummyContinuation);
/*
void HPHP::c_DummyContinuation::t___construct()
_ZN4HPHP19c_DummyContinuation13t___constructEv

this_ => rdi
*/

void th_17DummyContinuation___construct(ObjectData* this_) asm("_ZN4HPHP19c_DummyContinuation13t___constructEv");

TypedValue* tg_17DummyContinuation___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_17DummyContinuation___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyContinuation::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyContinuation::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DummyContinuation::t_current()
_ZN4HPHP19c_DummyContinuation9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_17DummyContinuation_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP19c_DummyContinuation9t_currentEv");

TypedValue* tg_17DummyContinuation_current(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_17DummyContinuation_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyContinuation::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyContinuation::current");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_DummyContinuation::t_key()
_ZN4HPHP19c_DummyContinuation5t_keyEv

(return value) => rax
this_ => rdi
*/

long th_17DummyContinuation_key(ObjectData* this_) asm("_ZN4HPHP19c_DummyContinuation5t_keyEv");

TypedValue* tg_17DummyContinuation_key(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_17DummyContinuation_key((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyContinuation::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyContinuation::key");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DummyContinuation::t_next()
_ZN4HPHP19c_DummyContinuation6t_nextEv

this_ => rdi
*/

void th_17DummyContinuation_next(ObjectData* this_) asm("_ZN4HPHP19c_DummyContinuation6t_nextEv");

TypedValue* tg_17DummyContinuation_next(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_17DummyContinuation_next((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyContinuation::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyContinuation::next");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DummyContinuation::t_rewind()
_ZN4HPHP19c_DummyContinuation8t_rewindEv

this_ => rdi
*/

void th_17DummyContinuation_rewind(ObjectData* this_) asm("_ZN4HPHP19c_DummyContinuation8t_rewindEv");

TypedValue* tg_17DummyContinuation_rewind(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_17DummyContinuation_rewind((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyContinuation::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyContinuation::rewind");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DummyContinuation::t_valid()
_ZN4HPHP19c_DummyContinuation7t_validEv

(return value) => rax
this_ => rdi
*/

bool th_17DummyContinuation_valid(ObjectData* this_) asm("_ZN4HPHP19c_DummyContinuation7t_validEv");

TypedValue* tg_17DummyContinuation_valid(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_17DummyContinuation_valid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyContinuation::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyContinuation::valid");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

