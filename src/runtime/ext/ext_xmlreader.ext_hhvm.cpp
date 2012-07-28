#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/exception_gate.h>
#include <exception>

namespace HPHP {

HPHP::VM::Instance* new_XMLReader_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_XMLReader) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_XMLReader(ObjectStaticCallbacks::encodeVMClass(cls));
  return inst;
}

/*
void HPHP::c_XMLReader::t___construct()
_ZN4HPHP11c_XMLReader13t___constructEv

this_ => rdi
*/

void th_9XMLReader___construct(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader13t___constructEv");

TypedValue* tg_9XMLReader___construct(HPHP::VM::ActRec *ar) {
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
        th_9XMLReader___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::__construct");
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
bool HPHP::c_XMLReader::t_open(HPHP::String const&, HPHP::String const&, long long)
_ZN4HPHP11c_XMLReader6t_openERKNS_6StringES3_x

(return value) => rax
this_ => rdi
uri => rsi
encoding => rdx
options => rcx
*/

bool th_9XMLReader_open(ObjectData* this_, Value* uri, Value* encoding, long long options) asm("_ZN4HPHP11c_XMLReader6t_openERKNS_6StringES3_x");

TypedValue* tg1_9XMLReader_open(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_open(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
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
  rv->m_data.num = (th_9XMLReader_open((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_open(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_open((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_open(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::open", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::open");
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
bool HPHP::c_XMLReader::t_xml(HPHP::String const&, HPHP::String const&, long long)
_ZN4HPHP11c_XMLReader5t_xmlERKNS_6StringES3_x

(return value) => rax
this_ => rdi
source => rsi
encoding => rdx
options => rcx
*/

bool th_9XMLReader_XML(ObjectData* this_, Value* source, Value* encoding, long long options) asm("_ZN4HPHP11c_XMLReader5t_xmlERKNS_6StringES3_x");

TypedValue* tg1_9XMLReader_XML(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_XML(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 3
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
  rv->m_data.num = (th_9XMLReader_XML((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_XML(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_XML((this_), (Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_XML(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::XML", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::XML");
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
bool HPHP::c_XMLReader::t_close()
_ZN4HPHP11c_XMLReader7t_closeEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLReader_close(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader7t_closeEv");

TypedValue* tg_9XMLReader_close(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLReader_close((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::close", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::close");
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
bool HPHP::c_XMLReader::t_read()
_ZN4HPHP11c_XMLReader6t_readEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLReader_read(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader6t_readEv");

TypedValue* tg_9XMLReader_read(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLReader_read((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::read", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::read");
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
bool HPHP::c_XMLReader::t_next(HPHP::String const&)
_ZN4HPHP11c_XMLReader6t_nextERKNS_6StringE

(return value) => rax
this_ => rdi
localname => rsi
*/

bool th_9XMLReader_next(ObjectData* this_, Value* localname) asm("_ZN4HPHP11c_XMLReader6t_nextERKNS_6StringE");

TypedValue* tg1_9XMLReader_next(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_next(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLReader_next((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_next(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_next((this_), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_next(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("XMLReader::next", 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::next");
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
HPHP::String HPHP::c_XMLReader::t_readstring()
_ZN4HPHP11c_XMLReader12t_readstringEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9XMLReader_readString(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_XMLReader12t_readstringEv");

TypedValue* tg_9XMLReader_readString(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_9XMLReader_readString((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::readString", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::readString");
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
HPHP::String HPHP::c_XMLReader::t_readinnerxml()
_ZN4HPHP11c_XMLReader14t_readinnerxmlEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9XMLReader_readInnerXML(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_XMLReader14t_readinnerxmlEv");

TypedValue* tg_9XMLReader_readInnerXML(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_9XMLReader_readInnerXML((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::readInnerXML", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::readInnerXML");
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
HPHP::String HPHP::c_XMLReader::t_readouterxml()
_ZN4HPHP11c_XMLReader14t_readouterxmlEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_9XMLReader_readOuterXML(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_XMLReader14t_readouterxmlEv");

TypedValue* tg_9XMLReader_readOuterXML(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_9XMLReader_readOuterXML((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::readOuterXML", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::readOuterXML");
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
bool HPHP::c_XMLReader::t_movetonextattribute()
_ZN4HPHP11c_XMLReader21t_movetonextattributeEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLReader_moveToNextAttribute(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader21t_movetonextattributeEv");

TypedValue* tg_9XMLReader_moveToNextAttribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLReader_moveToNextAttribute((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::moveToNextAttribute", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::moveToNextAttribute");
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
HPHP::Variant HPHP::c_XMLReader::t_getattribute(HPHP::String const&)
_ZN4HPHP11c_XMLReader14t_getattributeERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_9XMLReader_getAttribute(TypedValue* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLReader14t_getattributeERKNS_6StringE");

TypedValue* tg1_9XMLReader_getAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_getAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_9XMLReader_getAttribute((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9XMLReader_getAttribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_9XMLReader_getAttribute((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_getAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::getAttribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::getAttribute");
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
HPHP::Variant HPHP::c_XMLReader::t_getattributeno(long long)
_ZN4HPHP11c_XMLReader16t_getattributenoEx

(return value) => rax
_rv => rdi
this_ => rsi
index => rdx
*/

TypedValue* th_9XMLReader_getAttributeNo(TypedValue* _rv, ObjectData* this_, long long index) asm("_ZN4HPHP11c_XMLReader16t_getattributenoEx");

TypedValue* tg1_9XMLReader_getAttributeNo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_getAttributeNo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_9XMLReader_getAttributeNo((rv), (this_), (long long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9XMLReader_getAttributeNo(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          th_9XMLReader_getAttributeNo((&(rv)), (this_), (long long)(args[-0].m_data.num));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_getAttributeNo(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::getAttributeNo", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::getAttributeNo");
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
HPHP::Variant HPHP::c_XMLReader::t_getattributens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLReader16t_getattributensERKNS_6StringES3_

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
namespaceURI => rcx
*/

TypedValue* th_9XMLReader_getAttributeNs(TypedValue* _rv, ObjectData* this_, Value* name, Value* namespaceURI) asm("_ZN4HPHP11c_XMLReader16t_getattributensERKNS_6StringES3_");

TypedValue* tg1_9XMLReader_getAttributeNs(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_getAttributeNs(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_9XMLReader_getAttributeNs((rv), (this_), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9XMLReader_getAttributeNs(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          th_9XMLReader_getAttributeNs((&(rv)), (this_), (Value*)(args-0), (Value*)(args-1));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_getAttributeNs(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::getAttributeNs", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::getAttributeNs");
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
bool HPHP::c_XMLReader::t_movetoattribute(HPHP::String const&)
_ZN4HPHP11c_XMLReader17t_movetoattributeERKNS_6StringE

(return value) => rax
this_ => rdi
name => rsi
*/

bool th_9XMLReader_moveToAttribute(ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLReader17t_movetoattributeERKNS_6StringE");

TypedValue* tg1_9XMLReader_moveToAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_moveToAttribute(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLReader_moveToAttribute((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_moveToAttribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_moveToAttribute((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_moveToAttribute(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::moveToAttribute", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::moveToAttribute");
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
bool HPHP::c_XMLReader::t_movetoattributeno(long long)
_ZN4HPHP11c_XMLReader19t_movetoattributenoEx

(return value) => rax
this_ => rdi
index => rsi
*/

bool th_9XMLReader_moveToAttributeNo(ObjectData* this_, long long index) asm("_ZN4HPHP11c_XMLReader19t_movetoattributenoEx");

TypedValue* tg1_9XMLReader_moveToAttributeNo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_moveToAttributeNo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (th_9XMLReader_moveToAttributeNo((this_), (long long)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_moveToAttributeNo(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_moveToAttributeNo((this_), (long long)(args[-0].m_data.num))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_moveToAttributeNo(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::moveToAttributeNo", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::moveToAttributeNo");
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
bool HPHP::c_XMLReader::t_movetoattributens(HPHP::String const&, HPHP::String const&)
_ZN4HPHP11c_XMLReader19t_movetoattributensERKNS_6StringES3_

(return value) => rax
this_ => rdi
name => rsi
namespaceURI => rdx
*/

bool th_9XMLReader_moveToAttributeNs(ObjectData* this_, Value* name, Value* namespaceURI) asm("_ZN4HPHP11c_XMLReader19t_movetoattributensERKNS_6StringES3_");

TypedValue* tg1_9XMLReader_moveToAttributeNs(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_moveToAttributeNs(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (th_9XMLReader_moveToAttributeNs((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_moveToAttributeNs(HPHP::VM::ActRec *ar) {
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
          rv.m_data.num = (th_9XMLReader_moveToAttributeNs((this_), (Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_moveToAttributeNs(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::moveToAttributeNs", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::moveToAttributeNs");
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
bool HPHP::c_XMLReader::t_movetoelement()
_ZN4HPHP11c_XMLReader15t_movetoelementEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLReader_moveToElement(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader15t_movetoelementEv");

TypedValue* tg_9XMLReader_moveToElement(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLReader_moveToElement((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::moveToElement", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::moveToElement");
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
bool HPHP::c_XMLReader::t_movetofirstattribute()
_ZN4HPHP11c_XMLReader22t_movetofirstattributeEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLReader_moveToFirstAttribute(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader22t_movetofirstattributeEv");

TypedValue* tg_9XMLReader_moveToFirstAttribute(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLReader_moveToFirstAttribute((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::moveToFirstAttribute", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::moveToFirstAttribute");
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
bool HPHP::c_XMLReader::t_isvalid()
_ZN4HPHP11c_XMLReader9t_isvalidEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLReader_isValid(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader9t_isvalidEv");

TypedValue* tg_9XMLReader_isValid(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLReader_isValid((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::isValid", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::isValid");
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
bool HPHP::c_XMLReader::t_expand()
_ZN4HPHP11c_XMLReader8t_expandEv

(return value) => rax
this_ => rdi
*/

bool th_9XMLReader_expand(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader8t_expandEv");

TypedValue* tg_9XMLReader_expand(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_9XMLReader_expand((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::expand", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::expand");
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
HPHP::Variant HPHP::c_XMLReader::t___get(HPHP::Variant)
_ZN4HPHP11c_XMLReader7t___getENS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
name => rdx
*/

TypedValue* th_9XMLReader___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP11c_XMLReader7t___getENS_7VariantE");

TypedValue* tg_9XMLReader___get(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_9XMLReader___get((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("XMLReader::__get", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::__get");
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
bool HPHP::c_XMLReader::t_getparserproperty(long long)
_ZN4HPHP11c_XMLReader19t_getparserpropertyEx

(return value) => rax
this_ => rdi
property => rsi
*/

bool th_9XMLReader_getParserProperty(ObjectData* this_, long long property) asm("_ZN4HPHP11c_XMLReader19t_getparserpropertyEx");

TypedValue* tg1_9XMLReader_getParserProperty(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_getParserProperty(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (th_9XMLReader_getParserProperty((this_), (long long)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_getParserProperty(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_getParserProperty((this_), (long long)(args[-0].m_data.num))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_getParserProperty(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::getParserProperty", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::getParserProperty");
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
HPHP::Variant HPHP::c_XMLReader::t_lookupnamespace(HPHP::String const&)
_ZN4HPHP11c_XMLReader17t_lookupnamespaceERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
prefix => rdx
*/

TypedValue* th_9XMLReader_lookupNamespace(TypedValue* _rv, ObjectData* this_, Value* prefix) asm("_ZN4HPHP11c_XMLReader17t_lookupnamespaceERKNS_6StringE");

TypedValue* tg1_9XMLReader_lookupNamespace(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_lookupNamespace(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_9XMLReader_lookupNamespace((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_9XMLReader_lookupNamespace(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          th_9XMLReader_lookupNamespace((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_lookupNamespace(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::lookupNamespace", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::lookupNamespace");
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
bool HPHP::c_XMLReader::t_setschema(HPHP::String const&)
_ZN4HPHP11c_XMLReader11t_setschemaERKNS_6StringE

(return value) => rax
this_ => rdi
source => rsi
*/

bool th_9XMLReader_setSchema(ObjectData* this_, Value* source) asm("_ZN4HPHP11c_XMLReader11t_setschemaERKNS_6StringE");

TypedValue* tg1_9XMLReader_setSchema(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_setSchema(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLReader_setSchema((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_setSchema(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_setSchema((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_setSchema(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::setSchema", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::setSchema");
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
bool HPHP::c_XMLReader::t_setparserproperty(long long, bool)
_ZN4HPHP11c_XMLReader19t_setparserpropertyExb

(return value) => rax
this_ => rdi
property => rsi
value => rdx
*/

bool th_9XMLReader_setParserProperty(ObjectData* this_, long long property, bool value) asm("_ZN4HPHP11c_XMLReader19t_setparserpropertyExb");

TypedValue* tg1_9XMLReader_setParserProperty(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_setParserProperty(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (th_9XMLReader_setParserProperty((this_), (long long)(args[-0].m_data.num), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_setParserProperty(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if ((args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_setParserProperty((this_), (long long)(args[-0].m_data.num), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_setParserProperty(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::setParserProperty", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::setParserProperty");
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
bool HPHP::c_XMLReader::t_setrelaxngschema(HPHP::String const&)
_ZN4HPHP11c_XMLReader18t_setrelaxngschemaERKNS_6StringE

(return value) => rax
this_ => rdi
filename => rsi
*/

bool th_9XMLReader_setRelaxNGSchema(ObjectData* this_, Value* filename) asm("_ZN4HPHP11c_XMLReader18t_setrelaxngschemaERKNS_6StringE");

TypedValue* tg1_9XMLReader_setRelaxNGSchema(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_setRelaxNGSchema(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLReader_setRelaxNGSchema((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_setRelaxNGSchema(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_setRelaxNGSchema((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_setRelaxNGSchema(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::setRelaxNGSchema", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::setRelaxNGSchema");
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
bool HPHP::c_XMLReader::t_setrelaxngschemasource(HPHP::String const&)
_ZN4HPHP11c_XMLReader24t_setrelaxngschemasourceERKNS_6StringE

(return value) => rax
this_ => rdi
source => rsi
*/

bool th_9XMLReader_setRelaxNGSchemaSource(ObjectData* this_, Value* source) asm("_ZN4HPHP11c_XMLReader24t_setrelaxngschemasourceERKNS_6StringE");

TypedValue* tg1_9XMLReader_setRelaxNGSchemaSource(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_9XMLReader_setRelaxNGSchemaSource(TypedValue* rv, HPHP::VM::ActRec* ar, long long count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (th_9XMLReader_setRelaxNGSchemaSource((this_), (Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_9XMLReader_setRelaxNGSchemaSource(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_9XMLReader_setRelaxNGSchemaSource((this_), (Value*)(args-0))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_9XMLReader_setRelaxNGSchemaSource(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("XMLReader::setRelaxNGSchemaSource", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::setRelaxNGSchemaSource");
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
HPHP::Variant HPHP::c_XMLReader::t___destruct()
_ZN4HPHP11c_XMLReader12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_9XMLReader___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP11c_XMLReader12t___destructEv");

TypedValue* tg_9XMLReader___destruct(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_9XMLReader___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("XMLReader::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("XMLReader::__destruct");
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

