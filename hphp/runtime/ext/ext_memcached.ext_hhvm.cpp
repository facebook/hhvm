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

HPHP::VM::Instance* new_Memcached_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_Memcached) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_Memcached(cls);
  return inst;
}

IMPLEMENT_CLASS(Memcached);
/*
void HPHP::c_Memcached::t___construct(HPHP::String const&)
_ZN4HPHP11c_Memcached13t___constructERKNS_6StringE

this_ => rdi
persistent_id => rsi
*/

void th_9Memcached___construct(ObjectData* this_, Value* persistent_id) asm("_ZN4HPHP11c_Memcached13t___constructERKNS_6StringE");

TypedValue* tg1_9Memcached___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached___construct(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_9Memcached___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_9Memcached___construct(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv.m_type = KindOfNull;
          th_9Memcached___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("Memcached::__construct", 1, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_add(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP11c_Memcached5t_addERKNS_6StringERKNS_7VariantEi

(return value) => rax
this_ => rdi
key => rsi
value => rdx
expiration => rcx
*/

bool th_9Memcached_add(ObjectData* this_, Value* key, TypedValue* value, int expiration) asm("_ZN4HPHP11c_Memcached5t_addERKNS_6StringERKNS_7VariantEi");

TypedValue* tg1_9Memcached_add(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_add(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_add((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_add(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_add((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_add(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::add", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::add");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_addbykey(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP11c_Memcached10t_addbykeyERKNS_6StringES3_RKNS_7VariantEi

(return value) => rax
this_ => rdi
server_key => rsi
key => rdx
value => rcx
expiration => r8
*/

bool th_9Memcached_addByKey(ObjectData* this_, Value* server_key, Value* key, TypedValue* value, int expiration) asm("_ZN4HPHP11c_Memcached10t_addbykeyERKNS_6StringES3_RKNS_7VariantEi");

TypedValue* tg1_9Memcached_addByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_addByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_addByKey((this_), &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_addByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 3LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_addByKey((this_), &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_addByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::addByKey", count, 3, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::addByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_addserver(HPHP::String const&, int, int)
_ZN4HPHP11c_Memcached11t_addserverERKNS_6StringEii

(return value) => rax
this_ => rdi
host => rsi
port => rdx
weight => rcx
*/

bool th_9Memcached_addServer(ObjectData* this_, Value* host, int port, int weight) asm("_ZN4HPHP11c_Memcached11t_addserverERKNS_6StringEii");

TypedValue* tg1_9Memcached_addServer(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_addServer(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_addServer((this_), &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_addServer(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_addServer((this_), &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_addServer(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::addServer", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::addServer");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_addservers(HPHP::Array const&)
_ZN4HPHP11c_Memcached12t_addserversERKNS_5ArrayE

(return value) => rax
this_ => rdi
servers => rsi
*/

bool th_9Memcached_addServers(ObjectData* this_, Value* servers) asm("_ZN4HPHP11c_Memcached12t_addserversERKNS_5ArrayE");

TypedValue* tg1_9Memcached_addServers(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_addServers(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToArrayInPlace(args-0);
  rv->m_data.num = (th_9Memcached_addServers((this_), &args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_addServers(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfArray) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_addServers((this_), &args[-0].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_addServers(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::addServers", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::addServers");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_append(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_Memcached8t_appendERKNS_6StringES3_

(return value) => rax
this_ => rdi
key => rsi
value => rdx
*/

bool th_9Memcached_append(ObjectData* this_, Value* key, Value* value) asm("_ZN4HPHP11c_Memcached8t_appendERKNS_6StringES3_");

TypedValue* tg1_9Memcached_append(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_append(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_append((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_append(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_append((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_append(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::append", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::append");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_appendbykey(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_Memcached13t_appendbykeyERKNS_6StringES3_S3_

(return value) => rax
this_ => rdi
server_key => rsi
key => rdx
value => rcx
*/

bool th_9Memcached_appendByKey(ObjectData* this_, Value* server_key, Value* key, Value* value) asm("_ZN4HPHP11c_Memcached13t_appendbykeyERKNS_6StringES3_S3_");

TypedValue* tg1_9Memcached_appendByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_appendByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_appendByKey((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_appendByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_appendByKey((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_appendByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::appendByKey", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::appendByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_cas(double, HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP11c_Memcached5t_casEdRKNS_6StringERKNS_7VariantEi

(return value) => rax
this_ => rdi
cas_token => xmm0
key => rsi
value => rdx
expiration => rcx
*/

bool th_9Memcached_cas(ObjectData* this_, double cas_token, Value* key, TypedValue* value, int expiration) asm("_ZN4HPHP11c_Memcached5t_casEdRKNS_6StringERKNS_7VariantEi");

TypedValue* tg1_9Memcached_cas(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_cas(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_cas((this_), (args[-0].m_data.dbl), &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_cas(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 3LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfDouble) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_cas((this_), (args[-0].m_data.dbl), &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_cas(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::cas", count, 3, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::cas");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_casbykey(double, HPHP::String const&, HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP11c_Memcached10t_casbykeyEdRKNS_6StringES3_RKNS_7VariantEi

(return value) => rax
this_ => rdi
cas_token => xmm0
server_key => rsi
key => rdx
value => rcx
expiration => r8
*/

bool th_9Memcached_casByKey(ObjectData* this_, double cas_token, Value* server_key, Value* key, TypedValue* value, int expiration) asm("_ZN4HPHP11c_Memcached10t_casbykeyEdRKNS_6StringES3_RKNS_7VariantEi");

TypedValue* tg1_9Memcached_casByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_casByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_casByKey((this_), (args[-0].m_data.dbl), &args[-1].m_data, &args[-2].m_data, (args-3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_casByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 4LL && count <= 5LL) {
        if ((count <= 4 || (args-4)->m_type == KindOfInt64) && IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfDouble) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_casByKey((this_), (args[-0].m_data.dbl), &args[-1].m_data, &args[-2].m_data, (args-3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_casByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::casByKey", count, 4, 5, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::casByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_decrement(HPHP::String const&, long)
_ZN4HPHP11c_Memcached11t_decrementERKNS_6StringEl

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
offset => rcx
*/

TypedValue* th_9Memcached_decrement(TypedValue* _rv, ObjectData* this_, Value* key, long offset) asm("_ZN4HPHP11c_Memcached11t_decrementERKNS_6StringEl");

TypedValue* tg1_9Memcached_decrement(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_decrement(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_9Memcached_decrement((rv), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9Memcached_decrement(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          th_9Memcached_decrement((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(1));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_decrement(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::decrement", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::decrement");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_delete(HPHP::String const&, int)
_ZN4HPHP11c_Memcached8t_deleteERKNS_6StringEi

(return value) => rax
this_ => rdi
key => rsi
time => rdx
*/

bool th_9Memcached_delete(ObjectData* this_, Value* key, int time) asm("_ZN4HPHP11c_Memcached8t_deleteERKNS_6StringEi");

TypedValue* tg1_9Memcached_delete(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_delete(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  rv->m_data.num = (th_9Memcached_delete((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_delete(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_delete((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_delete(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::delete", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::delete");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_deletebykey(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP11c_Memcached13t_deletebykeyERKNS_6StringES3_i

(return value) => rax
this_ => rdi
server_key => rsi
key => rdx
time => rcx
*/

bool th_9Memcached_deleteByKey(ObjectData* this_, Value* server_key, Value* key, int time) asm("_ZN4HPHP11c_Memcached13t_deletebykeyERKNS_6StringES3_i");

TypedValue* tg1_9Memcached_deleteByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_deleteByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_deleteByKey((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_deleteByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_deleteByKey((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_deleteByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::deleteByKey", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::deleteByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_fetch()
_ZN4HPHP11c_Memcached7t_fetchEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_9Memcached_fetch(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP11c_Memcached7t_fetchEv");

TypedValue* tg_9Memcached_fetch(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_9Memcached_fetch((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcached::fetch", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::fetch");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_fetchall()
_ZN4HPHP11c_Memcached10t_fetchallEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_9Memcached_fetchAll(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP11c_Memcached10t_fetchallEv");

TypedValue* tg_9Memcached_fetchAll(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_9Memcached_fetchAll((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcached::fetchAll", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::fetchAll");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_flush(int)
_ZN4HPHP11c_Memcached7t_flushEi

(return value) => rax
this_ => rdi
delay => rsi
*/

bool th_9Memcached_flush(ObjectData* this_, int delay) asm("_ZN4HPHP11c_Memcached7t_flushEi");

TypedValue* tg1_9Memcached_flush(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_flush(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (th_9Memcached_flush((this_), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_flush(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_flush((this_), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_flush(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("Memcached::flush", 1, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::flush");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_get(HPHP::String const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP11c_Memcached5t_getERKNS_6StringERKNS_7VariantERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
cache_cb => rcx
cas_token => r8
*/

TypedValue* th_9Memcached_get(TypedValue* _rv, ObjectData* this_, Value* key, TypedValue* cache_cb, TypedValue* cas_token) asm("_ZN4HPHP11c_Memcached5t_getERKNS_6StringERKNS_7VariantERKNS_14VRefParamValueE");

TypedValue* tg1_9Memcached_get(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_get(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal2 = null_variant;
  th_9Memcached_get((rv), (this_), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&null_variant), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9Memcached_get(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          VRefParamValue defVal2 = null_variant;
          th_9Memcached_get((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&null_variant), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_get(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::get", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::get");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_getbykey(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, HPHP::VRefParamValue const&)
_ZN4HPHP11c_Memcached10t_getbykeyERKNS_6StringES3_RKNS_7VariantERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
this_ => rsi
server_key => rdx
key => rcx
cache_cb => r8
cas_token => r9
*/

TypedValue* th_9Memcached_getByKey(TypedValue* _rv, ObjectData* this_, Value* server_key, Value* key, TypedValue* cache_cb, TypedValue* cas_token) asm("_ZN4HPHP11c_Memcached10t_getbykeyERKNS_6StringES3_RKNS_7VariantERKNS_14VRefParamValueE");

TypedValue* tg1_9Memcached_getByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_getByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal3 = null_variant;
  th_9Memcached_getByKey((rv), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9Memcached_getByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 4LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          VRefParamValue defVal3 = null_variant;
          th_9Memcached_getByKey((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_getByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::getByKey", count, 2, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_getdelayed(HPHP::Array const&, bool, HPHP::Variant const&)
_ZN4HPHP11c_Memcached12t_getdelayedERKNS_5ArrayEbRKNS_7VariantE

(return value) => rax
this_ => rdi
keys => rsi
with_cas => rdx
value_cb => rcx
*/

bool th_9Memcached_getDelayed(ObjectData* this_, Value* keys, bool with_cas, TypedValue* value_cb) asm("_ZN4HPHP11c_Memcached12t_getdelayedERKNS_5ArrayEbRKNS_7VariantE");

TypedValue* tg1_9Memcached_getDelayed(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_getDelayed(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_getDelayed((this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_getDelayed(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfArray) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_getDelayed((this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_getDelayed(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::getDelayed", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getDelayed");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_getdelayedbykey(HPHP::String const&, HPHP::Array const&, bool, HPHP::Variant const&)
_ZN4HPHP11c_Memcached17t_getdelayedbykeyERKNS_6StringERKNS_5ArrayEbRKNS_7VariantE

(return value) => rax
this_ => rdi
server_key => rsi
keys => rdx
with_cas => rcx
value_cb => r8
*/

bool th_9Memcached_getDelayedByKey(ObjectData* this_, Value* server_key, Value* keys, bool with_cas, TypedValue* value_cb) asm("_ZN4HPHP11c_Memcached17t_getdelayedbykeyERKNS_6StringERKNS_5ArrayEbRKNS_7VariantE");

TypedValue* tg1_9Memcached_getDelayedByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_getDelayedByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_getDelayedByKey((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_getDelayedByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 4LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (args-1)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_getDelayedByKey((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_getDelayedByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::getDelayedByKey", count, 2, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getDelayedByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_getmulti(HPHP::Array const&, HPHP::VRefParamValue const&, int)
_ZN4HPHP11c_Memcached10t_getmultiERKNS_5ArrayERKNS_14VRefParamValueEi

(return value) => rax
_rv => rdi
this_ => rsi
keys => rdx
cas_tokens => rcx
flags => r8
*/

TypedValue* th_9Memcached_getMulti(TypedValue* _rv, ObjectData* this_, Value* keys, TypedValue* cas_tokens, int flags) asm("_ZN4HPHP11c_Memcached10t_getmultiERKNS_5ArrayERKNS_14VRefParamValueEi");

TypedValue* tg1_9Memcached_getMulti(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_getMulti(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-0);
  }
  VRefParamValue defVal1 = null_variant;
  th_9Memcached_getMulti((rv), (this_), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9Memcached_getMulti(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-0)->m_type == KindOfArray) {
          VRefParamValue defVal1 = null_variant;
          th_9Memcached_getMulti((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_getMulti(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::getMulti", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getMulti");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_getmultibykey(HPHP::String const&, HPHP::Array const&, HPHP::VRefParamValue const&, int)
_ZN4HPHP11c_Memcached15t_getmultibykeyERKNS_6StringERKNS_5ArrayERKNS_14VRefParamValueEi

(return value) => rax
_rv => rdi
this_ => rsi
server_key => rdx
keys => rcx
cas_tokens => r8
flags => r9
*/

TypedValue* th_9Memcached_getMultiByKey(TypedValue* _rv, ObjectData* this_, Value* server_key, Value* keys, TypedValue* cas_tokens, int flags) asm("_ZN4HPHP11c_Memcached15t_getmultibykeyERKNS_6StringERKNS_5ArrayERKNS_14VRefParamValueEi");

TypedValue* tg1_9Memcached_getMultiByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_getMultiByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal2 = null_variant;
  th_9Memcached_getMultiByKey((rv), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9Memcached_getMultiByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-1)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
          VRefParamValue defVal2 = null_variant;
          th_9Memcached_getMultiByKey((&(rv)), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_getMultiByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::getMultiByKey", count, 2, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getMultiByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_getoption(int)
_ZN4HPHP11c_Memcached11t_getoptionEi

(return value) => rax
_rv => rdi
this_ => rsi
option => rdx
*/

TypedValue* th_9Memcached_getOption(TypedValue* _rv, ObjectData* this_, int option) asm("_ZN4HPHP11c_Memcached11t_getoptionEi");

TypedValue* tg1_9Memcached_getOption(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_getOption(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_9Memcached_getOption((rv), (this_), (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9Memcached_getOption(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_9Memcached_getOption((&(rv)), (this_), (int)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_getOption(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::getOption", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getOption");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_Memcached::t_getresultcode()
_ZN4HPHP11c_Memcached15t_getresultcodeEv

(return value) => rax
this_ => rdi
*/

long th_9Memcached_getResultCode(ObjectData* this_) asm("_ZN4HPHP11c_Memcached15t_getresultcodeEv");

TypedValue* tg_9Memcached_getResultCode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_9Memcached_getResultCode((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcached::getResultCode", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getResultCode");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_Memcached::t_getresultmessage()
_ZN4HPHP11c_Memcached18t_getresultmessageEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9Memcached_getResultMessage(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_Memcached18t_getresultmessageEv");

TypedValue* tg_9Memcached_getResultMessage(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfString;
        th_9Memcached_getResultMessage((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcached::getResultMessage", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getResultMessage");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_getserverbykey(HPHP::String const&)
_ZN4HPHP11c_Memcached16t_getserverbykeyERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
server_key => rdx
*/

TypedValue* th_9Memcached_getServerByKey(TypedValue* _rv, ObjectData* this_, Value* server_key) asm("_ZN4HPHP11c_Memcached16t_getserverbykeyERKNS_6StringE");

TypedValue* tg1_9Memcached_getServerByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_getServerByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_9Memcached_getServerByKey((rv), (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9Memcached_getServerByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_9Memcached_getServerByKey((&(rv)), (this_), &args[-0].m_data);
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_getServerByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::getServerByKey", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getServerByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_Memcached::t_getserverlist()
_ZN4HPHP11c_Memcached15t_getserverlistEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9Memcached_getServerList(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_Memcached15t_getserverlistEv");

TypedValue* tg_9Memcached_getServerList(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_9Memcached_getServerList((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcached::getServerList", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getServerList");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_getstats()
_ZN4HPHP11c_Memcached10t_getstatsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_9Memcached_getStats(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP11c_Memcached10t_getstatsEv");

TypedValue* tg_9Memcached_getStats(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_9Memcached_getStats((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcached::getStats", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getStats");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_getversion()
_ZN4HPHP11c_Memcached12t_getversionEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_9Memcached_getVersion(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP11c_Memcached12t_getversionEv");

TypedValue* tg_9Memcached_getVersion(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_9Memcached_getVersion((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("Memcached::getVersion", 0, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::getVersion");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_Memcached::t_increment(HPHP::String const&, long)
_ZN4HPHP11c_Memcached11t_incrementERKNS_6StringEl

(return value) => rax
_rv => rdi
this_ => rsi
key => rdx
offset => rcx
*/

TypedValue* th_9Memcached_increment(TypedValue* _rv, ObjectData* this_, Value* key, long offset) asm("_ZN4HPHP11c_Memcached11t_incrementERKNS_6StringEl");

TypedValue* tg1_9Memcached_increment(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_increment(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  th_9Memcached_increment((rv), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9Memcached_increment(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          th_9Memcached_increment((&(rv)), (this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(1));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_increment(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::increment", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::increment");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_prepend(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_Memcached9t_prependERKNS_6StringES3_

(return value) => rax
this_ => rdi
key => rsi
value => rdx
*/

bool th_9Memcached_prepend(ObjectData* this_, Value* key, Value* value) asm("_ZN4HPHP11c_Memcached9t_prependERKNS_6StringES3_");

TypedValue* tg1_9Memcached_prepend(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_prepend(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_prepend((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_prepend(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_prepend((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_prepend(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::prepend", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::prepend");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_prependbykey(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_Memcached14t_prependbykeyERKNS_6StringES3_S3_

(return value) => rax
this_ => rdi
server_key => rsi
key => rdx
value => rcx
*/

bool th_9Memcached_prependByKey(ObjectData* this_, Value* server_key, Value* key, Value* value) asm("_ZN4HPHP11c_Memcached14t_prependbykeyERKNS_6StringES3_S3_");

TypedValue* tg1_9Memcached_prependByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_prependByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_prependByKey((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_prependByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_prependByKey((this_), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_prependByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::prependByKey", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::prependByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_replace(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP11c_Memcached9t_replaceERKNS_6StringERKNS_7VariantEi

(return value) => rax
this_ => rdi
key => rsi
value => rdx
expiration => rcx
*/

bool th_9Memcached_replace(ObjectData* this_, Value* key, TypedValue* value, int expiration) asm("_ZN4HPHP11c_Memcached9t_replaceERKNS_6StringERKNS_7VariantEi");

TypedValue* tg1_9Memcached_replace(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_replace(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_replace((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_replace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_replace((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_replace(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::replace", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::replace");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_replacebykey(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP11c_Memcached14t_replacebykeyERKNS_6StringES3_RKNS_7VariantEi

(return value) => rax
this_ => rdi
server_key => rsi
key => rdx
value => rcx
expiration => r8
*/

bool th_9Memcached_replaceByKey(ObjectData* this_, Value* server_key, Value* key, TypedValue* value, int expiration) asm("_ZN4HPHP11c_Memcached14t_replacebykeyERKNS_6StringES3_RKNS_7VariantEi");

TypedValue* tg1_9Memcached_replaceByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_replaceByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_replaceByKey((this_), &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_replaceByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 3LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_replaceByKey((this_), &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_replaceByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::replaceByKey", count, 3, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::replaceByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_set(HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP11c_Memcached5t_setERKNS_6StringERKNS_7VariantEi

(return value) => rax
this_ => rdi
key => rsi
value => rdx
expiration => rcx
*/

bool th_9Memcached_set(ObjectData* this_, Value* key, TypedValue* value, int expiration) asm("_ZN4HPHP11c_Memcached5t_setERKNS_6StringERKNS_7VariantEi");

TypedValue* tg1_9Memcached_set(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_set(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_set((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_set(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_set((this_), &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_set(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::set", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::set");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_setbykey(HPHP::String const&, HPHP::String const&, HPHP::Variant const&, int)
_ZN4HPHP11c_Memcached10t_setbykeyERKNS_6StringES3_RKNS_7VariantEi

(return value) => rax
this_ => rdi
server_key => rsi
key => rdx
value => rcx
expiration => r8
*/

bool th_9Memcached_setByKey(ObjectData* this_, Value* server_key, Value* key, TypedValue* value, int expiration) asm("_ZN4HPHP11c_Memcached10t_setbykeyERKNS_6StringES3_RKNS_7VariantEi");

TypedValue* tg1_9Memcached_setByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_setByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_setByKey((this_), &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_setByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 3LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_setByKey((this_), &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_setByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::setByKey", count, 3, 4, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::setByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_setmulti(HPHP::Array const&, int)
_ZN4HPHP11c_Memcached10t_setmultiERKNS_5ArrayEi

(return value) => rax
this_ => rdi
items => rsi
expiration => rdx
*/

bool th_9Memcached_setMulti(ObjectData* this_, Value* items, int expiration) asm("_ZN4HPHP11c_Memcached10t_setmultiERKNS_5ArrayEi");

TypedValue* tg1_9Memcached_setMulti(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_setMulti(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  if ((args-0)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_setMulti((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_setMulti(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfArray) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_setMulti((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_setMulti(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::setMulti", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::setMulti");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_setmultibykey(HPHP::String const&, HPHP::Array const&, int)
_ZN4HPHP11c_Memcached15t_setmultibykeyERKNS_6StringERKNS_5ArrayEi

(return value) => rax
this_ => rdi
server_key => rsi
items => rdx
expiration => rcx
*/

bool th_9Memcached_setMultiByKey(ObjectData* this_, Value* server_key, Value* items, int expiration) asm("_ZN4HPHP11c_Memcached15t_setmultibykeyERKNS_6StringERKNS_5ArrayEi");

TypedValue* tg1_9Memcached_setMultiByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_setMultiByKey(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
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
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9Memcached_setMultiByKey((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_setMultiByKey(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_setMultiByKey((this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_setMultiByKey(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::setMultiByKey", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::setMultiByKey");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_Memcached::t_setoption(int, HPHP::Variant const&)
_ZN4HPHP11c_Memcached11t_setoptionEiRKNS_7VariantE

(return value) => rax
this_ => rdi
option => rsi
value => rdx
*/

bool th_9Memcached_setOption(ObjectData* this_, int option, TypedValue* value) asm("_ZN4HPHP11c_Memcached11t_setoptionEiRKNS_7VariantE");

TypedValue* tg1_9Memcached_setOption(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9Memcached_setOption(TypedValue* rv, ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (th_9Memcached_setOption((this_), (int)(args[-0].m_data.num), (args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9Memcached_setOption(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9Memcached_setOption((this_), (int)(args[-0].m_data.num), (args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9Memcached_setOption(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("Memcached::setOption", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("Memcached::setOption");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

