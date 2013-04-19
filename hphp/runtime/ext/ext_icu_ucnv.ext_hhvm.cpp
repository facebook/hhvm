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


HPHP::VM::Instance* new_UConverter_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_UConverter) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_UConverter(cls);
  return inst;
}

IMPLEMENT_CLASS(UConverter);
void th_10UConverter___construct(ObjectData* this_, Value* toEncoding, Value* fromEncoding) asm("_ZN4HPHP12c_UConverter13t___constructERKNS_6StringES3_");

void tg1_10UConverter___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10UConverter___construct(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  rv->m_type = KindOfNull;
  String defVal0 = "utf-8";
  String defVal1 = "utf-8";
  th_10UConverter___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
}

TypedValue* tg_10UConverter___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count <= 2) {
      if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
          (count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
        rv->m_type = KindOfNull;
        String defVal0 = "utf-8";
        String defVal1 = "utf-8";
        th_10UConverter___construct((this_), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0), (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
      } else {
        tg1_10UConverter___construct(rv, ar, count, this_);
      }
    } else {
      throw_toomany_arguments_nr("UConverter::__construct", 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::__construct");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10UConverter___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter12t___destructEv");

TypedValue* tg_10UConverter___destruct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_10UConverter___destruct(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("UConverter::__destruct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::__destruct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10UConverter_setSourceEncoding(ObjectData* this_, Value* encoding) asm("_ZN4HPHP12c_UConverter19t_setsourceencodingERKNS_6StringE");

void tg1_10UConverter_setSourceEncoding(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10UConverter_setSourceEncoding(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_10UConverter_setSourceEncoding((this_), &args[-0].m_data);
}

TypedValue* tg_10UConverter_setSourceEncoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_10UConverter_setSourceEncoding((this_), &args[-0].m_data);
      } else {
        tg1_10UConverter_setSourceEncoding(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("UConverter::setSourceEncoding", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::setSourceEncoding");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_10UConverter_setDestinationEncoding(ObjectData* this_, Value* encoding) asm("_ZN4HPHP12c_UConverter24t_setdestinationencodingERKNS_6StringE");

void tg1_10UConverter_setDestinationEncoding(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10UConverter_setDestinationEncoding(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_10UConverter_setDestinationEncoding((this_), &args[-0].m_data);
}

TypedValue* tg_10UConverter_setDestinationEncoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_10UConverter_setDestinationEncoding((this_), &args[-0].m_data);
      } else {
        tg1_10UConverter_setDestinationEncoding(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("UConverter::setDestinationEncoding", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::setDestinationEncoding");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getSourceEncoding(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter19t_getsourceencodingEv");

TypedValue* tg_10UConverter_getSourceEncoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_10UConverter_getSourceEncoding(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("UConverter::getSourceEncoding", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::getSourceEncoding");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getDestinationEncoding(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter24t_getdestinationencodingEv");

TypedValue* tg_10UConverter_getDestinationEncoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_10UConverter_getDestinationEncoding(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("UConverter::getDestinationEncoding", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::getDestinationEncoding");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_10UConverter_getSourceType(ObjectData* this_) asm("_ZN4HPHP12c_UConverter15t_getsourcetypeEv");

TypedValue* tg_10UConverter_getSourceType(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_10UConverter_getSourceType((this_));
    } else {
      throw_toomany_arguments_nr("UConverter::getSourceType", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::getSourceType");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_10UConverter_getDestinationType(ObjectData* this_) asm("_ZN4HPHP12c_UConverter20t_getdestinationtypeEv");

TypedValue* tg_10UConverter_getDestinationType(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_10UConverter_getDestinationType((this_));
    } else {
      throw_toomany_arguments_nr("UConverter::getDestinationType", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::getDestinationType");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_10UConverter_setSubstChars(ObjectData* this_, Value* chars) asm("_ZN4HPHP12c_UConverter15t_setsubstcharsERKNS_6StringE");

void tg1_10UConverter_setSubstChars(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10UConverter_setSubstChars(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_10UConverter_setSubstChars((this_), &args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_10UConverter_setSubstChars(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_10UConverter_setSubstChars((this_), &args[-0].m_data)) ? 1LL : 0LL;
      } else {
        tg1_10UConverter_setSubstChars(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("UConverter::setSubstChars", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::setSubstChars");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getSubstChars(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter15t_getsubstcharsEv");

TypedValue* tg_10UConverter_getSubstChars(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_10UConverter_getSubstChars(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("UConverter::getSubstChars", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::getSubstChars");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10UConverter_fromUCallback(TypedValue* _rv, ObjectData* this_, long reason, Value* source, long codepoint, TypedValue* error) asm("_ZN4HPHP12c_UConverter15t_fromucallbackElRKNS_5ArrayElRKNS_14VRefParamValueE");

void tg1_10UConverter_fromUCallback(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10UConverter_fromUCallback(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_10UConverter_fromUCallback(rv, (this_), (long)(args[-0].m_data.num), &args[-1].m_data, (long)(args[-2].m_data.num), (args-3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10UConverter_fromUCallback(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 4) {
      if ((args - 2)->m_type == KindOfInt64 &&
          (args - 1)->m_type == KindOfArray &&
          (args - 0)->m_type == KindOfInt64) {
        th_10UConverter_fromUCallback(rv, (this_), (long)(args[-0].m_data.num), &args[-1].m_data, (long)(args[-2].m_data.num), (args-3));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_10UConverter_fromUCallback(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("UConverter::fromUCallback", count, 4, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::fromUCallback");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10UConverter_toUCallback(TypedValue* _rv, ObjectData* this_, long reason, Value* source, Value* codeunits, TypedValue* error) asm("_ZN4HPHP12c_UConverter13t_toucallbackElRKNS_6StringES3_RKNS_14VRefParamValueE");

void tg1_10UConverter_toUCallback(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10UConverter_toUCallback(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  th_10UConverter_toUCallback(rv, (this_), (long)(args[-0].m_data.num), &args[-1].m_data, &args[-2].m_data, (args-3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10UConverter_toUCallback(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 4) {
      if (IS_STRING_TYPE((args - 2)->m_type) &&
          IS_STRING_TYPE((args - 1)->m_type) &&
          (args - 0)->m_type == KindOfInt64) {
        th_10UConverter_toUCallback(rv, (this_), (long)(args[-0].m_data.num), &args[-1].m_data, &args[-2].m_data, (args-3));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_10UConverter_toUCallback(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("UConverter::toUCallback", count, 4, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::toUCallback");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10UConverter_convert(TypedValue* _rv, ObjectData* this_, Value* str, bool reverse) asm("_ZN4HPHP12c_UConverter9t_convertERKNS_6StringEb");

void tg1_10UConverter_convert(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_10UConverter_convert(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_10UConverter_convert(rv, (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10UConverter_convert(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 2) {
      if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        th_10UConverter_convert(rv, (this_), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_10UConverter_convert(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("UConverter::convert", count, 1, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::convert");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_10UConverter_transcode(TypedValue* _rv, char const* cls_, Value* str, Value* toEncoding, Value* fromEncoding, Value* options) asm("_ZN4HPHP12c_UConverter12ti_transcodeEPKcRKNS_6StringES5_S5_RKNS_5ArrayE");

void tg1_10UConverter_transcode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_10UConverter_transcode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  Array defVal3 = null_variant;
  th_10UConverter_transcode(rv, "UConverter", &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_10UConverter_transcode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfArray) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      Array defVal3 = null_variant;
      th_10UConverter_transcode(rv, "UConverter", &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      tg1_10UConverter_transcode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("UConverter::transcode", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_10UConverter_getErrorCode(ObjectData* this_) asm("_ZN4HPHP12c_UConverter14t_geterrorcodeEv");

TypedValue* tg_10UConverter_getErrorCode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_10UConverter_getErrorCode((this_));
    } else {
      throw_toomany_arguments_nr("UConverter::getErrorCode", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::getErrorCode");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getErrorMessage(Value* _rv, ObjectData* this_) asm("_ZN4HPHP12c_UConverter17t_geterrormessageEv");

TypedValue* tg_10UConverter_getErrorMessage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_10UConverter_getErrorMessage(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("UConverter::getErrorMessage", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("UConverter::getErrorMessage");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_reasonText(Value* _rv, char const* cls_, long reason) asm("_ZN4HPHP12c_UConverter13ti_reasontextEPKcl");

void tg1_10UConverter_reasonText(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_10UConverter_reasonText(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  th_10UConverter_reasonText(&(rv->m_data), "UConverter", (long)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_10UConverter_reasonText(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      th_10UConverter_reasonText(&(rv->m_data), "UConverter", (long)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      tg1_10UConverter_reasonText(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("UConverter::reasonText", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getAvailable(Value* _rv, char const* cls_) asm("_ZN4HPHP12c_UConverter15ti_getavailableEPKc");

TypedValue* tg_10UConverter_getAvailable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 0) {
    rv->m_type = KindOfArray;
    th_10UConverter_getAvailable(&(rv->m_data), "UConverter");
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("UConverter::getAvailable", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getAliases(Value* _rv, char const* cls_, Value* encoding) asm("_ZN4HPHP12c_UConverter13ti_getaliasesEPKcRKNS_6StringE");

void tg1_10UConverter_getAliases(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_10UConverter_getAliases(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  th_10UConverter_getAliases(&(rv->m_data), "UConverter", &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_10UConverter_getAliases(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      th_10UConverter_getAliases(&(rv->m_data), "UConverter", &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      tg1_10UConverter_getAliases(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("UConverter::getAliases", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getStandards(Value* _rv, char const* cls_) asm("_ZN4HPHP12c_UConverter15ti_getstandardsEPKc");

TypedValue* tg_10UConverter_getStandards(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 0) {
    rv->m_type = KindOfArray;
    th_10UConverter_getStandards(&(rv->m_data), "UConverter");
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("UConverter::getStandards", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getStandardName(Value* _rv, char const* cls_, Value* name, Value* standard) asm("_ZN4HPHP12c_UConverter18ti_getstandardnameEPKcRKNS_6StringES5_");

void tg1_10UConverter_getStandardName(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_10UConverter_getStandardName(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  th_10UConverter_getStandardName(&(rv->m_data), "UConverter", &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_10UConverter_getStandardName(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      th_10UConverter_getStandardName(&(rv->m_data), "UConverter", &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      tg1_10UConverter_getStandardName(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("UConverter::getStandardName", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_10UConverter_getMIMEName(Value* _rv, char const* cls_, Value* name) asm("_ZN4HPHP12c_UConverter14ti_getmimenameEPKcRKNS_6StringE");

void tg1_10UConverter_getMIMEName(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void tg1_10UConverter_getMIMEName(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  th_10UConverter_getMIMEName(&(rv->m_data), "UConverter", &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_10UConverter_getMIMEName(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      th_10UConverter_getMIMEName(&(rv->m_data), "UConverter", &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      tg1_10UConverter_getMIMEName(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("UConverter::getMIMEName", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
