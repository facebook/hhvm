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


HPHP::VM::Instance* new_XMLReader_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_XMLReader) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_XMLReader(cls);
  return inst;
}

IMPLEMENT_CLASS(XMLReader);
void th_9XMLReader___construct(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader13t___constructEv");

TypedValue* tg_9XMLReader___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_9XMLReader___construct((this_));
    } else {
      throw_toomany_arguments_nr("XMLReader::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_open(ObjectData* this_, Value* uri, Value* encoding, long options) asm("_ZN4HPHP11c_XMLReader6t_openERKNS_6StringES3_l");

void tg1_9XMLReader_open(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_open(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_open((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_open(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_open((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_open(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::open", count, 1, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::open");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_close(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader7t_closeEv");

TypedValue* tg_9XMLReader_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLReader_close((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLReader::close", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::close");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_XML(ObjectData* this_, Value* source, Value* encoding, long options) asm("_ZN4HPHP11c_XMLReader5t_xmlERKNS_6StringES3_l");

void tg1_9XMLReader_XML(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_XML(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_XML((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_XML(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 3) {
      if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_XML((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_XML(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::XML", count, 1, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::XML");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_read(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader6t_readEv");

TypedValue* tg_9XMLReader_read(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLReader_read((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLReader::read", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::read");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_next(ObjectData* this_, Value* localname) asm("_ZN4HPHP11c_XMLReader6t_nextERKNS_6StringE");

void tg1_9XMLReader_next(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_next(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_next((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_next(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_next((this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_next(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("XMLReader::next", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::next");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_9XMLReader_readString(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_XMLReader12t_readstringEv");

TypedValue* tg_9XMLReader_readString(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_9XMLReader_readString(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("XMLReader::readString", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::readString");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_9XMLReader_readInnerXML(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_XMLReader14t_readinnerxmlEv");

TypedValue* tg_9XMLReader_readInnerXML(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_9XMLReader_readInnerXML(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("XMLReader::readInnerXML", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::readInnerXML");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_9XMLReader_readOuterXML(Value* _rv, ObjectData* this_) asm("_ZN4HPHP11c_XMLReader14t_readouterxmlEv");

TypedValue* tg_9XMLReader_readOuterXML(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_9XMLReader_readOuterXML(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("XMLReader::readOuterXML", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::readOuterXML");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_9XMLReader_getAttribute(TypedValue* _rv, ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLReader14t_getattributeERKNS_6StringE");

void tg1_9XMLReader_getAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_getAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_9XMLReader_getAttribute(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_9XMLReader_getAttribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        th_9XMLReader_getAttribute(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_9XMLReader_getAttribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::getAttribute", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::getAttribute");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_9XMLReader_getAttributeNo(TypedValue* _rv, ObjectData* this_, long index) asm("_ZN4HPHP11c_XMLReader16t_getattributenoEl");

void tg1_9XMLReader_getAttributeNo(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_getAttributeNo(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  th_9XMLReader_getAttributeNo(rv, (this_), (long)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_9XMLReader_getAttributeNo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        th_9XMLReader_getAttributeNo(rv, (this_), (long)(args[-0].m_data.num));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_9XMLReader_getAttributeNo(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::getAttributeNo", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::getAttributeNo");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_9XMLReader_getAttributeNs(TypedValue* _rv, ObjectData* this_, Value* name, Value* namespaceURI) asm("_ZN4HPHP11c_XMLReader16t_getattributensERKNS_6StringES3_");

void tg1_9XMLReader_getAttributeNs(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_getAttributeNs(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_9XMLReader_getAttributeNs(rv, (this_), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_9XMLReader_getAttributeNs(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_9XMLReader_getAttributeNs(rv, (this_), &args[-0].m_data, &args[-1].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_9XMLReader_getAttributeNs(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::getAttributeNs", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::getAttributeNs");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_moveToAttribute(ObjectData* this_, Value* name) asm("_ZN4HPHP11c_XMLReader17t_movetoattributeERKNS_6StringE");

void tg1_9XMLReader_moveToAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_moveToAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_moveToAttribute((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_moveToAttribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_moveToAttribute((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_moveToAttribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::moveToAttribute", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::moveToAttribute");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_moveToAttributeNo(ObjectData* this_, long index) asm("_ZN4HPHP11c_XMLReader19t_movetoattributenoEl");

void tg1_9XMLReader_moveToAttributeNo(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_moveToAttributeNo(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_moveToAttributeNo((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_moveToAttributeNo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_moveToAttributeNo((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_moveToAttributeNo(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::moveToAttributeNo", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::moveToAttributeNo");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_moveToAttributeNs(ObjectData* this_, Value* name, Value* namespaceURI) asm("_ZN4HPHP11c_XMLReader19t_movetoattributensERKNS_6StringES3_");

void tg1_9XMLReader_moveToAttributeNs(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_moveToAttributeNs(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_moveToAttributeNs((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_moveToAttributeNs(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_moveToAttributeNs((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_moveToAttributeNs(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::moveToAttributeNs", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::moveToAttributeNs");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_moveToElement(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader15t_movetoelementEv");

TypedValue* tg_9XMLReader_moveToElement(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLReader_moveToElement((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLReader::moveToElement", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::moveToElement");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_moveToFirstAttribute(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader22t_movetofirstattributeEv");

TypedValue* tg_9XMLReader_moveToFirstAttribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLReader_moveToFirstAttribute((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLReader::moveToFirstAttribute", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::moveToFirstAttribute");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_moveToNextAttribute(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader21t_movetonextattributeEv");

TypedValue* tg_9XMLReader_moveToNextAttribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLReader_moveToNextAttribute((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLReader::moveToNextAttribute", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::moveToNextAttribute");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_isValid(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader9t_isvalidEv");

TypedValue* tg_9XMLReader_isValid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLReader_isValid((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLReader::isValid", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::isValid");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_expand(ObjectData* this_) asm("_ZN4HPHP11c_XMLReader8t_expandEv");

TypedValue* tg_9XMLReader_expand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_9XMLReader_expand((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("XMLReader::expand", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::expand");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_getParserProperty(ObjectData* this_, long property) asm("_ZN4HPHP11c_XMLReader19t_getparserpropertyEl");

void tg1_9XMLReader_getParserProperty(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_getParserProperty(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_getParserProperty((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_getParserProperty(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_getParserProperty((this_), (long)(args[-0].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_getParserProperty(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::getParserProperty", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::getParserProperty");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_9XMLReader_lookupNamespace(TypedValue* _rv, ObjectData* this_, Value* prefix) asm("_ZN4HPHP11c_XMLReader17t_lookupnamespaceERKNS_6StringE");

void tg1_9XMLReader_lookupNamespace(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_lookupNamespace(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_9XMLReader_lookupNamespace(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_9XMLReader_lookupNamespace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        th_9XMLReader_lookupNamespace(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_9XMLReader_lookupNamespace(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::lookupNamespace", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::lookupNamespace");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_setSchema(ObjectData* this_, Value* source) asm("_ZN4HPHP11c_XMLReader11t_setschemaERKNS_6StringE");

void tg1_9XMLReader_setSchema(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_setSchema(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_setSchema((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_setSchema(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_setSchema((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_setSchema(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::setSchema", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::setSchema");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_setParserProperty(ObjectData* this_, long property, bool value) asm("_ZN4HPHP11c_XMLReader19t_setparserpropertyElb");

void tg1_9XMLReader_setParserProperty(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_setParserProperty(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_setParserProperty((this_), (long)(args[-0].m_data.num), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_setParserProperty(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if ((args - 1)->m_type == KindOfBoolean &&
          (args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_setParserProperty((this_), (long)(args[-0].m_data.num), (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_setParserProperty(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::setParserProperty", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::setParserProperty");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_setRelaxNGSchema(ObjectData* this_, Value* filename) asm("_ZN4HPHP11c_XMLReader18t_setrelaxngschemaERKNS_6StringE");

void tg1_9XMLReader_setRelaxNGSchema(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_setRelaxNGSchema(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_setRelaxNGSchema((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_setRelaxNGSchema(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_setRelaxNGSchema((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_setRelaxNGSchema(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::setRelaxNGSchema", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::setRelaxNGSchema");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_9XMLReader_setRelaxNGSchemaSource(ObjectData* this_, Value* source) asm("_ZN4HPHP11c_XMLReader24t_setrelaxngschemasourceERKNS_6StringE");

void tg1_9XMLReader_setRelaxNGSchemaSource(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_9XMLReader_setRelaxNGSchemaSource(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_9XMLReader_setRelaxNGSchemaSource((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_9XMLReader_setRelaxNGSchemaSource(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_9XMLReader_setRelaxNGSchemaSource((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_9XMLReader_setRelaxNGSchemaSource(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("XMLReader::setRelaxNGSchemaSource", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::setRelaxNGSchemaSource");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_9XMLReader___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP11c_XMLReader7t___getENS_7VariantE");

TypedValue* tg_9XMLReader___get(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      th_9XMLReader___get(rv, (this_), (args-0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("XMLReader::__get", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("XMLReader::__get");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
