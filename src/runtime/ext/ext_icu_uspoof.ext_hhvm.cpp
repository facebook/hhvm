#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/exception_gate.h>
#include <exception>

namespace HPHP {

HPHP::VM::Instance* new_SpoofChecker_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SpoofChecker) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SpoofChecker(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_SpoofChecker::t___construct()
_ZN4HPHP14c_SpoofChecker13t___constructEv

this_ => rdi
*/

void th_12SpoofChecker___construct(ObjectData* this_) asm("_ZN4HPHP14c_SpoofChecker13t___constructEv");

TypedValue* tg_12SpoofChecker___construct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_12SpoofChecker___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SpoofChecker::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SpoofChecker::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SpoofChecker::t_issuspicious(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP14c_SpoofChecker14t_issuspiciousERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
this_ => rdi
text => rsi
issuesFound => rdx
*/

bool th_12SpoofChecker_isSuspicious(ObjectData* this_, Value* text, TypedValue* issuesFound) asm("_ZN4HPHP14c_SpoofChecker14t_issuspiciousERKNS_6StringERKNS_14VRefParamValueE");

TypedValue* tg1_12SpoofChecker_isSuspicious(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12SpoofChecker_isSuspicious(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = null;
  rv->m_data.num = (th_12SpoofChecker_isSuspicious((this_), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_12SpoofChecker_isSuspicious(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          VRefParamValue defVal1 = null;
          rv.m_data.num = (th_12SpoofChecker_isSuspicious((this_), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12SpoofChecker_isSuspicious(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SpoofChecker::isSuspicious", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SpoofChecker::isSuspicious");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SpoofChecker::t_areconfusable(HPHP::String const&, HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP14c_SpoofChecker15t_areconfusableERKNS_6StringES3_RKNS_14VRefParamValueE

(return value) => rax
this_ => rdi
s1 => rsi
s2 => rdx
issuesFound => rcx
*/

bool th_12SpoofChecker_areConfusable(ObjectData* this_, Value* s1, Value* s2, TypedValue* issuesFound) asm("_ZN4HPHP14c_SpoofChecker15t_areconfusableERKNS_6StringES3_RKNS_14VRefParamValueE");

TypedValue* tg1_12SpoofChecker_areConfusable(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12SpoofChecker_areConfusable(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal2 = null;
  rv->m_data.num = (th_12SpoofChecker_areConfusable((this_), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_12SpoofChecker_areConfusable(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          VRefParamValue defVal2 = null;
          rv.m_data.num = (th_12SpoofChecker_areConfusable((this_), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12SpoofChecker_areConfusable(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SpoofChecker::areConfusable", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("SpoofChecker::areConfusable");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
void HPHP::c_SpoofChecker::t_setallowedlocales(HPHP::String const&)
_ZN4HPHP14c_SpoofChecker19t_setallowedlocalesERKNS_6StringE

this_ => rdi
localesList => rsi
*/

void th_12SpoofChecker_setAllowedLocales(ObjectData* this_, Value* localesList) asm("_ZN4HPHP14c_SpoofChecker19t_setallowedlocalesERKNS_6StringE");

TypedValue* tg1_12SpoofChecker_setAllowedLocales(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12SpoofChecker_setAllowedLocales(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_12SpoofChecker_setAllowedLocales((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_12SpoofChecker_setAllowedLocales(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_12SpoofChecker_setAllowedLocales((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12SpoofChecker_setAllowedLocales(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SpoofChecker::setAllowedLocales", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SpoofChecker::setAllowedLocales");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
void HPHP::c_SpoofChecker::t_setchecks(int)
_ZN4HPHP14c_SpoofChecker11t_setchecksEi

this_ => rdi
checks => rsi
*/

void th_12SpoofChecker_setChecks(ObjectData* this_, int checks) asm("_ZN4HPHP14c_SpoofChecker11t_setchecksEi");

TypedValue* tg1_12SpoofChecker_setChecks(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12SpoofChecker_setChecks(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToInt64InPlace(args-0);
  th_12SpoofChecker_setChecks((this_), (int)(args[-0].m_data.num));
  return rv;
}

TypedValue* tg_12SpoofChecker_setChecks(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_12SpoofChecker_setChecks((this_), (int)(args[-0].m_data.num));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12SpoofChecker_setChecks(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SpoofChecker::setChecks", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SpoofChecker::setChecks");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_SpoofChecker::t___destruct()
_ZN4HPHP14c_SpoofChecker12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12SpoofChecker___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_SpoofChecker12t___destructEv");

TypedValue* tg_12SpoofChecker___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12SpoofChecker___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SpoofChecker::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SpoofChecker::__destruct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}


} // !HPHP

