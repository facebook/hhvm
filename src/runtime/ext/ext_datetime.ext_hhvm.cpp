#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/exception_gate.h>
#include <exception>

namespace HPHP {

class c_DateTime_Instance : public c_DateTime {
public:
  c_DateTime_Instance (HPHP::VM::Class* cls, unsigned nProps) {
    DECLARE_STACK_GC_ROOT(ObjectData, this);
    m_cls = cls;
    setAttributes(cls->getODAttrs()
                  | (cls->clsInfo()
                     ? 0 : IsInstance));
    m_propVec = (TypedValue *)((uintptr_t)this + sizeof(c_DateTime));
    if (cls->needInitialization()) {
      cls->initialize();
    }
    if (nProps > 0) {
      if (cls->pinitVec().size() > 0) {
        initialize(nProps);
      } else {
        ASSERT(nProps == cls->declPropInit().size());
        memcpy(m_propVec, &cls->declPropInit()[0], nProps * sizeof(TypedValue));
      }
    }
  }
  static HPHP::VM::Instance* new_Instance(HPHP::VM::Class* cls) {
    size_t nProps = cls->numDeclProperties();
    size_t builtinPropSize = sizeof(c_DateTime) - sizeof(ObjectData);
    size_t size = sizeForNProps(nProps) + builtinPropSize;
    HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
    new ((void *)inst) c_DateTime_Instance(cls, nProps);
    return inst;
  }
  void operator delete(void *p) {
    c_DateTime_Instance *this_ = (c_DateTime_Instance*)p;
    size_t nProps = this_->m_cls->numDeclProperties();
    size_t builtinPropSize UNUSED = sizeof(c_DateTime) - sizeof(ObjectData);
    for (size_t i = 0; i < nProps; ++i) {
      TypedValue *prop = &this_->m_propVec[i];
      tvRefcountedDecRef(prop);
    }
    DELETEOBJSZ(sizeForNProps(nProps) + builtinPropSize)(this_);
  }
  virtual bool o_instanceof(const HPHP::String& s) const {
    return Instance::o_instanceof(s) || c_DateTime::o_instanceof(s);
  }
  virtual Variant* o_realProp(CStrRef s, int flags, CStrRef context) const {
    Variant *v = Instance::o_realProp(s, flags, context);
    if (v) return v;
    return c_DateTime::o_realProp(s, flags, context);
  }
  virtual Variant* o_realPropPublic(CStrRef s, int flags) const {
    Variant *v = Instance::o_realPropPublic(s, flags);
    if (v) return v;
    return c_DateTime::o_realPropPublic(s, flags);
  }
  virtual void o_setArray(CArrRef props) {
    ClassInfo::SetArray(this, o_getClassPropTable(), props);
  }
  virtual void o_getArray(Array &props, bool pubOnly) const {
    ClassInfo::GetArray(this, o_getClassPropTable(), props, false);
}
  virtual ObjectData* cloneImpl() {
    return Instance::cloneImpl();
  }
  virtual void cloneSet(ObjectData *clone) {
    c_DateTime::cloneSet(clone);
    Instance::cloneSet(clone);
  }
};

HPHP::VM::Instance* new_DateTime_Instance(HPHP::VM::Class* cls) {
  return c_DateTime_Instance::new_Instance(cls);
}

/*
void HPHP::c_DateTime::t___construct(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP10c_DateTime13t___constructERKNS_6StringERKNS_6ObjectE

this_ => rdi
time => rsi
timezone => rdx
*/

void th_8DateTime___construct(ObjectData* this_, Value* time, Value* timezone) asm("_ZN4HPHP10c_DateTime13t___constructERKNS_6StringERKNS_6ObjectE");

TypedValue* tg1_8DateTime___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  String defVal0 = "now";
  th_8DateTime___construct((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_object));
  return rv;
}

TypedValue* tg_8DateTime___construct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfObject) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          String defVal0 = "now";
          th_8DateTime___construct((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_object));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("DateTime::__construct", 2, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::__construct");
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
HPHP::String HPHP::c_DateTime::t_format(HPHP::String const&)
_ZN4HPHP10c_DateTime8t_formatERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
format => rdx
*/

Value* th_8DateTime_format(Value* _rv, ObjectData* this_, Value* format) asm("_ZN4HPHP10c_DateTime8t_formatERKNS_6StringE");

TypedValue* tg1_8DateTime_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_format(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  th_8DateTime_format((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_format(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_8DateTime_format((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_format(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::format", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::format");
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
long long HPHP::c_DateTime::t_getoffset()
_ZN4HPHP10c_DateTime11t_getoffsetEv

(return value) => rax
this_ => rdi
*/

long long th_8DateTime_getOffset(ObjectData* this_) asm("_ZN4HPHP10c_DateTime11t_getoffsetEv");

TypedValue* tg_8DateTime_getOffset(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_8DateTime_getOffset((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTime::getOffset", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::getOffset");
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
HPHP::Variant HPHP::c_DateTime::t_gettimezone()
_ZN4HPHP10c_DateTime13t_gettimezoneEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_8DateTime_getTimezone(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP10c_DateTime13t_gettimezoneEv");

TypedValue* tg_8DateTime_getTimezone(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_8DateTime_getTimezone((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTime::getTimezone", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::getTimezone");
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
HPHP::Object HPHP::c_DateTime::t_modify(HPHP::String const&)
_ZN4HPHP10c_DateTime8t_modifyERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
modify => rdx
*/

Value* th_8DateTime_modify(Value* _rv, ObjectData* this_, Value* modify) asm("_ZN4HPHP10c_DateTime8t_modifyERKNS_6StringE");

TypedValue* tg1_8DateTime_modify(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_modify(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  th_8DateTime_modify((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_modify(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_modify((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_modify(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::modify", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::modify");
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
HPHP::Object HPHP::c_DateTime::t_setdate(long long, long long, long long)
_ZN4HPHP10c_DateTime9t_setdateExxx

(return value) => rax
_rv => rdi
this_ => rsi
year => rdx
month => rcx
day => r8
*/

Value* th_8DateTime_setDate(Value* _rv, ObjectData* this_, long long year, long long month, long long day) asm("_ZN4HPHP10c_DateTime9t_setdateExxx");

TypedValue* tg1_8DateTime_setDate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setDate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_8DateTime_setDate((Value*)(rv), (this_), (long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num), (long long)(args[-2].m_data.num));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setDate(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 3LL) {
        if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setDate((Value*)(&(rv)), (this_), (long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num), (long long)(args[-2].m_data.num));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setDate(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setDate", count, 3, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setDate");
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
HPHP::Object HPHP::c_DateTime::t_setisodate(long long, long long, long long)
_ZN4HPHP10c_DateTime12t_setisodateExxx

(return value) => rax
_rv => rdi
this_ => rsi
year => rdx
week => rcx
day => r8
*/

Value* th_8DateTime_setISODate(Value* _rv, ObjectData* this_, long long year, long long week, long long day) asm("_ZN4HPHP10c_DateTime12t_setisodateExxx");

TypedValue* tg1_8DateTime_setISODate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setISODate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
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
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_8DateTime_setISODate((Value*)(rv), (this_), (long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setISODate(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setISODate((Value*)(&(rv)), (this_), (long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(1));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setISODate(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setISODate", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setISODate");
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
HPHP::Object HPHP::c_DateTime::t_settime(long long, long long, long long)
_ZN4HPHP10c_DateTime9t_settimeExxx

(return value) => rax
_rv => rdi
this_ => rsi
hour => rdx
minute => rcx
second => r8
*/

Value* th_8DateTime_setTime(Value* _rv, ObjectData* this_, long long hour, long long minute, long long second) asm("_ZN4HPHP10c_DateTime9t_settimeExxx");

TypedValue* tg1_8DateTime_setTime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setTime(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
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
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_8DateTime_setTime((Value*)(rv), (this_), (long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setTime(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setTime((Value*)(&(rv)), (this_), (long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setTime(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setTime", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setTime");
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
HPHP::Object HPHP::c_DateTime::t_settimezone(HPHP::Object const&)
_ZN4HPHP10c_DateTime13t_settimezoneERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
timezone => rdx
*/

Value* th_8DateTime_setTimezone(Value* _rv, ObjectData* this_, Value* timezone) asm("_ZN4HPHP10c_DateTime13t_settimezoneERKNS_6ObjectE");

TypedValue* tg1_8DateTime_setTimezone(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_8DateTime_setTimezone(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  th_8DateTime_setTimezone((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_8DateTime_setTimezone(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_8DateTime_setTimezone((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_8DateTime_setTimezone(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTime::setTimezone", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::setTimezone");
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
HPHP::Variant HPHP::c_DateTime::t___destruct()
_ZN4HPHP10c_DateTime12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_8DateTime___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP10c_DateTime12t___destructEv");

TypedValue* tg_8DateTime___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_8DateTime___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTime::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTime::__destruct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

class c_DateTimeZone_Instance : public c_DateTimeZone {
public:
  c_DateTimeZone_Instance (HPHP::VM::Class* cls, unsigned nProps) {
    DECLARE_STACK_GC_ROOT(ObjectData, this);
    m_cls = cls;
    setAttributes(cls->getODAttrs()
                  | (cls->clsInfo()
                     ? 0 : IsInstance));
    m_propVec = (TypedValue *)((uintptr_t)this + sizeof(c_DateTimeZone));
    if (cls->needInitialization()) {
      cls->initialize();
    }
    if (nProps > 0) {
      if (cls->pinitVec().size() > 0) {
        initialize(nProps);
      } else {
        ASSERT(nProps == cls->declPropInit().size());
        memcpy(m_propVec, &cls->declPropInit()[0], nProps * sizeof(TypedValue));
      }
    }
  }
  static HPHP::VM::Instance* new_Instance(HPHP::VM::Class* cls) {
    size_t nProps = cls->numDeclProperties();
    size_t builtinPropSize = sizeof(c_DateTimeZone) - sizeof(ObjectData);
    size_t size = sizeForNProps(nProps) + builtinPropSize;
    HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
    new ((void *)inst) c_DateTimeZone_Instance(cls, nProps);
    return inst;
  }
  void operator delete(void *p) {
    c_DateTimeZone_Instance *this_ = (c_DateTimeZone_Instance*)p;
    size_t nProps = this_->m_cls->numDeclProperties();
    size_t builtinPropSize UNUSED = sizeof(c_DateTimeZone) - sizeof(ObjectData);
    for (size_t i = 0; i < nProps; ++i) {
      TypedValue *prop = &this_->m_propVec[i];
      tvRefcountedDecRef(prop);
    }
    DELETEOBJSZ(sizeForNProps(nProps) + builtinPropSize)(this_);
  }
  virtual bool o_instanceof(const HPHP::String& s) const {
    return Instance::o_instanceof(s) || c_DateTimeZone::o_instanceof(s);
  }
  virtual Variant* o_realProp(CStrRef s, int flags, CStrRef context) const {
    Variant *v = Instance::o_realProp(s, flags, context);
    if (v) return v;
    return c_DateTimeZone::o_realProp(s, flags, context);
  }
  virtual Variant* o_realPropPublic(CStrRef s, int flags) const {
    Variant *v = Instance::o_realPropPublic(s, flags);
    if (v) return v;
    return c_DateTimeZone::o_realPropPublic(s, flags);
  }
  virtual void o_setArray(CArrRef props) {
    ClassInfo::SetArray(this, o_getClassPropTable(), props);
  }
  virtual void o_getArray(Array &props, bool pubOnly) const {
    ClassInfo::GetArray(this, o_getClassPropTable(), props, false);
}
  virtual ObjectData* cloneImpl() {
    return Instance::cloneImpl();
  }
  virtual void cloneSet(ObjectData *clone) {
    c_DateTimeZone::cloneSet(clone);
    Instance::cloneSet(clone);
  }
};

HPHP::VM::Instance* new_DateTimeZone_Instance(HPHP::VM::Class* cls) {
  return c_DateTimeZone_Instance::new_Instance(cls);
}

/*
void HPHP::c_DateTimeZone::t___construct(HPHP::String const&)
_ZN4HPHP14c_DateTimeZone13t___constructERKNS_6StringE

this_ => rdi
timezone => rsi
*/

void th_12DateTimeZone___construct(ObjectData* this_, Value* timezone) asm("_ZN4HPHP14c_DateTimeZone13t___constructERKNS_6StringE");

TypedValue* tg1_12DateTimeZone___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateTimeZone___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_12DateTimeZone___construct((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_12DateTimeZone___construct(HPHP::VM::ActRec *ar) {
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
          th_12DateTimeZone___construct((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12DateTimeZone___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTimeZone::__construct", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::__construct");
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
HPHP::String HPHP::c_DateTimeZone::t_getname()
_ZN4HPHP14c_DateTimeZone9t_getnameEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_12DateTimeZone_getName(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_DateTimeZone9t_getnameEv");

TypedValue* tg_12DateTimeZone_getName(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_12DateTimeZone_getName((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTimeZone::getName", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::getName");
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
long long HPHP::c_DateTimeZone::t_getoffset(HPHP::Object const&)
_ZN4HPHP14c_DateTimeZone11t_getoffsetERKNS_6ObjectE

(return value) => rax
this_ => rdi
datetime => rsi
*/

long long th_12DateTimeZone_getOffset(ObjectData* this_, Value* datetime) asm("_ZN4HPHP14c_DateTimeZone11t_getoffsetERKNS_6ObjectE");

TypedValue* tg1_12DateTimeZone_getOffset(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_12DateTimeZone_getOffset(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (long long)th_12DateTimeZone_getOffset((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_12DateTimeZone_getOffset(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          rv._count = 0;
          rv.m_type = KindOfInt64;
          rv.m_data.num = (long long)th_12DateTimeZone_getOffset((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_12DateTimeZone_getOffset(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DateTimeZone::getOffset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::getOffset");
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
HPHP::Array HPHP::c_DateTimeZone::t_gettransitions()
_ZN4HPHP14c_DateTimeZone16t_gettransitionsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_12DateTimeZone_getTransitions(Value* _rv, ObjectData* this_) asm("_ZN4HPHP14c_DateTimeZone16t_gettransitionsEv");

TypedValue* tg_12DateTimeZone_getTransitions(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_12DateTimeZone_getTransitions((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTimeZone::getTransitions", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::getTransitions");
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
HPHP::Array HPHP::c_DateTimeZone::ti_listabbreviations(char const*)
_ZN4HPHP14c_DateTimeZone20ti_listabbreviationsEPKc

(return value) => rax
_rv => rdi
cls_ => rsi
*/

Value* th_12DateTimeZone_listAbbreviations(Value* _rv, char const* cls_) asm("_ZN4HPHP14c_DateTimeZone20ti_listabbreviationsEPKc");

TypedValue* tg_12DateTimeZone_listAbbreviations(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      th_12DateTimeZone_listAbbreviations((Value*)(&(rv)), ("DateTimeZone"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("DateTimeZone::listAbbreviations", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Array HPHP::c_DateTimeZone::ti_listidentifiers(char const*)
_ZN4HPHP14c_DateTimeZone18ti_listidentifiersEPKc

(return value) => rax
_rv => rdi
cls_ => rsi
*/

Value* th_12DateTimeZone_listIdentifiers(Value* _rv, char const* cls_) asm("_ZN4HPHP14c_DateTimeZone18ti_listidentifiersEPKc");

TypedValue* tg_12DateTimeZone_listIdentifiers(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      th_12DateTimeZone_listIdentifiers((Value*)(&(rv)), ("DateTimeZone"));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("DateTimeZone::listIdentifiers", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
HPHP::Variant HPHP::c_DateTimeZone::t___destruct()
_ZN4HPHP14c_DateTimeZone12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12DateTimeZone___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_DateTimeZone12t___destructEv");

TypedValue* tg_12DateTimeZone___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12DateTimeZone___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DateTimeZone::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DateTimeZone::__destruct");
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

