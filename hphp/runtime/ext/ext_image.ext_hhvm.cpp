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

/*
HPHP::Array HPHP::f_gd_info()
_ZN4HPHP9f_gd_infoEv

(return value) => rax
_rv => rdi
*/

Value* fh_gd_info(Value* _rv) asm("_ZN4HPHP9f_gd_infoEv");

TypedValue* fg_gd_info(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_gd_info((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("gd_info", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_getimagesize(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP14f_getimagesizeERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
filename => rsi
imageinfo => rdx
*/

TypedValue* fh_getimagesize(TypedValue* _rv, Value* filename, TypedValue* imageinfo) asm("_ZN4HPHP14f_getimagesizeERKNS_6StringERKNS_14VRefParamValueE");

TypedValue * fg1_getimagesize(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_getimagesize(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = uninit_null();
  fh_getimagesize((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_getimagesize(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        VRefParamValue defVal1 = uninit_null();
        fh_getimagesize((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_getimagesize(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("getimagesize", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_image_type_to_extension(int, bool)
_ZN4HPHP25f_image_type_to_extensionEib

(return value) => rax
_rv => rdi
imagetype => rsi
include_dot => rdx
*/

Value* fh_image_type_to_extension(Value* _rv, int imagetype, bool include_dot) asm("_ZN4HPHP25f_image_type_to_extensionEib");

TypedValue * fg1_image_type_to_extension(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_image_type_to_extension(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_image_type_to_extension((&rv->m_data), (int)(args[-0].m_data.num), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_image_type_to_extension(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfString;
        fh_image_type_to_extension((&rv.m_data), (int)(args[-0].m_data.num), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_image_type_to_extension(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("image_type_to_extension", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_image_type_to_mime_type(int)
_ZN4HPHP25f_image_type_to_mime_typeEi

(return value) => rax
_rv => rdi
imagetype => rsi
*/

Value* fh_image_type_to_mime_type(Value* _rv, int imagetype) asm("_ZN4HPHP25f_image_type_to_mime_typeEi");

TypedValue * fg1_image_type_to_mime_type(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_image_type_to_mime_type(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_image_type_to_mime_type((&rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_image_type_to_mime_type(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfString;
        fh_image_type_to_mime_type((&rv.m_data), (int)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_image_type_to_mime_type(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("image_type_to_mime_type", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_image2wbmp(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP12f_image2wbmpERKNS_6ObjectERKNS_6StringEi

(return value) => rax
image => rdi
filename => rsi
threshold => rdx
*/

bool fh_image2wbmp(Value* image, Value* filename, int threshold) asm("_ZN4HPHP12f_image2wbmpERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_image2wbmp(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_image2wbmp(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_image2wbmp(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_image2wbmp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_image2wbmp(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_image2wbmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("image2wbmp", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagealphablending(HPHP::Object const&, bool)
_ZN4HPHP20f_imagealphablendingERKNS_6ObjectEb

(return value) => rax
image => rdi
blendmode => rsi
*/

bool fh_imagealphablending(Value* image, bool blendmode) asm("_ZN4HPHP20f_imagealphablendingERKNS_6ObjectEb");

TypedValue * fg1_imagealphablending(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagealphablending(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagealphablending(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagealphablending(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagealphablending(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagealphablending(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagealphablending", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imageantialias(HPHP::Object const&, bool)
_ZN4HPHP16f_imageantialiasERKNS_6ObjectEb

(return value) => rax
image => rdi
on => rsi
*/

bool fh_imageantialias(Value* image, bool on) asm("_ZN4HPHP16f_imageantialiasERKNS_6ObjectEb");

TypedValue * fg1_imageantialias(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imageantialias(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imageantialias(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imageantialias(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imageantialias(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imageantialias(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imageantialias", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagearc(HPHP::Object const&, int, int, int, int, int, int, int)
_ZN4HPHP10f_imagearcERKNS_6ObjectEiiiiiii

(return value) => rax
image => rdi
cx => rsi
cy => rdx
width => rcx
height => r8
start => r9
end => st0
color => st8
*/

bool fh_imagearc(Value* image, int cx, int cy, int width, int height, int start, int end, int color) asm("_ZN4HPHP10f_imagearcERKNS_6ObjectEiiiiiii");

TypedValue * fg1_imagearc(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagearc(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-7)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagearc(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagearc(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 8LL) {
      if ((args-7)->m_type == KindOfInt64 && (args-6)->m_type == KindOfInt64 && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagearc(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagearc(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagearc", count, 8, 8, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagechar(HPHP::Object const&, int, int, int, HPHP::String const&, int)
_ZN4HPHP11f_imagecharERKNS_6ObjectEiiiRKNS_6StringEi

(return value) => rax
image => rdi
font => rsi
x => rdx
y => rcx
c => r8
color => r9
*/

bool fh_imagechar(Value* image, int font, int x, int y, Value* c, int color) asm("_ZN4HPHP11f_imagecharERKNS_6ObjectEiiiRKNS_6StringEi");

TypedValue * fg1_imagechar(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagechar(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if (!IS_STRING_TYPE((args-4)->m_type)) {
    tvCastToStringInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagechar(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagechar(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && IS_STRING_TYPE((args-4)->m_type) && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagechar(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagechar(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagechar", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagecharup(HPHP::Object const&, int, int, int, HPHP::String const&, int)
_ZN4HPHP13f_imagecharupERKNS_6ObjectEiiiRKNS_6StringEi

(return value) => rax
image => rdi
font => rsi
x => rdx
y => rcx
c => r8
color => r9
*/

bool fh_imagecharup(Value* image, int font, int x, int y, Value* c, int color) asm("_ZN4HPHP13f_imagecharupERKNS_6ObjectEiiiRKNS_6StringEi");

TypedValue * fg1_imagecharup(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecharup(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if (!IS_STRING_TYPE((args-4)->m_type)) {
    tvCastToStringInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagecharup(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagecharup(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && IS_STRING_TYPE((args-4)->m_type) && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagecharup(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecharup(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecharup", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorallocate(HPHP::Object const&, int, int, int)
_ZN4HPHP20f_imagecolorallocateERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorallocate(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP20f_imagecolorallocateERKNS_6ObjectEiii");

TypedValue * fg1_imagecolorallocate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorallocate(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorallocate((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorallocate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorallocate((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorallocate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorallocate", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorallocatealpha(HPHP::Object const&, int, int, int, int)
_ZN4HPHP25f_imagecolorallocatealphaERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
alpha => r9
*/

TypedValue* fh_imagecolorallocatealpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP25f_imagecolorallocatealphaERKNS_6ObjectEiiii");

TypedValue * fg1_imagecolorallocatealpha(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorallocatealpha(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorallocatealpha((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorallocatealpha(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorallocatealpha((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorallocatealpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorallocatealpha", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorat(HPHP::Object const&, int, int)
_ZN4HPHP14f_imagecoloratERKNS_6ObjectEii

(return value) => rax
_rv => rdi
image => rsi
x => rdx
y => rcx
*/

TypedValue* fh_imagecolorat(TypedValue* _rv, Value* image, int x, int y) asm("_ZN4HPHP14f_imagecoloratERKNS_6ObjectEii");

TypedValue * fg1_imagecolorat(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorat(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorat((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorat(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorat((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorat(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorat", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorclosest(HPHP::Object const&, int, int, int)
_ZN4HPHP19f_imagecolorclosestERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorclosest(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP19f_imagecolorclosestERKNS_6ObjectEiii");

TypedValue * fg1_imagecolorclosest(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorclosest(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorclosest((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorclosest(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorclosest((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorclosest(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorclosest", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorclosestalpha(HPHP::Object const&, int, int, int, int)
_ZN4HPHP24f_imagecolorclosestalphaERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
alpha => r9
*/

TypedValue* fh_imagecolorclosestalpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP24f_imagecolorclosestalphaERKNS_6ObjectEiiii");

TypedValue * fg1_imagecolorclosestalpha(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorclosestalpha(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorclosestalpha((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorclosestalpha(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorclosestalpha((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorclosestalpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorclosestalpha", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorclosesthwb(HPHP::Object const&, int, int, int)
_ZN4HPHP22f_imagecolorclosesthwbERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorclosesthwb(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP22f_imagecolorclosesthwbERKNS_6ObjectEiii");

TypedValue * fg1_imagecolorclosesthwb(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorclosesthwb(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorclosesthwb((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorclosesthwb(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorclosesthwb((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorclosesthwb(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorclosesthwb", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagecolordeallocate(HPHP::Object const&, int)
_ZN4HPHP22f_imagecolordeallocateERKNS_6ObjectEi

(return value) => rax
image => rdi
color => rsi
*/

bool fh_imagecolordeallocate(Value* image, int color) asm("_ZN4HPHP22f_imagecolordeallocateERKNS_6ObjectEi");

TypedValue * fg1_imagecolordeallocate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolordeallocate(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagecolordeallocate(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagecolordeallocate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagecolordeallocate(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolordeallocate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolordeallocate", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorexact(HPHP::Object const&, int, int, int)
_ZN4HPHP17f_imagecolorexactERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorexact(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP17f_imagecolorexactERKNS_6ObjectEiii");

TypedValue * fg1_imagecolorexact(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorexact(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorexact((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorexact(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorexact((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorexact(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorexact", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorexactalpha(HPHP::Object const&, int, int, int, int)
_ZN4HPHP22f_imagecolorexactalphaERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
alpha => r9
*/

TypedValue* fh_imagecolorexactalpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP22f_imagecolorexactalphaERKNS_6ObjectEiiii");

TypedValue * fg1_imagecolorexactalpha(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorexactalpha(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorexactalpha((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorexactalpha(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorexactalpha((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorexactalpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorexactalpha", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolormatch(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP17f_imagecolormatchERKNS_6ObjectES2_

(return value) => rax
_rv => rdi
image1 => rsi
image2 => rdx
*/

TypedValue* fh_imagecolormatch(TypedValue* _rv, Value* image1, Value* image2) asm("_ZN4HPHP17f_imagecolormatchERKNS_6ObjectES2_");

TypedValue * fg1_imagecolormatch(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolormatch(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolormatch((rv), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolormatch(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        fh_imagecolormatch((&(rv)), &args[-0].m_data, &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolormatch(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolormatch", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorresolve(HPHP::Object const&, int, int, int)
_ZN4HPHP19f_imagecolorresolveERKNS_6ObjectEiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
*/

TypedValue* fh_imagecolorresolve(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP19f_imagecolorresolveERKNS_6ObjectEiii");

TypedValue * fg1_imagecolorresolve(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorresolve(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorresolve((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorresolve(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorresolve((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorresolve(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorresolve", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorresolvealpha(HPHP::Object const&, int, int, int, int)
_ZN4HPHP24f_imagecolorresolvealphaERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
red => rdx
green => rcx
blue => r8
alpha => r9
*/

TypedValue* fh_imagecolorresolvealpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP24f_imagecolorresolvealphaERKNS_6ObjectEiiii");

TypedValue * fg1_imagecolorresolvealpha(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorresolvealpha(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorresolvealpha((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorresolvealpha(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorresolvealpha((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorresolvealpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorresolvealpha", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorset(HPHP::Object const&, int, int, int, int)
_ZN4HPHP15f_imagecolorsetERKNS_6ObjectEiiii

(return value) => rax
_rv => rdi
image => rsi
index => rdx
red => rcx
green => r8
blue => r9
*/

TypedValue* fh_imagecolorset(TypedValue* _rv, Value* image, int index, int red, int green, int blue) asm("_ZN4HPHP15f_imagecolorsetERKNS_6ObjectEiiii");

TypedValue * fg1_imagecolorset(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorset(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorset((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorset((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorset(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorset", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorsforindex(HPHP::Object const&, int)
_ZN4HPHP21f_imagecolorsforindexERKNS_6ObjectEi

(return value) => rax
_rv => rdi
image => rsi
index => rdx
*/

TypedValue* fh_imagecolorsforindex(TypedValue* _rv, Value* image, int index) asm("_ZN4HPHP21f_imagecolorsforindexERKNS_6ObjectEi");

TypedValue * fg1_imagecolorsforindex(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorsforindex(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorsforindex((rv), &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorsforindex(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_imagecolorsforindex((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorsforindex(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorsforindex", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolorstotal(HPHP::Object const&)
_ZN4HPHP18f_imagecolorstotalERKNS_6ObjectE

(return value) => rax
_rv => rdi
image => rsi
*/

TypedValue* fh_imagecolorstotal(TypedValue* _rv, Value* image) asm("_ZN4HPHP18f_imagecolorstotalERKNS_6ObjectE");

TypedValue * fg1_imagecolorstotal(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolorstotal(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imagecolorstotal((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolorstotal(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_imagecolorstotal((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolorstotal(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolorstotal", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecolortransparent(HPHP::Object const&, int)
_ZN4HPHP23f_imagecolortransparentERKNS_6ObjectEi

(return value) => rax
_rv => rdi
image => rsi
color => rdx
*/

TypedValue* fh_imagecolortransparent(TypedValue* _rv, Value* image, int color) asm("_ZN4HPHP23f_imagecolortransparentERKNS_6ObjectEi");

TypedValue * fg1_imagecolortransparent(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecolortransparent(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolortransparent((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecolortransparent(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_imagecolortransparent((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecolortransparent(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecolortransparent", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imageconvolution(HPHP::Object const&, HPHP::Array const&, double, double)
_ZN4HPHP18f_imageconvolutionERKNS_6ObjectERKNS_5ArrayEdd

(return value) => rax
image => rdi
matrix => rsi
div => xmm0
offset => xmm1
*/

bool fh_imageconvolution(Value* image, Value* matrix, double div, double offset) asm("_ZN4HPHP18f_imageconvolutionERKNS_6ObjectERKNS_5ArrayEdd");

TypedValue * fg1_imageconvolution(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imageconvolution(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imageconvolution(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imageconvolution(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfDouble && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imageconvolution(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imageconvolution(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imageconvolution", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagecopy(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int)
_ZN4HPHP11f_imagecopyERKNS_6ObjectES2_iiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
src_w => st0
src_h => st8
*/

bool fh_imagecopy(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h) asm("_ZN4HPHP11f_imagecopyERKNS_6ObjectES2_iiiiii");

TypedValue * fg1_imagecopy(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecopy(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-7)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagecopy(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagecopy(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 8LL) {
      if ((args-7)->m_type == KindOfInt64 && (args-6)->m_type == KindOfInt64 && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagecopy(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecopy(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecopy", count, 8, 8, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagecopymerge(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int, int)
_ZN4HPHP16f_imagecopymergeERKNS_6ObjectES2_iiiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
src_w => st0
src_h => st8
pct => st16
*/

bool fh_imagecopymerge(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct) asm("_ZN4HPHP16f_imagecopymergeERKNS_6ObjectES2_iiiiiii");

TypedValue * fg1_imagecopymerge(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecopymerge(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-8)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-8);
  }
  if ((args-7)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagecopymerge(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagecopymerge(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 9LL) {
      if ((args-8)->m_type == KindOfInt64 && (args-7)->m_type == KindOfInt64 && (args-6)->m_type == KindOfInt64 && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagecopymerge(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 9);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecopymerge(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 9);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecopymerge", count, 9, 9, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 9);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagecopymergegray(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int, int)
_ZN4HPHP20f_imagecopymergegrayERKNS_6ObjectES2_iiiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
src_w => st0
src_h => st8
pct => st16
*/

bool fh_imagecopymergegray(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct) asm("_ZN4HPHP20f_imagecopymergegrayERKNS_6ObjectES2_iiiiiii");

TypedValue * fg1_imagecopymergegray(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecopymergegray(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-8)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-8);
  }
  if ((args-7)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagecopymergegray(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagecopymergegray(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 9LL) {
      if ((args-8)->m_type == KindOfInt64 && (args-7)->m_type == KindOfInt64 && (args-6)->m_type == KindOfInt64 && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagecopymergegray(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 9);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecopymergegray(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 9);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecopymergegray", count, 9, 9, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 9);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagecopyresampled(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int, int, int)
_ZN4HPHP20f_imagecopyresampledERKNS_6ObjectES2_iiiiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
dst_w => st0
dst_h => st8
src_w => st16
src_h => st24
*/

bool fh_imagecopyresampled(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h) asm("_ZN4HPHP20f_imagecopyresampledERKNS_6ObjectES2_iiiiiiii");

TypedValue * fg1_imagecopyresampled(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecopyresampled(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-9)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-9);
  }
  if ((args-8)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-8);
  }
  if ((args-7)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagecopyresampled(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num), (int)(args[-9].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagecopyresampled(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 10LL) {
      if ((args-9)->m_type == KindOfInt64 && (args-8)->m_type == KindOfInt64 && (args-7)->m_type == KindOfInt64 && (args-6)->m_type == KindOfInt64 && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagecopyresampled(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num), (int)(args[-9].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 10);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecopyresampled(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 10);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecopyresampled", count, 10, 10, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 10);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagecopyresized(HPHP::Object const&, HPHP::Object const&, int, int, int, int, int, int, int, int)
_ZN4HPHP18f_imagecopyresizedERKNS_6ObjectES2_iiiiiiii

(return value) => rax
dst_im => rdi
src_im => rsi
dst_x => rdx
dst_y => rcx
src_x => r8
src_y => r9
dst_w => st0
dst_h => st8
src_w => st16
src_h => st24
*/

bool fh_imagecopyresized(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h) asm("_ZN4HPHP18f_imagecopyresizedERKNS_6ObjectES2_iiiiiiii");

TypedValue * fg1_imagecopyresized(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecopyresized(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-9)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-9);
  }
  if ((args-8)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-8);
  }
  if ((args-7)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagecopyresized(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num), (int)(args[-9].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagecopyresized(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 10LL) {
      if ((args-9)->m_type == KindOfInt64 && (args-8)->m_type == KindOfInt64 && (args-7)->m_type == KindOfInt64 && (args-6)->m_type == KindOfInt64 && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagecopyresized(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num), (int)(args[-9].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 10);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecopyresized(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 10);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecopyresized", count, 10, 10, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 10);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreate(int, int)
_ZN4HPHP13f_imagecreateEii

(return value) => rax
_rv => rdi
width => rsi
height => rdx
*/

TypedValue* fh_imagecreate(TypedValue* _rv, int width, int height) asm("_ZN4HPHP13f_imagecreateEii");

TypedValue * fg1_imagecreate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreate(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_imagecreate((rv), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        fh_imagecreate((&(rv)), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreate", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromgd2part(HPHP::String const&, int, int, int, int)
_ZN4HPHP24f_imagecreatefromgd2partERKNS_6StringEiiii

(return value) => rax
_rv => rdi
filename => rsi
srcx => rdx
srcy => rcx
width => r8
height => r9
*/

TypedValue* fh_imagecreatefromgd2part(TypedValue* _rv, Value* filename, int srcx, int srcy, int width, int height) asm("_ZN4HPHP24f_imagecreatefromgd2partERKNS_6StringEiiii");

TypedValue * fg1_imagecreatefromgd2part(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromgd2part(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_imagecreatefromgd2part((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromgd2part(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromgd2part((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromgd2part(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromgd2part", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromgd(HPHP::String const&)
_ZN4HPHP19f_imagecreatefromgdERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromgd(TypedValue* _rv, Value* filename) asm("_ZN4HPHP19f_imagecreatefromgdERKNS_6StringE");

TypedValue * fg1_imagecreatefromgd(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromgd(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromgd((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromgd(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromgd((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromgd(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromgd", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromgd2(HPHP::String const&)
_ZN4HPHP20f_imagecreatefromgd2ERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromgd2(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromgd2ERKNS_6StringE");

TypedValue * fg1_imagecreatefromgd2(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromgd2(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromgd2((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromgd2(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromgd2((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromgd2(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromgd2", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromgif(HPHP::String const&)
_ZN4HPHP20f_imagecreatefromgifERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromgif(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromgifERKNS_6StringE");

TypedValue * fg1_imagecreatefromgif(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromgif(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromgif((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromgif(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromgif((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromgif(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromgif", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromjpeg(HPHP::String const&)
_ZN4HPHP21f_imagecreatefromjpegERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromjpeg(TypedValue* _rv, Value* filename) asm("_ZN4HPHP21f_imagecreatefromjpegERKNS_6StringE");

TypedValue * fg1_imagecreatefromjpeg(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromjpeg(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromjpeg((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromjpeg(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromjpeg((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromjpeg(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromjpeg", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefrompng(HPHP::String const&)
_ZN4HPHP20f_imagecreatefrompngERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefrompng(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefrompngERKNS_6StringE");

TypedValue * fg1_imagecreatefrompng(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefrompng(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefrompng((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefrompng(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefrompng((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefrompng(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefrompng", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromstring(HPHP::String const&)
_ZN4HPHP23f_imagecreatefromstringERKNS_6StringE

(return value) => rax
_rv => rdi
data => rsi
*/

TypedValue* fh_imagecreatefromstring(TypedValue* _rv, Value* data) asm("_ZN4HPHP23f_imagecreatefromstringERKNS_6StringE");

TypedValue * fg1_imagecreatefromstring(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromstring(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromstring((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromstring(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromstring((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromstring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromstring", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromwbmp(HPHP::String const&)
_ZN4HPHP21f_imagecreatefromwbmpERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromwbmp(TypedValue* _rv, Value* filename) asm("_ZN4HPHP21f_imagecreatefromwbmpERKNS_6StringE");

TypedValue * fg1_imagecreatefromwbmp(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromwbmp(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromwbmp((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromwbmp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromwbmp((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromwbmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromwbmp", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromxbm(HPHP::String const&)
_ZN4HPHP20f_imagecreatefromxbmERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromxbm(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromxbmERKNS_6StringE");

TypedValue * fg1_imagecreatefromxbm(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromxbm(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromxbm((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromxbm(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromxbm((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromxbm(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromxbm", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatefromxpm(HPHP::String const&)
_ZN4HPHP20f_imagecreatefromxpmERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_imagecreatefromxpm(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromxpmERKNS_6StringE");

TypedValue * fg1_imagecreatefromxpm(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatefromxpm(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromxpm((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatefromxpm(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imagecreatefromxpm((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatefromxpm(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatefromxpm", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagecreatetruecolor(int, int)
_ZN4HPHP22f_imagecreatetruecolorEii

(return value) => rax
_rv => rdi
width => rsi
height => rdx
*/

TypedValue* fh_imagecreatetruecolor(TypedValue* _rv, int width, int height) asm("_ZN4HPHP22f_imagecreatetruecolorEii");

TypedValue * fg1_imagecreatetruecolor(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagecreatetruecolor(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_imagecreatetruecolor((rv), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagecreatetruecolor(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        fh_imagecreatetruecolor((&(rv)), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagecreatetruecolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagecreatetruecolor", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagedashedline(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP17f_imagedashedlineERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
x1 => rsi
y1 => rdx
x2 => rcx
y2 => r8
color => r9
*/

bool fh_imagedashedline(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP17f_imagedashedlineERKNS_6ObjectEiiiii");

TypedValue * fg1_imagedashedline(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagedashedline(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagedashedline(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagedashedline(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagedashedline(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagedashedline(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagedashedline", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagedestroy(HPHP::Object const&)
_ZN4HPHP14f_imagedestroyERKNS_6ObjectE

(return value) => rax
image => rdi
*/

bool fh_imagedestroy(Value* image) asm("_ZN4HPHP14f_imagedestroyERKNS_6ObjectE");

TypedValue * fg1_imagedestroy(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagedestroy(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_imagedestroy(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagedestroy(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagedestroy(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagedestroy(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagedestroy", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imageellipse(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP14f_imageellipseERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
cx => rsi
cy => rdx
width => rcx
height => r8
color => r9
*/

bool fh_imageellipse(Value* image, int cx, int cy, int width, int height, int color) asm("_ZN4HPHP14f_imageellipseERKNS_6ObjectEiiiii");

TypedValue * fg1_imageellipse(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imageellipse(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imageellipse(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imageellipse(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imageellipse(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imageellipse(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imageellipse", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagefill(HPHP::Object const&, int, int, int)
_ZN4HPHP11f_imagefillERKNS_6ObjectEiii

(return value) => rax
image => rdi
x => rsi
y => rdx
color => rcx
*/

bool fh_imagefill(Value* image, int x, int y, int color) asm("_ZN4HPHP11f_imagefillERKNS_6ObjectEiii");

TypedValue * fg1_imagefill(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefill(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagefill(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagefill(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagefill(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefill(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefill", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagefilledarc(HPHP::Object const&, int, int, int, int, int, int, int, int)
_ZN4HPHP16f_imagefilledarcERKNS_6ObjectEiiiiiiii

(return value) => rax
image => rdi
cx => rsi
cy => rdx
width => rcx
height => r8
start => r9
end => st0
color => st8
style => st16
*/

bool fh_imagefilledarc(Value* image, int cx, int cy, int width, int height, int start, int end, int color, int style) asm("_ZN4HPHP16f_imagefilledarcERKNS_6ObjectEiiiiiiii");

TypedValue * fg1_imagefilledarc(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefilledarc(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-8)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-8);
  }
  if ((args-7)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagefilledarc(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagefilledarc(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 9LL) {
      if ((args-8)->m_type == KindOfInt64 && (args-7)->m_type == KindOfInt64 && (args-6)->m_type == KindOfInt64 && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagefilledarc(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 9);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefilledarc(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 9);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefilledarc", count, 9, 9, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 9);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagefilledellipse(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP20f_imagefilledellipseERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
cx => rsi
cy => rdx
width => rcx
height => r8
color => r9
*/

bool fh_imagefilledellipse(Value* image, int cx, int cy, int width, int height, int color) asm("_ZN4HPHP20f_imagefilledellipseERKNS_6ObjectEiiiii");

TypedValue * fg1_imagefilledellipse(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefilledellipse(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagefilledellipse(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagefilledellipse(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagefilledellipse(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefilledellipse(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefilledellipse", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagefilledpolygon(HPHP::Object const&, HPHP::Array const&, int, int)
_ZN4HPHP20f_imagefilledpolygonERKNS_6ObjectERKNS_5ArrayEii

(return value) => rax
image => rdi
points => rsi
num_points => rdx
color => rcx
*/

bool fh_imagefilledpolygon(Value* image, Value* points, int num_points, int color) asm("_ZN4HPHP20f_imagefilledpolygonERKNS_6ObjectERKNS_5ArrayEii");

TypedValue * fg1_imagefilledpolygon(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefilledpolygon(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagefilledpolygon(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagefilledpolygon(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagefilledpolygon(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefilledpolygon(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefilledpolygon", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagefilledrectangle(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP22f_imagefilledrectangleERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
x1 => rsi
y1 => rdx
x2 => rcx
y2 => r8
color => r9
*/

bool fh_imagefilledrectangle(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP22f_imagefilledrectangleERKNS_6ObjectEiiiii");

TypedValue * fg1_imagefilledrectangle(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefilledrectangle(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagefilledrectangle(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagefilledrectangle(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagefilledrectangle(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefilledrectangle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefilledrectangle", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagefilltoborder(HPHP::Object const&, int, int, int, int)
_ZN4HPHP19f_imagefilltoborderERKNS_6ObjectEiiii

(return value) => rax
image => rdi
x => rsi
y => rdx
border => rcx
color => r8
*/

bool fh_imagefilltoborder(Value* image, int x, int y, int border, int color) asm("_ZN4HPHP19f_imagefilltoborderERKNS_6ObjectEiiii");

TypedValue * fg1_imagefilltoborder(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefilltoborder(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagefilltoborder(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagefilltoborder(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagefilltoborder(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefilltoborder(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefilltoborder", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagefilter(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP13f_imagefilterERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
filtertype => rsi
arg1 => rdx
arg2 => rcx
arg3 => r8
arg4 => r9
*/

bool fh_imagefilter(Value* image, int filtertype, int arg1, int arg2, int arg3, int arg4) asm("_ZN4HPHP13f_imagefilterERKNS_6ObjectEiiiii");

TypedValue * fg1_imagefilter(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefilter(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagefilter(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagefilter(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagefilter(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefilter(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefilter", count, 2, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_imagefontheight(int)
_ZN4HPHP17f_imagefontheightEi

(return value) => rax
font => rdi
*/

long fh_imagefontheight(int font) asm("_ZN4HPHP17f_imagefontheightEi");

TypedValue * fg1_imagefontheight(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefontheight(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (int64_t)fh_imagefontheight((int)(args[-0].m_data.num));
  return rv;
}

TypedValue* fg_imagefontheight(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_imagefontheight((int)(args[-0].m_data.num));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefontheight(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefontheight", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_imagefontwidth(int)
_ZN4HPHP16f_imagefontwidthEi

(return value) => rax
font => rdi
*/

long fh_imagefontwidth(int font) asm("_ZN4HPHP16f_imagefontwidthEi");

TypedValue * fg1_imagefontwidth(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefontwidth(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (int64_t)fh_imagefontwidth((int)(args[-0].m_data.num));
  return rv;
}

TypedValue* fg_imagefontwidth(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_imagefontwidth((int)(args[-0].m_data.num));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefontwidth(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefontwidth", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imageftbbox(double, double, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13f_imageftbboxEddRKNS_6StringES2_RKNS_5ArrayE

(return value) => rax
_rv => rdi
size => xmm0
angle => xmm1
font_file => rsi
text => rdx
extrainfo => rcx
*/

TypedValue* fh_imageftbbox(TypedValue* _rv, double size, double angle, Value* font_file, Value* text, Value* extrainfo) asm("_ZN4HPHP13f_imageftbboxEddRKNS_6StringES2_RKNS_5ArrayE");

TypedValue * fg1_imageftbbox(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imageftbbox(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-4);
    }
  case 4:
    break;
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  Array defVal4 = uninit_null();
  fh_imageftbbox((rv), (args[-0].m_data.dbl), (args[-1].m_data.dbl), &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&defVal4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imageftbbox(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfArray) && IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfDouble) {
        Array defVal4 = uninit_null();
        fh_imageftbbox((&(rv)), (args[-0].m_data.dbl), (args[-1].m_data.dbl), &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&defVal4));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imageftbbox(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imageftbbox", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagefttext(HPHP::Object const&, double, double, int, int, int, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP13f_imagefttextERKNS_6ObjectEddiiiRKNS_6StringES5_RKNS_5ArrayE

(return value) => rax
_rv => rdi
image => rsi
size => xmm0
angle => xmm1
x => rdx
y => rcx
col => r8
font_file => r9
text => st0
extrainfo => st8
*/

TypedValue* fh_imagefttext(TypedValue* _rv, Value* image, double size, double angle, int x, int y, int col, Value* font_file, Value* text, Value* extrainfo) asm("_ZN4HPHP13f_imagefttextERKNS_6ObjectEddiiiRKNS_6StringES5_RKNS_5ArrayE");

TypedValue * fg1_imagefttext(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagefttext(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 9
    if ((args-8)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-8);
    }
  case 8:
    break;
  }
  if (!IS_STRING_TYPE((args-7)->m_type)) {
    tvCastToStringInPlace(args-7);
  }
  if (!IS_STRING_TYPE((args-6)->m_type)) {
    tvCastToStringInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  Array defVal8 = uninit_null();
  fh_imagefttext((rv), &args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), &args[-6].m_data, &args[-7].m_data, (count > 8) ? &args[-8].m_data : (Value*)(&defVal8));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagefttext(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 8LL && count <= 9LL) {
      if ((count <= 8 || (args-8)->m_type == KindOfArray) && IS_STRING_TYPE((args-7)->m_type) && IS_STRING_TYPE((args-6)->m_type) && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        Array defVal8 = uninit_null();
        fh_imagefttext((&(rv)), &args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), &args[-6].m_data, &args[-7].m_data, (count > 8) ? &args[-8].m_data : (Value*)(&defVal8));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 9);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagefttext(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 9);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagefttext", count, 8, 9, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 9);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagegammacorrect(HPHP::Object const&, double, double)
_ZN4HPHP19f_imagegammacorrectERKNS_6ObjectEdd

(return value) => rax
image => rdi
inputgamma => xmm0
outputgamma => xmm1
*/

bool fh_imagegammacorrect(Value* image, double inputgamma, double outputgamma) asm("_ZN4HPHP19f_imagegammacorrectERKNS_6ObjectEdd");

TypedValue * fg1_imagegammacorrect(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagegammacorrect(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagegammacorrect(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagegammacorrect(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagegammacorrect(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagegammacorrect(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagegammacorrect", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagegd2(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP10f_imagegd2ERKNS_6ObjectERKNS_6StringEii

(return value) => rax
image => rdi
filename => rsi
chunk_size => rdx
type => rcx
*/

bool fh_imagegd2(Value* image, Value* filename, int chunk_size, int type) asm("_ZN4HPHP10f_imagegd2ERKNS_6ObjectERKNS_6StringEii");

TypedValue * fg1_imagegd2(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagegd2(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagegd2(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagegd2(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagegd2(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagegd2(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagegd2", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagegd(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP9f_imagegdERKNS_6ObjectERKNS_6StringE

(return value) => rax
image => rdi
filename => rsi
*/

bool fh_imagegd(Value* image, Value* filename) asm("_ZN4HPHP9f_imagegdERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_imagegd(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagegd(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagegd(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagegd(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagegd(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagegd(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagegd", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagegif(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP10f_imagegifERKNS_6ObjectERKNS_6StringE

(return value) => rax
image => rdi
filename => rsi
*/

bool fh_imagegif(Value* image, Value* filename) asm("_ZN4HPHP10f_imagegifERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_imagegif(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagegif(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagegif(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagegif(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagegif(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagegif(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagegif", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagegrabscreen()
_ZN4HPHP17f_imagegrabscreenEv

(return value) => rax
_rv => rdi
*/

TypedValue* fh_imagegrabscreen(TypedValue* _rv) asm("_ZN4HPHP17f_imagegrabscreenEv");

TypedValue* fg_imagegrabscreen(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      fh_imagegrabscreen((&(rv)));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("imagegrabscreen", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagegrabwindow(int, int)
_ZN4HPHP17f_imagegrabwindowEii

(return value) => rax
_rv => rdi
window => rsi
client_area => rdx
*/

TypedValue* fh_imagegrabwindow(TypedValue* _rv, int window, int client_area) asm("_ZN4HPHP17f_imagegrabwindowEii");

TypedValue * fg1_imagegrabwindow(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagegrabwindow(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_imagegrabwindow((rv), (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagegrabwindow(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_imagegrabwindow((&(rv)), (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagegrabwindow(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagegrabwindow", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imageinterlace(HPHP::Object const&, int)
_ZN4HPHP16f_imageinterlaceERKNS_6ObjectEi

(return value) => rax
_rv => rdi
image => rsi
interlace => rdx
*/

TypedValue* fh_imageinterlace(TypedValue* _rv, Value* image, int interlace) asm("_ZN4HPHP16f_imageinterlaceERKNS_6ObjectEi");

TypedValue * fg1_imageinterlace(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imageinterlace(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imageinterlace((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imageinterlace(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_imageinterlace((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imageinterlace(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imageinterlace", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imageistruecolor(HPHP::Object const&)
_ZN4HPHP18f_imageistruecolorERKNS_6ObjectE

(return value) => rax
image => rdi
*/

bool fh_imageistruecolor(Value* image) asm("_ZN4HPHP18f_imageistruecolorERKNS_6ObjectE");

TypedValue * fg1_imageistruecolor(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imageistruecolor(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_imageistruecolor(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imageistruecolor(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imageistruecolor(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imageistruecolor(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imageistruecolor", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagejpeg(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP11f_imagejpegERKNS_6ObjectERKNS_6StringEi

(return value) => rax
image => rdi
filename => rsi
quality => rdx
*/

bool fh_imagejpeg(Value* image, Value* filename, int quality) asm("_ZN4HPHP11f_imagejpegERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_imagejpeg(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagejpeg(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagejpeg(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagejpeg(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagejpeg(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagejpeg(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagejpeg", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagelayereffect(HPHP::Object const&, int)
_ZN4HPHP18f_imagelayereffectERKNS_6ObjectEi

(return value) => rax
image => rdi
effect => rsi
*/

bool fh_imagelayereffect(Value* image, int effect) asm("_ZN4HPHP18f_imagelayereffectERKNS_6ObjectEi");

TypedValue * fg1_imagelayereffect(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagelayereffect(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagelayereffect(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagelayereffect(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagelayereffect(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagelayereffect(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagelayereffect", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imageline(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP11f_imagelineERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
x1 => rsi
y1 => rdx
x2 => rcx
y2 => r8
color => r9
*/

bool fh_imageline(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP11f_imagelineERKNS_6ObjectEiiiii");

TypedValue * fg1_imageline(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imageline(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imageline(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imageline(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imageline(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imageline(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imageline", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imageloadfont(HPHP::String const&)
_ZN4HPHP15f_imageloadfontERKNS_6StringE

(return value) => rax
_rv => rdi
file => rsi
*/

TypedValue* fh_imageloadfont(TypedValue* _rv, Value* file) asm("_ZN4HPHP15f_imageloadfontERKNS_6StringE");

TypedValue * fg1_imageloadfont(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imageloadfont(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imageloadfont((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imageloadfont(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_imageloadfont((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imageloadfont(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imageloadfont", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_imagepalettecopy(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP18f_imagepalettecopyERKNS_6ObjectES2_

destination => rdi
source => rsi
*/

void fh_imagepalettecopy(Value* destination, Value* source) asm("_ZN4HPHP18f_imagepalettecopyERKNS_6ObjectES2_");

TypedValue * fg1_imagepalettecopy(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepalettecopy(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagepalettecopy(&args[-0].m_data, &args[-1].m_data);
  return rv;
}

TypedValue* fg_imagepalettecopy(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_imagepalettecopy(&args[-0].m_data, &args[-1].m_data);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepalettecopy(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepalettecopy", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagepng(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP10f_imagepngERKNS_6ObjectERKNS_6StringEii

(return value) => rax
image => rdi
filename => rsi
quality => rdx
filters => rcx
*/

bool fh_imagepng(Value* image, Value* filename, int quality, int filters) asm("_ZN4HPHP10f_imagepngERKNS_6ObjectERKNS_6StringEii");

TypedValue * fg1_imagepng(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepng(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagepng(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1), (count > 3) ? (int)(args[-3].m_data.num) : (int)(-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagepng(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagepng(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1), (count > 3) ? (int)(args[-3].m_data.num) : (int)(-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepng(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepng", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagepolygon(HPHP::Object const&, HPHP::Array const&, int, int)
_ZN4HPHP14f_imagepolygonERKNS_6ObjectERKNS_5ArrayEii

(return value) => rax
image => rdi
points => rsi
num_points => rdx
color => rcx
*/

bool fh_imagepolygon(Value* image, Value* points, int num_points, int color) asm("_ZN4HPHP14f_imagepolygonERKNS_6ObjectERKNS_5ArrayEii");

TypedValue * fg1_imagepolygon(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepolygon(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagepolygon(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagepolygon(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagepolygon(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepolygon(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepolygon", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_imagepsbbox(HPHP::String const&, int, int, int, int, double)
_ZN4HPHP13f_imagepsbboxERKNS_6StringEiiiid

(return value) => rax
_rv => rdi
text => rsi
font => rdx
size => rcx
space => r8
tightness => r9
angle => xmm0
*/

Value* fh_imagepsbbox(Value* _rv, Value* text, int font, int size, int space, int tightness, double angle) asm("_ZN4HPHP13f_imagepsbboxERKNS_6StringEiiiid");

TypedValue * fg1_imagepsbbox(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepsbbox(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_imagepsbbox((&rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (args[-5].m_data.dbl) : (double)(0.0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagepsbbox(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfDouble) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfArray;
        fh_imagepsbbox((&rv.m_data), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (args[-5].m_data.dbl) : (double)(0.0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepsbbox(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepsbbox", count, 3, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagepsencodefont(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19f_imagepsencodefontERKNS_6ObjectERKNS_6StringE

(return value) => rax
font_index => rdi
encodingfile => rsi
*/

bool fh_imagepsencodefont(Value* font_index, Value* encodingfile) asm("_ZN4HPHP19f_imagepsencodefontERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_imagepsencodefont(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepsencodefont(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagepsencodefont(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagepsencodefont(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagepsencodefont(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepsencodefont(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepsencodefont", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagepsextendfont(int, double)
_ZN4HPHP19f_imagepsextendfontEid

(return value) => rax
font_index => rdi
extend => xmm0
*/

bool fh_imagepsextendfont(int font_index, double extend) asm("_ZN4HPHP19f_imagepsextendfontEid");

TypedValue * fg1_imagepsextendfont(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepsextendfont(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_imagepsextendfont((int)(args[-0].m_data.num), (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagepsextendfont(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagepsextendfont((int)(args[-0].m_data.num), (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepsextendfont(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepsextendfont", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagepsfreefont(HPHP::Object const&)
_ZN4HPHP17f_imagepsfreefontERKNS_6ObjectE

(return value) => rax
fontindex => rdi
*/

bool fh_imagepsfreefont(Value* fontindex) asm("_ZN4HPHP17f_imagepsfreefontERKNS_6ObjectE");

TypedValue * fg1_imagepsfreefont(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepsfreefont(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_imagepsfreefont(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagepsfreefont(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagepsfreefont(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepsfreefont(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepsfreefont", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_imagepsloadfont(HPHP::String const&)
_ZN4HPHP17f_imagepsloadfontERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

Value* fh_imagepsloadfont(Value* _rv, Value* filename) asm("_ZN4HPHP17f_imagepsloadfontERKNS_6StringE");

TypedValue * fg1_imagepsloadfont(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepsloadfont(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToStringInPlace(args-0);
  fh_imagepsloadfont((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagepsloadfont(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfObject;
        fh_imagepsloadfont((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepsloadfont(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepsloadfont", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagepsslantfont(HPHP::Object const&, double)
_ZN4HPHP18f_imagepsslantfontERKNS_6ObjectEd

(return value) => rax
font_index => rdi
slant => xmm0
*/

bool fh_imagepsslantfont(Value* font_index, double slant) asm("_ZN4HPHP18f_imagepsslantfontERKNS_6ObjectEd");

TypedValue * fg1_imagepsslantfont(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepsslantfont(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagepsslantfont(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagepsslantfont(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagepsslantfont(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepsslantfont(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepsslantfont", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_imagepstext(HPHP::Object const&, HPHP::String const&, HPHP::Object const&, int, int, int, int, int, int, int, double, int)
_ZN4HPHP13f_imagepstextERKNS_6ObjectERKNS_6StringES2_iiiiiiidi

(return value) => rax
_rv => rdi
image => rsi
text => rdx
font => rcx
size => r8
foreground => r9
background => st0
x => st8
y => st16
space => st24
tightness => st32
angle => xmm0
antialias_steps => st40
*/

Value* fh_imagepstext(Value* _rv, Value* image, Value* text, Value* font, int size, int foreground, int background, int x, int y, int space, int tightness, double angle, int antialias_steps) asm("_ZN4HPHP13f_imagepstextERKNS_6ObjectERKNS_6StringES2_iiiiiiidi");

TypedValue * fg1_imagepstext(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagepstext(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfArray;
  switch (count) {
  default: // count >= 12
    if ((args-11)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-11);
    }
  case 11:
    if ((args-10)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-10);
    }
  case 10:
    if ((args-9)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-9);
    }
  case 9:
    if ((args-8)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-8);
    }
  case 8:
    break;
  }
  if ((args-7)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagepstext((&rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (count > 8) ? (int)(args[-8].m_data.num) : (int)(0), (count > 9) ? (int)(args[-9].m_data.num) : (int)(0), (count > 10) ? (args[-10].m_data.dbl) : (double)(0.0), (count > 11) ? (int)(args[-11].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagepstext(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 8LL && count <= 12LL) {
      if ((count <= 11 || (args-11)->m_type == KindOfInt64) && (count <= 10 || (args-10)->m_type == KindOfDouble) && (count <= 9 || (args-9)->m_type == KindOfInt64) && (count <= 8 || (args-8)->m_type == KindOfInt64) && (args-7)->m_type == KindOfInt64 && (args-6)->m_type == KindOfInt64 && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfObject && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfArray;
        fh_imagepstext((&rv.m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (count > 8) ? (int)(args[-8].m_data.num) : (int)(0), (count > 9) ? (int)(args[-9].m_data.num) : (int)(0), (count > 10) ? (args[-10].m_data.dbl) : (double)(0.0), (count > 11) ? (int)(args[-11].m_data.num) : (int)(0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 12);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagepstext(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 12);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagepstext", count, 8, 12, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 12);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagerectangle(HPHP::Object const&, int, int, int, int, int)
_ZN4HPHP16f_imagerectangleERKNS_6ObjectEiiiii

(return value) => rax
image => rdi
x1 => rsi
y1 => rdx
x2 => rcx
y2 => r8
color => r9
*/

bool fh_imagerectangle(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP16f_imagerectangleERKNS_6ObjectEiiiii");

TypedValue * fg1_imagerectangle(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagerectangle(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagerectangle(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagerectangle(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagerectangle(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagerectangle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagerectangle", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagerotate(HPHP::Object const&, double, int, int)
_ZN4HPHP13f_imagerotateERKNS_6ObjectEdii

(return value) => rax
_rv => rdi
source_image => rsi
angle => xmm0
bgd_color => rdx
ignore_transparent => rcx
*/

TypedValue* fh_imagerotate(TypedValue* _rv, Value* source_image, double angle, int bgd_color, int ignore_transparent) asm("_ZN4HPHP13f_imagerotateERKNS_6ObjectEdii");

TypedValue * fg1_imagerotate(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagerotate(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagerotate((rv), &args[-0].m_data, (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagerotate(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        fh_imagerotate((&(rv)), &args[-0].m_data, (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagerotate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagerotate", count, 3, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagesavealpha(HPHP::Object const&, bool)
_ZN4HPHP16f_imagesavealphaERKNS_6ObjectEb

(return value) => rax
image => rdi
saveflag => rsi
*/

bool fh_imagesavealpha(Value* image, bool saveflag) asm("_ZN4HPHP16f_imagesavealphaERKNS_6ObjectEb");

TypedValue * fg1_imagesavealpha(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagesavealpha(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagesavealpha(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagesavealpha(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagesavealpha(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagesavealpha(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagesavealpha", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagesetbrush(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP15f_imagesetbrushERKNS_6ObjectES2_

(return value) => rax
image => rdi
brush => rsi
*/

bool fh_imagesetbrush(Value* image, Value* brush) asm("_ZN4HPHP15f_imagesetbrushERKNS_6ObjectES2_");

TypedValue * fg1_imagesetbrush(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagesetbrush(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagesetbrush(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagesetbrush(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagesetbrush(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagesetbrush(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagesetbrush", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagesetpixel(HPHP::Object const&, int, int, int)
_ZN4HPHP15f_imagesetpixelERKNS_6ObjectEiii

(return value) => rax
image => rdi
x => rsi
y => rdx
color => rcx
*/

bool fh_imagesetpixel(Value* image, int x, int y, int color) asm("_ZN4HPHP15f_imagesetpixelERKNS_6ObjectEiii");

TypedValue * fg1_imagesetpixel(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagesetpixel(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagesetpixel(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagesetpixel(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagesetpixel(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagesetpixel(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagesetpixel", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagesetstyle(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP15f_imagesetstyleERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
image => rdi
style => rsi
*/

bool fh_imagesetstyle(Value* image, Value* style) asm("_ZN4HPHP15f_imagesetstyleERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_imagesetstyle(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagesetstyle(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagesetstyle(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagesetstyle(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagesetstyle(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagesetstyle(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagesetstyle", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagesetthickness(HPHP::Object const&, int)
_ZN4HPHP19f_imagesetthicknessERKNS_6ObjectEi

(return value) => rax
image => rdi
thickness => rsi
*/

bool fh_imagesetthickness(Value* image, int thickness) asm("_ZN4HPHP19f_imagesetthicknessERKNS_6ObjectEi");

TypedValue * fg1_imagesetthickness(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagesetthickness(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagesetthickness(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagesetthickness(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagesetthickness(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagesetthickness(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagesetthickness", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagesettile(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP14f_imagesettileERKNS_6ObjectES2_

(return value) => rax
image => rdi
tile => rsi
*/

bool fh_imagesettile(Value* image, Value* tile) asm("_ZN4HPHP14f_imagesettileERKNS_6ObjectES2_");

TypedValue * fg1_imagesettile(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagesettile(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagesettile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagesettile(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagesettile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagesettile(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagesettile", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagestring(HPHP::Object const&, int, int, int, HPHP::String const&, int)
_ZN4HPHP13f_imagestringERKNS_6ObjectEiiiRKNS_6StringEi

(return value) => rax
image => rdi
font => rsi
x => rdx
y => rcx
str => r8
color => r9
*/

bool fh_imagestring(Value* image, int font, int x, int y, Value* str, int color) asm("_ZN4HPHP13f_imagestringERKNS_6ObjectEiiiRKNS_6StringEi");

TypedValue * fg1_imagestring(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagestring(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if (!IS_STRING_TYPE((args-4)->m_type)) {
    tvCastToStringInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagestring(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagestring(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && IS_STRING_TYPE((args-4)->m_type) && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagestring(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagestring(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagestring", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagestringup(HPHP::Object const&, int, int, int, HPHP::String const&, int)
_ZN4HPHP15f_imagestringupERKNS_6ObjectEiiiRKNS_6StringEi

(return value) => rax
image => rdi
font => rsi
x => rdx
y => rcx
str => r8
color => r9
*/

bool fh_imagestringup(Value* image, int font, int x, int y, Value* str, int color) asm("_ZN4HPHP15f_imagestringupERKNS_6ObjectEiiiRKNS_6StringEi");

TypedValue * fg1_imagestringup(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagestringup(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if (!IS_STRING_TYPE((args-4)->m_type)) {
    tvCastToStringInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagestringup(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagestringup(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 6LL) {
      if ((args-5)->m_type == KindOfInt64 && IS_STRING_TYPE((args-4)->m_type) && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagestringup(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagestringup(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagestringup", count, 6, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagesx(HPHP::Object const&)
_ZN4HPHP9f_imagesxERKNS_6ObjectE

(return value) => rax
_rv => rdi
image => rsi
*/

TypedValue* fh_imagesx(TypedValue* _rv, Value* image) asm("_ZN4HPHP9f_imagesxERKNS_6ObjectE");

TypedValue * fg1_imagesx(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagesx(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imagesx((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagesx(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_imagesx((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagesx(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagesx", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagesy(HPHP::Object const&)
_ZN4HPHP9f_imagesyERKNS_6ObjectE

(return value) => rax
_rv => rdi
image => rsi
*/

TypedValue* fh_imagesy(TypedValue* _rv, Value* image) asm("_ZN4HPHP9f_imagesyERKNS_6ObjectE");

TypedValue * fg1_imagesy(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagesy(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imagesy((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagesy(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_imagesy((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagesy(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagesy", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagetruecolortopalette(HPHP::Object const&, bool, int)
_ZN4HPHP25f_imagetruecolortopaletteERKNS_6ObjectEbi

(return value) => rax
_rv => rdi
image => rsi
dither => rdx
ncolors => rcx
*/

TypedValue* fh_imagetruecolortopalette(TypedValue* _rv, Value* image, bool dither, int ncolors) asm("_ZN4HPHP25f_imagetruecolortopaletteERKNS_6ObjectEbi");

TypedValue * fg1_imagetruecolortopalette(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagetruecolortopalette(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagetruecolortopalette((rv), &args[-0].m_data, (bool)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagetruecolortopalette(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        fh_imagetruecolortopalette((&(rv)), &args[-0].m_data, (bool)(args[-1].m_data.num), (int)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagetruecolortopalette(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagetruecolortopalette", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagettfbbox(double, double, HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_imagettfbboxEddRKNS_6StringES2_

(return value) => rax
_rv => rdi
size => xmm0
angle => xmm1
fontfile => rsi
text => rdx
*/

TypedValue* fh_imagettfbbox(TypedValue* _rv, double size, double angle, Value* fontfile, Value* text) asm("_ZN4HPHP14f_imagettfbboxEddRKNS_6StringES2_");

TypedValue * fg1_imagettfbbox(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagettfbbox(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-0);
  }
  fh_imagettfbbox((rv), (args[-0].m_data.dbl), (args[-1].m_data.dbl), &args[-2].m_data, &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagettfbbox(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if (IS_STRING_TYPE((args-3)->m_type) && IS_STRING_TYPE((args-2)->m_type) && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfDouble) {
        fh_imagettfbbox((&(rv)), (args[-0].m_data.dbl), (args[-1].m_data.dbl), &args[-2].m_data, &args[-3].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagettfbbox(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagettfbbox", count, 4, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_imagettftext(HPHP::Object const&, double, double, int, int, int, HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_imagettftextERKNS_6ObjectEddiiiRKNS_6StringES5_

(return value) => rax
_rv => rdi
image => rsi
size => xmm0
angle => xmm1
x => rdx
y => rcx
color => r8
fontfile => r9
text => st0
*/

TypedValue* fh_imagettftext(TypedValue* _rv, Value* image, double size, double angle, int x, int y, int color, Value* fontfile, Value* text) asm("_ZN4HPHP14f_imagettftextERKNS_6ObjectEddiiiRKNS_6StringES5_");

TypedValue * fg1_imagettftext(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagettftext(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-7)->m_type)) {
    tvCastToStringInPlace(args-7);
  }
  if (!IS_STRING_TYPE((args-6)->m_type)) {
    tvCastToStringInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagettftext((rv), &args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), &args[-6].m_data, &args[-7].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_imagettftext(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 8LL) {
      if (IS_STRING_TYPE((args-7)->m_type) && IS_STRING_TYPE((args-6)->m_type) && (args-5)->m_type == KindOfInt64 && (args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfDouble && (args-1)->m_type == KindOfDouble && (args-0)->m_type == KindOfObject) {
        fh_imagettftext((&(rv)), &args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), &args[-6].m_data, &args[-7].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagettftext(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagettftext", count, 8, 8, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_imagetypes()
_ZN4HPHP12f_imagetypesEv

(return value) => rax
*/

long fh_imagetypes() asm("_ZN4HPHP12f_imagetypesEv");

TypedValue* fg_imagetypes(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_imagetypes();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("imagetypes", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagewbmp(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP11f_imagewbmpERKNS_6ObjectERKNS_6StringEi

(return value) => rax
image => rdi
filename => rsi
foreground => rdx
*/

bool fh_imagewbmp(Value* image, Value* filename, int foreground) asm("_ZN4HPHP11f_imagewbmpERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_imagewbmp(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagewbmp(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagewbmp(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagewbmp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagewbmp(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagewbmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagewbmp", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_imagexbm(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP10f_imagexbmERKNS_6ObjectERKNS_6StringEi

(return value) => rax
image => rdi
filename => rsi
foreground => rdx
*/

bool fh_imagexbm(Value* image, Value* filename, int foreground) asm("_ZN4HPHP10f_imagexbmERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_imagexbm(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_imagexbm(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_imagexbm(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_imagexbm(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_imagexbm(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_imagexbm(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("imagexbm", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iptcembed(HPHP::String const&, HPHP::String const&, int)
_ZN4HPHP11f_iptcembedERKNS_6StringES2_i

(return value) => rax
_rv => rdi
iptcdata => rsi
jpeg_file_name => rdx
spool => rcx
*/

TypedValue* fh_iptcembed(TypedValue* _rv, Value* iptcdata, Value* jpeg_file_name, int spool) asm("_ZN4HPHP11f_iptcembedERKNS_6StringES2_i");

TypedValue * fg1_iptcembed(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iptcembed(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  fh_iptcembed((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iptcembed(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iptcembed((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iptcembed(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iptcembed", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iptcparse(HPHP::String const&)
_ZN4HPHP11f_iptcparseERKNS_6StringE

(return value) => rax
_rv => rdi
iptcblock => rsi
*/

TypedValue* fh_iptcparse(TypedValue* _rv, Value* iptcblock) asm("_ZN4HPHP11f_iptcparseERKNS_6StringE");

TypedValue * fg1_iptcparse(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iptcparse(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_iptcparse((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iptcparse(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_iptcparse((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iptcparse(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iptcparse", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_jpeg2wbmp(HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP11f_jpeg2wbmpERKNS_6StringES2_iii

(return value) => rax
jpegname => rdi
wbmpname => rsi
dest_height => rdx
dest_width => rcx
threshold => r8
*/

bool fh_jpeg2wbmp(Value* jpegname, Value* wbmpname, int dest_height, int dest_width, int threshold) asm("_ZN4HPHP11f_jpeg2wbmpERKNS_6StringES2_iii");

TypedValue * fg1_jpeg2wbmp(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_jpeg2wbmp(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_jpeg2wbmp(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_jpeg2wbmp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_jpeg2wbmp(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_jpeg2wbmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("jpeg2wbmp", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_png2wbmp(HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP10f_png2wbmpERKNS_6StringES2_iii

(return value) => rax
pngname => rdi
wbmpname => rsi
dest_height => rdx
dest_width => rcx
threshold => r8
*/

bool fh_png2wbmp(Value* pngname, Value* wbmpname, int dest_height, int dest_width, int threshold) asm("_ZN4HPHP10f_png2wbmpERKNS_6StringES2_iii");

TypedValue * fg1_png2wbmp(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_png2wbmp(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_png2wbmp(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_png2wbmp(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 5LL) {
      if ((args-4)->m_type == KindOfInt64 && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_png2wbmp(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_png2wbmp(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("png2wbmp", count, 5, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_exif_imagetype(HPHP::String const&)
_ZN4HPHP16f_exif_imagetypeERKNS_6StringE

(return value) => rax
_rv => rdi
filename => rsi
*/

TypedValue* fh_exif_imagetype(TypedValue* _rv, Value* filename) asm("_ZN4HPHP16f_exif_imagetypeERKNS_6StringE");

TypedValue * fg1_exif_imagetype(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_exif_imagetype(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_exif_imagetype((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_exif_imagetype(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        fh_exif_imagetype((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_exif_imagetype(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("exif_imagetype", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_exif_read_data(HPHP::String const&, HPHP::String const&, bool, bool)
_ZN4HPHP16f_exif_read_dataERKNS_6StringES2_bb

(return value) => rax
_rv => rdi
filename => rsi
sections => rdx
arrays => rcx
thumbnail => r8
*/

TypedValue* fh_exif_read_data(TypedValue* _rv, Value* filename, Value* sections, bool arrays, bool thumbnail) asm("_ZN4HPHP16f_exif_read_dataERKNS_6StringES2_bb");

TypedValue * fg1_exif_read_data(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_exif_read_data(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
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
  fh_exif_read_data((rv), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_exif_read_data(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_exif_read_data((&(rv)), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_exif_read_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("exif_read_data", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_read_exif_data(HPHP::String const&, HPHP::String const&, bool, bool)
_ZN4HPHP16f_read_exif_dataERKNS_6StringES2_bb

(return value) => rax
_rv => rdi
filename => rsi
sections => rdx
arrays => rcx
thumbnail => r8
*/

TypedValue* fh_read_exif_data(TypedValue* _rv, Value* filename, Value* sections, bool arrays, bool thumbnail) asm("_ZN4HPHP16f_read_exif_dataERKNS_6StringES2_bb");

TypedValue * fg1_read_exif_data(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_read_exif_data(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
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
  fh_read_exif_data((rv), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_read_exif_data(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || (args-2)->m_type == KindOfBoolean) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_read_exif_data((&(rv)), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_read_exif_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("read_exif_data", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_exif_tagname(int)
_ZN4HPHP14f_exif_tagnameEi

(return value) => rax
_rv => rdi
index => rsi
*/

TypedValue* fh_exif_tagname(TypedValue* _rv, int index) asm("_ZN4HPHP14f_exif_tagnameEi");

TypedValue * fg1_exif_tagname(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_exif_tagname(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_exif_tagname((rv), (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_exif_tagname(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        fh_exif_tagname((&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_exif_tagname(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("exif_tagname", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_exif_thumbnail(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP16f_exif_thumbnailERKNS_6StringERKNS_14VRefParamValueES5_S5_

(return value) => rax
_rv => rdi
filename => rsi
width => rdx
height => rcx
imagetype => r8
*/

TypedValue* fh_exif_thumbnail(TypedValue* _rv, Value* filename, TypedValue* width, TypedValue* height, TypedValue* imagetype) asm("_ZN4HPHP16f_exif_thumbnailERKNS_6StringERKNS_14VRefParamValueES5_S5_");

TypedValue * fg1_exif_thumbnail(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_exif_thumbnail(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = uninit_null();
  VRefParamValue defVal2 = uninit_null();
  VRefParamValue defVal3 = uninit_null();
  fh_exif_thumbnail((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_exif_thumbnail(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        VRefParamValue defVal1 = uninit_null();
        VRefParamValue defVal2 = uninit_null();
        VRefParamValue defVal3 = uninit_null();
        fh_exif_thumbnail((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_exif_thumbnail(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("exif_thumbnail", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

