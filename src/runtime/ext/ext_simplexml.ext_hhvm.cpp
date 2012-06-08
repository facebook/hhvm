#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/exception_gate.h>
#include <exception>

namespace HPHP {

/*
HPHP::Variant HPHP::f_simplexml_load_string(HPHP::String const&, HPHP::String const&, long long, HPHP::String const&, bool)
_ZN4HPHP23f_simplexml_load_stringERKNS_6StringES2_xS2_b

(return value) => rax
_rv => rdi
data => rsi
class_name => rdx
options => rcx
ns => r8
is_prefix => r9
*/

TypedValue* fh_simplexml_load_string(TypedValue* _rv, Value* data, Value* class_name, long long options, Value* ns, bool is_prefix) asm("_ZN4HPHP23f_simplexml_load_stringERKNS_6StringES2_xS2_b");

TypedValue * fg1_simplexml_load_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_simplexml_load_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal1 = "SimpleXMLElement";
  fh_simplexml_load_string((rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0), (count > 3) ? (Value*)(args-3) : (Value*)(&empty_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_simplexml_load_string(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfBoolean) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        String defVal1 = "SimpleXMLElement";
        fh_simplexml_load_string((&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0), (count > 3) ? (Value*)(args-3) : (Value*)(&empty_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_simplexml_load_string(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("simplexml_load_string", count, 1, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_simplexml_load_file(HPHP::String const&, HPHP::String const&, long long, HPHP::String const&, bool)
_ZN4HPHP21f_simplexml_load_fileERKNS_6StringES2_xS2_b

(return value) => rax
_rv => rdi
filename => rsi
class_name => rdx
options => rcx
ns => r8
is_prefix => r9
*/

TypedValue* fh_simplexml_load_file(TypedValue* _rv, Value* filename, Value* class_name, long long options, Value* ns, bool is_prefix) asm("_ZN4HPHP21f_simplexml_load_fileERKNS_6StringES2_xS2_b");

TypedValue * fg1_simplexml_load_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_simplexml_load_file(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  String defVal1 = "SimpleXMLElement";
  fh_simplexml_load_file((rv), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0), (count > 3) ? (Value*)(args-3) : (Value*)(&empty_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_simplexml_load_file(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfBoolean) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        String defVal1 = "SimpleXMLElement";
        fh_simplexml_load_file((&(rv)), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0), (count > 3) ? (Value*)(args-3) : (Value*)(&empty_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_simplexml_load_file(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("simplexml_load_file", count, 1, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_libxml_get_errors()
_ZN4HPHP19f_libxml_get_errorsEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_libxml_get_errors(TypedValue* _rv) asm("_ZN4HPHP19f_libxml_get_errorsEv");

TypedValue* fg_libxml_get_errors(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_libxml_get_errors((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("libxml_get_errors", 0, 1);
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
HPHP::Variant HPHP::f_libxml_get_last_error()
_ZN4HPHP23f_libxml_get_last_errorEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_libxml_get_last_error(TypedValue* _rv) asm("_ZN4HPHP23f_libxml_get_last_errorEv");

TypedValue* fg_libxml_get_last_error(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_libxml_get_last_error((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("libxml_get_last_error", 0, 1);
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
void HPHP::f_libxml_clear_errors()
_ZN4HPHP21f_libxml_clear_errorsEv

*/

void fh_libxml_clear_errors() asm("_ZN4HPHP21f_libxml_clear_errorsEv");

TypedValue* fg_libxml_clear_errors(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_data.num = 0LL;
      rv._count = 0;
      rv.m_type = KindOfNull;
      fh_libxml_clear_errors();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("libxml_clear_errors", 0, 1);
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
bool HPHP::f_libxml_use_internal_errors(HPHP::Variant const&)
_ZN4HPHP28f_libxml_use_internal_errorsERKNS_7VariantE

(return value) => rax
use_errors => rdi
*/

bool fh_libxml_use_internal_errors(TypedValue* use_errors) asm("_ZN4HPHP28f_libxml_use_internal_errorsERKNS_7VariantE");

TypedValue* fg_libxml_use_internal_errors(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_libxml_use_internal_errors((count > 0) ? (args-0) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("libxml_use_internal_errors", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
void HPHP::f_libxml_set_streams_context(HPHP::Object const&)
_ZN4HPHP28f_libxml_set_streams_contextERKNS_6ObjectE

streams_context => rdi
*/

void fh_libxml_set_streams_context(Value* streams_context) asm("_ZN4HPHP28f_libxml_set_streams_contextERKNS_6ObjectE");

TypedValue * fg1_libxml_set_streams_context(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_libxml_set_streams_context(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_libxml_set_streams_context((Value*)(args-0));
  return rv;
}

TypedValue* fg_libxml_set_streams_context(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_libxml_set_streams_context((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_libxml_set_streams_context(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("libxml_set_streams_context", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_libxml_disable_entity_loader(bool)
_ZN4HPHP30f_libxml_disable_entity_loaderEb

(return value) => rax
disable => rdi
*/

bool fh_libxml_disable_entity_loader(bool disable) asm("_ZN4HPHP30f_libxml_disable_entity_loaderEb");

TypedValue * fg1_libxml_disable_entity_loader(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_libxml_disable_entity_loader(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToBooleanInPlace(args-0);
  rv->m_data.num = (fh_libxml_disable_entity_loader((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_libxml_disable_entity_loader(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_libxml_disable_entity_loader((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_libxml_disable_entity_loader(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("libxml_disable_entity_loader", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



class c_SimpleXMLElement_Instance : public c_SimpleXMLElement {
public:
  c_SimpleXMLElement_Instance (HPHP::VM::Class* cls, unsigned nProps) {
    DECLARE_STACK_GC_ROOT(ObjectData, this);
    m_cls = cls;
    setAttributes(cls->getODAttrs()
                  | (cls->clsInfo()
                     ? 0 : IsInstance));
    m_propVec = (TypedValue *)((uintptr_t)this + sizeof(c_SimpleXMLElement));
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
    size_t builtinPropSize = sizeof(c_SimpleXMLElement) - sizeof(ObjectData);
    size_t size = sizeForNProps(nProps) + builtinPropSize;
    HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
    new ((void *)inst) c_SimpleXMLElement_Instance(cls, nProps);
    return inst;
  }
  void operator delete(void *p) {
    c_SimpleXMLElement_Instance *this_ = (c_SimpleXMLElement_Instance*)p;
    size_t nProps = this_->m_cls->numDeclProperties();
    size_t builtinPropSize UNUSED = sizeof(c_SimpleXMLElement) - sizeof(ObjectData);
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
    return Instance::o_instanceof(s) || c_SimpleXMLElement::o_instanceof(s);
  }
  virtual Variant* o_realProp(CStrRef s, int flags, CStrRef context) const {
    Variant *v = Instance::o_realProp(s, flags, context);
    if (v) return v;
    return c_SimpleXMLElement::o_realProp(s, flags, context);
  }
  virtual Variant* o_realPropPublic(CStrRef s, int flags) const {
    Variant *v = Instance::o_realPropPublic(s, flags);
    if (v) return v;
    return c_SimpleXMLElement::o_realPropPublic(s, flags);
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
    c_SimpleXMLElement::cloneSet(clone);
    Instance::cloneSet(clone);
  }
};

HPHP::VM::Instance* new_SimpleXMLElement_Instance(HPHP::VM::Class* cls) {
  return c_SimpleXMLElement_Instance::new_Instance(cls);
}

/*
void HPHP::c_SimpleXMLElement::t___construct(HPHP::String const&, long long, bool, HPHP::String const&, bool)
_ZN4HPHP18c_SimpleXMLElement13t___constructERKNS_6StringExbS3_b

this_ => rdi
data => rsi
options => rdx
data_is_url => rcx
ns => r8
is_prefix => r9
*/

void th_16SimpleXMLElement___construct(ObjectData* this_, Value* data, long long options, bool data_is_url, Value* ns, bool is_prefix) asm("_ZN4HPHP18c_SimpleXMLElement13t___constructERKNS_6StringExbS3_b");

TypedValue* tg1_16SimpleXMLElement___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement___construct(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
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
  th_16SimpleXMLElement___construct((this_), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (Value*)(args-3) : (Value*)(&empty_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
  return rv;
}

TypedValue* tg_16SimpleXMLElement___construct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 5LL) {
        if ((count <= 4 || (args-4)->m_type == KindOfBoolean) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_16SimpleXMLElement___construct((this_), (Value*)(args-0), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (Value*)(args-3) : (Value*)(&empty_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement___construct(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 5);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::__construct", count, 1, 5, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

/*
bool HPHP::c_SimpleXMLElement::t_offsetexists(HPHP::Variant const&)
_ZN4HPHP18c_SimpleXMLElement14t_offsetexistsERKNS_7VariantE

(return value) => rax
this_ => rdi
index => rsi
*/

bool th_16SimpleXMLElement_offsetExists(ObjectData* this_, TypedValue* index) asm("_ZN4HPHP18c_SimpleXMLElement14t_offsetexistsERKNS_7VariantE");

TypedValue* tg_16SimpleXMLElement_offsetExists(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_16SimpleXMLElement_offsetExists((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::offsetExists", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::offsetExists");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t_offsetget(HPHP::Variant const&)
_ZN4HPHP18c_SimpleXMLElement11t_offsetgetERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
index => rdx
*/

TypedValue* th_16SimpleXMLElement_offsetGet(TypedValue* _rv, ObjectData* this_, TypedValue* index) asm("_ZN4HPHP18c_SimpleXMLElement11t_offsetgetERKNS_7VariantE");

TypedValue* tg_16SimpleXMLElement_offsetGet(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_16SimpleXMLElement_offsetGet((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::offsetGet", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::offsetGet");
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
void HPHP::c_SimpleXMLElement::t_offsetset(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP18c_SimpleXMLElement11t_offsetsetERKNS_7VariantES3_

this_ => rdi
index => rsi
newvalue => rdx
*/

void th_16SimpleXMLElement_offsetSet(ObjectData* this_, TypedValue* index, TypedValue* newvalue) asm("_ZN4HPHP18c_SimpleXMLElement11t_offsetsetERKNS_7VariantES3_");

TypedValue* tg_16SimpleXMLElement_offsetSet(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_16SimpleXMLElement_offsetSet((this_), (args-0), (args-1));
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::offsetSet", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::offsetSet");
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
void HPHP::c_SimpleXMLElement::t_offsetunset(HPHP::Variant const&)
_ZN4HPHP18c_SimpleXMLElement13t_offsetunsetERKNS_7VariantE

this_ => rdi
index => rsi
*/

void th_16SimpleXMLElement_offsetUnset(ObjectData* this_, TypedValue* index) asm("_ZN4HPHP18c_SimpleXMLElement13t_offsetunsetERKNS_7VariantE");

TypedValue* tg_16SimpleXMLElement_offsetUnset(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_16SimpleXMLElement_offsetUnset((this_), (args-0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::offsetUnset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::offsetUnset");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t_getiterator()
_ZN4HPHP18c_SimpleXMLElement13t_getiteratorEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_16SimpleXMLElement_getIterator(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement13t_getiteratorEv");

TypedValue* tg_16SimpleXMLElement_getIterator(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_16SimpleXMLElement_getIterator((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::getIterator", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::getIterator");
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
long long HPHP::c_SimpleXMLElement::t_count()
_ZN4HPHP18c_SimpleXMLElement7t_countEv

(return value) => rax
this_ => rdi
*/

long long th_16SimpleXMLElement_count(ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement7t_countEv");

TypedValue* tg_16SimpleXMLElement_count(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)th_16SimpleXMLElement_count((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::count", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::count");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t_xpath(HPHP::String const&)
_ZN4HPHP18c_SimpleXMLElement7t_xpathERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
path => rdx
*/

TypedValue* th_16SimpleXMLElement_xpath(TypedValue* _rv, ObjectData* this_, Value* path) asm("_ZN4HPHP18c_SimpleXMLElement7t_xpathERKNS_6StringE");

TypedValue* tg1_16SimpleXMLElement_xpath(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_xpath(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_16SimpleXMLElement_xpath((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_16SimpleXMLElement_xpath(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_16SimpleXMLElement_xpath((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_xpath(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::xpath", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::xpath");
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
bool HPHP::c_SimpleXMLElement::t_registerxpathnamespace(HPHP::String const&, HPHP::String const&)
_ZN4HPHP18c_SimpleXMLElement24t_registerxpathnamespaceERKNS_6StringES3_

(return value) => rax
this_ => rdi
prefix => rsi
ns => rdx
*/

bool th_16SimpleXMLElement_registerXPathNamespace(ObjectData* this_, Value* prefix, Value* ns) asm("_ZN4HPHP18c_SimpleXMLElement24t_registerxpathnamespaceERKNS_6StringES3_");

TypedValue* tg1_16SimpleXMLElement_registerXPathNamespace(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_registerXPathNamespace(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_16SimpleXMLElement_registerXPathNamespace((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_16SimpleXMLElement_registerXPathNamespace(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_16SimpleXMLElement_registerXPathNamespace((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_registerXPathNamespace(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::registerXPathNamespace", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::registerXPathNamespace");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t_asxml(HPHP::String const&)
_ZN4HPHP18c_SimpleXMLElement7t_asxmlERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
filename => rdx
*/

TypedValue* th_16SimpleXMLElement_asXML(TypedValue* _rv, ObjectData* this_, Value* filename) asm("_ZN4HPHP18c_SimpleXMLElement7t_asxmlERKNS_6StringE");

TypedValue* tg1_16SimpleXMLElement_asXML(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_asXML(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_16SimpleXMLElement_asXML((rv), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&empty_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_16SimpleXMLElement_asXML(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          th_16SimpleXMLElement_asXML((&(rv)), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&empty_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_asXML(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::asXML", 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::asXML");
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
HPHP::Array HPHP::c_SimpleXMLElement::t_getnamespaces(bool)
_ZN4HPHP18c_SimpleXMLElement15t_getnamespacesEb

(return value) => rax
_rv => rdi
this_ => rsi
recursive => rdx
*/

Value* th_16SimpleXMLElement_getNamespaces(Value* _rv, ObjectData* this_, bool recursive) asm("_ZN4HPHP18c_SimpleXMLElement15t_getnamespacesEb");

TypedValue* tg1_16SimpleXMLElement_getNamespaces(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_getNamespaces(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToBooleanInPlace(args-0);
  th_16SimpleXMLElement_getNamespaces((Value*)(rv), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_16SimpleXMLElement_getNamespaces(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
          rv._count = 0;
          rv.m_type = KindOfArray;
          th_16SimpleXMLElement_getNamespaces((Value*)(&(rv)), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_getNamespaces(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::getNamespaces", 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::getNamespaces");
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
HPHP::Array HPHP::c_SimpleXMLElement::t_getdocnamespaces(bool)
_ZN4HPHP18c_SimpleXMLElement18t_getdocnamespacesEb

(return value) => rax
_rv => rdi
this_ => rsi
recursive => rdx
*/

Value* th_16SimpleXMLElement_getDocNamespaces(Value* _rv, ObjectData* this_, bool recursive) asm("_ZN4HPHP18c_SimpleXMLElement18t_getdocnamespacesEb");

TypedValue* tg1_16SimpleXMLElement_getDocNamespaces(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_getDocNamespaces(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToBooleanInPlace(args-0);
  th_16SimpleXMLElement_getDocNamespaces((Value*)(rv), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_16SimpleXMLElement_getDocNamespaces(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
          rv._count = 0;
          rv.m_type = KindOfArray;
          th_16SimpleXMLElement_getDocNamespaces((Value*)(&(rv)), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_getDocNamespaces(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::getDocNamespaces", 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::getDocNamespaces");
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
HPHP::Object HPHP::c_SimpleXMLElement::t_children(HPHP::String const&, bool)
_ZN4HPHP18c_SimpleXMLElement10t_childrenERKNS_6StringEb

(return value) => rax
_rv => rdi
this_ => rsi
ns => rdx
is_prefix => rcx
*/

Value* th_16SimpleXMLElement_children(Value* _rv, ObjectData* this_, Value* ns, bool is_prefix) asm("_ZN4HPHP18c_SimpleXMLElement10t_childrenERKNS_6StringEb");

TypedValue* tg1_16SimpleXMLElement_children(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_children(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  th_16SimpleXMLElement_children((Value*)(rv), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&empty_string), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_16SimpleXMLElement_children(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_16SimpleXMLElement_children((Value*)(&(rv)), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&empty_string), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_children(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::children", 2, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::children");
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
HPHP::String HPHP::c_SimpleXMLElement::t_getname()
_ZN4HPHP18c_SimpleXMLElement9t_getnameEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_16SimpleXMLElement_getName(Value* _rv, ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement9t_getnameEv");

TypedValue* tg_16SimpleXMLElement_getName(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_16SimpleXMLElement_getName((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::getName", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::getName");
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
HPHP::Object HPHP::c_SimpleXMLElement::t_attributes(HPHP::String const&, bool)
_ZN4HPHP18c_SimpleXMLElement12t_attributesERKNS_6StringEb

(return value) => rax
_rv => rdi
this_ => rsi
ns => rdx
is_prefix => rcx
*/

Value* th_16SimpleXMLElement_attributes(Value* _rv, ObjectData* this_, Value* ns, bool is_prefix) asm("_ZN4HPHP18c_SimpleXMLElement12t_attributesERKNS_6StringEb");

TypedValue* tg1_16SimpleXMLElement_attributes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_attributes(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  th_16SimpleXMLElement_attributes((Value*)(rv), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&empty_string), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_16SimpleXMLElement_attributes(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv._count = 0;
          rv.m_type = KindOfObject;
          th_16SimpleXMLElement_attributes((Value*)(&(rv)), (this_), (count > 0) ? (Value*)(args-0) : (Value*)(&empty_string), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_attributes(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::attributes", 2, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::attributes");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t_addchild(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP18c_SimpleXMLElement10t_addchildERKNS_6StringES3_S3_

(return value) => rax
_rv => rdi
this_ => rsi
qname => rdx
value => rcx
ns => r8
*/

TypedValue* th_16SimpleXMLElement_addChild(TypedValue* _rv, ObjectData* this_, Value* qname, Value* value, Value* ns) asm("_ZN4HPHP18c_SimpleXMLElement10t_addchildERKNS_6StringES3_S3_");

TypedValue* tg1_16SimpleXMLElement_addChild(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_addChild(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_16SimpleXMLElement_addChild((rv), (this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_16SimpleXMLElement_addChild(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          th_16SimpleXMLElement_addChild((&(rv)), (this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_addChild(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::addChild", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::addChild");
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
void HPHP::c_SimpleXMLElement::t_addattribute(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP18c_SimpleXMLElement14t_addattributeERKNS_6StringES3_S3_

this_ => rdi
qname => rsi
value => rdx
ns => rcx
*/

void th_16SimpleXMLElement_addAttribute(ObjectData* this_, Value* qname, Value* value, Value* ns) asm("_ZN4HPHP18c_SimpleXMLElement14t_addattributeERKNS_6StringES3_S3_");

TypedValue* tg1_16SimpleXMLElement_addAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_16SimpleXMLElement_addAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_16SimpleXMLElement_addAttribute((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
  return rv;
}

TypedValue* tg_16SimpleXMLElement_addAttribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_16SimpleXMLElement_addAttribute((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string));
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_16SimpleXMLElement_addAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::addAttribute", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::addAttribute");
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
HPHP::String HPHP::c_SimpleXMLElement::t___tostring()
_ZN4HPHP18c_SimpleXMLElement12t___tostringEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_16SimpleXMLElement___toString(Value* _rv, ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement12t___tostringEv");

TypedValue* tg_16SimpleXMLElement___toString(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_16SimpleXMLElement___toString((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::__toString", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::__toString");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t___get(HPHP::Variant)
_ZN4HPHP18c_SimpleXMLElement7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_16SimpleXMLElement___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP18c_SimpleXMLElement7t___getENS_7VariantE");

TypedValue* tg_16SimpleXMLElement___get(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_16SimpleXMLElement___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::__get");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t___set(HPHP::Variant, HPHP::Variant)
_ZN4HPHP18c_SimpleXMLElement7t___setENS_7VariantES1_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
value => rcx
*/

TypedValue* th_16SimpleXMLElement___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP18c_SimpleXMLElement7t___setENS_7VariantES1_");

TypedValue* tg_16SimpleXMLElement___set(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_16SimpleXMLElement___set((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::__set", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::__set");
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
bool HPHP::c_SimpleXMLElement::t___isset(HPHP::Variant)
_ZN4HPHP18c_SimpleXMLElement9t___issetENS_7VariantE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_16SimpleXMLElement___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP18c_SimpleXMLElement9t___issetENS_7VariantE");

TypedValue* tg_16SimpleXMLElement___isset(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_16SimpleXMLElement___isset((this_), (args-0))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::__isset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::__isset");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t___unset(HPHP::Variant)
_ZN4HPHP18c_SimpleXMLElement9t___unsetENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_16SimpleXMLElement___unset(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP18c_SimpleXMLElement9t___unsetENS_7VariantE");

TypedValue* tg_16SimpleXMLElement___unset(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_16SimpleXMLElement___unset((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("SimpleXMLElement::__unset", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::__unset");
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
HPHP::Variant HPHP::c_SimpleXMLElement::t___destruct()
_ZN4HPHP18c_SimpleXMLElement12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_16SimpleXMLElement___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement12t___destructEv");

TypedValue* tg_16SimpleXMLElement___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_16SimpleXMLElement___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElement::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElement::__destruct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

class c_LibXMLError_Instance : public c_LibXMLError {
public:
  c_LibXMLError_Instance (HPHP::VM::Class* cls, unsigned nProps) {
    DECLARE_STACK_GC_ROOT(ObjectData, this);
    m_cls = cls;
    setAttributes(cls->getODAttrs()
                  | (cls->clsInfo()
                     ? 0 : IsInstance));
    m_propVec = (TypedValue *)((uintptr_t)this + sizeof(c_LibXMLError));
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
    size_t builtinPropSize = sizeof(c_LibXMLError) - sizeof(ObjectData);
    size_t size = sizeForNProps(nProps) + builtinPropSize;
    HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
    new ((void *)inst) c_LibXMLError_Instance(cls, nProps);
    return inst;
  }
  void operator delete(void *p) {
    c_LibXMLError_Instance *this_ = (c_LibXMLError_Instance*)p;
    size_t nProps = this_->m_cls->numDeclProperties();
    size_t builtinPropSize UNUSED = sizeof(c_LibXMLError) - sizeof(ObjectData);
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
    return Instance::o_instanceof(s) || c_LibXMLError::o_instanceof(s);
  }
  virtual Variant* o_realProp(CStrRef s, int flags, CStrRef context) const {
    Variant *v = Instance::o_realProp(s, flags, context);
    if (v) return v;
    return c_LibXMLError::o_realProp(s, flags, context);
  }
  virtual Variant* o_realPropPublic(CStrRef s, int flags) const {
    Variant *v = Instance::o_realPropPublic(s, flags);
    if (v) return v;
    return c_LibXMLError::o_realPropPublic(s, flags);
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
    c_LibXMLError::cloneSet(clone);
    Instance::cloneSet(clone);
  }
};

HPHP::VM::Instance* new_LibXMLError_Instance(HPHP::VM::Class* cls) {
  return c_LibXMLError_Instance::new_Instance(cls);
}

/*
void HPHP::c_LibXMLError::t___construct()
_ZN4HPHP13c_LibXMLError13t___constructEv

this_ => rdi
*/

void th_11LibXMLError___construct(ObjectData* this_) asm("_ZN4HPHP13c_LibXMLError13t___constructEv");

TypedValue* tg_11LibXMLError___construct(HPHP::VM::ActRec *ar) {
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
        th_11LibXMLError___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("LibXMLError::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("LibXMLError::__construct");
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
HPHP::Variant HPHP::c_LibXMLError::t___destruct()
_ZN4HPHP13c_LibXMLError12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11LibXMLError___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_LibXMLError12t___destructEv");

TypedValue* tg_11LibXMLError___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11LibXMLError___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("LibXMLError::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("LibXMLError::__destruct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}

class c_SimpleXMLElementIterator_Instance : public c_SimpleXMLElementIterator {
public:
  c_SimpleXMLElementIterator_Instance (HPHP::VM::Class* cls, unsigned nProps) {
    DECLARE_STACK_GC_ROOT(ObjectData, this);
    m_cls = cls;
    setAttributes(cls->getODAttrs()
                  | (cls->clsInfo()
                     ? 0 : IsInstance));
    m_propVec = (TypedValue *)((uintptr_t)this + sizeof(c_SimpleXMLElementIterator));
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
    size_t builtinPropSize = sizeof(c_SimpleXMLElementIterator) - sizeof(ObjectData);
    size_t size = sizeForNProps(nProps) + builtinPropSize;
    HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
    new ((void *)inst) c_SimpleXMLElementIterator_Instance(cls, nProps);
    return inst;
  }
  void operator delete(void *p) {
    c_SimpleXMLElementIterator_Instance *this_ = (c_SimpleXMLElementIterator_Instance*)p;
    size_t nProps = this_->m_cls->numDeclProperties();
    size_t builtinPropSize UNUSED = sizeof(c_SimpleXMLElementIterator) - sizeof(ObjectData);
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
    return Instance::o_instanceof(s) || c_SimpleXMLElementIterator::o_instanceof(s);
  }
  virtual Variant* o_realProp(CStrRef s, int flags, CStrRef context) const {
    Variant *v = Instance::o_realProp(s, flags, context);
    if (v) return v;
    return c_SimpleXMLElementIterator::o_realProp(s, flags, context);
  }
  virtual Variant* o_realPropPublic(CStrRef s, int flags) const {
    Variant *v = Instance::o_realPropPublic(s, flags);
    if (v) return v;
    return c_SimpleXMLElementIterator::o_realPropPublic(s, flags);
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
    c_SimpleXMLElementIterator::cloneSet(clone);
    Instance::cloneSet(clone);
  }
};

HPHP::VM::Instance* new_SimpleXMLElementIterator_Instance(HPHP::VM::Class* cls) {
  return c_SimpleXMLElementIterator_Instance::new_Instance(cls);
}

/*
void HPHP::c_SimpleXMLElementIterator::t___construct()
_ZN4HPHP26c_SimpleXMLElementIterator13t___constructEv

this_ => rdi
*/

void th_24SimpleXMLElementIterator___construct(ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator13t___constructEv");

TypedValue* tg_24SimpleXMLElementIterator___construct(HPHP::VM::ActRec *ar) {
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
        th_24SimpleXMLElementIterator___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElementIterator::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElementIterator::__construct");
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
HPHP::Variant HPHP::c_SimpleXMLElementIterator::t_current()
_ZN4HPHP26c_SimpleXMLElementIterator9t_currentEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_24SimpleXMLElementIterator_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator9t_currentEv");

TypedValue* tg_24SimpleXMLElementIterator_current(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_24SimpleXMLElementIterator_current((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElementIterator::current", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElementIterator::current");
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
HPHP::Variant HPHP::c_SimpleXMLElementIterator::t_key()
_ZN4HPHP26c_SimpleXMLElementIterator5t_keyEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_24SimpleXMLElementIterator_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator5t_keyEv");

TypedValue* tg_24SimpleXMLElementIterator_key(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_24SimpleXMLElementIterator_key((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElementIterator::key", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElementIterator::key");
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
HPHP::Variant HPHP::c_SimpleXMLElementIterator::t_next()
_ZN4HPHP26c_SimpleXMLElementIterator6t_nextEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_24SimpleXMLElementIterator_next(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator6t_nextEv");

TypedValue* tg_24SimpleXMLElementIterator_next(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_24SimpleXMLElementIterator_next((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElementIterator::next", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElementIterator::next");
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
HPHP::Variant HPHP::c_SimpleXMLElementIterator::t_rewind()
_ZN4HPHP26c_SimpleXMLElementIterator8t_rewindEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_24SimpleXMLElementIterator_rewind(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator8t_rewindEv");

TypedValue* tg_24SimpleXMLElementIterator_rewind(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_24SimpleXMLElementIterator_rewind((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElementIterator::rewind", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElementIterator::rewind");
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
HPHP::Variant HPHP::c_SimpleXMLElementIterator::t_valid()
_ZN4HPHP26c_SimpleXMLElementIterator7t_validEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_24SimpleXMLElementIterator_valid(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator7t_validEv");

TypedValue* tg_24SimpleXMLElementIterator_valid(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_24SimpleXMLElementIterator_valid((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElementIterator::valid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElementIterator::valid");
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
HPHP::Variant HPHP::c_SimpleXMLElementIterator::t___destruct()
_ZN4HPHP26c_SimpleXMLElementIterator12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_24SimpleXMLElementIterator___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator12t___destructEv");

TypedValue* tg_24SimpleXMLElementIterator___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_24SimpleXMLElementIterator___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("SimpleXMLElementIterator::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("SimpleXMLElementIterator::__destruct");
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

