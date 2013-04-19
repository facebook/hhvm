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

#include "runtime/ext_hhvm/ext_hhvm.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/base/array/array_init.h"
#include "runtime/ext/ext.h"
#include "runtime/vm/class.h"
#include "runtime/vm/runtime.h"
#include <exception>

namespace HPHP {


HPHP::VM::Instance* new_Memcache_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Memcache) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Memcache(cls);
  return inst;
}

IMPLEMENT_CLASS(Memcache);
void th_8Memcache___construct(ObjectData* this_) asm("_ZN4HPHP10c_Memcache13t___constructEv");

TypedValue* tg_8Memcache___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_8Memcache___construct((this_));
    } else {
      throw_toomany_arguments_nr("Memcache::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_connect(ObjectData* this_, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP10c_Memcache9t_connectERKNS_6StringEiii");

void tg1_8Memcache_connect(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_connect(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_connect((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_connect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 4) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_connect((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_connect(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::connect", count, 1, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::connect");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_pconnect(ObjectData* this_, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP10c_Memcache10t_pconnectERKNS_6StringEiii");

void tg1_8Memcache_pconnect(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_pconnect(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_pconnect((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_pconnect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 4) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_pconnect((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_pconnect(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::pconnect", count, 1, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::pconnect");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_add(ObjectData* this_, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP10c_Memcache5t_addERKNS_6StringERKNS_7VariantEii");

void tg1_8Memcache_add(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_add(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_add((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_add(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 4) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_add((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_add(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::add", count, 2, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::add");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_set(ObjectData* this_, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP10c_Memcache5t_setERKNS_6StringERKNS_7VariantEii");

void tg1_8Memcache_set(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_set(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_set((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_set(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 4) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_set((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_set(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::set", count, 2, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::set");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_replace(ObjectData* this_, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP10c_Memcache9t_replaceERKNS_6StringERKNS_7VariantEii");

void tg1_8Memcache_replace(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_replace(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_replace((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_replace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2 && count <= 4) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_replace((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_replace(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::replace", count, 2, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::replace");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_8Memcache_get(TypedValue* _rv, ObjectData* this_, TypedValue* key, TypedValue* flags) asm("_ZN4HPHP10c_Memcache5t_getERKNS_7VariantERKNS_14VRefParamValueE");

TypedValue* tg_8Memcache_get(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      VRefParamValue defVal1 = uninit_null();
      th_8Memcache_get(rv, (this_), (args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("Memcache::get", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::get");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_delete(ObjectData* this_, Value* key, int expire) asm("_ZN4HPHP10c_Memcache8t_deleteERKNS_6StringEi");

void tg1_8Memcache_delete(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_delete(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_delete((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_delete(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_delete((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_delete(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::delete", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::delete");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_8Memcache_increment(ObjectData* this_, Value* key, int offset) asm("_ZN4HPHP10c_Memcache11t_incrementERKNS_6StringEi");

void tg1_8Memcache_increment(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_increment(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)th_8Memcache_increment((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
}

TypedValue* tg_8Memcache_increment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfInt64;
        rv->m_data.num = (int64_t)th_8Memcache_increment((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
      } else {
        tg1_8Memcache_increment(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::increment", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::increment");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_8Memcache_decrement(ObjectData* this_, Value* key, int offset) asm("_ZN4HPHP10c_Memcache11t_decrementERKNS_6StringEi");

void tg1_8Memcache_decrement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_decrement(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)th_8Memcache_decrement((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
}

TypedValue* tg_8Memcache_decrement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfInt64;
        rv->m_data.num = (int64_t)th_8Memcache_decrement((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
      } else {
        tg1_8Memcache_decrement(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::decrement", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::decrement");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_close(ObjectData* this_) asm("_ZN4HPHP10c_Memcache7t_closeEv");

TypedValue* tg_8Memcache_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_8Memcache_close((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("Memcache::close", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::close");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_8Memcache_getversion(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP10c_Memcache12t_getversionEv");

TypedValue* tg_8Memcache_getversion(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_8Memcache_getversion(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("Memcache::getversion", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::getversion");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_flush(ObjectData* this_, int expire) asm("_ZN4HPHP10c_Memcache7t_flushEi");

void tg1_8Memcache_flush(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_flush(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_flush((this_), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfInt64)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_flush((this_), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_flush(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("Memcache::flush", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::flush");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_setoptimeout(ObjectData* this_, long timeoutms) asm("_ZN4HPHP10c_Memcache14t_setoptimeoutEl");

void tg1_8Memcache_setoptimeout(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_setoptimeout(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_setoptimeout((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_setoptimeout(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_setoptimeout((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_setoptimeout(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::setoptimeout", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::setoptimeout");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_8Memcache_getserverstatus(ObjectData* this_, Value* host, int port) asm("_ZN4HPHP10c_Memcache17t_getserverstatusERKNS_6StringEi");

void tg1_8Memcache_getserverstatus(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_getserverstatus(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)th_8Memcache_getserverstatus((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
}

TypedValue* tg_8Memcache_getserverstatus(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfInt64;
        rv->m_data.num = (int64_t)th_8Memcache_getserverstatus((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
      } else {
        tg1_8Memcache_getserverstatus(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::getserverstatus", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::getserverstatus");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_setcompressthreshold(ObjectData* this_, int threshold, double min_savings) asm("_ZN4HPHP10c_Memcache22t_setcompressthresholdEid");

void tg1_8Memcache_setcompressthreshold(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_setcompressthreshold(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_setcompressthreshold((this_), (int)(args[-0].m_data.num), (count > 1) ? (args[-1].m_data.dbl) : (double)(0.2))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_setcompressthreshold(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfDouble) &&
          (args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_setcompressthreshold((this_), (int)(args[-0].m_data.num), (count > 1) ? (args[-1].m_data.dbl) : (double)(0.2))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_setcompressthreshold(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::setcompressthreshold", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::setcompressthreshold");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_8Memcache_getstats(Value* _rv, ObjectData* this_, Value* type, int slabid, int limit) asm("_ZN4HPHP10c_Memcache10t_getstatsERKNS_6StringEii");

void tg1_8Memcache_getstats(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_getstats(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfArray;
  th_8Memcache_getstats(&(rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(100));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_8Memcache_getstats(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        rv->m_type = KindOfArray;
        th_8Memcache_getstats(&(rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(100));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_8Memcache_getstats(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("Memcache::getstats", 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::getstats");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_8Memcache_getextendedstats(Value* _rv, ObjectData* this_, Value* type, int slabid, int limit) asm("_ZN4HPHP10c_Memcache18t_getextendedstatsERKNS_6StringEii");

void tg1_8Memcache_getextendedstats(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_getextendedstats(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfArray;
  th_8Memcache_getextendedstats(&(rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(100));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_8Memcache_getextendedstats(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        rv->m_type = KindOfArray;
        th_8Memcache_getextendedstats(&(rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(100));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_8Memcache_getextendedstats(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("Memcache::getextendedstats", 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::getextendedstats");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_setserverparams(ObjectData* this_, Value* host, int port, int timeout, int retry_interval, bool status, TypedValue* failure_callback) asm("_ZN4HPHP10c_Memcache17t_setserverparamsERKNS_6StringEiiibRKNS_7VariantE");

void tg1_8Memcache_setserverparams(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_setserverparams(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_setserverparams((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(11211), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(true), (count > 5) ? (args-5) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_setserverparams(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 6) {
      if ((count <= 4 || (args - 4)->m_type == KindOfBoolean) &&
          (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_setserverparams((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(11211), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(true), (count > 5) ? (args-5) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_setserverparams(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::setserverparams", count, 1, 6, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::setserverparams");
  }
  frame_free_locals_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_8Memcache_addserver(ObjectData* this_, Value* host, int port, bool persistent, int weight, int timeout, int retry_interval, bool status, TypedValue* failure_callback, int timeoutms) asm("_ZN4HPHP10c_Memcache11t_addserverERKNS_6StringEibiiibRKNS_7VariantEi");

void tg1_8Memcache_addserver(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_8Memcache_addserver(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_8Memcache_addserver((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(11211), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(true), (count > 7) ? (args-7) : (TypedValue*)(&null_variant), (count > 8) ? (int)(args[-8].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* tg_8Memcache_addserver(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 9) {
      if ((count <= 8 || (args - 8)->m_type == KindOfInt64) &&
          (count <= 6 || (args - 6)->m_type == KindOfBoolean) &&
          (count <= 5 || (args - 5)->m_type == KindOfInt64) &&
          (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
          (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_8Memcache_addserver((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(11211), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(true), (count > 7) ? (args-7) : (TypedValue*)(&null_variant), (count > 8) ? (int)(args[-8].m_data.num) : (int)(0))) ? 1LL : 0LL;
      } else {
        tg1_8Memcache_addserver(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("Memcache::addserver", count, 1, 9, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::addserver");
  }
  frame_free_locals_inl(ar, 9);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_8Memcache___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP10c_Memcache12t___destructEv");

TypedValue* tg_8Memcache___destruct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_8Memcache___destruct(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("Memcache::__destruct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("Memcache::__destruct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_memcache_connect(Value* _rv, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP18f_memcache_connectERKNS_6StringEiii");

void fg1_memcache_connect(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_connect(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfObject;
  fh_memcache_connect(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_memcache_connect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfObject;
      fh_memcache_connect(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_memcache_connect(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_connect", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_memcache_pconnect(Value* _rv, Value* host, int port, int timeout, int timeoutms) asm("_ZN4HPHP19f_memcache_pconnectERKNS_6StringEiii");

void fg1_memcache_pconnect(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_pconnect(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfObject;
  fh_memcache_pconnect(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_memcache_pconnect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfObject;
      fh_memcache_pconnect(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_memcache_pconnect(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_pconnect", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_add(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP14f_memcache_addERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

void fg1_memcache_add(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_add(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_add(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_add(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_add(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_memcache_add(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_add", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_set(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP14f_memcache_setERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

void fg1_memcache_set(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_set(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_set(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_set(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_set(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_memcache_set(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_set", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_replace(Value* memcache, Value* key, TypedValue* var, int flag, int expire) asm("_ZN4HPHP18f_memcache_replaceERKNS_6ObjectERKNS_6StringERKNS_7VariantEii");

void fg1_memcache_replace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_replace(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_replace(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_replace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_replace(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_memcache_replace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_replace", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_memcache_get(TypedValue* _rv, Value* memcache, TypedValue* key, TypedValue* flags) asm("_ZN4HPHP14f_memcache_getERKNS_6ObjectERKNS_7VariantERKNS_14VRefParamValueE");

void fg1_memcache_get(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_get(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  VRefParamValue defVal2 = uninit_null();
  fh_memcache_get(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_memcache_get(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((args - 0)->m_type == KindOfObject) {
      VRefParamValue defVal2 = uninit_null();
      fh_memcache_get(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_memcache_get(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_get", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_delete(Value* memcache, Value* key, int expire) asm("_ZN4HPHP17f_memcache_deleteERKNS_6ObjectERKNS_6StringEi");

void fg1_memcache_delete(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_delete(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_delete(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_delete(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_delete(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_memcache_delete(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_delete", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_memcache_increment(Value* memcache, Value* key, int offset) asm("_ZN4HPHP20f_memcache_incrementERKNS_6ObjectERKNS_6StringEi");

void fg1_memcache_increment(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_increment(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_memcache_increment(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(1));
}

TypedValue* fg_memcache_increment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_memcache_increment(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(1));
    } else {
      fg1_memcache_increment(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_increment", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_memcache_decrement(Value* memcache, Value* key, int offset) asm("_ZN4HPHP20f_memcache_decrementERKNS_6ObjectERKNS_6StringEi");

void fg1_memcache_decrement(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_decrement(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_memcache_decrement(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(1));
}

TypedValue* fg_memcache_decrement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_memcache_decrement(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(1));
    } else {
      fg1_memcache_decrement(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_decrement", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_close(Value* memcache) asm("_ZN4HPHP16f_memcache_closeERKNS_6ObjectE");

void fg1_memcache_close(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_close(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_close(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_memcache_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_close(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_memcache_close(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_close", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_debug(bool onoff) asm("_ZN4HPHP16f_memcache_debugEb");

void fg1_memcache_debug(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_debug(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_debug((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_debug(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfBoolean) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_debug((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_memcache_debug(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_debug", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_memcache_get_version(TypedValue* _rv, Value* memcache) asm("_ZN4HPHP22f_memcache_get_versionERKNS_6ObjectE");

void fg1_memcache_get_version(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_get_version(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_memcache_get_version(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_memcache_get_version(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_memcache_get_version(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_memcache_get_version(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_get_version", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_flush(Value* memcache, int timestamp) asm("_ZN4HPHP16f_memcache_flushERKNS_6ObjectEi");

void fg1_memcache_flush(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_flush(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_flush(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_flush(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_memcache_flush(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_flush", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_setoptimeout(Value* memcache, int timeoutms) asm("_ZN4HPHP23f_memcache_setoptimeoutERKNS_6ObjectEi");

void fg1_memcache_setoptimeout(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_setoptimeout(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_setoptimeout(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_setoptimeout(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_setoptimeout(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_memcache_setoptimeout(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_setoptimeout", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_memcache_get_server_status(Value* memcache, Value* host, int port) asm("_ZN4HPHP28f_memcache_get_server_statusERKNS_6ObjectERKNS_6StringEi");

void fg1_memcache_get_server_status(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_get_server_status(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_memcache_get_server_status(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
}

TypedValue* fg_memcache_get_server_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_memcache_get_server_status(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
    } else {
      fg1_memcache_get_server_status(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_get_server_status", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_set_compress_threshold(Value* memcache, int threshold, double min_savings) asm("_ZN4HPHP33f_memcache_set_compress_thresholdERKNS_6ObjectEid");

void fg1_memcache_set_compress_threshold(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_set_compress_threshold(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_set_compress_threshold(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.2))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_set_compress_threshold(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfDouble) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_set_compress_threshold(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (args[-2].m_data.dbl) : (double)(0.2))) ? 1LL : 0LL;
    } else {
      fg1_memcache_set_compress_threshold(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_set_compress_threshold", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_memcache_get_stats(Value* _rv, Value* memcache, Value* type, int slabid, int limit) asm("_ZN4HPHP20f_memcache_get_statsERKNS_6ObjectERKNS_6StringEii");

void fg1_memcache_get_stats(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_get_stats(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfArray;
  fh_memcache_get_stats(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(100));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_memcache_get_stats(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_memcache_get_stats(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(100));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_memcache_get_stats(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_get_stats", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_memcache_get_extended_stats(Value* _rv, Value* memcache, Value* type, int slabid, int limit) asm("_ZN4HPHP29f_memcache_get_extended_statsERKNS_6ObjectERKNS_6StringEii");

void fg1_memcache_get_extended_stats(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_get_extended_stats(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfArray;
  fh_memcache_get_extended_stats(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(100));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_memcache_get_extended_stats(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_memcache_get_extended_stats(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(100));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_memcache_get_extended_stats(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_get_extended_stats", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_set_server_params(Value* memcache, Value* host, int port, int timeout, int retry_interval, bool status, TypedValue* failure_callback) asm("_ZN4HPHP28f_memcache_set_server_paramsERKNS_6ObjectERKNS_6StringEiiibRKNS_7VariantE");

void fg1_memcache_set_server_params(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_set_server_params(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_set_server_params(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(11211), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(true), (count > 6) ? (args-6) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_set_server_params(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 7) {
    if ((count <= 5 || (args - 5)->m_type == KindOfBoolean) &&
        (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_set_server_params(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(11211), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(true), (count > 6) ? (args-6) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
    } else {
      fg1_memcache_set_server_params(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_set_server_params", count, 2, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_memcache_add_server(Value* memcache, Value* host, int port, bool persistent, int weight, int timeout, int retry_interval, bool status, TypedValue* failure_callback, int timeoutms) asm("_ZN4HPHP21f_memcache_add_serverERKNS_6ObjectERKNS_6StringEibiiibRKNS_7VariantEi");

void fg1_memcache_add_server(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_memcache_add_server(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_memcache_add_server(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(11211), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (int)(args[-6].m_data.num) : (int)(0), (count > 7) ? (bool)(args[-7].m_data.num) : (bool)(true), (count > 8) ? (args-8) : (TypedValue*)(&null_variant), (count > 9) ? (int)(args[-9].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_memcache_add_server(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 10) {
    if ((count <= 9 || (args - 9)->m_type == KindOfInt64) &&
        (count <= 7 || (args - 7)->m_type == KindOfBoolean) &&
        (count <= 6 || (args - 6)->m_type == KindOfInt64) &&
        (count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_memcache_add_server(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(11211), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (int)(args[-6].m_data.num) : (int)(0), (count > 7) ? (bool)(args[-7].m_data.num) : (bool)(true), (count > 8) ? (args-8) : (TypedValue*)(&null_variant), (count > 9) ? (int)(args[-9].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_memcache_add_server(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("memcache_add_server", count, 2, 10, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 10);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
