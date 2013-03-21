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

HPHP::VM::Instance* new_ImageSprite_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_ImageSprite) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_ImageSprite(cls);
  return inst;
}

IMPLEMENT_CLASS(ImageSprite);
/*
void HPHP::c_ImageSprite::t___construct()
_ZN4HPHP13c_ImageSprite13t___constructEv

this_ => rdi
*/

void th_11ImageSprite___construct(ObjectData* this_) asm("_ZN4HPHP13c_ImageSprite13t___constructEv");

TypedValue* tg_11ImageSprite___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        th_11ImageSprite___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("ImageSprite::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::__construct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_ImageSprite::t_addfile(HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13c_ImageSprite9t_addfileERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
this_ => rsi
file => rdx
options => rcx
*/

Value* th_11ImageSprite_addFile(Value* _rv, ObjectData* this_, Value* file, Value* options) asm("_ZN4HPHP13c_ImageSprite9t_addfileERKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_11ImageSprite_addFile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11ImageSprite_addFile(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Array defVal1 = uninit_null();
  th_11ImageSprite_addFile((&rv->m_data), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11ImageSprite_addFile(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 2LL) {
        if ((count <= 1 || (args-1)->m_type == KindOfArray) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfObject;
          Array defVal1 = uninit_null();
          th_11ImageSprite_addFile((&rv.m_data), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11ImageSprite_addFile(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("ImageSprite::addFile", count, 1, 2, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::addFile");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_ImageSprite::t_addstring(HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13c_ImageSprite11t_addstringERKNS_6StringES3_RKNS_5ArrayE

(return value) => rax
_rv => rdi
this_ => rsi
id => rdx
data => rcx
options => r8
*/

Value* th_11ImageSprite_addString(Value* _rv, ObjectData* this_, Value* id, Value* data, Value* options) asm("_ZN4HPHP13c_ImageSprite11t_addstringERKNS_6StringES3_RKNS_5ArrayE");

TypedValue* tg1_11ImageSprite_addString(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11ImageSprite_addString(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-2);
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
  Array defVal2 = uninit_null();
  th_11ImageSprite_addString((&rv->m_data), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&defVal2));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11ImageSprite_addString(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfArray) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfObject;
          Array defVal2 = uninit_null();
          th_11ImageSprite_addString((&rv.m_data), (this_), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&defVal2));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11ImageSprite_addString(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("ImageSprite::addString", count, 2, 3, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::addString");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_ImageSprite::t_addurl(HPHP::String const&, int, HPHP::Array const&)
_ZN4HPHP13c_ImageSprite8t_addurlERKNS_6StringEiRKNS_5ArrayE

(return value) => rax
_rv => rdi
this_ => rsi
url => rdx
timeout_ms => rcx
Options => r8
*/

Value* th_11ImageSprite_addUrl(Value* _rv, ObjectData* this_, Value* url, int timeout_ms, Value* Options) asm("_ZN4HPHP13c_ImageSprite8t_addurlERKNS_6StringEiRKNS_5ArrayE");

TypedValue* tg1_11ImageSprite_addUrl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11ImageSprite_addUrl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-2);
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
  Array defVal2 = uninit_null();
  th_11ImageSprite_addUrl((&rv->m_data), (this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? &args[-2].m_data : (Value*)(&defVal2));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11ImageSprite_addUrl(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfArray) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfObject;
          Array defVal2 = uninit_null();
          th_11ImageSprite_addUrl((&rv.m_data), (this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? &args[-2].m_data : (Value*)(&defVal2));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11ImageSprite_addUrl(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("ImageSprite::addUrl", count, 1, 3, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::addUrl");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_ImageSprite::t_clear(HPHP::Variant const&)
_ZN4HPHP13c_ImageSprite7t_clearERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
paths => rdx
*/

Value* th_11ImageSprite_clear(Value* _rv, ObjectData* this_, TypedValue* paths) asm("_ZN4HPHP13c_ImageSprite7t_clearERKNS_7VariantE");

TypedValue* tg_11ImageSprite_clear(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        rv.m_type = KindOfObject;
        Variant defVal0;
        th_11ImageSprite_clear((&rv.m_data), (this_), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("ImageSprite::clear", 1, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::clear");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_ImageSprite::t_loaddims(bool)
_ZN4HPHP13c_ImageSprite10t_loaddimsEb

(return value) => rax
_rv => rdi
this_ => rsi
block => rdx
*/

Value* th_11ImageSprite_loadDims(Value* _rv, ObjectData* this_, bool block) asm("_ZN4HPHP13c_ImageSprite10t_loaddimsEb");

TypedValue* tg1_11ImageSprite_loadDims(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11ImageSprite_loadDims(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToBooleanInPlace(args-0);
  th_11ImageSprite_loadDims((&rv->m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11ImageSprite_loadDims(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
          rv.m_type = KindOfObject;
          th_11ImageSprite_loadDims((&rv.m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11ImageSprite_loadDims(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("ImageSprite::loadDims", 1, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::loadDims");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Object HPHP::c_ImageSprite::t_loadimages(bool)
_ZN4HPHP13c_ImageSprite12t_loadimagesEb

(return value) => rax
_rv => rdi
this_ => rsi
block => rdx
*/

Value* th_11ImageSprite_loadImages(Value* _rv, ObjectData* this_, bool block) asm("_ZN4HPHP13c_ImageSprite12t_loadimagesEb");

TypedValue* tg1_11ImageSprite_loadImages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11ImageSprite_loadImages(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToBooleanInPlace(args-0);
  th_11ImageSprite_loadImages((&rv->m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11ImageSprite_loadImages(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 1LL) {
        if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
          rv.m_type = KindOfObject;
          th_11ImageSprite_loadImages((&rv.m_data), (this_), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11ImageSprite_loadImages(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("ImageSprite::loadImages", 1, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::loadImages");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_ImageSprite::t_output(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP13c_ImageSprite8t_outputERKNS_6StringES3_i

(return value) => rax
_rv => rdi
this_ => rsi
output_file => rdx
format => rcx
quality => r8
*/

Value* th_11ImageSprite_output(Value* _rv, ObjectData* this_, Value* output_file, Value* format, int quality) asm("_ZN4HPHP13c_ImageSprite8t_outputERKNS_6StringES3_i");

TypedValue* tg1_11ImageSprite_output(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11ImageSprite_output(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
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
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  String defVal1 = "png";
  th_11ImageSprite_output((&rv->m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(75));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11ImageSprite_output(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count <= 3LL) {
        if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
          rv.m_type = KindOfString;
          String defVal1 = "png";
          th_11ImageSprite_output((&rv.m_data), (this_), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(75));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11ImageSprite_output(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 3);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_toomany_arguments_nr("ImageSprite::output", 3, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::output");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_ImageSprite::t_css(HPHP::String const&, HPHP::String const&, HPHP::String const&, bool)
_ZN4HPHP13c_ImageSprite5t_cssERKNS_6StringES3_S3_b

(return value) => rax
_rv => rdi
this_ => rsi
css_namespace => rdx
sprite_file => rcx
output_file => r8
verbose => r9
*/

Value* th_11ImageSprite_css(Value* _rv, ObjectData* this_, Value* css_namespace, Value* sprite_file, Value* output_file, bool verbose) asm("_ZN4HPHP13c_ImageSprite5t_cssERKNS_6StringES3_S3_b");

TypedValue* tg1_11ImageSprite_css(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_11ImageSprite_css(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
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
  th_11ImageSprite_css((&rv->m_data), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_11ImageSprite_css(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_type = KindOfString;
          th_11ImageSprite_css((&rv.m_data), (this_), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_11ImageSprite_css(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("ImageSprite::css", count, 1, 4, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::css");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_ImageSprite::t_geterrors()
_ZN4HPHP13c_ImageSprite11t_geterrorsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_11ImageSprite_getErrors(Value* _rv, ObjectData* this_) asm("_ZN4HPHP13c_ImageSprite11t_geterrorsEv");

TypedValue* tg_11ImageSprite_getErrors(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_11ImageSprite_getErrors((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("ImageSprite::getErrors", 0, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::getErrors");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_ImageSprite::t_mapping()
_ZN4HPHP13c_ImageSprite9t_mappingEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_11ImageSprite_mapping(Value* _rv, ObjectData* this_) asm("_ZN4HPHP13c_ImageSprite9t_mappingEv");

TypedValue* tg_11ImageSprite_mapping(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_type = KindOfArray;
        th_11ImageSprite_mapping((&rv.m_data), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("ImageSprite::mapping", 0, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::mapping");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_ImageSprite::t___destruct()
_ZN4HPHP13c_ImageSprite12t___destructEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_11ImageSprite___destruct(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP13c_ImageSprite12t___destructEv");

TypedValue* tg_11ImageSprite___destruct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_11ImageSprite___destruct((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("ImageSprite::__destruct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("ImageSprite::__destruct");
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

