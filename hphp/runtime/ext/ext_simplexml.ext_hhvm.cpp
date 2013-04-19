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

TypedValue* fh_simplexml_load_string(TypedValue* _rv, Value* data, Value* class_name, long options, Value* ns, bool is_prefix) asm("_ZN4HPHP23f_simplexml_load_stringERKNS_6StringES2_lS2_b");

void fg1_simplexml_load_string(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_simplexml_load_string(TypedValue* rv, ActRec* ar, int32_t count) {
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
  String defVal3 = "";
  fh_simplexml_load_string(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_simplexml_load_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfBoolean) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      String defVal1 = "SimpleXMLElement";
      String defVal3 = "";
      fh_simplexml_load_string(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_simplexml_load_string(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("simplexml_load_string", count, 1, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_simplexml_load_file(TypedValue* _rv, Value* filename, Value* class_name, long options, Value* ns, bool is_prefix) asm("_ZN4HPHP21f_simplexml_load_fileERKNS_6StringES2_lS2_b");

void fg1_simplexml_load_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_simplexml_load_file(TypedValue* rv, ActRec* ar, int32_t count) {
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
  String defVal3 = "";
  fh_simplexml_load_file(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_simplexml_load_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfBoolean) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      String defVal1 = "SimpleXMLElement";
      String defVal3 = "";
      fh_simplexml_load_file(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_simplexml_load_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("simplexml_load_file", count, 1, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_SimpleXMLElement_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SimpleXMLElement) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SimpleXMLElement(cls);
  return inst;
}

IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(SimpleXMLElement);
void th_16SimpleXMLElement___construct(ObjectData* this_, Value* data, long options, bool data_is_url, Value* ns, bool is_prefix) asm("_ZN4HPHP18c_SimpleXMLElement13t___constructERKNS_6StringElbS3_b");

void tg1_16SimpleXMLElement___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  rv->m_type = KindOfNull;
  String defVal3 = "";
  th_16SimpleXMLElement___construct((this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
}

TypedValue* tg_16SimpleXMLElement___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 5) {
      if ((count <= 4 || (args - 4)->m_type == KindOfBoolean) &&
          (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
          (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        String defVal3 = "";
        th_16SimpleXMLElement___construct((this_), &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false));
      } else {
        tg1_16SimpleXMLElement___construct(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::__construct", count, 1, 5, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::__construct");
  }
  frame_free_locals_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_16SimpleXMLElement_xpath(TypedValue* _rv, ObjectData* this_, Value* path) asm("_ZN4HPHP18c_SimpleXMLElement7t_xpathERKNS_6StringE");

void tg1_16SimpleXMLElement_xpath(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_xpath(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  th_16SimpleXMLElement_xpath(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_16SimpleXMLElement_xpath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        th_16SimpleXMLElement_xpath(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_16SimpleXMLElement_xpath(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::xpath", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::xpath");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_16SimpleXMLElement_registerXPathNamespace(ObjectData* this_, Value* prefix, Value* ns) asm("_ZN4HPHP18c_SimpleXMLElement24t_registerxpathnamespaceERKNS_6StringES3_");

void tg1_16SimpleXMLElement_registerXPathNamespace(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_registerXPathNamespace(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_16SimpleXMLElement_registerXPathNamespace((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_16SimpleXMLElement_registerXPathNamespace(ActRec* ar) {
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
        rv->m_data.num = (th_16SimpleXMLElement_registerXPathNamespace((this_), &args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
      } else {
        tg1_16SimpleXMLElement_registerXPathNamespace(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::registerXPathNamespace", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::registerXPathNamespace");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_16SimpleXMLElement_asXML(TypedValue* _rv, ObjectData* this_, Value* filename) asm("_ZN4HPHP18c_SimpleXMLElement7t_asxmlERKNS_6StringE");

void tg1_16SimpleXMLElement_asXML(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_asXML(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  String defVal0 = "";
  th_16SimpleXMLElement_asXML(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_16SimpleXMLElement_asXML(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        String defVal0 = "";
        th_16SimpleXMLElement_asXML(rv, (this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_16SimpleXMLElement_asXML(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::asXML", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::asXML");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_16SimpleXMLElement_getNamespaces(Value* _rv, ObjectData* this_, bool recursive) asm("_ZN4HPHP18c_SimpleXMLElement15t_getnamespacesEb");

void tg1_16SimpleXMLElement_getNamespaces(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_getNamespaces(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfArray;
  th_16SimpleXMLElement_getNamespaces(&(rv->m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_16SimpleXMLElement_getNamespaces(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
        rv->m_type = KindOfArray;
        th_16SimpleXMLElement_getNamespaces(&(rv->m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_16SimpleXMLElement_getNamespaces(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::getNamespaces", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::getNamespaces");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_16SimpleXMLElement_getDocNamespaces(Value* _rv, ObjectData* this_, bool recursive) asm("_ZN4HPHP18c_SimpleXMLElement18t_getdocnamespacesEb");

void tg1_16SimpleXMLElement_getDocNamespaces(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_getDocNamespaces(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfArray;
  th_16SimpleXMLElement_getDocNamespaces(&(rv->m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_16SimpleXMLElement_getDocNamespaces(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 1) {
      if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
        rv->m_type = KindOfArray;
        th_16SimpleXMLElement_getDocNamespaces(&(rv->m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_16SimpleXMLElement_getDocNamespaces(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::getDocNamespaces", 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::getDocNamespaces");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_16SimpleXMLElement_children(Value* _rv, ObjectData* this_, Value* ns, bool is_prefix) asm("_ZN4HPHP18c_SimpleXMLElement10t_childrenERKNS_6StringEb");

void tg1_16SimpleXMLElement_children(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_children(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfObject;
  String defVal0 = "";
  th_16SimpleXMLElement_children(&(rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_16SimpleXMLElement_children(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
          (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        rv->m_type = KindOfObject;
        String defVal0 = "";
        th_16SimpleXMLElement_children(&(rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_16SimpleXMLElement_children(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::children", 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::children");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_16SimpleXMLElement_getName(Value* _rv, ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement9t_getnameEv");

TypedValue* tg_16SimpleXMLElement_getName(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_16SimpleXMLElement_getName(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::getName", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::getName");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_16SimpleXMLElement_attributes(Value* _rv, ObjectData* this_, Value* ns, bool is_prefix) asm("_ZN4HPHP18c_SimpleXMLElement12t_attributesERKNS_6StringEb");

void tg1_16SimpleXMLElement_attributes(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_attributes(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfObject;
  String defVal0 = "";
  th_16SimpleXMLElement_attributes(&(rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_16SimpleXMLElement_attributes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
          (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        rv->m_type = KindOfObject;
        String defVal0 = "";
        th_16SimpleXMLElement_attributes(&(rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_16SimpleXMLElement_attributes(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::attributes", 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::attributes");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_16SimpleXMLElement_addChild(TypedValue* _rv, ObjectData* this_, Value* qname, Value* value, Value* ns) asm("_ZN4HPHP18c_SimpleXMLElement10t_addchildERKNS_6StringES3_S3_");

void tg1_16SimpleXMLElement_addChild(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_addChild(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  th_16SimpleXMLElement_addChild(rv, (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_16SimpleXMLElement_addChild(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 3) {
      if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_16SimpleXMLElement_addChild(rv, (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_16SimpleXMLElement_addChild(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::addChild", count, 1, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::addChild");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_16SimpleXMLElement_addAttribute(ObjectData* this_, Value* qname, Value* value, Value* ns) asm("_ZN4HPHP18c_SimpleXMLElement14t_addattributeERKNS_6StringES3_S3_");

void tg1_16SimpleXMLElement_addAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_16SimpleXMLElement_addAttribute(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
  rv->m_type = KindOfNull;
  th_16SimpleXMLElement_addAttribute((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
}

TypedValue* tg_16SimpleXMLElement_addAttribute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 3) {
      if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
          (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_16SimpleXMLElement_addAttribute((this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      } else {
        tg1_16SimpleXMLElement_addAttribute(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::addAttribute", count, 1, 3, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::addAttribute");
  }
  frame_free_locals_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_16SimpleXMLElement___toString(Value* _rv, ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement12t___tostringEv");

TypedValue* tg_16SimpleXMLElement___toString(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_16SimpleXMLElement___toString(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::__toString", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::__toString");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_16SimpleXMLElement___get(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP18c_SimpleXMLElement7t___getENS_7VariantE");

TypedValue* tg_16SimpleXMLElement___get(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      th_16SimpleXMLElement___get(rv, (this_), (args-0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::__get", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::__get");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_16SimpleXMLElement___unset(TypedValue* _rv, ObjectData* this_, TypedValue* name) asm("_ZN4HPHP18c_SimpleXMLElement9t___unsetENS_7VariantE");

TypedValue* tg_16SimpleXMLElement___unset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      th_16SimpleXMLElement___unset(rv, (this_), (args-0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::__unset", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::__unset");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_16SimpleXMLElement___isset(ObjectData* this_, TypedValue* name) asm("_ZN4HPHP18c_SimpleXMLElement9t___issetENS_7VariantE");

TypedValue* tg_16SimpleXMLElement___isset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_16SimpleXMLElement___isset((this_), (args-0))) ? 1LL : 0LL;
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::__isset", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::__isset");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_16SimpleXMLElement___set(TypedValue* _rv, ObjectData* this_, TypedValue* name, TypedValue* value) asm("_ZN4HPHP18c_SimpleXMLElement7t___setENS_7VariantES1_");

TypedValue* tg_16SimpleXMLElement___set(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      th_16SimpleXMLElement___set(rv, (this_), (args-0), (args-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::__set", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::__set");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_16SimpleXMLElement_getIterator(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement13t_getiteratorEv");

TypedValue* tg_16SimpleXMLElement_getIterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_16SimpleXMLElement_getIterator(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::getIterator", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::getIterator");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_16SimpleXMLElement_count(ObjectData* this_) asm("_ZN4HPHP18c_SimpleXMLElement7t_countEv");

TypedValue* tg_16SimpleXMLElement_count(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_16SimpleXMLElement_count((this_));
    } else {
      throw_toomany_arguments_nr("SimpleXMLElement::count", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::count");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_16SimpleXMLElement_offsetExists(ObjectData* this_, TypedValue* index) asm("_ZN4HPHP18c_SimpleXMLElement14t_offsetexistsERKNS_7VariantE");

TypedValue* tg_16SimpleXMLElement_offsetExists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_16SimpleXMLElement_offsetExists((this_), (args-0))) ? 1LL : 0LL;
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::offsetExists", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::offsetExists");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_16SimpleXMLElement_offsetGet(TypedValue* _rv, ObjectData* this_, TypedValue* index) asm("_ZN4HPHP18c_SimpleXMLElement11t_offsetgetERKNS_7VariantE");

TypedValue* tg_16SimpleXMLElement_offsetGet(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      th_16SimpleXMLElement_offsetGet(rv, (this_), (args-0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::offsetGet", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::offsetGet");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_16SimpleXMLElement_offsetSet(ObjectData* this_, TypedValue* index, TypedValue* newvalue) asm("_ZN4HPHP18c_SimpleXMLElement11t_offsetsetERKNS_7VariantES3_");

TypedValue* tg_16SimpleXMLElement_offsetSet(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      rv->m_type = KindOfNull;
      th_16SimpleXMLElement_offsetSet((this_), (args-0), (args-1));
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::offsetSet", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::offsetSet");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_16SimpleXMLElement_offsetUnset(ObjectData* this_, TypedValue* index) asm("_ZN4HPHP18c_SimpleXMLElement13t_offsetunsetERKNS_7VariantE");

TypedValue* tg_16SimpleXMLElement_offsetUnset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      rv->m_type = KindOfNull;
      th_16SimpleXMLElement_offsetUnset((this_), (args-0));
    } else {
      throw_wrong_arguments_nr("SimpleXMLElement::offsetUnset", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElement::offsetUnset");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_SimpleXMLElementIterator_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_SimpleXMLElementIterator) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_SimpleXMLElementIterator(cls);
  return inst;
}

IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(SimpleXMLElementIterator);
void th_24SimpleXMLElementIterator___construct(ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator13t___constructEv");

TypedValue* tg_24SimpleXMLElementIterator___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_24SimpleXMLElementIterator___construct((this_));
    } else {
      throw_toomany_arguments_nr("SimpleXMLElementIterator::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElementIterator::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_24SimpleXMLElementIterator_current(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator9t_currentEv");

TypedValue* tg_24SimpleXMLElementIterator_current(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_24SimpleXMLElementIterator_current(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SimpleXMLElementIterator::current", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElementIterator::current");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_24SimpleXMLElementIterator_key(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator5t_keyEv");

TypedValue* tg_24SimpleXMLElementIterator_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_24SimpleXMLElementIterator_key(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SimpleXMLElementIterator::key", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElementIterator::key");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_24SimpleXMLElementIterator_next(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator6t_nextEv");

TypedValue* tg_24SimpleXMLElementIterator_next(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_24SimpleXMLElementIterator_next(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SimpleXMLElementIterator::next", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElementIterator::next");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_24SimpleXMLElementIterator_rewind(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator8t_rewindEv");

TypedValue* tg_24SimpleXMLElementIterator_rewind(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_24SimpleXMLElementIterator_rewind(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SimpleXMLElementIterator::rewind", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElementIterator::rewind");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_24SimpleXMLElementIterator_valid(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP26c_SimpleXMLElementIterator7t_validEv");

TypedValue* tg_24SimpleXMLElementIterator_valid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_24SimpleXMLElementIterator_valid(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("SimpleXMLElementIterator::valid", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("SimpleXMLElementIterator::valid");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}


HPHP::VM::Instance* new_LibXMLError_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_LibXMLError) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_LibXMLError(cls);
  return inst;
}

IMPLEMENT_CLASS(LibXMLError);
void th_11LibXMLError___construct(ObjectData* this_) asm("_ZN4HPHP13c_LibXMLError13t___constructEv");

TypedValue* tg_11LibXMLError___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_11LibXMLError___construct((this_));
    } else {
      throw_toomany_arguments_nr("LibXMLError::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("LibXMLError::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_libxml_get_errors(TypedValue* _rv) asm("_ZN4HPHP19f_libxml_get_errorsEv");

TypedValue* fg_libxml_get_errors(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_libxml_get_errors(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("libxml_get_errors", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_libxml_get_last_error(TypedValue* _rv) asm("_ZN4HPHP23f_libxml_get_last_errorEv");

TypedValue* fg_libxml_get_last_error(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_libxml_get_last_error(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("libxml_get_last_error", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_libxml_clear_errors() asm("_ZN4HPHP21f_libxml_clear_errorsEv");

TypedValue* fg_libxml_clear_errors(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_libxml_clear_errors();
  } else {
    throw_toomany_arguments_nr("libxml_clear_errors", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_libxml_use_internal_errors(TypedValue* use_errors) asm("_ZN4HPHP28f_libxml_use_internal_errorsERKNS_7VariantE");

TypedValue* fg_libxml_use_internal_errors(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_libxml_use_internal_errors((count > 0) ? (args-0) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("libxml_use_internal_errors", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_libxml_set_streams_context(Value* streams_context) asm("_ZN4HPHP28f_libxml_set_streams_contextERKNS_6ObjectE");

void fg1_libxml_set_streams_context(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_libxml_set_streams_context(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_libxml_set_streams_context(&args[-0].m_data);
}

TypedValue* fg_libxml_set_streams_context(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_libxml_set_streams_context(&args[-0].m_data);
    } else {
      fg1_libxml_set_streams_context(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("libxml_set_streams_context", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_libxml_disable_entity_loader(bool disable) asm("_ZN4HPHP30f_libxml_disable_entity_loaderEb");

void fg1_libxml_disable_entity_loader(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_libxml_disable_entity_loader(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_libxml_disable_entity_loader((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_libxml_disable_entity_loader(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_libxml_disable_entity_loader((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_libxml_disable_entity_loader(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("libxml_disable_entity_loader", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
