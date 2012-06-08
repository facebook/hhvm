#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/exception_gate.h>
#include <exception>

namespace HPHP {

class c_DummyClosure_Instance : public c_DummyClosure {
public:
  c_DummyClosure_Instance (HPHP::VM::Class* cls, unsigned nProps) {
    DECLARE_STACK_GC_ROOT(ObjectData, this);
    m_cls = cls;
    setAttributes(cls->getODAttrs()
                  | (cls->clsInfo()
                     ? 0 : IsInstance));
    m_propVec = (TypedValue *)((uintptr_t)this + sizeof(c_DummyClosure));
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
    size_t builtinPropSize = sizeof(c_DummyClosure) - sizeof(ObjectData);
    size_t size = sizeForNProps(nProps) + builtinPropSize;
    HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
    new ((void *)inst) c_DummyClosure_Instance(cls, nProps);
    return inst;
  }
  void operator delete(void *p) {
    c_DummyClosure_Instance *this_ = (c_DummyClosure_Instance*)p;
    size_t nProps = this_->m_cls->numDeclProperties();
    size_t builtinPropSize UNUSED = sizeof(c_DummyClosure) - sizeof(ObjectData);
    if (this_->m_propMap) {
      this_->m_propMap->release();
    }
    for (size_t i = 0; i < nProps; ++i) {
      TypedValue *prop = &this_->m_propVec[i];
      tvRefcountedDecRef(prop);
    }
    DELETEOBJSZ(sizeForNProps(nProps) + builtinPropSize)(this_);
  }
  virtual bool o_instanceof(const HPHP::String& s) const {
    return Instance::o_instanceof(s) || c_DummyClosure::o_instanceof(s);
  }
  virtual Variant* o_realProp(CStrRef s, int flags, CStrRef context) const {
    Variant *v = Instance::o_realProp(s, flags, context);
    if (v) return v;
    return c_DummyClosure::o_realProp(s, flags, context);
  }
  virtual Variant* o_realPropPublic(CStrRef s, int flags) const {
    Variant *v = Instance::o_realPropPublic(s, flags);
    if (v) return v;
    return c_DummyClosure::o_realPropPublic(s, flags);
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
    c_DummyClosure::cloneSet(clone);
    Instance::cloneSet(clone);
  }
};

HPHP::VM::Instance* new_DummyClosure_Instance(HPHP::VM::Class* cls) {
  return c_DummyClosure_Instance::new_Instance(cls);
}

/*
void HPHP::c_DummyClosure::t___construct()
_ZN4HPHP14c_DummyClosure13t___constructEv

this_ => rdi
*/

void th_12DummyClosure___construct(ObjectData* this_) asm("_ZN4HPHP14c_DummyClosure13t___constructEv");

TypedValue* tg_12DummyClosure___construct(HPHP::VM::ActRec *ar) {
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
        th_12DummyClosure___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyClosure::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyClosure::__construct");
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
HPHP::Variant HPHP::c_DummyClosure::t___destruct()
_ZN4HPHP14c_DummyClosure12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_12DummyClosure___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP14c_DummyClosure12t___destructEv");

TypedValue* tg_12DummyClosure___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_12DummyClosure___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DummyClosure::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DummyClosure::__destruct");
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

