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

Value* fh_image_type_to_mime_type(Value* _rv, int imagetype) asm("_ZN4HPHP25f_image_type_to_mime_typeEi");

void fg1_image_type_to_mime_type(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_image_type_to_mime_type(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_image_type_to_mime_type(&(rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_image_type_to_mime_type(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_image_type_to_mime_type(&(rv->m_data), (int)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_image_type_to_mime_type(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("image_type_to_mime_type", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_image_type_to_extension(Value* _rv, int imagetype, bool include_dot) asm("_ZN4HPHP25f_image_type_to_extensionEib");

void fg1_image_type_to_extension(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_image_type_to_extension(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfString;
  fh_image_type_to_extension(&(rv->m_data), (int)(args[-0].m_data.num), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_image_type_to_extension(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_image_type_to_extension(&(rv->m_data), (int)(args[-0].m_data.num), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_image_type_to_extension(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("image_type_to_extension", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_getimagesize(TypedValue* _rv, Value* filename, TypedValue* imageinfo) asm("_ZN4HPHP14f_getimagesizeERKNS_6StringERKNS_14VRefParamValueE");

void fg1_getimagesize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_getimagesize(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = uninit_null();
  fh_getimagesize(rv, &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_getimagesize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal1 = uninit_null();
      fh_getimagesize(rv, &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_getimagesize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("getimagesize", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_gd_info(Value* _rv) asm("_ZN4HPHP9f_gd_infoEv");

TypedValue* fg_gd_info(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_gd_info(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("gd_info", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imageloadfont(TypedValue* _rv, Value* file) asm("_ZN4HPHP15f_imageloadfontERKNS_6StringE");

void fg1_imageloadfont(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imageloadfont(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imageloadfont(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imageloadfont(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imageloadfont(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imageloadfont(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imageloadfont", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagesetstyle(Value* image, Value* style) asm("_ZN4HPHP15f_imagesetstyleERKNS_6ObjectERKNS_5ArrayE");

void fg1_imagesetstyle(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagesetstyle(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagesetstyle(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imagesetstyle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagesetstyle(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imagesetstyle(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagesetstyle", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatetruecolor(TypedValue* _rv, int width, int height) asm("_ZN4HPHP22f_imagecreatetruecolorEii");

void fg1_imagecreatetruecolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatetruecolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_imagecreatetruecolor(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatetruecolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      fh_imagecreatetruecolor(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatetruecolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatetruecolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imageistruecolor(Value* image) asm("_ZN4HPHP18f_imageistruecolorERKNS_6ObjectE");

void fg1_imageistruecolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imageistruecolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imageistruecolor(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imageistruecolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imageistruecolor(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imageistruecolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imageistruecolor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagetruecolortopalette(TypedValue* _rv, Value* image, bool dither, int ncolors) asm("_ZN4HPHP25f_imagetruecolortopaletteERKNS_6ObjectEbi");

void fg1_imagetruecolortopalette(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagetruecolortopalette(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagetruecolortopalette(rv, &args[-0].m_data, (bool)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagetruecolortopalette(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfBoolean &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagetruecolortopalette(rv, &args[-0].m_data, (bool)(args[-1].m_data.num), (int)(args[-2].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagetruecolortopalette(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagetruecolortopalette", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolormatch(TypedValue* _rv, Value* image1, Value* image2) asm("_ZN4HPHP17f_imagecolormatchERKNS_6ObjectES2_");

void fg1_imagecolormatch(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolormatch(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolormatch(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolormatch(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolormatch(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolormatch(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolormatch", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagesetthickness(Value* image, int thickness) asm("_ZN4HPHP19f_imagesetthicknessERKNS_6ObjectEi");

void fg1_imagesetthickness(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagesetthickness(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagesetthickness(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagesetthickness(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagesetthickness(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagesetthickness(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagesetthickness", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagefilledellipse(Value* image, int cx, int cy, int width, int height, int color) asm("_ZN4HPHP20f_imagefilledellipseERKNS_6ObjectEiiiii");

void fg1_imagefilledellipse(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefilledellipse(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagefilledellipse(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagefilledellipse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagefilledellipse(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagefilledellipse(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefilledellipse", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagefilledarc(Value* image, int cx, int cy, int width, int height, int start, int end, int color, int style) asm("_ZN4HPHP16f_imagefilledarcERKNS_6ObjectEiiiiiiii");

void fg1_imagefilledarc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefilledarc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagefilledarc(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagefilledarc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 9) {
    if ((args - 8)->m_type == KindOfInt64 &&
        (args - 7)->m_type == KindOfInt64 &&
        (args - 6)->m_type == KindOfInt64 &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagefilledarc(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagefilledarc(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefilledarc", count, 9, 9, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 9);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagealphablending(Value* image, bool blendmode) asm("_ZN4HPHP20f_imagealphablendingERKNS_6ObjectEb");

void fg1_imagealphablending(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagealphablending(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagealphablending(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagealphablending(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfBoolean &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagealphablending(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagealphablending(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagealphablending", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagesavealpha(Value* image, bool saveflag) asm("_ZN4HPHP16f_imagesavealphaERKNS_6ObjectEb");

void fg1_imagesavealpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagesavealpha(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagesavealpha(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagesavealpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfBoolean &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagesavealpha(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagesavealpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagesavealpha", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagelayereffect(Value* image, int effect) asm("_ZN4HPHP18f_imagelayereffectERKNS_6ObjectEi");

void fg1_imagelayereffect(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagelayereffect(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagelayereffect(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagelayereffect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagelayereffect(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagelayereffect(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagelayereffect", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorallocatealpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP25f_imagecolorallocatealphaERKNS_6ObjectEiiii");

void fg1_imagecolorallocatealpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorallocatealpha(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorallocatealpha(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorallocatealpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorallocatealpha(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorallocatealpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorallocatealpha", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorresolvealpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP24f_imagecolorresolvealphaERKNS_6ObjectEiiii");

void fg1_imagecolorresolvealpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorresolvealpha(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorresolvealpha(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorresolvealpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorresolvealpha(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorresolvealpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorresolvealpha", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorclosestalpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP24f_imagecolorclosestalphaERKNS_6ObjectEiiii");

void fg1_imagecolorclosestalpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorclosestalpha(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorclosestalpha(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorclosestalpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorclosestalpha(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorclosestalpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorclosestalpha", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorexactalpha(TypedValue* _rv, Value* image, int red, int green, int blue, int alpha) asm("_ZN4HPHP22f_imagecolorexactalphaERKNS_6ObjectEiiii");

void fg1_imagecolorexactalpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorexactalpha(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorexactalpha(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorexactalpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorexactalpha(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorexactalpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorexactalpha", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagecopyresampled(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h) asm("_ZN4HPHP20f_imagecopyresampledERKNS_6ObjectES2_iiiiiiii");

void fg1_imagecopyresampled(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecopyresampled(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagecopyresampled(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num), (int)(args[-9].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagecopyresampled(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 10) {
    if ((args - 9)->m_type == KindOfInt64 &&
        (args - 8)->m_type == KindOfInt64 &&
        (args - 7)->m_type == KindOfInt64 &&
        (args - 6)->m_type == KindOfInt64 &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagecopyresampled(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num), (int)(args[-9].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagecopyresampled(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecopyresampled", count, 10, 10, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 10);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagegrabwindow(TypedValue* _rv, int window, int client_area) asm("_ZN4HPHP17f_imagegrabwindowEii");

void fg1_imagegrabwindow(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagegrabwindow(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagegrabwindow(rv, (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagegrabwindow(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      fh_imagegrabwindow(rv, (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagegrabwindow(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagegrabwindow", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagegrabscreen(TypedValue* _rv) asm("_ZN4HPHP17f_imagegrabscreenEv");

TypedValue* fg_imagegrabscreen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_imagegrabscreen(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("imagegrabscreen", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagerotate(TypedValue* _rv, Value* source_image, double angle, int bgd_color, int ignore_transparent) asm("_ZN4HPHP13f_imagerotateERKNS_6ObjectEdii");

void fg1_imagerotate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagerotate(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagerotate(rv, &args[-0].m_data, (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagerotate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagerotate(rv, &args[-0].m_data, (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagerotate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagerotate", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagesettile(Value* image, Value* tile) asm("_ZN4HPHP14f_imagesettileERKNS_6ObjectES2_");

void fg1_imagesettile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagesettile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagesettile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imagesettile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagesettile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imagesettile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagesettile", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagesetbrush(Value* image, Value* brush) asm("_ZN4HPHP15f_imagesetbrushERKNS_6ObjectES2_");

void fg1_imagesetbrush(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagesetbrush(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagesetbrush(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imagesetbrush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagesetbrush(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imagesetbrush(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagesetbrush", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreate(TypedValue* _rv, int width, int height) asm("_ZN4HPHP13f_imagecreateEii");

void fg1_imagecreate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreate(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_imagecreate(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      fh_imagecreate(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreate", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_imagetypes() asm("_ZN4HPHP12f_imagetypesEv");

TypedValue* fg_imagetypes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_imagetypes();
  } else {
    throw_toomany_arguments_nr("imagetypes", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromstring(TypedValue* _rv, Value* data) asm("_ZN4HPHP23f_imagecreatefromstringERKNS_6StringE");

void fg1_imagecreatefromstring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromstring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromstring(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromstring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromstring(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromstring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromstring", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromgif(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromgifERKNS_6StringE");

void fg1_imagecreatefromgif(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromgif(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromgif(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromgif(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromgif(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromgif(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromgif", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromjpeg(TypedValue* _rv, Value* filename) asm("_ZN4HPHP21f_imagecreatefromjpegERKNS_6StringE");

void fg1_imagecreatefromjpeg(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromjpeg(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromjpeg(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromjpeg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromjpeg(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromjpeg(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromjpeg", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefrompng(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefrompngERKNS_6StringE");

void fg1_imagecreatefrompng(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefrompng(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefrompng(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefrompng(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefrompng(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefrompng(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefrompng", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromxbm(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromxbmERKNS_6StringE");

void fg1_imagecreatefromxbm(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromxbm(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromxbm(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromxbm(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromxbm(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromxbm(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromxbm", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromxpm(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromxpmERKNS_6StringE");

void fg1_imagecreatefromxpm(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromxpm(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromxpm(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromxpm(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromxpm(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromxpm(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromxpm", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromwbmp(TypedValue* _rv, Value* filename) asm("_ZN4HPHP21f_imagecreatefromwbmpERKNS_6StringE");

void fg1_imagecreatefromwbmp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromwbmp(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromwbmp(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromwbmp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromwbmp(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromwbmp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromwbmp", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromgd(TypedValue* _rv, Value* filename) asm("_ZN4HPHP19f_imagecreatefromgdERKNS_6StringE");

void fg1_imagecreatefromgd(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromgd(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromgd(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromgd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromgd(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromgd(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromgd", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromgd2(TypedValue* _rv, Value* filename) asm("_ZN4HPHP20f_imagecreatefromgd2ERKNS_6StringE");

void fg1_imagecreatefromgd2(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromgd2(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imagecreatefromgd2(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromgd2(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromgd2(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromgd2(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromgd2", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecreatefromgd2part(TypedValue* _rv, Value* filename, int srcx, int srcy, int width, int height) asm("_ZN4HPHP24f_imagecreatefromgd2partERKNS_6StringEiiii");

void fg1_imagecreatefromgd2part(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecreatefromgd2part(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecreatefromgd2part(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecreatefromgd2part(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imagecreatefromgd2part(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecreatefromgd2part(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecreatefromgd2part", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagexbm(Value* image, Value* filename, int foreground) asm("_ZN4HPHP10f_imagexbmERKNS_6ObjectERKNS_6StringEi");

void fg1_imagexbm(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagexbm(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagexbm(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
}

TypedValue* fg_imagexbm(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagexbm(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
    } else {
      fg1_imagexbm(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagexbm", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagegif(Value* image, Value* filename) asm("_ZN4HPHP10f_imagegifERKNS_6ObjectERKNS_6StringE");

void fg1_imagegif(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagegif(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagegif(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_imagegif(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagegif(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_imagegif(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagegif", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagepng(Value* image, Value* filename, int quality, int filters) asm("_ZN4HPHP10f_imagepngERKNS_6ObjectERKNS_6StringEii");

void fg1_imagepng(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepng(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagepng(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1), (count > 3) ? (int)(args[-3].m_data.num) : (int)(-1))) ? 1LL : 0LL;
}

TypedValue* fg_imagepng(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagepng(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1), (count > 3) ? (int)(args[-3].m_data.num) : (int)(-1))) ? 1LL : 0LL;
    } else {
      fg1_imagepng(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepng", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagejpeg(Value* image, Value* filename, int quality) asm("_ZN4HPHP11f_imagejpegERKNS_6ObjectERKNS_6StringEi");

void fg1_imagejpeg(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagejpeg(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagejpeg(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
}

TypedValue* fg_imagejpeg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagejpeg(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
    } else {
      fg1_imagejpeg(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagejpeg", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagewbmp(Value* image, Value* filename, int foreground) asm("_ZN4HPHP11f_imagewbmpERKNS_6ObjectERKNS_6StringEi");

void fg1_imagewbmp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagewbmp(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagewbmp(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
}

TypedValue* fg_imagewbmp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagewbmp(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
    } else {
      fg1_imagewbmp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagewbmp", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagegd(Value* image, Value* filename) asm("_ZN4HPHP9f_imagegdERKNS_6ObjectERKNS_6StringE");

void fg1_imagegd(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagegd(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagegd(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_imagegd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagegd(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_imagegd(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagegd", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagegd2(Value* image, Value* filename, int chunk_size, int type) asm("_ZN4HPHP10f_imagegd2ERKNS_6ObjectERKNS_6StringEii");

void fg1_imagegd2(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagegd2(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagegd2(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imagegd2(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagegd2(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_imagegd2(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagegd2", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagedestroy(Value* image) asm("_ZN4HPHP14f_imagedestroyERKNS_6ObjectE");

void fg1_imagedestroy(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagedestroy(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagedestroy(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imagedestroy(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagedestroy(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imagedestroy(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagedestroy", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorallocate(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP20f_imagecolorallocateERKNS_6ObjectEiii");

void fg1_imagecolorallocate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorallocate(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorallocate(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorallocate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorallocate(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorallocate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorallocate", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_imagepalettecopy(Value* destination, Value* source) asm("_ZN4HPHP18f_imagepalettecopyERKNS_6ObjectES2_");

void fg1_imagepalettecopy(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepalettecopy(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_imagepalettecopy(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_imagepalettecopy(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_imagepalettecopy(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_imagepalettecopy(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepalettecopy", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorat(TypedValue* _rv, Value* image, int x, int y) asm("_ZN4HPHP14f_imagecoloratERKNS_6ObjectEii");

void fg1_imagecolorat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorat(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorat(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorat(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorat", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorclosest(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP19f_imagecolorclosestERKNS_6ObjectEiii");

void fg1_imagecolorclosest(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorclosest(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorclosest(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorclosest(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorclosest(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorclosest(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorclosest", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorclosesthwb(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP22f_imagecolorclosesthwbERKNS_6ObjectEiii");

void fg1_imagecolorclosesthwb(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorclosesthwb(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorclosesthwb(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorclosesthwb(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorclosesthwb(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorclosesthwb(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorclosesthwb", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagecolordeallocate(Value* image, int color) asm("_ZN4HPHP22f_imagecolordeallocateERKNS_6ObjectEi");

void fg1_imagecolordeallocate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolordeallocate(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagecolordeallocate(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagecolordeallocate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagecolordeallocate(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagecolordeallocate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolordeallocate", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorresolve(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP19f_imagecolorresolveERKNS_6ObjectEiii");

void fg1_imagecolorresolve(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorresolve(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorresolve(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorresolve(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorresolve(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorresolve(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorresolve", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorexact(TypedValue* _rv, Value* image, int red, int green, int blue) asm("_ZN4HPHP17f_imagecolorexactERKNS_6ObjectEiii");

void fg1_imagecolorexact(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorexact(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorexact(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorexact(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorexact(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorexact(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorexact", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorset(TypedValue* _rv, Value* image, int index, int red, int green, int blue) asm("_ZN4HPHP15f_imagecolorsetERKNS_6ObjectEiiii");

void fg1_imagecolorset(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorset(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolorset(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorset(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorset(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorset", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorsforindex(TypedValue* _rv, Value* image, int index) asm("_ZN4HPHP21f_imagecolorsforindexERKNS_6ObjectEi");

void fg1_imagecolorsforindex(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorsforindex(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imagecolorsforindex(rv, &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorsforindex(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolorsforindex(rv, &args[-0].m_data, (int)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorsforindex(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorsforindex", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagegammacorrect(Value* image, double inputgamma, double outputgamma) asm("_ZN4HPHP19f_imagegammacorrectERKNS_6ObjectEdd");

void fg1_imagegammacorrect(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagegammacorrect(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagegammacorrect(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_imagegammacorrect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagegammacorrect(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_imagegammacorrect(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagegammacorrect", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagesetpixel(Value* image, int x, int y, int color) asm("_ZN4HPHP15f_imagesetpixelERKNS_6ObjectEiii");

void fg1_imagesetpixel(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagesetpixel(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagesetpixel(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagesetpixel(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagesetpixel(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagesetpixel(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagesetpixel", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imageline(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP11f_imagelineERKNS_6ObjectEiiiii");

void fg1_imageline(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imageline(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imageline(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imageline(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imageline(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imageline(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imageline", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagedashedline(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP17f_imagedashedlineERKNS_6ObjectEiiiii");

void fg1_imagedashedline(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagedashedline(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagedashedline(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagedashedline(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagedashedline(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagedashedline(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagedashedline", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagerectangle(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP16f_imagerectangleERKNS_6ObjectEiiiii");

void fg1_imagerectangle(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagerectangle(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagerectangle(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagerectangle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagerectangle(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagerectangle(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagerectangle", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagefilledrectangle(Value* image, int x1, int y1, int x2, int y2, int color) asm("_ZN4HPHP22f_imagefilledrectangleERKNS_6ObjectEiiiii");

void fg1_imagefilledrectangle(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefilledrectangle(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagefilledrectangle(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagefilledrectangle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagefilledrectangle(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagefilledrectangle(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefilledrectangle", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagearc(Value* image, int cx, int cy, int width, int height, int start, int end, int color) asm("_ZN4HPHP10f_imagearcERKNS_6ObjectEiiiiiii");

void fg1_imagearc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagearc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagearc(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagearc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 8) {
    if ((args - 7)->m_type == KindOfInt64 &&
        (args - 6)->m_type == KindOfInt64 &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagearc(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagearc(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagearc", count, 8, 8, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 8);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imageellipse(Value* image, int cx, int cy, int width, int height, int color) asm("_ZN4HPHP14f_imageellipseERKNS_6ObjectEiiiii");

void fg1_imageellipse(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imageellipse(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imageellipse(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imageellipse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imageellipse(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imageellipse(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imageellipse", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagefilltoborder(Value* image, int x, int y, int border, int color) asm("_ZN4HPHP19f_imagefilltoborderERKNS_6ObjectEiiii");

void fg1_imagefilltoborder(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefilltoborder(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagefilltoborder(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagefilltoborder(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagefilltoborder(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagefilltoborder(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefilltoborder", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagefill(Value* image, int x, int y, int color) asm("_ZN4HPHP11f_imagefillERKNS_6ObjectEiii");

void fg1_imagefill(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefill(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagefill(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagefill(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagefill(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagefill(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefill", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolorstotal(TypedValue* _rv, Value* image) asm("_ZN4HPHP18f_imagecolorstotalERKNS_6ObjectE");

void fg1_imagecolorstotal(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolorstotal(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imagecolorstotal(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolorstotal(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_imagecolorstotal(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolorstotal(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolorstotal", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagecolortransparent(TypedValue* _rv, Value* image, int color) asm("_ZN4HPHP23f_imagecolortransparentERKNS_6ObjectEi");

void fg1_imagecolortransparent(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecolortransparent(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagecolortransparent(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagecolortransparent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagecolortransparent(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagecolortransparent(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecolortransparent", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imageinterlace(TypedValue* _rv, Value* image, int interlace) asm("_ZN4HPHP16f_imageinterlaceERKNS_6ObjectEi");

void fg1_imageinterlace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imageinterlace(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imageinterlace(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imageinterlace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imageinterlace(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imageinterlace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imageinterlace", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagepolygon(Value* image, Value* points, int num_points, int color) asm("_ZN4HPHP14f_imagepolygonERKNS_6ObjectERKNS_5ArrayEii");

void fg1_imagepolygon(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepolygon(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagepolygon(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagepolygon(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagepolygon(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagepolygon(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepolygon", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagefilledpolygon(Value* image, Value* points, int num_points, int color) asm("_ZN4HPHP20f_imagefilledpolygonERKNS_6ObjectERKNS_5ArrayEii");

void fg1_imagefilledpolygon(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefilledpolygon(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagefilledpolygon(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagefilledpolygon(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagefilledpolygon(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagefilledpolygon(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefilledpolygon", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_imagefontwidth(int font) asm("_ZN4HPHP16f_imagefontwidthEi");

void fg1_imagefontwidth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefontwidth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_imagefontwidth((int)(args[-0].m_data.num));
}

TypedValue* fg_imagefontwidth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_imagefontwidth((int)(args[-0].m_data.num));
    } else {
      fg1_imagefontwidth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefontwidth", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_imagefontheight(int font) asm("_ZN4HPHP17f_imagefontheightEi");

void fg1_imagefontheight(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefontheight(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_imagefontheight((int)(args[-0].m_data.num));
}

TypedValue* fg_imagefontheight(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_imagefontheight((int)(args[-0].m_data.num));
    } else {
      fg1_imagefontheight(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefontheight", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagechar(Value* image, int font, int x, int y, Value* c, int color) asm("_ZN4HPHP11f_imagecharERKNS_6ObjectEiiiRKNS_6StringEi");

void fg1_imagechar(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagechar(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagechar(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagechar(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 4)->m_type) &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagechar(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagechar(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagechar", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagecharup(Value* image, int font, int x, int y, Value* c, int color) asm("_ZN4HPHP13f_imagecharupERKNS_6ObjectEiiiRKNS_6StringEi");

void fg1_imagecharup(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecharup(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagecharup(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagecharup(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 4)->m_type) &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagecharup(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagecharup(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecharup", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagestring(Value* image, int font, int x, int y, Value* str, int color) asm("_ZN4HPHP13f_imagestringERKNS_6ObjectEiiiRKNS_6StringEi");

void fg1_imagestring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagestring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagestring(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagestring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 4)->m_type) &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagestring(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagestring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagestring", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagestringup(Value* image, int font, int x, int y, Value* str, int color) asm("_ZN4HPHP15f_imagestringupERKNS_6ObjectEiiiRKNS_6StringEi");

void fg1_imagestringup(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagestringup(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagestringup(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagestringup(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 4)->m_type) &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagestringup(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagestringup(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagestringup", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagecopy(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h) asm("_ZN4HPHP11f_imagecopyERKNS_6ObjectES2_iiiiii");

void fg1_imagecopy(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecopy(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagecopy(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagecopy(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 8) {
    if ((args - 7)->m_type == KindOfInt64 &&
        (args - 6)->m_type == KindOfInt64 &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagecopy(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagecopy(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecopy", count, 8, 8, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 8);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagecopymerge(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct) asm("_ZN4HPHP16f_imagecopymergeERKNS_6ObjectES2_iiiiiii");

void fg1_imagecopymerge(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecopymerge(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagecopymerge(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagecopymerge(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 9) {
    if ((args - 8)->m_type == KindOfInt64 &&
        (args - 7)->m_type == KindOfInt64 &&
        (args - 6)->m_type == KindOfInt64 &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagecopymerge(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagecopymerge(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecopymerge", count, 9, 9, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 9);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagecopymergegray(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int src_w, int src_h, int pct) asm("_ZN4HPHP20f_imagecopymergegrayERKNS_6ObjectES2_iiiiiii");

void fg1_imagecopymergegray(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecopymergegray(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagecopymergegray(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagecopymergegray(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 9) {
    if ((args - 8)->m_type == KindOfInt64 &&
        (args - 7)->m_type == KindOfInt64 &&
        (args - 6)->m_type == KindOfInt64 &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagecopymergegray(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagecopymergegray(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecopymergegray", count, 9, 9, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 9);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagecopyresized(Value* dst_im, Value* src_im, int dst_x, int dst_y, int src_x, int src_y, int dst_w, int dst_h, int src_w, int src_h) asm("_ZN4HPHP18f_imagecopyresizedERKNS_6ObjectES2_iiiiiiii");

void fg1_imagecopyresized(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagecopyresized(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagecopyresized(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num), (int)(args[-9].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imagecopyresized(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 10) {
    if ((args - 9)->m_type == KindOfInt64 &&
        (args - 8)->m_type == KindOfInt64 &&
        (args - 7)->m_type == KindOfInt64 &&
        (args - 6)->m_type == KindOfInt64 &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagecopyresized(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (int)(args[-8].m_data.num), (int)(args[-9].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imagecopyresized(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagecopyresized", count, 10, 10, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 10);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagesx(TypedValue* _rv, Value* image) asm("_ZN4HPHP9f_imagesxERKNS_6ObjectE");

void fg1_imagesx(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagesx(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imagesx(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagesx(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_imagesx(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagesx(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagesx", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagesy(TypedValue* _rv, Value* image) asm("_ZN4HPHP9f_imagesyERKNS_6ObjectE");

void fg1_imagesy(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagesy(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imagesy(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagesy(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_imagesy(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagesy(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagesy", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imageftbbox(TypedValue* _rv, double size, double angle, Value* font_file, Value* text, Value* extrainfo) asm("_ZN4HPHP13f_imageftbboxEddRKNS_6StringES2_RKNS_5ArrayE");

void fg1_imageftbbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imageftbbox(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imageftbbox(rv, (args[-0].m_data.dbl), (args[-1].m_data.dbl), &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&defVal4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imageftbbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfArray) &&
        IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfDouble) {
      Array defVal4 = uninit_null();
      fh_imageftbbox(rv, (args[-0].m_data.dbl), (args[-1].m_data.dbl), &args[-2].m_data, &args[-3].m_data, (count > 4) ? &args[-4].m_data : (Value*)(&defVal4));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imageftbbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imageftbbox", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagefttext(TypedValue* _rv, Value* image, double size, double angle, int x, int y, int col, Value* font_file, Value* text, Value* extrainfo) asm("_ZN4HPHP13f_imagefttextERKNS_6ObjectEddiiiRKNS_6StringES5_RKNS_5ArrayE");

void fg1_imagefttext(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefttext(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagefttext(rv, &args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), &args[-6].m_data, &args[-7].m_data, (count > 8) ? &args[-8].m_data : (Value*)(&defVal8));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagefttext(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 8 && count <= 9) {
    if ((count <= 8 || (args - 8)->m_type == KindOfArray) &&
        IS_STRING_TYPE((args - 7)->m_type) &&
        IS_STRING_TYPE((args - 6)->m_type) &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      Array defVal8 = uninit_null();
      fh_imagefttext(rv, &args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), &args[-6].m_data, &args[-7].m_data, (count > 8) ? &args[-8].m_data : (Value*)(&defVal8));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagefttext(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefttext", count, 8, 9, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 9);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagettfbbox(TypedValue* _rv, double size, double angle, Value* fontfile, Value* text) asm("_ZN4HPHP14f_imagettfbboxEddRKNS_6StringES2_");

void fg1_imagettfbbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagettfbbox(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagettfbbox(rv, (args[-0].m_data.dbl), (args[-1].m_data.dbl), &args[-2].m_data, &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagettfbbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfDouble) {
      fh_imagettfbbox(rv, (args[-0].m_data.dbl), (args[-1].m_data.dbl), &args[-2].m_data, &args[-3].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagettfbbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagettfbbox", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imagettftext(TypedValue* _rv, Value* image, double size, double angle, int x, int y, int color, Value* fontfile, Value* text) asm("_ZN4HPHP14f_imagettftextERKNS_6ObjectEddiiiRKNS_6StringES5_");

void fg1_imagettftext(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagettftext(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imagettftext(rv, &args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), &args[-6].m_data, &args[-7].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imagettftext(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 8) {
    if (IS_STRING_TYPE((args - 7)->m_type) &&
        IS_STRING_TYPE((args - 6)->m_type) &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      fh_imagettftext(rv, &args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), &args[-6].m_data, &args[-7].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imagettftext(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagettftext", count, 8, 8, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 8);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_imagepsloadfont(Value* _rv, Value* filename) asm("_ZN4HPHP17f_imagepsloadfontERKNS_6StringE");

void fg1_imagepsloadfont(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepsloadfont(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_imagepsloadfont(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_imagepsloadfont(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfObject;
      fh_imagepsloadfont(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_imagepsloadfont(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepsloadfont", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagepsfreefont(Value* fontindex) asm("_ZN4HPHP17f_imagepsfreefontERKNS_6ObjectE");

void fg1_imagepsfreefont(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepsfreefont(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagepsfreefont(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imagepsfreefont(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagepsfreefont(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imagepsfreefont(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepsfreefont", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagepsencodefont(Value* font_index, Value* encodingfile) asm("_ZN4HPHP19f_imagepsencodefontERKNS_6ObjectERKNS_6StringE");

void fg1_imagepsencodefont(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepsencodefont(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagepsencodefont(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imagepsencodefont(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagepsencodefont(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imagepsencodefont(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepsencodefont", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagepsextendfont(int font_index, double extend) asm("_ZN4HPHP19f_imagepsextendfontEid");

void fg1_imagepsextendfont(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepsextendfont(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagepsextendfont((int)(args[-0].m_data.num), (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_imagepsextendfont(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagepsextendfont((int)(args[-0].m_data.num), (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_imagepsextendfont(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepsextendfont", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagepsslantfont(Value* font_index, double slant) asm("_ZN4HPHP18f_imagepsslantfontERKNS_6ObjectEd");

void fg1_imagepsslantfont(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepsslantfont(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagepsslantfont(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_imagepsslantfont(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagepsslantfont(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_imagepsslantfont(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepsslantfont", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_imagepstext(Value* _rv, Value* image, Value* text, Value* font, int size, int foreground, int background, int x, int y, int space, int tightness, double angle, int antialias_steps) asm("_ZN4HPHP13f_imagepstextERKNS_6ObjectERKNS_6StringES2_iiiiiiidi");

void fg1_imagepstext(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepstext(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfArray;
  fh_imagepstext(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (count > 8) ? (int)(args[-8].m_data.num) : (int)(0), (count > 9) ? (int)(args[-9].m_data.num) : (int)(0), (count > 10) ? (args[-10].m_data.dbl) : (double)(0.0), (count > 11) ? (int)(args[-11].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_imagepstext(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 8 && count <= 12) {
    if ((count <= 11 || (args - 11)->m_type == KindOfInt64) &&
        (count <= 10 || (args - 10)->m_type == KindOfDouble) &&
        (count <= 9 || (args - 9)->m_type == KindOfInt64) &&
        (count <= 8 || (args - 8)->m_type == KindOfInt64) &&
        (args - 7)->m_type == KindOfInt64 &&
        (args - 6)->m_type == KindOfInt64 &&
        (args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfObject &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_imagepstext(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num), (int)(args[-6].m_data.num), (int)(args[-7].m_data.num), (count > 8) ? (int)(args[-8].m_data.num) : (int)(0), (count > 9) ? (int)(args[-9].m_data.num) : (int)(0), (count > 10) ? (args[-10].m_data.dbl) : (double)(0.0), (count > 11) ? (int)(args[-11].m_data.num) : (int)(0));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_imagepstext(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepstext", count, 8, 12, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 12);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_imagepsbbox(Value* _rv, Value* text, int font, int size, int space, int tightness, double angle) asm("_ZN4HPHP13f_imagepsbboxERKNS_6StringEiiiid");

void fg1_imagepsbbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagepsbbox(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfArray;
  fh_imagepsbbox(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (args[-5].m_data.dbl) : (double)(0.0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_imagepsbbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 6) {
    if ((count <= 5 || (args - 5)->m_type == KindOfDouble) &&
        (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_imagepsbbox(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (args[-5].m_data.dbl) : (double)(0.0));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_imagepsbbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagepsbbox", count, 3, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_image2wbmp(Value* image, Value* filename, int threshold) asm("_ZN4HPHP12f_image2wbmpERKNS_6ObjectERKNS_6StringEi");

void fg1_image2wbmp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_image2wbmp(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_image2wbmp(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
}

TypedValue* fg_image2wbmp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_image2wbmp(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1))) ? 1LL : 0LL;
    } else {
      fg1_image2wbmp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("image2wbmp", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_jpeg2wbmp(Value* jpegname, Value* wbmpname, int dest_height, int dest_width, int threshold) asm("_ZN4HPHP11f_jpeg2wbmpERKNS_6StringES2_iii");

void fg1_jpeg2wbmp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_jpeg2wbmp(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_jpeg2wbmp(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_jpeg2wbmp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_jpeg2wbmp(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_jpeg2wbmp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("jpeg2wbmp", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_png2wbmp(Value* pngname, Value* wbmpname, int dest_height, int dest_width, int threshold) asm("_ZN4HPHP10f_png2wbmpERKNS_6StringES2_iii");

void fg1_png2wbmp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_png2wbmp(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_png2wbmp(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_png2wbmp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_png2wbmp(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_png2wbmp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("png2wbmp", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imagefilter(Value* image, int filtertype, int arg1, int arg2, int arg3, int arg4) asm("_ZN4HPHP13f_imagefilterERKNS_6ObjectEiiiii");

void fg1_imagefilter(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imagefilter(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imagefilter(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imagefilter(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 6) {
    if ((count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imagefilter(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_imagefilter(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imagefilter", count, 2, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imageconvolution(Value* image, Value* matrix, double div, double offset) asm("_ZN4HPHP18f_imageconvolutionERKNS_6ObjectERKNS_5ArrayEdd");

void fg1_imageconvolution(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imageconvolution(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imageconvolution(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_imageconvolution(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imageconvolution(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_imageconvolution(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imageconvolution", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imageantialias(Value* image, bool on) asm("_ZN4HPHP16f_imageantialiasERKNS_6ObjectEb");

void fg1_imageantialias(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imageantialias(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imageantialias(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imageantialias(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfBoolean &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imageantialias(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imageantialias(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imageantialias", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_iptcembed(TypedValue* _rv, Value* iptcdata, Value* jpeg_file_name, int spool) asm("_ZN4HPHP11f_iptcembedERKNS_6StringES2_i");

void fg1_iptcembed(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_iptcembed(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_iptcembed(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_iptcembed(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_iptcembed(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_iptcembed(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("iptcembed", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_iptcparse(TypedValue* _rv, Value* iptcblock) asm("_ZN4HPHP11f_iptcparseERKNS_6StringE");

void fg1_iptcparse(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_iptcparse(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_iptcparse(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_iptcparse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_iptcparse(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_iptcparse(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("iptcparse", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_exif_tagname(TypedValue* _rv, int index) asm("_ZN4HPHP14f_exif_tagnameEi");

void fg1_exif_tagname(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_exif_tagname(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_exif_tagname(rv, (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_exif_tagname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      fh_exif_tagname(rv, (int)(args[-0].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_exif_tagname(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("exif_tagname", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_exif_read_data(TypedValue* _rv, Value* filename, Value* sections, bool arrays, bool thumbnail) asm("_ZN4HPHP16f_exif_read_dataERKNS_6StringES2_bb");

void fg1_exif_read_data(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_exif_read_data(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_exif_read_data(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_exif_read_data(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_exif_read_data(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_exif_read_data(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("exif_read_data", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_read_exif_data(TypedValue* _rv, Value* filename, Value* sections, bool arrays, bool thumbnail) asm("_ZN4HPHP16f_read_exif_dataERKNS_6StringES2_bb");

void fg1_read_exif_data(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_read_exif_data(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_read_exif_data(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_read_exif_data(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_read_exif_data(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_read_exif_data(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("read_exif_data", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_exif_thumbnail(TypedValue* _rv, Value* filename, TypedValue* width, TypedValue* height, TypedValue* imagetype) asm("_ZN4HPHP16f_exif_thumbnailERKNS_6StringERKNS_14VRefParamValueES5_S5_");

void fg1_exif_thumbnail(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_exif_thumbnail(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = uninit_null();
  VRefParamValue defVal2 = uninit_null();
  VRefParamValue defVal3 = uninit_null();
  fh_exif_thumbnail(rv, &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_exif_thumbnail(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal1 = uninit_null();
      VRefParamValue defVal2 = uninit_null();
      VRefParamValue defVal3 = uninit_null();
      fh_exif_thumbnail(rv, &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_exif_thumbnail(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("exif_thumbnail", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_exif_imagetype(TypedValue* _rv, Value* filename) asm("_ZN4HPHP16f_exif_imagetypeERKNS_6StringE");

void fg1_exif_imagetype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_exif_imagetype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_exif_imagetype(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_exif_imagetype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_exif_imagetype(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_exif_imagetype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("exif_imagetype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
