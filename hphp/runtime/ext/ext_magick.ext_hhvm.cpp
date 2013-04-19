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

Value* fh_magickgetcopyright(Value* _rv) asm("_ZN4HPHP20f_magickgetcopyrightEv");

TypedValue* fg_magickgetcopyright(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_magickgetcopyright(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("magickgetcopyright", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgethomeurl(Value* _rv) asm("_ZN4HPHP18f_magickgethomeurlEv");

TypedValue* fg_magickgethomeurl(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_magickgethomeurl(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("magickgethomeurl", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetpackagename(Value* _rv) asm("_ZN4HPHP22f_magickgetpackagenameEv");

TypedValue* fg_magickgetpackagename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_magickgetpackagename(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("magickgetpackagename", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetquantumdepth() asm("_ZN4HPHP23f_magickgetquantumdepthEv");

TypedValue* fg_magickgetquantumdepth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfDouble;
    rv->m_data.dbl = fh_magickgetquantumdepth();
  } else {
    throw_toomany_arguments_nr("magickgetquantumdepth", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetreleasedate(Value* _rv) asm("_ZN4HPHP22f_magickgetreleasedateEv");

TypedValue* fg_magickgetreleasedate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_magickgetreleasedate(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("magickgetreleasedate", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetresourcelimit(int resource_type) asm("_ZN4HPHP24f_magickgetresourcelimitEi");

void fg1_magickgetresourcelimit(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetresourcelimit(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetresourcelimit((int)(args[-0].m_data.num));
}

TypedValue* fg_magickgetresourcelimit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetresourcelimit((int)(args[-0].m_data.num));
    } else {
      fg1_magickgetresourcelimit(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetresourcelimit", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetversion(Value* _rv) asm("_ZN4HPHP18f_magickgetversionEv");

TypedValue* fg_magickgetversion(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_magickgetversion(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("magickgetversion", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetversionnumber() asm("_ZN4HPHP24f_magickgetversionnumberEv");

TypedValue* fg_magickgetversionnumber(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_magickgetversionnumber();
  } else {
    throw_toomany_arguments_nr("magickgetversionnumber", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetversionstring(Value* _rv) asm("_ZN4HPHP24f_magickgetversionstringEv");

TypedValue* fg_magickgetversionstring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_magickgetversionstring(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("magickgetversionstring", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickqueryconfigureoption(Value* _rv, Value* option) asm("_ZN4HPHP28f_magickqueryconfigureoptionERKNS_6StringE");

void fg1_magickqueryconfigureoption(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickqueryconfigureoption(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickqueryconfigureoption(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickqueryconfigureoption(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_magickqueryconfigureoption(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickqueryconfigureoption(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickqueryconfigureoption", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickqueryconfigureoptions(Value* _rv, Value* pattern) asm("_ZN4HPHP29f_magickqueryconfigureoptionsERKNS_6StringE");

void fg1_magickqueryconfigureoptions(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickqueryconfigureoptions(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickqueryconfigureoptions(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickqueryconfigureoptions(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_magickqueryconfigureoptions(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickqueryconfigureoptions(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickqueryconfigureoptions", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickqueryfonts(Value* _rv, Value* pattern) asm("_ZN4HPHP18f_magickqueryfontsERKNS_6StringE");

void fg1_magickqueryfonts(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickqueryfonts(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickqueryfonts(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickqueryfonts(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_magickqueryfonts(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickqueryfonts(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickqueryfonts", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickqueryformats(Value* _rv, Value* pattern) asm("_ZN4HPHP20f_magickqueryformatsERKNS_6StringE");

void fg1_magickqueryformats(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickqueryformats(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickqueryformats(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickqueryformats(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_magickqueryformats(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickqueryformats(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickqueryformats", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetresourcelimit(int resource_type, double limit) asm("_ZN4HPHP24f_magicksetresourcelimitEid");

void fg1_magicksetresourcelimit(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetresourcelimit(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetresourcelimit((int)(args[-0].m_data.num), (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetresourcelimit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetresourcelimit((int)(args[-0].m_data.num), (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetresourcelimit(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetresourcelimit", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_newdrawingwand(Value* _rv) asm("_ZN4HPHP16f_newdrawingwandEv");

TypedValue* fg_newdrawingwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfObject;
    fh_newdrawingwand(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("newdrawingwand", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_newmagickwand(Value* _rv) asm("_ZN4HPHP15f_newmagickwandEv");

TypedValue* fg_newmagickwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfObject;
    fh_newmagickwand(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("newmagickwand", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_newpixeliterator(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP18f_newpixeliteratorERKNS_6ObjectE");

void fg1_newpixeliterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_newpixeliterator(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_newpixeliterator(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_newpixeliterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_newpixeliterator(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_newpixeliterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("newpixeliterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_newpixelregioniterator(Value* _rv, Value* mgck_wnd, int x, int y, int columns, int rows) asm("_ZN4HPHP24f_newpixelregioniteratorERKNS_6ObjectEiiii");

void fg1_newpixelregioniterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_newpixelregioniterator(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfObject;
  fh_newpixelregioniterator(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_newpixelregioniterator(ActRec* ar) {
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
      rv->m_type = KindOfObject;
      fh_newpixelregioniterator(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_newpixelregioniterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("newpixelregioniterator", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_newpixelwand(Value* _rv, Value* imagemagick_col_str) asm("_ZN4HPHP14f_newpixelwandERKNS_6StringE");

void fg1_newpixelwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_newpixelwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_newpixelwand(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_newpixelwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfObject;
      fh_newpixelwand(&(rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_newpixelwand(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("newpixelwand", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_newpixelwandarray(Value* _rv, int num_pxl_wnds) asm("_ZN4HPHP19f_newpixelwandarrayEi");

void fg1_newpixelwandarray(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_newpixelwandarray(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfArray;
  fh_newpixelwandarray(&(rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_newpixelwandarray(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfArray;
      fh_newpixelwandarray(&(rv->m_data), (int)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_newpixelwandarray(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("newpixelwandarray", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_newpixelwands(Value* _rv, int num_pxl_wnds) asm("_ZN4HPHP15f_newpixelwandsEi");

void fg1_newpixelwands(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_newpixelwands(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfArray;
  fh_newpixelwands(&(rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_newpixelwands(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfArray;
      fh_newpixelwands(&(rv->m_data), (int)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_newpixelwands(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("newpixelwands", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_destroydrawingwand(Value* drw_wnd) asm("_ZN4HPHP20f_destroydrawingwandERKNS_6ObjectE");

void fg1_destroydrawingwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_destroydrawingwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_destroydrawingwand(&args[-0].m_data);
}

TypedValue* fg_destroydrawingwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_destroydrawingwand(&args[-0].m_data);
    } else {
      fg1_destroydrawingwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("destroydrawingwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_destroymagickwand(Value* mgck_wnd) asm("_ZN4HPHP19f_destroymagickwandERKNS_6ObjectE");

void fg1_destroymagickwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_destroymagickwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_destroymagickwand(&args[-0].m_data);
}

TypedValue* fg_destroymagickwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_destroymagickwand(&args[-0].m_data);
    } else {
      fg1_destroymagickwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("destroymagickwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_destroypixeliterator(Value* pxl_iter) asm("_ZN4HPHP22f_destroypixeliteratorERKNS_6ObjectE");

void fg1_destroypixeliterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_destroypixeliterator(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_destroypixeliterator(&args[-0].m_data);
}

TypedValue* fg_destroypixeliterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_destroypixeliterator(&args[-0].m_data);
    } else {
      fg1_destroypixeliterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("destroypixeliterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_destroypixelwand(Value* pxl_wnd) asm("_ZN4HPHP18f_destroypixelwandERKNS_6ObjectE");

void fg1_destroypixelwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_destroypixelwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_destroypixelwand(&args[-0].m_data);
}

TypedValue* fg_destroypixelwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_destroypixelwand(&args[-0].m_data);
    } else {
      fg1_destroypixelwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("destroypixelwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_destroypixelwandarray(Value* pxl_wnd_array) asm("_ZN4HPHP23f_destroypixelwandarrayERKNS_5ArrayE");

void fg1_destroypixelwandarray(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_destroypixelwandarray(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_destroypixelwandarray(&args[-0].m_data);
}

TypedValue* fg_destroypixelwandarray(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfArray) {
      rv->m_type = KindOfNull;
      fh_destroypixelwandarray(&args[-0].m_data);
    } else {
      fg1_destroypixelwandarray(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("destroypixelwandarray", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_destroypixelwands(Value* pxl_wnd_array) asm("_ZN4HPHP19f_destroypixelwandsERKNS_5ArrayE");

void fg1_destroypixelwands(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_destroypixelwands(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_destroypixelwands(&args[-0].m_data);
}

TypedValue* fg_destroypixelwands(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfArray) {
      rv->m_type = KindOfNull;
      fh_destroypixelwands(&args[-0].m_data);
    } else {
      fg1_destroypixelwands(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("destroypixelwands", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_isdrawingwand(TypedValue* var) asm("_ZN4HPHP15f_isdrawingwandERKNS_7VariantE");

TypedValue* fg_isdrawingwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_isdrawingwand((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("isdrawingwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_ismagickwand(TypedValue* var) asm("_ZN4HPHP14f_ismagickwandERKNS_7VariantE");

TypedValue* fg_ismagickwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_ismagickwand((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("ismagickwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_ispixeliterator(TypedValue* var) asm("_ZN4HPHP17f_ispixeliteratorERKNS_7VariantE");

TypedValue* fg_ispixeliterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_ispixeliterator((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("ispixeliterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_ispixelwand(TypedValue* var) asm("_ZN4HPHP13f_ispixelwandERKNS_7VariantE");

TypedValue* fg_ispixelwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_ispixelwand((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("ispixelwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_cleardrawingwand(Value* drw_wnd) asm("_ZN4HPHP18f_cleardrawingwandERKNS_6ObjectE");

void fg1_cleardrawingwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_cleardrawingwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_cleardrawingwand(&args[-0].m_data);
}

TypedValue* fg_cleardrawingwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_cleardrawingwand(&args[-0].m_data);
    } else {
      fg1_cleardrawingwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("cleardrawingwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_clearmagickwand(Value* mgck_wnd) asm("_ZN4HPHP17f_clearmagickwandERKNS_6ObjectE");

void fg1_clearmagickwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_clearmagickwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_clearmagickwand(&args[-0].m_data);
}

TypedValue* fg_clearmagickwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_clearmagickwand(&args[-0].m_data);
    } else {
      fg1_clearmagickwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("clearmagickwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_clearpixeliterator(Value* pxl_iter) asm("_ZN4HPHP20f_clearpixeliteratorERKNS_6ObjectE");

void fg1_clearpixeliterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_clearpixeliterator(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_clearpixeliterator(&args[-0].m_data);
}

TypedValue* fg_clearpixeliterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_clearpixeliterator(&args[-0].m_data);
    } else {
      fg1_clearpixeliterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("clearpixeliterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_clearpixelwand(Value* pxl_wnd) asm("_ZN4HPHP16f_clearpixelwandERKNS_6ObjectE");

void fg1_clearpixelwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_clearpixelwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_clearpixelwand(&args[-0].m_data);
}

TypedValue* fg_clearpixelwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_clearpixelwand(&args[-0].m_data);
    } else {
      fg1_clearpixelwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("clearpixelwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_clonedrawingwand(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_clonedrawingwandERKNS_6ObjectE");

void fg1_clonedrawingwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_clonedrawingwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_clonedrawingwand(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_clonedrawingwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_clonedrawingwand(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_clonedrawingwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("clonedrawingwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_clonemagickwand(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP17f_clonemagickwandERKNS_6ObjectE");

void fg1_clonemagickwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_clonemagickwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_clonemagickwand(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_clonemagickwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_clonemagickwand(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_clonemagickwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("clonemagickwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_wandgetexception(Value* _rv, Value* wnd) asm("_ZN4HPHP18f_wandgetexceptionERKNS_6ObjectE");

void fg1_wandgetexception(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_wandgetexception(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_wandgetexception(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_wandgetexception(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_wandgetexception(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_wandgetexception(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("wandgetexception", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_wandgetexceptionstring(Value* _rv, Value* wnd) asm("_ZN4HPHP24f_wandgetexceptionstringERKNS_6ObjectE");

void fg1_wandgetexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_wandgetexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_wandgetexceptionstring(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_wandgetexceptionstring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_wandgetexceptionstring(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_wandgetexceptionstring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("wandgetexceptionstring", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_wandgetexceptiontype(Value* wnd) asm("_ZN4HPHP22f_wandgetexceptiontypeERKNS_6ObjectE");

void fg1_wandgetexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_wandgetexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_wandgetexceptiontype(&args[-0].m_data);
}

TypedValue* fg_wandgetexceptiontype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_wandgetexceptiontype(&args[-0].m_data);
    } else {
      fg1_wandgetexceptiontype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("wandgetexceptiontype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_wandhasexception(Value* wnd) asm("_ZN4HPHP18f_wandhasexceptionERKNS_6ObjectE");

void fg1_wandhasexception(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_wandhasexception(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_wandhasexception(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_wandhasexception(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_wandhasexception(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_wandhasexception(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("wandhasexception", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawaffine(Value* drw_wnd, double sx, double sy, double rx, double ry, double tx, double ty) asm("_ZN4HPHP12f_drawaffineERKNS_6ObjectEdddddd");

void fg1_drawaffine(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawaffine(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawaffine(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
}

TypedValue* fg_drawaffine(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 7) {
    if ((args - 6)->m_type == KindOfDouble &&
        (args - 5)->m_type == KindOfDouble &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawaffine(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
    } else {
      fg1_drawaffine(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawaffine", count, 7, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawannotation(Value* drw_wnd, double x, double y, Value* text) asm("_ZN4HPHP16f_drawannotationERKNS_6ObjectEddRKNS_6StringE");

void fg1_drawannotation(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawannotation(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawannotation(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), &args[-3].m_data);
}

TypedValue* fg_drawannotation(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawannotation(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), &args[-3].m_data);
    } else {
      fg1_drawannotation(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawannotation", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawarc(Value* drw_wnd, double sx, double sy, double ex, double ey, double sd, double ed) asm("_ZN4HPHP9f_drawarcERKNS_6ObjectEdddddd");

void fg1_drawarc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawarc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawarc(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
}

TypedValue* fg_drawarc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 7) {
    if ((args - 6)->m_type == KindOfDouble &&
        (args - 5)->m_type == KindOfDouble &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawarc(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
    } else {
      fg1_drawarc(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawarc", count, 7, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawbezier(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP12f_drawbezierERKNS_6ObjectERKNS_5ArrayE");

void fg1_drawbezier(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawbezier(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawbezier(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawbezier(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawbezier(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawbezier(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawbezier", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawcircle(Value* drw_wnd, double ox, double oy, double px, double py) asm("_ZN4HPHP12f_drawcircleERKNS_6ObjectEdddd");

void fg1_drawcircle(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawcircle(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawcircle(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
}

TypedValue* fg_drawcircle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawcircle(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
    } else {
      fg1_drawcircle(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawcircle", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawcolor(Value* drw_wnd, double x, double y, int paint_method) asm("_ZN4HPHP11f_drawcolorERKNS_6ObjectEddi");

void fg1_drawcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  fh_drawcolor(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num));
}

TypedValue* fg_drawcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawcolor(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num));
    } else {
      fg1_drawcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawcolor", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawcomment(Value* drw_wnd, Value* comment) asm("_ZN4HPHP13f_drawcommentERKNS_6ObjectERKNS_6StringE");

void fg1_drawcomment(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawcomment(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawcomment(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawcomment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawcomment(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawcomment(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawcomment", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawcomposite(Value* drw_wnd, int composite_operator, double x, double y, double width, double height, Value* mgck_wnd) asm("_ZN4HPHP15f_drawcompositeERKNS_6ObjectEiddddS2_");

void fg1_drawcomposite(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawcomposite(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-6)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawcomposite(&args[-0].m_data, (int)(args[-1].m_data.num), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), &args[-6].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawcomposite(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 7) {
    if ((args - 6)->m_type == KindOfObject &&
        (args - 5)->m_type == KindOfDouble &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawcomposite(&args[-0].m_data, (int)(args[-1].m_data.num), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), &args[-6].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawcomposite(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawcomposite", count, 7, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawellipse(Value* drw_wnd, double ox, double oy, double rx, double ry, double start, double end) asm("_ZN4HPHP13f_drawellipseERKNS_6ObjectEdddddd");

void fg1_drawellipse(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawellipse(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawellipse(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
}

TypedValue* fg_drawellipse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 7) {
    if ((args - 6)->m_type == KindOfDouble &&
        (args - 5)->m_type == KindOfDouble &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawellipse(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
    } else {
      fg1_drawellipse(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawellipse", count, 7, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetclippath(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP17f_drawgetclippathERKNS_6ObjectE");

void fg1_drawgetclippath(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetclippath(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_drawgetclippath(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetclippath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_drawgetclippath(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetclippath(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetclippath", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetcliprule(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetclipruleERKNS_6ObjectE");

void fg1_drawgetcliprule(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetcliprule(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetcliprule(&args[-0].m_data);
}

TypedValue* fg_drawgetcliprule(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetcliprule(&args[-0].m_data);
    } else {
      fg1_drawgetcliprule(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetcliprule", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetclipunits(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetclipunitsERKNS_6ObjectE");

void fg1_drawgetclipunits(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetclipunits(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetclipunits(&args[-0].m_data);
}

TypedValue* fg_drawgetclipunits(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetclipunits(&args[-0].m_data);
    } else {
      fg1_drawgetclipunits(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetclipunits", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetexception(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_drawgetexceptionERKNS_6ObjectE");

void fg1_drawgetexception(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetexception(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_drawgetexception(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetexception(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_drawgetexception(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetexception(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetexception", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetexceptionstring(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP24f_drawgetexceptionstringERKNS_6ObjectE");

void fg1_drawgetexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_drawgetexceptionstring(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetexceptionstring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_drawgetexceptionstring(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetexceptionstring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetexceptionstring", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetexceptiontype(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetexceptiontypeERKNS_6ObjectE");

void fg1_drawgetexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetexceptiontype(&args[-0].m_data);
}

TypedValue* fg_drawgetexceptiontype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetexceptiontype(&args[-0].m_data);
    } else {
      fg1_drawgetexceptiontype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetexceptiontype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetfillalpha(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfillalphaERKNS_6ObjectE");

void fg1_drawgetfillalpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfillalpha(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetfillalpha(&args[-0].m_data);
}

TypedValue* fg_drawgetfillalpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetfillalpha(&args[-0].m_data);
    } else {
      fg1_drawgetfillalpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfillalpha", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetfillcolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfillcolorERKNS_6ObjectE");

void fg1_drawgetfillcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfillcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_drawgetfillcolor(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetfillcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_drawgetfillcolor(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetfillcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfillcolor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetfillopacity(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetfillopacityERKNS_6ObjectE");

void fg1_drawgetfillopacity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfillopacity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetfillopacity(&args[-0].m_data);
}

TypedValue* fg_drawgetfillopacity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetfillopacity(&args[-0].m_data);
    } else {
      fg1_drawgetfillopacity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfillopacity", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetfillrule(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetfillruleERKNS_6ObjectE");

void fg1_drawgetfillrule(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfillrule(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetfillrule(&args[-0].m_data);
}

TypedValue* fg_drawgetfillrule(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetfillrule(&args[-0].m_data);
    } else {
      fg1_drawgetfillrule(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfillrule", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetfont(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP13f_drawgetfontERKNS_6ObjectE");

void fg1_drawgetfont(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfont(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_drawgetfont(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetfont(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_drawgetfont(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetfont(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfont", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetfontfamily(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP19f_drawgetfontfamilyERKNS_6ObjectE");

void fg1_drawgetfontfamily(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfontfamily(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_drawgetfontfamily(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetfontfamily(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_drawgetfontfamily(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetfontfamily(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfontfamily", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetfontsize(Value* drw_wnd) asm("_ZN4HPHP17f_drawgetfontsizeERKNS_6ObjectE");

void fg1_drawgetfontsize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfontsize(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetfontsize(&args[-0].m_data);
}

TypedValue* fg_drawgetfontsize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetfontsize(&args[-0].m_data);
    } else {
      fg1_drawgetfontsize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfontsize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetfontstretch(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetfontstretchERKNS_6ObjectE");

void fg1_drawgetfontstretch(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfontstretch(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetfontstretch(&args[-0].m_data);
}

TypedValue* fg_drawgetfontstretch(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetfontstretch(&args[-0].m_data);
    } else {
      fg1_drawgetfontstretch(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfontstretch", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetfontstyle(Value* drw_wnd) asm("_ZN4HPHP18f_drawgetfontstyleERKNS_6ObjectE");

void fg1_drawgetfontstyle(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfontstyle(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetfontstyle(&args[-0].m_data);
}

TypedValue* fg_drawgetfontstyle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetfontstyle(&args[-0].m_data);
    } else {
      fg1_drawgetfontstyle(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfontstyle", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetfontweight(Value* drw_wnd) asm("_ZN4HPHP19f_drawgetfontweightERKNS_6ObjectE");

void fg1_drawgetfontweight(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetfontweight(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetfontweight(&args[-0].m_data);
}

TypedValue* fg_drawgetfontweight(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetfontweight(&args[-0].m_data);
    } else {
      fg1_drawgetfontweight(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetfontweight", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetgravity(Value* drw_wnd) asm("_ZN4HPHP16f_drawgetgravityERKNS_6ObjectE");

void fg1_drawgetgravity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetgravity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetgravity(&args[-0].m_data);
}

TypedValue* fg_drawgetgravity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetgravity(&args[-0].m_data);
    } else {
      fg1_drawgetgravity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetgravity", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetstrokealpha(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokealphaERKNS_6ObjectE");

void fg1_drawgetstrokealpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokealpha(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetstrokealpha(&args[-0].m_data);
}

TypedValue* fg_drawgetstrokealpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetstrokealpha(&args[-0].m_data);
    } else {
      fg1_drawgetstrokealpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokealpha", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawgetstrokeantialias(Value* drw_wnd) asm("_ZN4HPHP24f_drawgetstrokeantialiasERKNS_6ObjectE");

void fg1_drawgetstrokeantialias(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokeantialias(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawgetstrokeantialias(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawgetstrokeantialias(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawgetstrokeantialias(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawgetstrokeantialias(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokeantialias", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetstrokecolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokecolorERKNS_6ObjectE");

void fg1_drawgetstrokecolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokecolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_drawgetstrokecolor(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetstrokecolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_drawgetstrokecolor(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetstrokecolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokecolor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetstrokedasharray(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP24f_drawgetstrokedasharrayERKNS_6ObjectE");

void fg1_drawgetstrokedasharray(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokedasharray(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_drawgetstrokedasharray(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetstrokedasharray(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_drawgetstrokedasharray(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetstrokedasharray(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokedasharray", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetstrokedashoffset(Value* drw_wnd) asm("_ZN4HPHP25f_drawgetstrokedashoffsetERKNS_6ObjectE");

void fg1_drawgetstrokedashoffset(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokedashoffset(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetstrokedashoffset(&args[-0].m_data);
}

TypedValue* fg_drawgetstrokedashoffset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetstrokedashoffset(&args[-0].m_data);
    } else {
      fg1_drawgetstrokedashoffset(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokedashoffset", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetstrokelinecap(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetstrokelinecapERKNS_6ObjectE");

void fg1_drawgetstrokelinecap(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokelinecap(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetstrokelinecap(&args[-0].m_data);
}

TypedValue* fg_drawgetstrokelinecap(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetstrokelinecap(&args[-0].m_data);
    } else {
      fg1_drawgetstrokelinecap(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokelinecap", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgetstrokelinejoin(Value* drw_wnd) asm("_ZN4HPHP23f_drawgetstrokelinejoinERKNS_6ObjectE");

void fg1_drawgetstrokelinejoin(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokelinejoin(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgetstrokelinejoin(&args[-0].m_data);
}

TypedValue* fg_drawgetstrokelinejoin(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgetstrokelinejoin(&args[-0].m_data);
    } else {
      fg1_drawgetstrokelinejoin(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokelinejoin", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetstrokemiterlimit(Value* drw_wnd) asm("_ZN4HPHP25f_drawgetstrokemiterlimitERKNS_6ObjectE");

void fg1_drawgetstrokemiterlimit(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokemiterlimit(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetstrokemiterlimit(&args[-0].m_data);
}

TypedValue* fg_drawgetstrokemiterlimit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetstrokemiterlimit(&args[-0].m_data);
    } else {
      fg1_drawgetstrokemiterlimit(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokemiterlimit", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetstrokeopacity(Value* drw_wnd) asm("_ZN4HPHP22f_drawgetstrokeopacityERKNS_6ObjectE");

void fg1_drawgetstrokeopacity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokeopacity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetstrokeopacity(&args[-0].m_data);
}

TypedValue* fg_drawgetstrokeopacity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetstrokeopacity(&args[-0].m_data);
    } else {
      fg1_drawgetstrokeopacity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokeopacity", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_drawgetstrokewidth(Value* drw_wnd) asm("_ZN4HPHP20f_drawgetstrokewidthERKNS_6ObjectE");

void fg1_drawgetstrokewidth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetstrokewidth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_drawgetstrokewidth(&args[-0].m_data);
}

TypedValue* fg_drawgetstrokewidth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_drawgetstrokewidth(&args[-0].m_data);
    } else {
      fg1_drawgetstrokewidth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetstrokewidth", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgettextalignment(Value* drw_wnd) asm("_ZN4HPHP22f_drawgettextalignmentERKNS_6ObjectE");

void fg1_drawgettextalignment(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgettextalignment(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgettextalignment(&args[-0].m_data);
}

TypedValue* fg_drawgettextalignment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgettextalignment(&args[-0].m_data);
    } else {
      fg1_drawgettextalignment(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgettextalignment", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawgettextantialias(Value* drw_wnd) asm("_ZN4HPHP22f_drawgettextantialiasERKNS_6ObjectE");

void fg1_drawgettextantialias(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgettextantialias(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawgettextantialias(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawgettextantialias(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawgettextantialias(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawgettextantialias(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgettextantialias", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_drawgettextdecoration(Value* drw_wnd) asm("_ZN4HPHP23f_drawgettextdecorationERKNS_6ObjectE");

void fg1_drawgettextdecoration(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgettextdecoration(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_drawgettextdecoration(&args[-0].m_data);
}

TypedValue* fg_drawgettextdecoration(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_drawgettextdecoration(&args[-0].m_data);
    } else {
      fg1_drawgettextdecoration(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgettextdecoration", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgettextencoding(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP21f_drawgettextencodingERKNS_6ObjectE");

void fg1_drawgettextencoding(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgettextencoding(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_drawgettextencoding(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgettextencoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_drawgettextencoding(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgettextencoding(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgettextencoding", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgettextundercolor(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP23f_drawgettextundercolorERKNS_6ObjectE");

void fg1_drawgettextundercolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgettextundercolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_drawgettextundercolor(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgettextundercolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_drawgettextundercolor(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgettextundercolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgettextundercolor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_drawgetvectorgraphics(Value* _rv, Value* drw_wnd) asm("_ZN4HPHP23f_drawgetvectorgraphicsERKNS_6ObjectE");

void fg1_drawgetvectorgraphics(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawgetvectorgraphics(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_drawgetvectorgraphics(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_drawgetvectorgraphics(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_drawgetvectorgraphics(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_drawgetvectorgraphics(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawgetvectorgraphics", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawline(Value* drw_wnd, double sx, double sy, double ex, double ey) asm("_ZN4HPHP10f_drawlineERKNS_6ObjectEdddd");

void fg1_drawline(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawline(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawline(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
}

TypedValue* fg_drawline(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawline(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
    } else {
      fg1_drawline(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawline", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawmatte(Value* drw_wnd, double x, double y, int paint_method) asm("_ZN4HPHP11f_drawmatteERKNS_6ObjectEddi");

void fg1_drawmatte(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawmatte(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfNull;
  fh_drawmatte(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num));
}

TypedValue* fg_drawmatte(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawmatte(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num));
    } else {
      fg1_drawmatte(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawmatte", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathclose(Value* drw_wnd) asm("_ZN4HPHP15f_drawpathcloseERKNS_6ObjectE");

void fg1_drawpathclose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathclose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_drawpathclose(&args[-0].m_data);
}

TypedValue* fg_drawpathclose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathclose(&args[-0].m_data);
    } else {
      fg1_drawpathclose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathclose", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathcurvetoabsolute(Value* drw_wnd, double x1, double y1, double x2, double y2, double x, double y) asm("_ZN4HPHP25f_drawpathcurvetoabsoluteERKNS_6ObjectEdddddd");

void fg1_drawpathcurvetoabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathcurvetoabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawpathcurvetoabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
}

TypedValue* fg_drawpathcurvetoabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 7) {
    if ((args - 6)->m_type == KindOfDouble &&
        (args - 5)->m_type == KindOfDouble &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathcurvetoabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
    } else {
      fg1_drawpathcurvetoabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathcurvetoabsolute", count, 7, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathcurvetoquadraticbezierabsolute(Value* drw_wnd, double x1, double y1, double x, double y) asm("_ZN4HPHP40f_drawpathcurvetoquadraticbezierabsoluteERKNS_6ObjectEdddd");

void fg1_drawpathcurvetoquadraticbezierabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathcurvetoquadraticbezierabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawpathcurvetoquadraticbezierabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
}

TypedValue* fg_drawpathcurvetoquadraticbezierabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathcurvetoquadraticbezierabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
    } else {
      fg1_drawpathcurvetoquadraticbezierabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathcurvetoquadraticbezierabsolute", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathcurvetoquadraticbezierrelative(Value* drw_wnd, double x1, double y1, double x, double y) asm("_ZN4HPHP40f_drawpathcurvetoquadraticbezierrelativeERKNS_6ObjectEdddd");

void fg1_drawpathcurvetoquadraticbezierrelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathcurvetoquadraticbezierrelative(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawpathcurvetoquadraticbezierrelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
}

TypedValue* fg_drawpathcurvetoquadraticbezierrelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathcurvetoquadraticbezierrelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
    } else {
      fg1_drawpathcurvetoquadraticbezierrelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathcurvetoquadraticbezierrelative", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathcurvetoquadraticbeziersmoothabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothabsoluteERKNS_6ObjectEdd");

void fg1_drawpathcurvetoquadraticbeziersmoothabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathcurvetoquadraticbeziersmoothabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawpathcurvetoquadraticbeziersmoothabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawpathcurvetoquadraticbeziersmoothabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathcurvetoquadraticbeziersmoothabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawpathcurvetoquadraticbeziersmoothabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathcurvetoquadraticbeziersmoothabsolute", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathcurvetoquadraticbeziersmoothrelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP46f_drawpathcurvetoquadraticbeziersmoothrelativeERKNS_6ObjectEdd");

void fg1_drawpathcurvetoquadraticbeziersmoothrelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathcurvetoquadraticbeziersmoothrelative(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawpathcurvetoquadraticbeziersmoothrelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawpathcurvetoquadraticbeziersmoothrelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathcurvetoquadraticbeziersmoothrelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawpathcurvetoquadraticbeziersmoothrelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathcurvetoquadraticbeziersmoothrelative", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathcurvetorelative(Value* drw_wnd, double x1, double y1, double x2, double y2, double x, double y) asm("_ZN4HPHP25f_drawpathcurvetorelativeERKNS_6ObjectEdddddd");

void fg1_drawpathcurvetorelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathcurvetorelative(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawpathcurvetorelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
}

TypedValue* fg_drawpathcurvetorelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 7) {
    if ((args - 6)->m_type == KindOfDouble &&
        (args - 5)->m_type == KindOfDouble &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathcurvetorelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
    } else {
      fg1_drawpathcurvetorelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathcurvetorelative", count, 7, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathcurvetosmoothabsolute(Value* drw_wnd, double x2, double y2, double x, double y) asm("_ZN4HPHP31f_drawpathcurvetosmoothabsoluteERKNS_6ObjectEdddd");

void fg1_drawpathcurvetosmoothabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathcurvetosmoothabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawpathcurvetosmoothabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
}

TypedValue* fg_drawpathcurvetosmoothabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathcurvetosmoothabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
    } else {
      fg1_drawpathcurvetosmoothabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathcurvetosmoothabsolute", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathcurvetosmoothrelative(Value* drw_wnd, double x2, double y2, double x, double y) asm("_ZN4HPHP31f_drawpathcurvetosmoothrelativeERKNS_6ObjectEdddd");

void fg1_drawpathcurvetosmoothrelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathcurvetosmoothrelative(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawpathcurvetosmoothrelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
}

TypedValue* fg_drawpathcurvetosmoothrelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathcurvetosmoothrelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
    } else {
      fg1_drawpathcurvetosmoothrelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathcurvetosmoothrelative", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathellipticarcabsolute(Value* drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) asm("_ZN4HPHP29f_drawpathellipticarcabsoluteERKNS_6ObjectEdddbbdd");

void fg1_drawpathellipticarcabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathellipticarcabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-7)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-7);
  }
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawpathellipticarcabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num), (args[-6].m_data.dbl), (args[-7].m_data.dbl));
}

TypedValue* fg_drawpathellipticarcabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 8) {
    if ((args - 7)->m_type == KindOfDouble &&
        (args - 6)->m_type == KindOfDouble &&
        (args - 5)->m_type == KindOfBoolean &&
        (args - 4)->m_type == KindOfBoolean &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathellipticarcabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num), (args[-6].m_data.dbl), (args[-7].m_data.dbl));
    } else {
      fg1_drawpathellipticarcabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathellipticarcabsolute", count, 8, 8, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 8);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathellipticarcrelative(Value* drw_wnd, double rx, double ry, double x_axis_rotation, bool large_arc_flag, bool sweep_flag, double x, double y) asm("_ZN4HPHP29f_drawpathellipticarcrelativeERKNS_6ObjectEdddbbdd");

void fg1_drawpathellipticarcrelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathellipticarcrelative(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-7)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-7);
  }
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawpathellipticarcrelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num), (args[-6].m_data.dbl), (args[-7].m_data.dbl));
}

TypedValue* fg_drawpathellipticarcrelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 8) {
    if ((args - 7)->m_type == KindOfDouble &&
        (args - 6)->m_type == KindOfDouble &&
        (args - 5)->m_type == KindOfBoolean &&
        (args - 4)->m_type == KindOfBoolean &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathellipticarcrelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num), (args[-6].m_data.dbl), (args[-7].m_data.dbl));
    } else {
      fg1_drawpathellipticarcrelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathellipticarcrelative", count, 8, 8, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 8);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathfinish(Value* drw_wnd) asm("_ZN4HPHP16f_drawpathfinishERKNS_6ObjectE");

void fg1_drawpathfinish(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathfinish(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_drawpathfinish(&args[-0].m_data);
}

TypedValue* fg_drawpathfinish(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathfinish(&args[-0].m_data);
    } else {
      fg1_drawpathfinish(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathfinish", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathlinetoabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathlinetoabsoluteERKNS_6ObjectEdd");

void fg1_drawpathlinetoabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathlinetoabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawpathlinetoabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawpathlinetoabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathlinetoabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawpathlinetoabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathlinetoabsolute", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathlinetohorizontalabsolute(Value* drw_wnd, double x) asm("_ZN4HPHP34f_drawpathlinetohorizontalabsoluteERKNS_6ObjectEd");

void fg1_drawpathlinetohorizontalabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathlinetohorizontalabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawpathlinetohorizontalabsolute(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawpathlinetohorizontalabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathlinetohorizontalabsolute(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawpathlinetohorizontalabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathlinetohorizontalabsolute", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathlinetohorizontalrelative(Value* drw_wnd, double x) asm("_ZN4HPHP34f_drawpathlinetohorizontalrelativeERKNS_6ObjectEd");

void fg1_drawpathlinetohorizontalrelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathlinetohorizontalrelative(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawpathlinetohorizontalrelative(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawpathlinetohorizontalrelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathlinetohorizontalrelative(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawpathlinetohorizontalrelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathlinetohorizontalrelative", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathlinetorelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathlinetorelativeERKNS_6ObjectEdd");

void fg1_drawpathlinetorelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathlinetorelative(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawpathlinetorelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawpathlinetorelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathlinetorelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawpathlinetorelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathlinetorelative", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathlinetoverticalabsolute(Value* drw_wnd, double y) asm("_ZN4HPHP32f_drawpathlinetoverticalabsoluteERKNS_6ObjectEd");

void fg1_drawpathlinetoverticalabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathlinetoverticalabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawpathlinetoverticalabsolute(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawpathlinetoverticalabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathlinetoverticalabsolute(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawpathlinetoverticalabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathlinetoverticalabsolute", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathlinetoverticalrelative(Value* drw_wnd, double y) asm("_ZN4HPHP32f_drawpathlinetoverticalrelativeERKNS_6ObjectEd");

void fg1_drawpathlinetoverticalrelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathlinetoverticalrelative(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawpathlinetoverticalrelative(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawpathlinetoverticalrelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathlinetoverticalrelative(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawpathlinetoverticalrelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathlinetoverticalrelative", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathmovetoabsolute(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathmovetoabsoluteERKNS_6ObjectEdd");

void fg1_drawpathmovetoabsolute(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathmovetoabsolute(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawpathmovetoabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawpathmovetoabsolute(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathmovetoabsolute(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawpathmovetoabsolute(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathmovetoabsolute", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathmovetorelative(Value* drw_wnd, double x, double y) asm("_ZN4HPHP24f_drawpathmovetorelativeERKNS_6ObjectEdd");

void fg1_drawpathmovetorelative(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathmovetorelative(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawpathmovetorelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawpathmovetorelative(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathmovetorelative(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawpathmovetorelative(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathmovetorelative", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpathstart(Value* drw_wnd) asm("_ZN4HPHP15f_drawpathstartERKNS_6ObjectE");

void fg1_drawpathstart(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpathstart(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_drawpathstart(&args[-0].m_data);
}

TypedValue* fg_drawpathstart(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpathstart(&args[-0].m_data);
    } else {
      fg1_drawpathstart(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpathstart", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpoint(Value* drw_wnd, double x, double y) asm("_ZN4HPHP11f_drawpointERKNS_6ObjectEdd");

void fg1_drawpoint(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpoint(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawpoint(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawpoint(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpoint(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawpoint(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpoint", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpolygon(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP13f_drawpolygonERKNS_6ObjectERKNS_5ArrayE");

void fg1_drawpolygon(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpolygon(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawpolygon(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawpolygon(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpolygon(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawpolygon(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpolygon", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpolyline(Value* drw_wnd, Value* x_y_points_array) asm("_ZN4HPHP14f_drawpolylineERKNS_6ObjectERKNS_5ArrayE");

void fg1_drawpolyline(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpolyline(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawpolyline(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawpolyline(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpolyline(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawpolyline(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpolyline", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawrectangle(Value* drw_wnd, double x1, double y1, double x2, double y2) asm("_ZN4HPHP15f_drawrectangleERKNS_6ObjectEdddd");

void fg1_drawrectangle(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawrectangle(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawrectangle(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
}

TypedValue* fg_drawrectangle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawrectangle(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
    } else {
      fg1_drawrectangle(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawrectangle", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawrender(Value* drw_wnd) asm("_ZN4HPHP12f_drawrenderERKNS_6ObjectE");

void fg1_drawrender(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawrender(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawrender(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawrender(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawrender(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawrender(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawrender", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawrotate(Value* drw_wnd, double degrees) asm("_ZN4HPHP12f_drawrotateERKNS_6ObjectEd");

void fg1_drawrotate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawrotate(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawrotate(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawrotate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawrotate(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawrotate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawrotate", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawroundrectangle(Value* drw_wnd, double x1, double y1, double x2, double y2, double rx, double ry) asm("_ZN4HPHP20f_drawroundrectangleERKNS_6ObjectEdddddd");

void fg1_drawroundrectangle(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawroundrectangle(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-6)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-6);
  }
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawroundrectangle(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
}

TypedValue* fg_drawroundrectangle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 7) {
    if ((args - 6)->m_type == KindOfDouble &&
        (args - 5)->m_type == KindOfDouble &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawroundrectangle(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl), (args[-6].m_data.dbl));
    } else {
      fg1_drawroundrectangle(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawroundrectangle", count, 7, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawscale(Value* drw_wnd, double x, double y) asm("_ZN4HPHP11f_drawscaleERKNS_6ObjectEdd");

void fg1_drawscale(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawscale(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawscale(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawscale(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawscale(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawscale(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawscale", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawsetclippath(Value* drw_wnd, Value* clip_path) asm("_ZN4HPHP17f_drawsetclippathERKNS_6ObjectERKNS_6StringE");

void fg1_drawsetclippath(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetclippath(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawsetclippath(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawsetclippath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawsetclippath(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawsetclippath(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetclippath", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetcliprule(Value* drw_wnd, int fill_rule) asm("_ZN4HPHP17f_drawsetclipruleERKNS_6ObjectEi");

void fg1_drawsetcliprule(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetcliprule(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetcliprule(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsetcliprule(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetcliprule(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsetcliprule(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetcliprule", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetclipunits(Value* drw_wnd, int clip_path_units) asm("_ZN4HPHP18f_drawsetclipunitsERKNS_6ObjectEi");

void fg1_drawsetclipunits(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetclipunits(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetclipunits(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsetclipunits(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetclipunits(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsetclipunits(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetclipunits", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetfillalpha(Value* drw_wnd, double fill_opacity) asm("_ZN4HPHP18f_drawsetfillalphaERKNS_6ObjectEd");

void fg1_drawsetfillalpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfillalpha(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetfillalpha(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetfillalpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetfillalpha(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetfillalpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfillalpha", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetfillcolor(Value* drw_wnd, Value* fill_pxl_wnd) asm("_ZN4HPHP18f_drawsetfillcolorERKNS_6ObjectES2_");

void fg1_drawsetfillcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfillcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetfillcolor(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawsetfillcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetfillcolor(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawsetfillcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfillcolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetfillopacity(Value* drw_wnd, double fill_opacity) asm("_ZN4HPHP20f_drawsetfillopacityERKNS_6ObjectEd");

void fg1_drawsetfillopacity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfillopacity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetfillopacity(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetfillopacity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetfillopacity(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetfillopacity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfillopacity", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawsetfillpatternurl(Value* drw_wnd, Value* fill_url) asm("_ZN4HPHP23f_drawsetfillpatternurlERKNS_6ObjectERKNS_6StringE");

void fg1_drawsetfillpatternurl(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfillpatternurl(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawsetfillpatternurl(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawsetfillpatternurl(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawsetfillpatternurl(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawsetfillpatternurl(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfillpatternurl", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetfillrule(Value* drw_wnd, int fill_rule) asm("_ZN4HPHP17f_drawsetfillruleERKNS_6ObjectEi");

void fg1_drawsetfillrule(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfillrule(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetfillrule(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsetfillrule(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetfillrule(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsetfillrule(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfillrule", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawsetfont(Value* drw_wnd, Value* font_file) asm("_ZN4HPHP13f_drawsetfontERKNS_6ObjectERKNS_6StringE");

void fg1_drawsetfont(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfont(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawsetfont(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawsetfont(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawsetfont(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawsetfont(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfont", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawsetfontfamily(Value* drw_wnd, Value* font_family) asm("_ZN4HPHP19f_drawsetfontfamilyERKNS_6ObjectERKNS_6StringE");

void fg1_drawsetfontfamily(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfontfamily(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawsetfontfamily(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawsetfontfamily(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawsetfontfamily(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawsetfontfamily(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfontfamily", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetfontsize(Value* drw_wnd, double pointsize) asm("_ZN4HPHP17f_drawsetfontsizeERKNS_6ObjectEd");

void fg1_drawsetfontsize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfontsize(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetfontsize(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetfontsize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetfontsize(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetfontsize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfontsize", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetfontstretch(Value* drw_wnd, int stretch_type) asm("_ZN4HPHP20f_drawsetfontstretchERKNS_6ObjectEi");

void fg1_drawsetfontstretch(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfontstretch(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetfontstretch(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsetfontstretch(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetfontstretch(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsetfontstretch(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfontstretch", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetfontstyle(Value* drw_wnd, int style_type) asm("_ZN4HPHP18f_drawsetfontstyleERKNS_6ObjectEi");

void fg1_drawsetfontstyle(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfontstyle(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetfontstyle(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsetfontstyle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetfontstyle(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsetfontstyle(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfontstyle", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetfontweight(Value* drw_wnd, double font_weight) asm("_ZN4HPHP19f_drawsetfontweightERKNS_6ObjectEd");

void fg1_drawsetfontweight(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetfontweight(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetfontweight(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetfontweight(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetfontweight(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetfontweight(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetfontweight", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetgravity(Value* drw_wnd, int gravity_type) asm("_ZN4HPHP16f_drawsetgravityERKNS_6ObjectEi");

void fg1_drawsetgravity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetgravity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetgravity(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsetgravity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetgravity(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsetgravity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetgravity", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokealpha(Value* drw_wnd, double stroke_opacity) asm("_ZN4HPHP20f_drawsetstrokealphaERKNS_6ObjectEd");

void fg1_drawsetstrokealpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokealpha(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokealpha(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetstrokealpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokealpha(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetstrokealpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokealpha", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokeantialias(Value* drw_wnd, bool stroke_antialias) asm("_ZN4HPHP24f_drawsetstrokeantialiasERKNS_6ObjectEb");

void fg1_drawsetstrokeantialias(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokeantialias(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokeantialias(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
}

TypedValue* fg_drawsetstrokeantialias(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokeantialias(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
    } else {
      fg1_drawsetstrokeantialias(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokeantialias", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokecolor(Value* drw_wnd, Value* strokecolor_pxl_wnd) asm("_ZN4HPHP20f_drawsetstrokecolorERKNS_6ObjectES2_");

void fg1_drawsetstrokecolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokecolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokecolor(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawsetstrokecolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokecolor(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawsetstrokecolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokecolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokedasharray(Value* drw_wnd, Value* dash_array) asm("_ZN4HPHP24f_drawsetstrokedasharrayERKNS_6ObjectERKNS_5ArrayE");

void fg1_drawsetstrokedasharray(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokedasharray(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokedasharray(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
}

TypedValue* fg_drawsetstrokedasharray(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfArray) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokedasharray(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
    } else {
      fg1_drawsetstrokedasharray(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokedasharray", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokedashoffset(Value* drw_wnd, double dash_offset) asm("_ZN4HPHP25f_drawsetstrokedashoffsetERKNS_6ObjectEd");

void fg1_drawsetstrokedashoffset(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokedashoffset(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokedashoffset(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetstrokedashoffset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokedashoffset(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetstrokedashoffset(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokedashoffset", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokelinecap(Value* drw_wnd, int line_cap) asm("_ZN4HPHP22f_drawsetstrokelinecapERKNS_6ObjectEi");

void fg1_drawsetstrokelinecap(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokelinecap(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokelinecap(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsetstrokelinecap(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokelinecap(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsetstrokelinecap(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokelinecap", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokelinejoin(Value* drw_wnd, int line_join) asm("_ZN4HPHP23f_drawsetstrokelinejoinERKNS_6ObjectEi");

void fg1_drawsetstrokelinejoin(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokelinejoin(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokelinejoin(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsetstrokelinejoin(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokelinejoin(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsetstrokelinejoin(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokelinejoin", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokemiterlimit(Value* drw_wnd, double miterlimit) asm("_ZN4HPHP25f_drawsetstrokemiterlimitERKNS_6ObjectEd");

void fg1_drawsetstrokemiterlimit(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokemiterlimit(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokemiterlimit(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetstrokemiterlimit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokemiterlimit(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetstrokemiterlimit(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokemiterlimit", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokeopacity(Value* drw_wnd, double stroke_opacity) asm("_ZN4HPHP22f_drawsetstrokeopacityERKNS_6ObjectEd");

void fg1_drawsetstrokeopacity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokeopacity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokeopacity(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetstrokeopacity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokeopacity(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetstrokeopacity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokeopacity", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawsetstrokepatternurl(Value* drw_wnd, Value* stroke_url) asm("_ZN4HPHP25f_drawsetstrokepatternurlERKNS_6ObjectERKNS_6StringE");

void fg1_drawsetstrokepatternurl(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokepatternurl(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawsetstrokepatternurl(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawsetstrokepatternurl(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawsetstrokepatternurl(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawsetstrokepatternurl(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokepatternurl", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetstrokewidth(Value* drw_wnd, double stroke_width) asm("_ZN4HPHP20f_drawsetstrokewidthERKNS_6ObjectEd");

void fg1_drawsetstrokewidth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetstrokewidth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsetstrokewidth(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawsetstrokewidth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetstrokewidth(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawsetstrokewidth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetstrokewidth", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsettextalignment(Value* drw_wnd, int align_type) asm("_ZN4HPHP22f_drawsettextalignmentERKNS_6ObjectEi");

void fg1_drawsettextalignment(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsettextalignment(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsettextalignment(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsettextalignment(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsettextalignment(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsettextalignment(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsettextalignment", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsettextantialias(Value* drw_wnd, bool text_antialias) asm("_ZN4HPHP22f_drawsettextantialiasERKNS_6ObjectEb");

void fg1_drawsettextantialias(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsettextantialias(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsettextantialias(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
}

TypedValue* fg_drawsettextantialias(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsettextantialias(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
    } else {
      fg1_drawsettextantialias(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsettextantialias", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsettextdecoration(Value* drw_wnd, int decoration_type) asm("_ZN4HPHP23f_drawsettextdecorationERKNS_6ObjectEi");

void fg1_drawsettextdecoration(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsettextdecoration(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsettextdecoration(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_drawsettextdecoration(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsettextdecoration(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_drawsettextdecoration(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsettextdecoration", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsettextencoding(Value* drw_wnd, Value* encoding) asm("_ZN4HPHP21f_drawsettextencodingERKNS_6ObjectERKNS_6StringE");

void fg1_drawsettextencoding(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsettextencoding(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsettextencoding(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawsettextencoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsettextencoding(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawsettextencoding(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsettextencoding", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsettextundercolor(Value* drw_wnd, Value* undercolor_pxl_wnd) asm("_ZN4HPHP23f_drawsettextundercolorERKNS_6ObjectES2_");

void fg1_drawsettextundercolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsettextundercolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawsettextundercolor(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawsettextundercolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsettextundercolor(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawsettextundercolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsettextundercolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_drawsetvectorgraphics(Value* drw_wnd, Value* vector_graphics) asm("_ZN4HPHP23f_drawsetvectorgraphicsERKNS_6ObjectERKNS_6StringE");

void fg1_drawsetvectorgraphics(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetvectorgraphics(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_drawsetvectorgraphics(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_drawsetvectorgraphics(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_drawsetvectorgraphics(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_drawsetvectorgraphics(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetvectorgraphics", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawsetviewbox(Value* drw_wnd, double x1, double y1, double x2, double y2) asm("_ZN4HPHP16f_drawsetviewboxERKNS_6ObjectEdddd");

void fg1_drawsetviewbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawsetviewbox(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_drawsetviewbox(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
}

TypedValue* fg_drawsetviewbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawsetviewbox(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl));
    } else {
      fg1_drawsetviewbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawsetviewbox", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawskewx(Value* drw_wnd, double degrees) asm("_ZN4HPHP11f_drawskewxERKNS_6ObjectEd");

void fg1_drawskewx(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawskewx(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawskewx(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawskewx(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawskewx(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawskewx(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawskewx", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawskewy(Value* drw_wnd, double degrees) asm("_ZN4HPHP11f_drawskewyERKNS_6ObjectEd");

void fg1_drawskewy(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawskewy(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawskewy(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_drawskewy(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawskewy(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_drawskewy(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawskewy", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawtranslate(Value* drw_wnd, double x, double y) asm("_ZN4HPHP15f_drawtranslateERKNS_6ObjectEdd");

void fg1_drawtranslate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawtranslate(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_drawtranslate(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
}

TypedValue* fg_drawtranslate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawtranslate(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl));
    } else {
      fg1_drawtranslate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawtranslate", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pushdrawingwand(Value* drw_wnd) asm("_ZN4HPHP17f_pushdrawingwandERKNS_6ObjectE");

void fg1_pushdrawingwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pushdrawingwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_pushdrawingwand(&args[-0].m_data);
}

TypedValue* fg_pushdrawingwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pushdrawingwand(&args[-0].m_data);
    } else {
      fg1_pushdrawingwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pushdrawingwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpushclippath(Value* drw_wnd, Value* clip_path_id) asm("_ZN4HPHP18f_drawpushclippathERKNS_6ObjectERKNS_6StringE");

void fg1_drawpushclippath(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpushclippath(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawpushclippath(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_drawpushclippath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpushclippath(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_drawpushclippath(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpushclippath", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpushdefs(Value* drw_wnd) asm("_ZN4HPHP14f_drawpushdefsERKNS_6ObjectE");

void fg1_drawpushdefs(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpushdefs(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_drawpushdefs(&args[-0].m_data);
}

TypedValue* fg_drawpushdefs(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpushdefs(&args[-0].m_data);
    } else {
      fg1_drawpushdefs(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpushdefs", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpushpattern(Value* drw_wnd, Value* pattern_id, double x, double y, double width, double height) asm("_ZN4HPHP17f_drawpushpatternERKNS_6ObjectERKNS_6StringEdddd");

void fg1_drawpushpattern(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpushpattern(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_drawpushpattern(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl));
}

TypedValue* fg_drawpushpattern(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfDouble &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpushpattern(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (args[-5].m_data.dbl));
    } else {
      fg1_drawpushpattern(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpushpattern", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_popdrawingwand(Value* drw_wnd) asm("_ZN4HPHP16f_popdrawingwandERKNS_6ObjectE");

void fg1_popdrawingwand(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_popdrawingwand(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_popdrawingwand(&args[-0].m_data);
}

TypedValue* fg_popdrawingwand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_popdrawingwand(&args[-0].m_data);
    } else {
      fg1_popdrawingwand(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("popdrawingwand", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpopclippath(Value* drw_wnd) asm("_ZN4HPHP17f_drawpopclippathERKNS_6ObjectE");

void fg1_drawpopclippath(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpopclippath(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_drawpopclippath(&args[-0].m_data);
}

TypedValue* fg_drawpopclippath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpopclippath(&args[-0].m_data);
    } else {
      fg1_drawpopclippath(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpopclippath", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpopdefs(Value* drw_wnd) asm("_ZN4HPHP13f_drawpopdefsERKNS_6ObjectE");

void fg1_drawpopdefs(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpopdefs(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_drawpopdefs(&args[-0].m_data);
}

TypedValue* fg_drawpopdefs(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpopdefs(&args[-0].m_data);
    } else {
      fg1_drawpopdefs(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpopdefs", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_drawpoppattern(Value* drw_wnd) asm("_ZN4HPHP16f_drawpoppatternERKNS_6ObjectE");

void fg1_drawpoppattern(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_drawpoppattern(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_drawpoppattern(&args[-0].m_data);
}

TypedValue* fg_drawpoppattern(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_drawpoppattern(&args[-0].m_data);
    } else {
      fg1_drawpoppattern(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("drawpoppattern", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickadaptivethresholdimage(Value* mgck_wnd, double width, double height, double offset) asm("_ZN4HPHP30f_magickadaptivethresholdimageERKNS_6ObjectEddd");

void fg1_magickadaptivethresholdimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickadaptivethresholdimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickadaptivethresholdimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickadaptivethresholdimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickadaptivethresholdimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickadaptivethresholdimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickadaptivethresholdimage", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickaddimage(Value* mgck_wnd, Value* add_wand) asm("_ZN4HPHP16f_magickaddimageERKNS_6ObjectES2_");

void fg1_magickaddimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickaddimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickaddimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickaddimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickaddimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickaddimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickaddimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickaddnoiseimage(Value* mgck_wnd, int noise_type) asm("_ZN4HPHP21f_magickaddnoiseimageERKNS_6ObjectEi");

void fg1_magickaddnoiseimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickaddnoiseimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickaddnoiseimage(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickaddnoiseimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickaddnoiseimage(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickaddnoiseimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickaddnoiseimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickaffinetransformimage(Value* mgck_wnd, Value* drw_wnd) asm("_ZN4HPHP28f_magickaffinetransformimageERKNS_6ObjectES2_");

void fg1_magickaffinetransformimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickaffinetransformimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickaffinetransformimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickaffinetransformimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickaffinetransformimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickaffinetransformimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickaffinetransformimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickannotateimage(Value* mgck_wnd, Value* drw_wnd, double x, double y, double angle, Value* text) asm("_ZN4HPHP21f_magickannotateimageERKNS_6ObjectES2_dddRKNS_6StringE");

void fg1_magickannotateimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickannotateimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickannotateimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), &args[-5].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickannotateimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if (IS_STRING_TYPE((args - 5)->m_type) &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickannotateimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), &args[-5].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickannotateimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickannotateimage", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickappendimages(Value* _rv, Value* mgck_wnd, bool stack_vertical) asm("_ZN4HPHP20f_magickappendimagesERKNS_6ObjectEb");

void fg1_magickappendimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickappendimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magickappendimages(&(rv->m_data), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickappendimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickappendimages(&(rv->m_data), &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickappendimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickappendimages", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickaverageimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickaverageimagesERKNS_6ObjectE");

void fg1_magickaverageimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickaverageimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickaverageimages(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickaverageimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickaverageimages(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickaverageimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickaverageimages", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickblackthresholdimage(Value* mgck_wnd, Value* threshold_pxl_wnd) asm("_ZN4HPHP27f_magickblackthresholdimageERKNS_6ObjectES2_");

void fg1_magickblackthresholdimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickblackthresholdimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickblackthresholdimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickblackthresholdimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickblackthresholdimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickblackthresholdimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickblackthresholdimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickblurimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP17f_magickblurimageERKNS_6ObjectEddi");

void fg1_magickblurimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickblurimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickblurimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magickblurimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickblurimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magickblurimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickblurimage", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickborderimage(Value* mgck_wnd, Value* bordercolor, double width, double height) asm("_ZN4HPHP19f_magickborderimageERKNS_6ObjectES2_dd");

void fg1_magickborderimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickborderimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickborderimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickborderimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickborderimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickborderimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickborderimage", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickcharcoalimage(Value* mgck_wnd, double radius, double sigma) asm("_ZN4HPHP21f_magickcharcoalimageERKNS_6ObjectEdd");

void fg1_magickcharcoalimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcharcoalimage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magickcharcoalimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickcharcoalimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickcharcoalimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickcharcoalimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcharcoalimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickchopimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP17f_magickchopimageERKNS_6ObjectEddii");

void fg1_magickchopimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickchopimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickchopimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickchopimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickchopimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickchopimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickchopimage", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickclipimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickclipimageERKNS_6ObjectE");

void fg1_magickclipimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickclipimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickclipimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickclipimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickclipimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickclipimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickclipimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickclippathimage(Value* mgck_wnd, Value* pathname, bool inside) asm("_ZN4HPHP21f_magickclippathimageERKNS_6ObjectERKNS_6StringEb");

void fg1_magickclippathimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickclippathimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickclippathimage(&args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickclippathimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfBoolean &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickclippathimage(&args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickclippathimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickclippathimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickcoalesceimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP22f_magickcoalesceimagesERKNS_6ObjectE");

void fg1_magickcoalesceimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcoalesceimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickcoalesceimages(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickcoalesceimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickcoalesceimages(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickcoalesceimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcoalesceimages", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickcolorfloodfillimage(Value* mgck_wnd, Value* fillcolor_pxl_wnd, double fuzz, Value* bordercolor_pxl_wnd, int x, int y) asm("_ZN4HPHP27f_magickcolorfloodfillimageERKNS_6ObjectES2_dS2_ii");

void fg1_magickcolorfloodfillimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcolorfloodfillimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickcolorfloodfillimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), &args[-3].m_data, (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickcolorfloodfillimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfObject &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickcolorfloodfillimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), &args[-3].m_data, (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickcolorfloodfillimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcolorfloodfillimage", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickcolorizeimage(Value* mgck_wnd, Value* colorize, Value* opacity_pxl_wnd) asm("_ZN4HPHP21f_magickcolorizeimageERKNS_6ObjectES2_S2_");

void fg1_magickcolorizeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcolorizeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickcolorizeimage(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickcolorizeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfObject &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickcolorizeimage(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickcolorizeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcolorizeimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickcombineimages(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP21f_magickcombineimagesERKNS_6ObjectEi");

void fg1_magickcombineimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcombineimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magickcombineimages(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickcombineimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickcombineimages(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickcombineimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcombineimages", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickcommentimage(Value* mgck_wnd, Value* comment) asm("_ZN4HPHP20f_magickcommentimageERKNS_6ObjectERKNS_6StringE");

void fg1_magickcommentimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcommentimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickcommentimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickcommentimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickcommentimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickcommentimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcommentimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickcompareimages(Value* _rv, Value* mgck_wnd, Value* reference_wnd, int metric_type, int channel_type) asm("_ZN4HPHP21f_magickcompareimagesERKNS_6ObjectES2_ii");

void fg1_magickcompareimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcompareimages(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfArray;
  fh_magickcompareimages(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickcompareimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickcompareimages(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickcompareimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcompareimages", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickcompositeimage(Value* mgck_wnd, Value* composite_wnd, int composite_operator, int x, int y) asm("_ZN4HPHP22f_magickcompositeimageERKNS_6ObjectES2_iii");

void fg1_magickcompositeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcompositeimage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickcompositeimage(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickcompositeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickcompositeimage(&args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickcompositeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcompositeimage", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickconstituteimage(Value* mgck_wnd, double columns, double rows, Value* smap, int storage_type, Value* pixel_array) asm("_ZN4HPHP23f_magickconstituteimageERKNS_6ObjectEddRKNS_6StringEiRKNS_5ArrayE");

void fg1_magickconstituteimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickconstituteimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickconstituteimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), &args[-3].m_data, (int)(args[-4].m_data.num), &args[-5].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickconstituteimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfArray &&
        (args - 4)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 3)->m_type) &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickconstituteimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), &args[-3].m_data, (int)(args[-4].m_data.num), &args[-5].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickconstituteimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickconstituteimage", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickcontrastimage(Value* mgck_wnd, bool sharpen) asm("_ZN4HPHP21f_magickcontrastimageERKNS_6ObjectEb");

void fg1_magickcontrastimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcontrastimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickcontrastimage(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickcontrastimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfBoolean &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickcontrastimage(&args[-0].m_data, (bool)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickcontrastimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcontrastimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickconvolveimage(Value* mgck_wnd, Value* kernel_array, int channel_type) asm("_ZN4HPHP21f_magickconvolveimageERKNS_6ObjectERKNS_5ArrayEi");

void fg1_magickconvolveimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickconvolveimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickconvolveimage(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magickconvolveimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickconvolveimage(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magickconvolveimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickconvolveimage", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickcropimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP17f_magickcropimageERKNS_6ObjectEddii");

void fg1_magickcropimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcropimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickcropimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickcropimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickcropimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickcropimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcropimage", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickcyclecolormapimage(Value* mgck_wnd, int num_positions) asm("_ZN4HPHP26f_magickcyclecolormapimageERKNS_6ObjectEi");

void fg1_magickcyclecolormapimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickcyclecolormapimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickcyclecolormapimage(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickcyclecolormapimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickcyclecolormapimage(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickcyclecolormapimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickcyclecolormapimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickdeconstructimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickdeconstructimagesERKNS_6ObjectE");

void fg1_magickdeconstructimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickdeconstructimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickdeconstructimages(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickdeconstructimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickdeconstructimages(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickdeconstructimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickdeconstructimages", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickdescribeimage(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickdescribeimageERKNS_6ObjectE");

void fg1_magickdescribeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickdescribeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickdescribeimage(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickdescribeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickdescribeimage(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickdescribeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickdescribeimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickdespeckleimage(Value* mgck_wnd) asm("_ZN4HPHP22f_magickdespeckleimageERKNS_6ObjectE");

void fg1_magickdespeckleimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickdespeckleimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickdespeckleimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickdespeckleimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickdespeckleimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickdespeckleimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickdespeckleimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickdrawimage(Value* mgck_wnd, Value* drw_wnd) asm("_ZN4HPHP17f_magickdrawimageERKNS_6ObjectES2_");

void fg1_magickdrawimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickdrawimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickdrawimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickdrawimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickdrawimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickdrawimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickdrawimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickechoimageblob(Value* mgck_wnd) asm("_ZN4HPHP21f_magickechoimageblobERKNS_6ObjectE");

void fg1_magickechoimageblob(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickechoimageblob(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickechoimageblob(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickechoimageblob(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickechoimageblob(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickechoimageblob(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickechoimageblob", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickechoimagesblob(Value* mgck_wnd) asm("_ZN4HPHP22f_magickechoimagesblobERKNS_6ObjectE");

void fg1_magickechoimagesblob(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickechoimagesblob(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickechoimagesblob(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickechoimagesblob(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickechoimagesblob(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickechoimagesblob(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickechoimagesblob", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickedgeimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP17f_magickedgeimageERKNS_6ObjectEd");

void fg1_magickedgeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickedgeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickedgeimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickedgeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickedgeimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickedgeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickedgeimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickembossimage(Value* mgck_wnd, double radius, double sigma) asm("_ZN4HPHP19f_magickembossimageERKNS_6ObjectEdd");

void fg1_magickembossimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickembossimage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magickembossimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickembossimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickembossimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickembossimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickembossimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickenhanceimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickenhanceimageERKNS_6ObjectE");

void fg1_magickenhanceimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickenhanceimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickenhanceimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickenhanceimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickenhanceimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickenhanceimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickenhanceimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickequalizeimage(Value* mgck_wnd) asm("_ZN4HPHP21f_magickequalizeimageERKNS_6ObjectE");

void fg1_magickequalizeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickequalizeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickequalizeimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickequalizeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickequalizeimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickequalizeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickequalizeimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickevaluateimage(Value* mgck_wnd, int evaluate_op, double constant, int channel_type) asm("_ZN4HPHP21f_magickevaluateimageERKNS_6ObjectEidi");

void fg1_magickevaluateimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickevaluateimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickevaluateimage(&args[-0].m_data, (int)(args[-1].m_data.num), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magickevaluateimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickevaluateimage(&args[-0].m_data, (int)(args[-1].m_data.num), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magickevaluateimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickevaluateimage", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickflattenimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickflattenimagesERKNS_6ObjectE");

void fg1_magickflattenimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickflattenimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickflattenimages(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickflattenimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickflattenimages(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickflattenimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickflattenimages", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickflipimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickflipimageERKNS_6ObjectE");

void fg1_magickflipimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickflipimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickflipimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickflipimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickflipimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickflipimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickflipimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickflopimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magickflopimageERKNS_6ObjectE");

void fg1_magickflopimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickflopimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickflopimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickflopimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickflopimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickflopimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickflopimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickframeimage(Value* mgck_wnd, Value* matte_color, double width, double height, int inner_bevel, int outer_bevel) asm("_ZN4HPHP18f_magickframeimageERKNS_6ObjectES2_ddii");

void fg1_magickframeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickframeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickframeimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickframeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickframeimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl), (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickframeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickframeimage", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickfximage(Value* _rv, Value* mgck_wnd, Value* expression, int channel_type) asm("_ZN4HPHP15f_magickfximageERKNS_6ObjectERKNS_6StringEi");

void fg1_magickfximage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickfximage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magickfximage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickfximage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickfximage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickfximage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickfximage", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickgammaimage(Value* mgck_wnd, double gamma, int channel_type) asm("_ZN4HPHP18f_magickgammaimageERKNS_6ObjectEdi");

void fg1_magickgammaimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgammaimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickgammaimage(&args[-0].m_data, (args[-1].m_data.dbl), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magickgammaimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickgammaimage(&args[-0].m_data, (args[-1].m_data.dbl), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magickgammaimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgammaimage", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickgaussianblurimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP25f_magickgaussianblurimageERKNS_6ObjectEddi");

void fg1_magickgaussianblurimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgaussianblurimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickgaussianblurimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magickgaussianblurimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickgaussianblurimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magickgaussianblurimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgaussianblurimage", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetcharheight(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP21f_magickgetcharheightERKNS_6ObjectES2_RKNS_6StringEb");

void fg1_magickgetcharheight(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetcharheight(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetcharheight(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
}

TypedValue* fg_magickgetcharheight(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetcharheight(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
    } else {
      fg1_magickgetcharheight(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetcharheight", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetcharwidth(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP20f_magickgetcharwidthERKNS_6ObjectES2_RKNS_6StringEb");

void fg1_magickgetcharwidth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetcharwidth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetcharwidth(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
}

TypedValue* fg_magickgetcharwidth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetcharwidth(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
    } else {
      fg1_magickgetcharwidth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetcharwidth", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetexception(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetexceptionERKNS_6ObjectE");

void fg1_magickgetexception(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetexception(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetexception(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetexception(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetexception(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetexception(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetexception", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetexceptionstring(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetexceptionstringERKNS_6ObjectE");

void fg1_magickgetexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetexceptionstring(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetexceptionstring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetexceptionstring(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetexceptionstring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetexceptionstring", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetexceptiontype(Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetexceptiontypeERKNS_6ObjectE");

void fg1_magickgetexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetexceptiontype(&args[-0].m_data);
}

TypedValue* fg_magickgetexceptiontype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetexceptiontype(&args[-0].m_data);
    } else {
      fg1_magickgetexceptiontype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetexceptiontype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetfilename(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetfilenameERKNS_6ObjectE");

void fg1_magickgetfilename(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetfilename(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetfilename(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetfilename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetfilename(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetfilename(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetfilename", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetformat(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP17f_magickgetformatERKNS_6ObjectE");

void fg1_magickgetformat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetformat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetformat(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetformat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetformat(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetformat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetformat", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimage(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP16f_magickgetimageERKNS_6ObjectE");

void fg1_magickgetimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickgetimage(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickgetimage(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagebackgroundcolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimagebackgroundcolorERKNS_6ObjectE");

void fg1_magickgetimagebackgroundcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagebackgroundcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickgetimagebackgroundcolor(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagebackgroundcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickgetimagebackgroundcolor(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagebackgroundcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagebackgroundcolor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimageblob(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimageblobERKNS_6ObjectE");

void fg1_magickgetimageblob(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageblob(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetimageblob(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimageblob(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetimageblob(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimageblob(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageblob", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimageblueprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimageblueprimaryERKNS_6ObjectE");

void fg1_magickgetimageblueprimary(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageblueprimary(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetimageblueprimary(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimageblueprimary(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimageblueprimary(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimageblueprimary(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageblueprimary", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagebordercolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimagebordercolorERKNS_6ObjectE");

void fg1_magickgetimagebordercolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagebordercolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickgetimagebordercolor(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagebordercolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickgetimagebordercolor(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagebordercolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagebordercolor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagechannelmean(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP27f_magickgetimagechannelmeanERKNS_6ObjectEi");

void fg1_magickgetimagechannelmean(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagechannelmean(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfArray;
  fh_magickgetimagechannelmean(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagechannelmean(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimagechannelmean(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagechannelmean(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagechannelmean", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagecolormapcolor(Value* _rv, Value* mgck_wnd, double index) asm("_ZN4HPHP29f_magickgetimagecolormapcolorERKNS_6ObjectEd");

void fg1_magickgetimagecolormapcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagecolormapcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magickgetimagecolormapcolor(&(rv->m_data), &args[-0].m_data, (args[-1].m_data.dbl));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagecolormapcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickgetimagecolormapcolor(&(rv->m_data), &args[-0].m_data, (args[-1].m_data.dbl));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagecolormapcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagecolormapcolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimagecolors(Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimagecolorsERKNS_6ObjectE");

void fg1_magickgetimagecolors(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagecolors(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimagecolors(&args[-0].m_data);
}

TypedValue* fg_magickgetimagecolors(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimagecolors(&args[-0].m_data);
    } else {
      fg1_magickgetimagecolors(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagecolors", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimagecolorspace(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagecolorspaceERKNS_6ObjectE");

void fg1_magickgetimagecolorspace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagecolorspace(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimagecolorspace(&args[-0].m_data);
}

TypedValue* fg_magickgetimagecolorspace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimagecolorspace(&args[-0].m_data);
    } else {
      fg1_magickgetimagecolorspace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagecolorspace", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimagecompose(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetimagecomposeERKNS_6ObjectE");

void fg1_magickgetimagecompose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagecompose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimagecompose(&args[-0].m_data);
}

TypedValue* fg_magickgetimagecompose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimagecompose(&args[-0].m_data);
    } else {
      fg1_magickgetimagecompose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagecompose", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimagecompression(Value* mgck_wnd) asm("_ZN4HPHP27f_magickgetimagecompressionERKNS_6ObjectE");

void fg1_magickgetimagecompression(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagecompression(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimagecompression(&args[-0].m_data);
}

TypedValue* fg_magickgetimagecompression(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimagecompression(&args[-0].m_data);
    } else {
      fg1_magickgetimagecompression(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagecompression", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimagecompressionquality(Value* mgck_wnd) asm("_ZN4HPHP34f_magickgetimagecompressionqualityERKNS_6ObjectE");

void fg1_magickgetimagecompressionquality(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagecompressionquality(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimagecompressionquality(&args[-0].m_data);
}

TypedValue* fg_magickgetimagecompressionquality(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimagecompressionquality(&args[-0].m_data);
    } else {
      fg1_magickgetimagecompressionquality(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagecompressionquality", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimagedelay(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagedelayERKNS_6ObjectE");

void fg1_magickgetimagedelay(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagedelay(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimagedelay(&args[-0].m_data);
}

TypedValue* fg_magickgetimagedelay(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimagedelay(&args[-0].m_data);
    } else {
      fg1_magickgetimagedelay(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagedelay", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimagedepth(Value* mgck_wnd, int channel_type) asm("_ZN4HPHP21f_magickgetimagedepthERKNS_6ObjectEi");

void fg1_magickgetimagedepth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagedepth(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimagedepth(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
}

TypedValue* fg_magickgetimagedepth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimagedepth(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
    } else {
      fg1_magickgetimagedepth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagedepth", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimagedispose(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetimagedisposeERKNS_6ObjectE");

void fg1_magickgetimagedispose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagedispose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimagedispose(&args[-0].m_data);
}

TypedValue* fg_magickgetimagedispose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimagedispose(&args[-0].m_data);
    } else {
      fg1_magickgetimagedispose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagedispose", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimageextrema(Value* _rv, Value* mgck_wnd, int channel_type) asm("_ZN4HPHP23f_magickgetimageextremaERKNS_6ObjectEi");

void fg1_magickgetimageextrema(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageextrema(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfArray;
  fh_magickgetimageextrema(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimageextrema(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimageextrema(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimageextrema(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageextrema", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagefilename(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetimagefilenameERKNS_6ObjectE");

void fg1_magickgetimagefilename(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagefilename(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetimagefilename(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagefilename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetimagefilename(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagefilename(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagefilename", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimageformat(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimageformatERKNS_6ObjectE");

void fg1_magickgetimageformat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageformat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetimageformat(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimageformat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetimageformat(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimageformat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageformat", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimagegamma(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagegammaERKNS_6ObjectE");

void fg1_magickgetimagegamma(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagegamma(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimagegamma(&args[-0].m_data);
}

TypedValue* fg_magickgetimagegamma(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimagegamma(&args[-0].m_data);
    } else {
      fg1_magickgetimagegamma(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagegamma", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagegreenprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP28f_magickgetimagegreenprimaryERKNS_6ObjectE");

void fg1_magickgetimagegreenprimary(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagegreenprimary(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetimagegreenprimary(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagegreenprimary(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimagegreenprimary(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagegreenprimary(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagegreenprimary", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimageheight(Value* mgck_wnd) asm("_ZN4HPHP22f_magickgetimageheightERKNS_6ObjectE");

void fg1_magickgetimageheight(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageheight(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimageheight(&args[-0].m_data);
}

TypedValue* fg_magickgetimageheight(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimageheight(&args[-0].m_data);
    } else {
      fg1_magickgetimageheight(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageheight", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagehistogram(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickgetimagehistogramERKNS_6ObjectE");

void fg1_magickgetimagehistogram(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagehistogram(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetimagehistogram(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagehistogram(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimagehistogram(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagehistogram(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagehistogram", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimageindex(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimageindexERKNS_6ObjectE");

void fg1_magickgetimageindex(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageindex(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimageindex(&args[-0].m_data);
}

TypedValue* fg_magickgetimageindex(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimageindex(&args[-0].m_data);
    } else {
      fg1_magickgetimageindex(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageindex", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimageinterlacescheme(Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimageinterlaceschemeERKNS_6ObjectE");

void fg1_magickgetimageinterlacescheme(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageinterlacescheme(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimageinterlacescheme(&args[-0].m_data);
}

TypedValue* fg_magickgetimageinterlacescheme(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimageinterlacescheme(&args[-0].m_data);
    } else {
      fg1_magickgetimageinterlacescheme(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageinterlacescheme", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimageiterations(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageiterationsERKNS_6ObjectE");

void fg1_magickgetimageiterations(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageiterations(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimageiterations(&args[-0].m_data);
}

TypedValue* fg_magickgetimageiterations(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimageiterations(&args[-0].m_data);
    } else {
      fg1_magickgetimageiterations(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageiterations", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagemattecolor(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagemattecolorERKNS_6ObjectE");

void fg1_magickgetimagemattecolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagemattecolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickgetimagemattecolor(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagemattecolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickgetimagemattecolor(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagemattecolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagemattecolor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagemimetype(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP24f_magickgetimagemimetypeERKNS_6ObjectE");

void fg1_magickgetimagemimetype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagemimetype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetimagemimetype(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagemimetype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetimagemimetype(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagemimetype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagemimetype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagepixels(Value* _rv, Value* mgck_wnd, int x_offset, int y_offset, double columns, double rows, Value* smap, int storage_type) asm("_ZN4HPHP22f_magickgetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEi");

void fg1_magickgetimagepixels(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagepixels(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfArray;
  fh_magickgetimagepixels(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (args[-4].m_data.dbl), &args[-5].m_data, (int)(args[-6].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagepixels(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 7) {
    if ((args - 6)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 5)->m_type) &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimagepixels(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (args[-4].m_data.dbl), &args[-5].m_data, (int)(args[-6].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagepixels(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagepixels", count, 7, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimageprofile(Value* _rv, Value* mgck_wnd, Value* name) asm("_ZN4HPHP23f_magickgetimageprofileERKNS_6ObjectERKNS_6StringE");

void fg1_magickgetimageprofile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageprofile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_magickgetimageprofile(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimageprofile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetimageprofile(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimageprofile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageprofile", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimageredprimary(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageredprimaryERKNS_6ObjectE");

void fg1_magickgetimageredprimary(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageredprimary(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetimageredprimary(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimageredprimary(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimageredprimary(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimageredprimary(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageredprimary", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimagerenderingintent(Value* mgck_wnd) asm("_ZN4HPHP31f_magickgetimagerenderingintentERKNS_6ObjectE");

void fg1_magickgetimagerenderingintent(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagerenderingintent(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimagerenderingintent(&args[-0].m_data);
}

TypedValue* fg_magickgetimagerenderingintent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimagerenderingintent(&args[-0].m_data);
    } else {
      fg1_magickgetimagerenderingintent(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagerenderingintent", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimageresolution(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimageresolutionERKNS_6ObjectE");

void fg1_magickgetimageresolution(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageresolution(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetimageresolution(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimageresolution(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimageresolution(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimageresolution(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageresolution", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimagescene(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagesceneERKNS_6ObjectE");

void fg1_magickgetimagescene(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagescene(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimagescene(&args[-0].m_data);
}

TypedValue* fg_magickgetimagescene(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimagescene(&args[-0].m_data);
    } else {
      fg1_magickgetimagescene(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagescene", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagesignature(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP25f_magickgetimagesignatureERKNS_6ObjectE");

void fg1_magickgetimagesignature(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagesignature(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetimagesignature(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagesignature(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetimagesignature(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagesignature(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagesignature", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimagesize(Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimagesizeERKNS_6ObjectE");

void fg1_magickgetimagesize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagesize(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimagesize(&args[-0].m_data);
}

TypedValue* fg_magickgetimagesize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimagesize(&args[-0].m_data);
    } else {
      fg1_magickgetimagesize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagesize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimagetype(Value* mgck_wnd) asm("_ZN4HPHP20f_magickgetimagetypeERKNS_6ObjectE");

void fg1_magickgetimagetype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagetype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimagetype(&args[-0].m_data);
}

TypedValue* fg_magickgetimagetype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimagetype(&args[-0].m_data);
    } else {
      fg1_magickgetimagetype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagetype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimageunits(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimageunitsERKNS_6ObjectE");

void fg1_magickgetimageunits(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimageunits(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimageunits(&args[-0].m_data);
}

TypedValue* fg_magickgetimageunits(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimageunits(&args[-0].m_data);
    } else {
      fg1_magickgetimageunits(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimageunits", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetimagevirtualpixelmethod(Value* mgck_wnd) asm("_ZN4HPHP34f_magickgetimagevirtualpixelmethodERKNS_6ObjectE");

void fg1_magickgetimagevirtualpixelmethod(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagevirtualpixelmethod(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetimagevirtualpixelmethod(&args[-0].m_data);
}

TypedValue* fg_magickgetimagevirtualpixelmethod(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetimagevirtualpixelmethod(&args[-0].m_data);
    } else {
      fg1_magickgetimagevirtualpixelmethod(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagevirtualpixelmethod", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagewhitepoint(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetimagewhitepointERKNS_6ObjectE");

void fg1_magickgetimagewhitepoint(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagewhitepoint(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetimagewhitepoint(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagewhitepoint(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetimagewhitepoint(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagewhitepoint(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagewhitepoint", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetimagewidth(Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagewidthERKNS_6ObjectE");

void fg1_magickgetimagewidth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagewidth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetimagewidth(&args[-0].m_data);
}

TypedValue* fg_magickgetimagewidth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetimagewidth(&args[-0].m_data);
    } else {
      fg1_magickgetimagewidth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagewidth", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetimagesblob(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP21f_magickgetimagesblobERKNS_6ObjectE");

void fg1_magickgetimagesblob(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetimagesblob(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetimagesblob(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetimagesblob(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetimagesblob(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetimagesblob(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetimagesblob", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_magickgetinterlacescheme(Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetinterlaceschemeERKNS_6ObjectE");

void fg1_magickgetinterlacescheme(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetinterlacescheme(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_magickgetinterlacescheme(&args[-0].m_data);
}

TypedValue* fg_magickgetinterlacescheme(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_magickgetinterlacescheme(&args[-0].m_data);
    } else {
      fg1_magickgetinterlacescheme(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetinterlacescheme", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetmaxtextadvance(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP25f_magickgetmaxtextadvanceERKNS_6ObjectES2_RKNS_6StringEb");

void fg1_magickgetmaxtextadvance(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetmaxtextadvance(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetmaxtextadvance(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
}

TypedValue* fg_magickgetmaxtextadvance(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetmaxtextadvance(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
    } else {
      fg1_magickgetmaxtextadvance(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetmaxtextadvance", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetmimetype(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetmimetypeERKNS_6ObjectE");

void fg1_magickgetmimetype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetmimetype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_magickgetmimetype(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetmimetype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickgetmimetype(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetmimetype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetmimetype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetnumberimages(Value* mgck_wnd) asm("_ZN4HPHP23f_magickgetnumberimagesERKNS_6ObjectE");

void fg1_magickgetnumberimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetnumberimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetnumberimages(&args[-0].m_data);
}

TypedValue* fg_magickgetnumberimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetnumberimages(&args[-0].m_data);
    } else {
      fg1_magickgetnumberimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetnumberimages", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetsamplingfactors(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP26f_magickgetsamplingfactorsERKNS_6ObjectE");

void fg1_magickgetsamplingfactors(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetsamplingfactors(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetsamplingfactors(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetsamplingfactors(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetsamplingfactors(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetsamplingfactors(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetsamplingfactors", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetsize(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP15f_magickgetsizeERKNS_6ObjectE");

void fg1_magickgetsize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetsize(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetsize(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetsize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetsize(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetsize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetsize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetstringheight(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP23f_magickgetstringheightERKNS_6ObjectES2_RKNS_6StringEb");

void fg1_magickgetstringheight(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetstringheight(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetstringheight(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
}

TypedValue* fg_magickgetstringheight(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetstringheight(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
    } else {
      fg1_magickgetstringheight(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetstringheight", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgetstringwidth(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP22f_magickgetstringwidthERKNS_6ObjectES2_RKNS_6StringEb");

void fg1_magickgetstringwidth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetstringwidth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgetstringwidth(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
}

TypedValue* fg_magickgetstringwidth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgetstringwidth(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
    } else {
      fg1_magickgetstringwidth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetstringwidth", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgettextascent(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP21f_magickgettextascentERKNS_6ObjectES2_RKNS_6StringEb");

void fg1_magickgettextascent(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgettextascent(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgettextascent(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
}

TypedValue* fg_magickgettextascent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgettextascent(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
    } else {
      fg1_magickgettextascent(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgettextascent", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_magickgettextdescent(Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP22f_magickgettextdescentERKNS_6ObjectES2_RKNS_6StringEb");

void fg1_magickgettextdescent(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgettextdescent(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_magickgettextdescent(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
}

TypedValue* fg_magickgettextdescent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_magickgettextdescent(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
    } else {
      fg1_magickgettextdescent(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgettextdescent", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickgetwandsize(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP19f_magickgetwandsizeERKNS_6ObjectE");

void fg1_magickgetwandsize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickgetwandsize(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_magickgetwandsize(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickgetwandsize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickgetwandsize(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickgetwandsize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickgetwandsize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickhasnextimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickhasnextimageERKNS_6ObjectE");

void fg1_magickhasnextimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickhasnextimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickhasnextimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickhasnextimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickhasnextimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickhasnextimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickhasnextimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickhaspreviousimage(Value* mgck_wnd) asm("_ZN4HPHP24f_magickhaspreviousimageERKNS_6ObjectE");

void fg1_magickhaspreviousimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickhaspreviousimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickhaspreviousimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickhaspreviousimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickhaspreviousimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickhaspreviousimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickhaspreviousimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickimplodeimage(Value* mgck_wnd, double amount) asm("_ZN4HPHP20f_magickimplodeimageERKNS_6ObjectEd");

void fg1_magickimplodeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickimplodeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickimplodeimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickimplodeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickimplodeimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickimplodeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickimplodeimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicklabelimage(Value* mgck_wnd, Value* label) asm("_ZN4HPHP18f_magicklabelimageERKNS_6ObjectERKNS_6StringE");

void fg1_magicklabelimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicklabelimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicklabelimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicklabelimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicklabelimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicklabelimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicklabelimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicklevelimage(Value* mgck_wnd, double black_point, double gamma, double white_point, int channel_type) asm("_ZN4HPHP18f_magicklevelimageERKNS_6ObjectEdddi");

void fg1_magicklevelimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicklevelimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicklevelimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magicklevelimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicklevelimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magicklevelimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicklevelimage", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickmagnifyimage(Value* mgck_wnd) asm("_ZN4HPHP20f_magickmagnifyimageERKNS_6ObjectE");

void fg1_magickmagnifyimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmagnifyimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickmagnifyimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickmagnifyimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickmagnifyimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickmagnifyimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmagnifyimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickmapimage(Value* mgck_wnd, Value* map_wand, bool dither) asm("_ZN4HPHP16f_magickmapimageERKNS_6ObjectES2_b");

void fg1_magickmapimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmapimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickmapimage(&args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickmapimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfBoolean &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickmapimage(&args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickmapimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmapimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickmattefloodfillimage(Value* mgck_wnd, double opacity, double fuzz, Value* bordercolor_pxl_wnd, int x, int y) asm("_ZN4HPHP27f_magickmattefloodfillimageERKNS_6ObjectEddS2_ii");

void fg1_magickmattefloodfillimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmattefloodfillimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickmattefloodfillimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), &args[-3].m_data, (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickmattefloodfillimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfInt64 &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfObject &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickmattefloodfillimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), &args[-3].m_data, (int)(args[-4].m_data.num), (int)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickmattefloodfillimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmattefloodfillimage", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickmedianfilterimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP25f_magickmedianfilterimageERKNS_6ObjectEd");

void fg1_magickmedianfilterimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmedianfilterimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickmedianfilterimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickmedianfilterimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickmedianfilterimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickmedianfilterimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmedianfilterimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickminifyimage(Value* mgck_wnd) asm("_ZN4HPHP19f_magickminifyimageERKNS_6ObjectE");

void fg1_magickminifyimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickminifyimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickminifyimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickminifyimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickminifyimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickminifyimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickminifyimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickmodulateimage(Value* mgck_wnd, double brightness, double saturation, double hue) asm("_ZN4HPHP21f_magickmodulateimageERKNS_6ObjectEddd");

void fg1_magickmodulateimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmodulateimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickmodulateimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickmodulateimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickmodulateimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickmodulateimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmodulateimage", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickmontageimage(Value* _rv, Value* mgck_wnd, Value* drw_wnd, Value* tile_geometry, Value* thumbnail_geometry, int montage_mode, Value* frame) asm("_ZN4HPHP20f_magickmontageimageERKNS_6ObjectES2_RKNS_6StringES5_iS5_");

void fg1_magickmontageimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmontageimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magickmontageimage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, (int)(args[-4].m_data.num), &args[-5].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickmontageimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if (IS_STRING_TYPE((args - 5)->m_type) &&
        (args - 4)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickmontageimage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data, (int)(args[-4].m_data.num), &args[-5].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickmontageimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmontageimage", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickmorphimages(Value* _rv, Value* mgck_wnd, double number_frames) asm("_ZN4HPHP19f_magickmorphimagesERKNS_6ObjectEd");

void fg1_magickmorphimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmorphimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magickmorphimages(&(rv->m_data), &args[-0].m_data, (args[-1].m_data.dbl));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickmorphimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickmorphimages(&(rv->m_data), &args[-0].m_data, (args[-1].m_data.dbl));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickmorphimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmorphimages", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickmosaicimages(Value* _rv, Value* mgck_wnd) asm("_ZN4HPHP20f_magickmosaicimagesERKNS_6ObjectE");

void fg1_magickmosaicimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmosaicimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_magickmosaicimages(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickmosaicimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickmosaicimages(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickmosaicimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmosaicimages", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickmotionblurimage(Value* mgck_wnd, double radius, double sigma, double angle) asm("_ZN4HPHP23f_magickmotionblurimageERKNS_6ObjectEddd");

void fg1_magickmotionblurimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickmotionblurimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickmotionblurimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickmotionblurimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickmotionblurimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickmotionblurimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickmotionblurimage", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicknegateimage(Value* mgck_wnd, bool only_the_gray, int channel_type) asm("_ZN4HPHP19f_magicknegateimageERKNS_6ObjectEbi");

void fg1_magicknegateimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicknegateimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicknegateimage(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magicknegateimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicknegateimage(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magicknegateimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicknegateimage", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicknewimage(Value* mgck_wnd, double width, double height, Value* imagemagick_col_str) asm("_ZN4HPHP16f_magicknewimageERKNS_6ObjectEddRKNS_6StringE");

void fg1_magicknewimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicknewimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicknewimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_magicknewimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicknewimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? &args[-3].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_magicknewimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicknewimage", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicknextimage(Value* mgck_wnd) asm("_ZN4HPHP17f_magicknextimageERKNS_6ObjectE");

void fg1_magicknextimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicknextimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicknextimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicknextimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicknextimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicknextimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicknextimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicknormalizeimage(Value* mgck_wnd) asm("_ZN4HPHP22f_magicknormalizeimageERKNS_6ObjectE");

void fg1_magicknormalizeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicknormalizeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicknormalizeimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicknormalizeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicknormalizeimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicknormalizeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicknormalizeimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickoilpaintimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP21f_magickoilpaintimageERKNS_6ObjectEd");

void fg1_magickoilpaintimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickoilpaintimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickoilpaintimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickoilpaintimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickoilpaintimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickoilpaintimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickoilpaintimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickpaintopaqueimage(Value* mgck_wnd, Value* target_pxl_wnd, Value* fill_pxl_wnd, double fuzz) asm("_ZN4HPHP24f_magickpaintopaqueimageERKNS_6ObjectES2_S2_d");

void fg1_magickpaintopaqueimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickpaintopaqueimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickpaintopaqueimage(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0))) ? 1LL : 0LL;
}

TypedValue* fg_magickpaintopaqueimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfDouble) &&
        (args - 2)->m_type == KindOfObject &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickpaintopaqueimage(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0))) ? 1LL : 0LL;
    } else {
      fg1_magickpaintopaqueimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickpaintopaqueimage", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickpainttransparentimage(Value* mgck_wnd, Value* target, double opacity, double fuzz) asm("_ZN4HPHP29f_magickpainttransparentimageERKNS_6ObjectES2_dd");

void fg1_magickpainttransparentimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickpainttransparentimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickpainttransparentimage(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (args[-2].m_data.dbl) : (double)(k_MW_TransparentOpacity), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0))) ? 1LL : 0LL;
}

TypedValue* fg_magickpainttransparentimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfDouble) &&
        (count <= 2 || (args - 2)->m_type == KindOfDouble) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickpainttransparentimage(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (args[-2].m_data.dbl) : (double)(k_MW_TransparentOpacity), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0))) ? 1LL : 0LL;
    } else {
      fg1_magickpainttransparentimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickpainttransparentimage", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickpingimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP17f_magickpingimageERKNS_6ObjectERKNS_6StringE");

void fg1_magickpingimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickpingimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickpingimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickpingimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickpingimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickpingimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickpingimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickposterizeimage(Value* mgck_wnd, double levels, bool dither) asm("_ZN4HPHP22f_magickposterizeimageERKNS_6ObjectEdb");

void fg1_magickposterizeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickposterizeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickposterizeimage(&args[-0].m_data, (args[-1].m_data.dbl), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickposterizeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfBoolean &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickposterizeimage(&args[-0].m_data, (args[-1].m_data.dbl), (bool)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickposterizeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickposterizeimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickpreviewimages(Value* _rv, Value* mgck_wnd, int preview) asm("_ZN4HPHP21f_magickpreviewimagesERKNS_6ObjectEi");

void fg1_magickpreviewimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickpreviewimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magickpreviewimages(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickpreviewimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magickpreviewimages(&(rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickpreviewimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickpreviewimages", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickpreviousimage(Value* mgck_wnd) asm("_ZN4HPHP21f_magickpreviousimageERKNS_6ObjectE");

void fg1_magickpreviousimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickpreviousimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickpreviousimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickpreviousimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickpreviousimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickpreviousimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickpreviousimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickprofileimage(Value* mgck_wnd, Value* name, Value* profile) asm("_ZN4HPHP20f_magickprofileimageERKNS_6ObjectERKNS_6StringES5_");

void fg1_magickprofileimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickprofileimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickprofileimage(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_magickprofileimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickprofileimage(&args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_magickprofileimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickprofileimage", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickquantizeimage(Value* mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) asm("_ZN4HPHP21f_magickquantizeimageERKNS_6ObjectEdidbb");

void fg1_magickquantizeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickquantizeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickquantizeimage(&args[-0].m_data, (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickquantizeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfBoolean &&
        (args - 4)->m_type == KindOfBoolean &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickquantizeimage(&args[-0].m_data, (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickquantizeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickquantizeimage", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickquantizeimages(Value* mgck_wnd, double number_colors, int colorspace_type, double treedepth, bool dither, bool measure_error) asm("_ZN4HPHP22f_magickquantizeimagesERKNS_6ObjectEdidbb");

void fg1_magickquantizeimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickquantizeimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickquantizeimages(&args[-0].m_data, (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickquantizeimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfBoolean &&
        (args - 4)->m_type == KindOfBoolean &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickquantizeimages(&args[-0].m_data, (args[-1].m_data.dbl), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (bool)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickquantizeimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickquantizeimages", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickqueryfontmetrics(Value* _rv, Value* mgck_wnd, Value* drw_wnd, Value* txt, bool multiline) asm("_ZN4HPHP24f_magickqueryfontmetricsERKNS_6ObjectES2_RKNS_6StringEb");

void fg1_magickqueryfontmetrics(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickqueryfontmetrics(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfArray;
  fh_magickqueryfontmetrics(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickqueryfontmetrics(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_magickqueryfontmetrics(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickqueryfontmetrics(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickqueryfontmetrics", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickradialblurimage(Value* mgck_wnd, double angle) asm("_ZN4HPHP23f_magickradialblurimageERKNS_6ObjectEd");

void fg1_magickradialblurimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickradialblurimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickradialblurimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickradialblurimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickradialblurimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickradialblurimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickradialblurimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickraiseimage(Value* mgck_wnd, double width, double height, int x, int y, bool raise) asm("_ZN4HPHP18f_magickraiseimageERKNS_6ObjectEddiib");

void fg1_magickraiseimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickraiseimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-5)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-5);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickraiseimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickraiseimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 6) {
    if ((args - 5)->m_type == KindOfBoolean &&
        (args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickraiseimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num), (bool)(args[-5].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickraiseimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickraiseimage", count, 6, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickreadimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP17f_magickreadimageERKNS_6ObjectERKNS_6StringE");

void fg1_magickreadimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickreadimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickreadimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickreadimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickreadimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickreadimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickreadimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickreadimageblob(Value* mgck_wnd, Value* blob) asm("_ZN4HPHP21f_magickreadimageblobERKNS_6ObjectERKNS_6StringE");

void fg1_magickreadimageblob(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickreadimageblob(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickreadimageblob(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickreadimageblob(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickreadimageblob(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickreadimageblob(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickreadimageblob", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickreadimagefile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP21f_magickreadimagefileERKNS_6ObjectES2_");

void fg1_magickreadimagefile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickreadimagefile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickreadimagefile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickreadimagefile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickreadimagefile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickreadimagefile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickreadimagefile", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickreadimages(Value* mgck_wnd, Value* img_filenames_array) asm("_ZN4HPHP18f_magickreadimagesERKNS_6ObjectERKNS_5ArrayE");

void fg1_magickreadimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickreadimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickreadimages(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickreadimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickreadimages(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickreadimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickreadimages", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickreducenoiseimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP24f_magickreducenoiseimageERKNS_6ObjectEd");

void fg1_magickreducenoiseimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickreducenoiseimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickreducenoiseimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickreducenoiseimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickreducenoiseimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickreducenoiseimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickreducenoiseimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickremoveimage(Value* mgck_wnd) asm("_ZN4HPHP19f_magickremoveimageERKNS_6ObjectE");

void fg1_magickremoveimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickremoveimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickremoveimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickremoveimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickremoveimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickremoveimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickremoveimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magickremoveimageprofile(Value* _rv, Value* mgck_wnd, Value* name) asm("_ZN4HPHP26f_magickremoveimageprofileERKNS_6ObjectERKNS_6StringE");

void fg1_magickremoveimageprofile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickremoveimageprofile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_magickremoveimageprofile(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magickremoveimageprofile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_magickremoveimageprofile(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magickremoveimageprofile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickremoveimageprofile", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickremoveimageprofiles(Value* mgck_wnd) asm("_ZN4HPHP27f_magickremoveimageprofilesERKNS_6ObjectE");

void fg1_magickremoveimageprofiles(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickremoveimageprofiles(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickremoveimageprofiles(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickremoveimageprofiles(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickremoveimageprofiles(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickremoveimageprofiles(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickremoveimageprofiles", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickresampleimage(Value* mgck_wnd, double x_resolution, double y_resolution, int filter_type, double blur) asm("_ZN4HPHP21f_magickresampleimageERKNS_6ObjectEddid");

void fg1_magickresampleimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickresampleimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickresampleimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (args[-4].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickresampleimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickresampleimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (args[-4].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickresampleimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickresampleimage", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_magickresetiterator(Value* mgck_wnd) asm("_ZN4HPHP21f_magickresetiteratorERKNS_6ObjectE");

void fg1_magickresetiterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickresetiterator(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_magickresetiterator(&args[-0].m_data);
}

TypedValue* fg_magickresetiterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_magickresetiterator(&args[-0].m_data);
    } else {
      fg1_magickresetiterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickresetiterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickresizeimage(Value* mgck_wnd, double columns, double rows, int filter_type, double blur) asm("_ZN4HPHP19f_magickresizeimageERKNS_6ObjectEddid");

void fg1_magickresizeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickresizeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickresizeimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (args[-4].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickresizeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickresizeimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (args[-4].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickresizeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickresizeimage", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickrollimage(Value* mgck_wnd, int x_offset, int y_offset) asm("_ZN4HPHP17f_magickrollimageERKNS_6ObjectEii");

void fg1_magickrollimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickrollimage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickrollimage(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickrollimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickrollimage(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickrollimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickrollimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickrotateimage(Value* mgck_wnd, Value* background, double degrees) asm("_ZN4HPHP19f_magickrotateimageERKNS_6ObjectES2_d");

void fg1_magickrotateimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickrotateimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickrotateimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickrotateimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickrotateimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickrotateimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickrotateimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksampleimage(Value* mgck_wnd, double columns, double rows) asm("_ZN4HPHP19f_magicksampleimageERKNS_6ObjectEdd");

void fg1_magicksampleimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksampleimage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksampleimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksampleimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksampleimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksampleimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksampleimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickscaleimage(Value* mgck_wnd, double columns, double rows) asm("_ZN4HPHP18f_magickscaleimageERKNS_6ObjectEdd");

void fg1_magickscaleimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickscaleimage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magickscaleimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickscaleimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickscaleimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickscaleimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickscaleimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickseparateimagechannel(Value* mgck_wnd, int channel_type) asm("_ZN4HPHP28f_magickseparateimagechannelERKNS_6ObjectEi");

void fg1_magickseparateimagechannel(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickseparateimagechannel(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickseparateimagechannel(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickseparateimagechannel(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickseparateimagechannel(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickseparateimagechannel(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickseparateimagechannel", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetcompressionquality(Value* mgck_wnd, double quality) asm("_ZN4HPHP29f_magicksetcompressionqualityERKNS_6ObjectEd");

void fg1_magicksetcompressionquality(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetcompressionquality(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetcompressionquality(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetcompressionquality(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetcompressionquality(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetcompressionquality(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetcompressionquality", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetfilename(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP19f_magicksetfilenameERKNS_6ObjectERKNS_6StringE");

void fg1_magicksetfilename(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetfilename(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksetfilename(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetfilename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetfilename(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_magicksetfilename(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetfilename", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_magicksetfirstiterator(Value* mgck_wnd) asm("_ZN4HPHP24f_magicksetfirstiteratorERKNS_6ObjectE");

void fg1_magicksetfirstiterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetfirstiterator(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_magicksetfirstiterator(&args[-0].m_data);
}

TypedValue* fg_magicksetfirstiterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_magicksetfirstiterator(&args[-0].m_data);
    } else {
      fg1_magicksetfirstiterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetfirstiterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetformat(Value* mgck_wnd, Value* format) asm("_ZN4HPHP17f_magicksetformatERKNS_6ObjectERKNS_6StringE");

void fg1_magicksetformat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetformat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetformat(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetformat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetformat(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetformat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetformat", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimage(Value* mgck_wnd, Value* replace_wand) asm("_ZN4HPHP16f_magicksetimageERKNS_6ObjectES2_");

void fg1_magicksetimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagebackgroundcolor(Value* mgck_wnd, Value* background_pxl_wnd) asm("_ZN4HPHP31f_magicksetimagebackgroundcolorERKNS_6ObjectES2_");

void fg1_magicksetimagebackgroundcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagebackgroundcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagebackgroundcolor(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagebackgroundcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagebackgroundcolor(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagebackgroundcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagebackgroundcolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagebias(Value* mgck_wnd, double bias) asm("_ZN4HPHP20f_magicksetimagebiasERKNS_6ObjectEd");

void fg1_magicksetimagebias(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagebias(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagebias(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagebias(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagebias(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagebias(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagebias", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageblueprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP27f_magicksetimageblueprimaryERKNS_6ObjectEdd");

void fg1_magicksetimageblueprimary(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageblueprimary(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksetimageblueprimary(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageblueprimary(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageblueprimary(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageblueprimary(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageblueprimary", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagebordercolor(Value* mgck_wnd, Value* border_pxl_wnd) asm("_ZN4HPHP27f_magicksetimagebordercolorERKNS_6ObjectES2_");

void fg1_magicksetimagebordercolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagebordercolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagebordercolor(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagebordercolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagebordercolor(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagebordercolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagebordercolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagecolormapcolor(Value* mgck_wnd, double index, Value* mapcolor_pxl_wnd) asm("_ZN4HPHP29f_magicksetimagecolormapcolorERKNS_6ObjectEdS2_");

void fg1_magicksetimagecolormapcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagecolormapcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagecolormapcolor(&args[-0].m_data, (args[-1].m_data.dbl), &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagecolormapcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfObject &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagecolormapcolor(&args[-0].m_data, (args[-1].m_data.dbl), &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagecolormapcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagecolormapcolor", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagecolorspace(Value* mgck_wnd, int colorspace_type) asm("_ZN4HPHP26f_magicksetimagecolorspaceERKNS_6ObjectEi");

void fg1_magicksetimagecolorspace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagecolorspace(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagecolorspace(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagecolorspace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagecolorspace(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagecolorspace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagecolorspace", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagecompose(Value* mgck_wnd, int composite_operator) asm("_ZN4HPHP23f_magicksetimagecomposeERKNS_6ObjectEi");

void fg1_magicksetimagecompose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagecompose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagecompose(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagecompose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagecompose(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagecompose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagecompose", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagecompression(Value* mgck_wnd, int compression_type) asm("_ZN4HPHP27f_magicksetimagecompressionERKNS_6ObjectEi");

void fg1_magicksetimagecompression(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagecompression(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagecompression(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagecompression(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagecompression(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagecompression(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagecompression", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagecompressionquality(Value* mgck_wnd, double quality) asm("_ZN4HPHP34f_magicksetimagecompressionqualityERKNS_6ObjectEd");

void fg1_magicksetimagecompressionquality(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagecompressionquality(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagecompressionquality(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagecompressionquality(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagecompressionquality(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagecompressionquality(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagecompressionquality", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagedelay(Value* mgck_wnd, double delay) asm("_ZN4HPHP21f_magicksetimagedelayERKNS_6ObjectEd");

void fg1_magicksetimagedelay(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagedelay(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagedelay(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagedelay(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagedelay(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagedelay(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagedelay", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagedepth(Value* mgck_wnd, int depth, int channel_type) asm("_ZN4HPHP21f_magicksetimagedepthERKNS_6ObjectEii");

void fg1_magicksetimagedepth(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagedepth(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagedepth(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagedepth(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagedepth(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagedepth(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagedepth", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagedispose(Value* mgck_wnd, int dispose_type) asm("_ZN4HPHP23f_magicksetimagedisposeERKNS_6ObjectEi");

void fg1_magicksetimagedispose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagedispose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagedispose(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagedispose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagedispose(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagedispose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagedispose", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagefilename(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP24f_magicksetimagefilenameERKNS_6ObjectERKNS_6StringE");

void fg1_magicksetimagefilename(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagefilename(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksetimagefilename(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagefilename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagefilename(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagefilename(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagefilename", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageformat(Value* mgck_wnd, Value* format) asm("_ZN4HPHP22f_magicksetimageformatERKNS_6ObjectERKNS_6StringE");

void fg1_magicksetimageformat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageformat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimageformat(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageformat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageformat(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageformat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageformat", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagegamma(Value* mgck_wnd, double gamma) asm("_ZN4HPHP21f_magicksetimagegammaERKNS_6ObjectEd");

void fg1_magicksetimagegamma(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagegamma(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagegamma(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagegamma(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagegamma(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagegamma(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagegamma", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagegreenprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP28f_magicksetimagegreenprimaryERKNS_6ObjectEdd");

void fg1_magicksetimagegreenprimary(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagegreenprimary(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksetimagegreenprimary(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagegreenprimary(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagegreenprimary(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagegreenprimary(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagegreenprimary", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageindex(Value* mgck_wnd, int index) asm("_ZN4HPHP21f_magicksetimageindexERKNS_6ObjectEi");

void fg1_magicksetimageindex(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageindex(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimageindex(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageindex(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageindex(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageindex(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageindex", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageinterlacescheme(Value* mgck_wnd, int interlace_type) asm("_ZN4HPHP31f_magicksetimageinterlaceschemeERKNS_6ObjectEi");

void fg1_magicksetimageinterlacescheme(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageinterlacescheme(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimageinterlacescheme(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageinterlacescheme(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageinterlacescheme(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageinterlacescheme(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageinterlacescheme", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageiterations(Value* mgck_wnd, double iterations) asm("_ZN4HPHP26f_magicksetimageiterationsERKNS_6ObjectEd");

void fg1_magicksetimageiterations(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageiterations(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimageiterations(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageiterations(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageiterations(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageiterations(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageiterations", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagemattecolor(Value* mgck_wnd, Value* matte_pxl_wnd) asm("_ZN4HPHP26f_magicksetimagemattecolorERKNS_6ObjectES2_");

void fg1_magicksetimagemattecolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagemattecolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagemattecolor(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagemattecolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagemattecolor(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagemattecolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagemattecolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageoption(Value* mgck_wnd, Value* format, Value* key, Value* value) asm("_ZN4HPHP22f_magicksetimageoptionERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_magicksetimageoption(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageoption(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimageoption(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageoption(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageoption(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageoption(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageoption", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagepixels(Value* mgck_wnd, int x_offset, int y_offset, double columns, double rows, Value* smap, int storage_type, Value* pixel_array) asm("_ZN4HPHP22f_magicksetimagepixelsERKNS_6ObjectEiiddRKNS_6StringEiRKNS_5ArrayE");

void fg1_magicksetimagepixels(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagepixels(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-7)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-7);
  }
  if ((args-6)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-6);
  }
  if (!IS_STRING_TYPE((args-5)->m_type)) {
    tvCastToStringInPlace(args-5);
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_data.num = (fh_magicksetimagepixels(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (args[-4].m_data.dbl), &args[-5].m_data, (int)(args[-6].m_data.num), &args[-7].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagepixels(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 8) {
    if ((args - 7)->m_type == KindOfArray &&
        (args - 6)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 5)->m_type) &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagepixels(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args[-3].m_data.dbl), (args[-4].m_data.dbl), &args[-5].m_data, (int)(args[-6].m_data.num), &args[-7].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagepixels(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagepixels", count, 8, 8, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 8);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageprofile(Value* mgck_wnd, Value* name, Value* profile) asm("_ZN4HPHP23f_magicksetimageprofileERKNS_6ObjectERKNS_6StringES5_");

void fg1_magicksetimageprofile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageprofile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimageprofile(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageprofile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageprofile(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageprofile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageprofile", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageredprimary(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP26f_magicksetimageredprimaryERKNS_6ObjectEdd");

void fg1_magicksetimageredprimary(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageredprimary(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksetimageredprimary(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageredprimary(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageredprimary(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageredprimary(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageredprimary", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagerenderingintent(Value* mgck_wnd, int rendering_intent) asm("_ZN4HPHP31f_magicksetimagerenderingintentERKNS_6ObjectEi");

void fg1_magicksetimagerenderingintent(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagerenderingintent(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagerenderingintent(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagerenderingintent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagerenderingintent(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagerenderingintent(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagerenderingintent", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageresolution(Value* mgck_wnd, double x_resolution, double y_resolution) asm("_ZN4HPHP26f_magicksetimageresolutionERKNS_6ObjectEdd");

void fg1_magicksetimageresolution(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageresolution(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksetimageresolution(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageresolution(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageresolution(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageresolution(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageresolution", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagescene(Value* mgck_wnd, double scene) asm("_ZN4HPHP21f_magicksetimagesceneERKNS_6ObjectEd");

void fg1_magicksetimagescene(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagescene(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagescene(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagescene(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagescene(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagescene(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagescene", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagetype(Value* mgck_wnd, int image_type) asm("_ZN4HPHP20f_magicksetimagetypeERKNS_6ObjectEi");

void fg1_magicksetimagetype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagetype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagetype(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagetype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagetype(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagetype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagetype", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimageunits(Value* mgck_wnd, int resolution_type) asm("_ZN4HPHP21f_magicksetimageunitsERKNS_6ObjectEi");

void fg1_magicksetimageunits(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimageunits(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimageunits(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimageunits(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimageunits(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimageunits(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimageunits", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagevirtualpixelmethod(Value* mgck_wnd, int virtual_pixel_method) asm("_ZN4HPHP34f_magicksetimagevirtualpixelmethodERKNS_6ObjectEi");

void fg1_magicksetimagevirtualpixelmethod(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagevirtualpixelmethod(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetimagevirtualpixelmethod(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagevirtualpixelmethod(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagevirtualpixelmethod(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagevirtualpixelmethod(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagevirtualpixelmethod", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetimagewhitepoint(Value* mgck_wnd, double x, double y) asm("_ZN4HPHP26f_magicksetimagewhitepointERKNS_6ObjectEdd");

void fg1_magicksetimagewhitepoint(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetimagewhitepoint(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksetimagewhitepoint(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetimagewhitepoint(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetimagewhitepoint(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetimagewhitepoint(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetimagewhitepoint", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetinterlacescheme(Value* mgck_wnd, int interlace_type) asm("_ZN4HPHP26f_magicksetinterlaceschemeERKNS_6ObjectEi");

void fg1_magicksetinterlacescheme(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetinterlacescheme(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetinterlacescheme(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetinterlacescheme(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetinterlacescheme(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetinterlacescheme(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetinterlacescheme", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_magicksetlastiterator(Value* mgck_wnd) asm("_ZN4HPHP23f_magicksetlastiteratorERKNS_6ObjectE");

void fg1_magicksetlastiterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetlastiterator(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_magicksetlastiterator(&args[-0].m_data);
}

TypedValue* fg_magicksetlastiterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_magicksetlastiterator(&args[-0].m_data);
    } else {
      fg1_magicksetlastiterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetlastiterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetpassphrase(Value* mgck_wnd, Value* passphrase) asm("_ZN4HPHP21f_magicksetpassphraseERKNS_6ObjectERKNS_6StringE");

void fg1_magicksetpassphrase(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetpassphrase(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetpassphrase(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetpassphrase(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetpassphrase(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetpassphrase(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetpassphrase", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetresolution(Value* mgck_wnd, double x_resolution, double y_resolution) asm("_ZN4HPHP21f_magicksetresolutionERKNS_6ObjectEdd");

void fg1_magicksetresolution(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetresolution(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magicksetresolution(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetresolution(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetresolution(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksetresolution(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetresolution", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetsamplingfactors(Value* mgck_wnd, double number_factors, Value* sampling_factors) asm("_ZN4HPHP26f_magicksetsamplingfactorsERKNS_6ObjectEdRKNS_5ArrayE");

void fg1_magicksetsamplingfactors(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetsamplingfactors(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetsamplingfactors(&args[-0].m_data, (args[-1].m_data.dbl), &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicksetsamplingfactors(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfArray &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetsamplingfactors(&args[-0].m_data, (args[-1].m_data.dbl), &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicksetsamplingfactors(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetsamplingfactors", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetsize(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP15f_magicksetsizeERKNS_6ObjectEii");

void fg1_magicksetsize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetsize(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetsize(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetsize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetsize(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetsize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetsize", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksetwandsize(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP19f_magicksetwandsizeERKNS_6ObjectEii");

void fg1_magicksetwandsize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksetwandsize(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksetwandsize(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magicksetwandsize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksetwandsize(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magicksetwandsize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksetwandsize", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksharpenimage(Value* mgck_wnd, double radius, double sigma, int channel_type) asm("_ZN4HPHP20f_magicksharpenimageERKNS_6ObjectEddi");

void fg1_magicksharpenimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksharpenimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksharpenimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magicksharpenimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksharpenimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magicksharpenimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksharpenimage", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickshaveimage(Value* mgck_wnd, int columns, int rows) asm("_ZN4HPHP18f_magickshaveimageERKNS_6ObjectEii");

void fg1_magickshaveimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickshaveimage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickshaveimage(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickshaveimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickshaveimage(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickshaveimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickshaveimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickshearimage(Value* mgck_wnd, Value* background, double x_shear, double y_shear) asm("_ZN4HPHP18f_magickshearimageERKNS_6ObjectES2_dd");

void fg1_magickshearimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickshearimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickshearimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickshearimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickshearimage(&args[-0].m_data, &args[-1].m_data, (args[-2].m_data.dbl), (args[-3].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickshearimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickshearimage", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicksolarizeimage(Value* mgck_wnd, double threshold) asm("_ZN4HPHP21f_magicksolarizeimageERKNS_6ObjectEd");

void fg1_magicksolarizeimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksolarizeimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicksolarizeimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicksolarizeimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicksolarizeimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicksolarizeimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksolarizeimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickspliceimage(Value* mgck_wnd, double width, double height, int x, int y) asm("_ZN4HPHP19f_magickspliceimageERKNS_6ObjectEddii");

void fg1_magickspliceimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickspliceimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickspliceimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_magickspliceimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickspliceimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (int)(args[-3].m_data.num), (int)(args[-4].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_magickspliceimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickspliceimage", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickspreadimage(Value* mgck_wnd, double radius) asm("_ZN4HPHP19f_magickspreadimageERKNS_6ObjectEd");

void fg1_magickspreadimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickspreadimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickspreadimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickspreadimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickspreadimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickspreadimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickspreadimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magicksteganoimage(Value* _rv, Value* mgck_wnd, Value* watermark_wand, int offset) asm("_ZN4HPHP20f_magicksteganoimageERKNS_6ObjectES2_i");

void fg1_magicksteganoimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicksteganoimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magicksteganoimage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magicksteganoimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magicksteganoimage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magicksteganoimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicksteganoimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickstereoimage(Value* mgck_wnd, Value* offset_wand) asm("_ZN4HPHP19f_magickstereoimageERKNS_6ObjectES2_");

void fg1_magickstereoimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickstereoimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickstereoimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickstereoimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickstereoimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickstereoimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickstereoimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickstripimage(Value* mgck_wnd) asm("_ZN4HPHP18f_magickstripimageERKNS_6ObjectE");

void fg1_magickstripimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickstripimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickstripimage(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickstripimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickstripimage(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickstripimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickstripimage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickswirlimage(Value* mgck_wnd, double degrees) asm("_ZN4HPHP18f_magickswirlimageERKNS_6ObjectEd");

void fg1_magickswirlimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickswirlimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickswirlimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickswirlimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickswirlimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickswirlimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickswirlimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magicktextureimage(Value* _rv, Value* mgck_wnd, Value* texture_wand) asm("_ZN4HPHP20f_magicktextureimageERKNS_6ObjectES2_");

void fg1_magicktextureimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicktextureimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magicktextureimage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magicktextureimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magicktextureimage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magicktextureimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicktextureimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickthresholdimage(Value* mgck_wnd, double threshold, int channel_type) asm("_ZN4HPHP22f_magickthresholdimageERKNS_6ObjectEdi");

void fg1_magickthresholdimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickthresholdimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickthresholdimage(&args[-0].m_data, (args[-1].m_data.dbl), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magickthresholdimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickthresholdimage(&args[-0].m_data, (args[-1].m_data.dbl), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magickthresholdimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickthresholdimage", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicktintimage(Value* mgck_wnd, int tint_pxl_wnd, Value* opacity_pxl_wnd) asm("_ZN4HPHP17f_magicktintimageERKNS_6ObjectEiS2_");

void fg1_magicktintimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicktintimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicktintimage(&args[-0].m_data, (int)(args[-1].m_data.num), &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magicktintimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfObject &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicktintimage(&args[-0].m_data, (int)(args[-1].m_data.num), &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magicktintimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicktintimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_magicktransformimage(Value* _rv, Value* mgck_wnd, Value* crop, Value* geometry) asm("_ZN4HPHP22f_magicktransformimageERKNS_6ObjectERKNS_6StringES5_");

void fg1_magicktransformimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicktransformimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_magicktransformimage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_magicktransformimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_magicktransformimage(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_magicktransformimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicktransformimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magicktrimimage(Value* mgck_wnd, double fuzz) asm("_ZN4HPHP17f_magicktrimimageERKNS_6ObjectEd");

void fg1_magicktrimimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magicktrimimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magicktrimimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magicktrimimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magicktrimimage(&args[-0].m_data, (args[-1].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magicktrimimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magicktrimimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickunsharpmaskimage(Value* mgck_wnd, double radius, double sigma, double amount, double threshold, int channel_type) asm("_ZN4HPHP24f_magickunsharpmaskimageERKNS_6ObjectEddddi");

void fg1_magickunsharpmaskimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickunsharpmaskimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    break;
  }
  if ((args-4)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickunsharpmaskimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_magickunsharpmaskimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 5 && count <= 6) {
    if ((count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        (args - 4)->m_type == KindOfDouble &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickunsharpmaskimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (args[-4].m_data.dbl), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_magickunsharpmaskimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickunsharpmaskimage", count, 5, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickwaveimage(Value* mgck_wnd, double amplitude, double wave_length) asm("_ZN4HPHP17f_magickwaveimageERKNS_6ObjectEdd");

void fg1_magickwaveimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickwaveimage(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_magickwaveimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
}

TypedValue* fg_magickwaveimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickwaveimage(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl))) ? 1LL : 0LL;
    } else {
      fg1_magickwaveimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickwaveimage", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickwhitethresholdimage(Value* mgck_wnd, Value* threshold_pxl_wnd) asm("_ZN4HPHP27f_magickwhitethresholdimageERKNS_6ObjectES2_");

void fg1_magickwhitethresholdimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickwhitethresholdimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickwhitethresholdimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickwhitethresholdimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickwhitethresholdimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickwhitethresholdimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickwhitethresholdimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickwriteimage(Value* mgck_wnd, Value* filename) asm("_ZN4HPHP18f_magickwriteimageERKNS_6ObjectERKNS_6StringE");

void fg1_magickwriteimage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickwriteimage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickwriteimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickwriteimage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickwriteimage(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickwriteimage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickwriteimage", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickwriteimagefile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP22f_magickwriteimagefileERKNS_6ObjectES2_");

void fg1_magickwriteimagefile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickwriteimagefile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickwriteimagefile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickwriteimagefile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickwriteimagefile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickwriteimagefile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickwriteimagefile", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickwriteimages(Value* mgck_wnd, Value* filename, bool join_images) asm("_ZN4HPHP19f_magickwriteimagesERKNS_6ObjectERKNS_6StringEb");

void fg1_magickwriteimages(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickwriteimages(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  String defVal1 = "";
  rv->m_data.num = (fh_magickwriteimages(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_magickwriteimages(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      String defVal1 = "";
      rv->m_data.num = (fh_magickwriteimages(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_magickwriteimages(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickwriteimages", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_magickwriteimagesfile(Value* mgck_wnd, Value* handle) asm("_ZN4HPHP23f_magickwriteimagesfileERKNS_6ObjectES2_");

void fg1_magickwriteimagesfile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_magickwriteimagesfile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_magickwriteimagesfile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_magickwriteimagesfile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_magickwriteimagesfile(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_magickwriteimagesfile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("magickwriteimagesfile", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetalpha(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetalphaERKNS_6ObjectE");

void fg1_pixelgetalpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetalpha(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetalpha(&args[-0].m_data);
}

TypedValue* fg_pixelgetalpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetalpha(&args[-0].m_data);
    } else {
      fg1_pixelgetalpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetalpha", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetalphaquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetalphaquantumERKNS_6ObjectE");

void fg1_pixelgetalphaquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetalphaquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetalphaquantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetalphaquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetalphaquantum(&args[-0].m_data);
    } else {
      fg1_pixelgetalphaquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetalphaquantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetblack(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetblackERKNS_6ObjectE");

void fg1_pixelgetblack(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetblack(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetblack(&args[-0].m_data);
}

TypedValue* fg_pixelgetblack(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetblack(&args[-0].m_data);
    } else {
      fg1_pixelgetblack(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetblack", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetblackquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetblackquantumERKNS_6ObjectE");

void fg1_pixelgetblackquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetblackquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetblackquantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetblackquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetblackquantum(&args[-0].m_data);
    } else {
      fg1_pixelgetblackquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetblackquantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetblue(Value* pxl_wnd) asm("_ZN4HPHP14f_pixelgetblueERKNS_6ObjectE");

void fg1_pixelgetblue(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetblue(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetblue(&args[-0].m_data);
}

TypedValue* fg_pixelgetblue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetblue(&args[-0].m_data);
    } else {
      fg1_pixelgetblue(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetblue", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetbluequantum(Value* pxl_wnd) asm("_ZN4HPHP21f_pixelgetbluequantumERKNS_6ObjectE");

void fg1_pixelgetbluequantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetbluequantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetbluequantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetbluequantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetbluequantum(&args[-0].m_data);
    } else {
      fg1_pixelgetbluequantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetbluequantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pixelgetcolorasstring(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetcolorasstringERKNS_6ObjectE");

void fg1_pixelgetcolorasstring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetcolorasstring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_pixelgetcolorasstring(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pixelgetcolorasstring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_pixelgetcolorasstring(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pixelgetcolorasstring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetcolorasstring", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetcolorcount(Value* pxl_wnd) asm("_ZN4HPHP20f_pixelgetcolorcountERKNS_6ObjectE");

void fg1_pixelgetcolorcount(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetcolorcount(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetcolorcount(&args[-0].m_data);
}

TypedValue* fg_pixelgetcolorcount(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetcolorcount(&args[-0].m_data);
    } else {
      fg1_pixelgetcolorcount(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetcolorcount", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetcyan(Value* pxl_wnd) asm("_ZN4HPHP14f_pixelgetcyanERKNS_6ObjectE");

void fg1_pixelgetcyan(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetcyan(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetcyan(&args[-0].m_data);
}

TypedValue* fg_pixelgetcyan(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetcyan(&args[-0].m_data);
    } else {
      fg1_pixelgetcyan(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetcyan", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetcyanquantum(Value* pxl_wnd) asm("_ZN4HPHP21f_pixelgetcyanquantumERKNS_6ObjectE");

void fg1_pixelgetcyanquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetcyanquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetcyanquantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetcyanquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetcyanquantum(&args[-0].m_data);
    } else {
      fg1_pixelgetcyanquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetcyanquantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pixelgetexception(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP19f_pixelgetexceptionERKNS_6ObjectE");

void fg1_pixelgetexception(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetexception(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_pixelgetexception(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pixelgetexception(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_pixelgetexception(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pixelgetexception(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetexception", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pixelgetexceptionstring(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP25f_pixelgetexceptionstringERKNS_6ObjectE");

void fg1_pixelgetexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_pixelgetexceptionstring(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pixelgetexceptionstring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_pixelgetexceptionstring(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pixelgetexceptionstring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetexceptionstring", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pixelgetexceptiontype(Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetexceptiontypeERKNS_6ObjectE");

void fg1_pixelgetexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pixelgetexceptiontype(&args[-0].m_data);
}

TypedValue* fg_pixelgetexceptiontype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pixelgetexceptiontype(&args[-0].m_data);
    } else {
      fg1_pixelgetexceptiontype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetexceptiontype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetgreen(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetgreenERKNS_6ObjectE");

void fg1_pixelgetgreen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetgreen(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetgreen(&args[-0].m_data);
}

TypedValue* fg_pixelgetgreen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetgreen(&args[-0].m_data);
    } else {
      fg1_pixelgetgreen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetgreen", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetgreenquantum(Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetgreenquantumERKNS_6ObjectE");

void fg1_pixelgetgreenquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetgreenquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetgreenquantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetgreenquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetgreenquantum(&args[-0].m_data);
    } else {
      fg1_pixelgetgreenquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetgreenquantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetindex(Value* pxl_wnd) asm("_ZN4HPHP15f_pixelgetindexERKNS_6ObjectE");

void fg1_pixelgetindex(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetindex(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetindex(&args[-0].m_data);
}

TypedValue* fg_pixelgetindex(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetindex(&args[-0].m_data);
    } else {
      fg1_pixelgetindex(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetindex", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetmagenta(Value* pxl_wnd) asm("_ZN4HPHP17f_pixelgetmagentaERKNS_6ObjectE");

void fg1_pixelgetmagenta(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetmagenta(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetmagenta(&args[-0].m_data);
}

TypedValue* fg_pixelgetmagenta(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetmagenta(&args[-0].m_data);
    } else {
      fg1_pixelgetmagenta(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetmagenta", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetmagentaquantum(Value* pxl_wnd) asm("_ZN4HPHP24f_pixelgetmagentaquantumERKNS_6ObjectE");

void fg1_pixelgetmagentaquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetmagentaquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetmagentaquantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetmagentaquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetmagentaquantum(&args[-0].m_data);
    } else {
      fg1_pixelgetmagentaquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetmagentaquantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetopacity(Value* pxl_wnd) asm("_ZN4HPHP17f_pixelgetopacityERKNS_6ObjectE");

void fg1_pixelgetopacity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetopacity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetopacity(&args[-0].m_data);
}

TypedValue* fg_pixelgetopacity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetopacity(&args[-0].m_data);
    } else {
      fg1_pixelgetopacity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetopacity", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetopacityquantum(Value* pxl_wnd) asm("_ZN4HPHP24f_pixelgetopacityquantumERKNS_6ObjectE");

void fg1_pixelgetopacityquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetopacityquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetopacityquantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetopacityquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetopacityquantum(&args[-0].m_data);
    } else {
      fg1_pixelgetopacityquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetopacityquantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pixelgetquantumcolor(Value* _rv, Value* pxl_wnd) asm("_ZN4HPHP22f_pixelgetquantumcolorERKNS_6ObjectE");

void fg1_pixelgetquantumcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetquantumcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_pixelgetquantumcolor(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pixelgetquantumcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_pixelgetquantumcolor(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pixelgetquantumcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetquantumcolor", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetred(Value* pxl_wnd) asm("_ZN4HPHP13f_pixelgetredERKNS_6ObjectE");

void fg1_pixelgetred(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetred(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetred(&args[-0].m_data);
}

TypedValue* fg_pixelgetred(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetred(&args[-0].m_data);
    } else {
      fg1_pixelgetred(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetred", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetredquantum(Value* pxl_wnd) asm("_ZN4HPHP20f_pixelgetredquantumERKNS_6ObjectE");

void fg1_pixelgetredquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetredquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetredquantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetredquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetredquantum(&args[-0].m_data);
    } else {
      fg1_pixelgetredquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetredquantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetyellow(Value* pxl_wnd) asm("_ZN4HPHP16f_pixelgetyellowERKNS_6ObjectE");

void fg1_pixelgetyellow(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetyellow(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetyellow(&args[-0].m_data);
}

TypedValue* fg_pixelgetyellow(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetyellow(&args[-0].m_data);
    } else {
      fg1_pixelgetyellow(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetyellow", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

double fh_pixelgetyellowquantum(Value* pxl_wnd) asm("_ZN4HPHP23f_pixelgetyellowquantumERKNS_6ObjectE");

void fg1_pixelgetyellowquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetyellowquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfDouble;
  rv->m_data.dbl = fh_pixelgetyellowquantum(&args[-0].m_data);
}

TypedValue* fg_pixelgetyellowquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfDouble;
      rv->m_data.dbl = fh_pixelgetyellowquantum(&args[-0].m_data);
    } else {
      fg1_pixelgetyellowquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetyellowquantum", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetalpha(Value* pxl_wnd, double alpha) asm("_ZN4HPHP15f_pixelsetalphaERKNS_6ObjectEd");

void fg1_pixelsetalpha(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetalpha(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetalpha(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetalpha(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetalpha(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetalpha(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetalpha", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetalphaquantum(Value* pxl_wnd, double alpha) asm("_ZN4HPHP22f_pixelsetalphaquantumERKNS_6ObjectEd");

void fg1_pixelsetalphaquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetalphaquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetalphaquantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetalphaquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetalphaquantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetalphaquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetalphaquantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetblack(Value* pxl_wnd, double black) asm("_ZN4HPHP15f_pixelsetblackERKNS_6ObjectEd");

void fg1_pixelsetblack(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetblack(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetblack(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetblack(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetblack(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetblack(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetblack", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetblackquantum(Value* pxl_wnd, double black) asm("_ZN4HPHP22f_pixelsetblackquantumERKNS_6ObjectEd");

void fg1_pixelsetblackquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetblackquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetblackquantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetblackquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetblackquantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetblackquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetblackquantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetblue(Value* pxl_wnd, double blue) asm("_ZN4HPHP14f_pixelsetblueERKNS_6ObjectEd");

void fg1_pixelsetblue(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetblue(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetblue(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetblue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetblue(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetblue(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetblue", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetbluequantum(Value* pxl_wnd, double blue) asm("_ZN4HPHP21f_pixelsetbluequantumERKNS_6ObjectEd");

void fg1_pixelsetbluequantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetbluequantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetbluequantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetbluequantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetbluequantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetbluequantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetbluequantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetcolor(Value* pxl_wnd, Value* imagemagick_col_str) asm("_ZN4HPHP15f_pixelsetcolorERKNS_6ObjectERKNS_6StringE");

void fg1_pixelsetcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetcolor(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_pixelsetcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetcolor(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_pixelsetcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetcolor", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetcolorcount(Value* pxl_wnd, int count) asm("_ZN4HPHP20f_pixelsetcolorcountERKNS_6ObjectEi");

void fg1_pixelsetcolorcount(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetcolorcount(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetcolorcount(&args[-0].m_data, (int)(args[-1].m_data.num));
}

TypedValue* fg_pixelsetcolorcount(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetcolorcount(&args[-0].m_data, (int)(args[-1].m_data.num));
    } else {
      fg1_pixelsetcolorcount(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetcolorcount", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetcyan(Value* pxl_wnd, double cyan) asm("_ZN4HPHP14f_pixelsetcyanERKNS_6ObjectEd");

void fg1_pixelsetcyan(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetcyan(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetcyan(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetcyan(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetcyan(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetcyan(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetcyan", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetcyanquantum(Value* pxl_wnd, double cyan) asm("_ZN4HPHP21f_pixelsetcyanquantumERKNS_6ObjectEd");

void fg1_pixelsetcyanquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetcyanquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetcyanquantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetcyanquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetcyanquantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetcyanquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetcyanquantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetgreen(Value* pxl_wnd, double green) asm("_ZN4HPHP15f_pixelsetgreenERKNS_6ObjectEd");

void fg1_pixelsetgreen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetgreen(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetgreen(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetgreen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetgreen(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetgreen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetgreen", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetgreenquantum(Value* pxl_wnd, double green) asm("_ZN4HPHP22f_pixelsetgreenquantumERKNS_6ObjectEd");

void fg1_pixelsetgreenquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetgreenquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetgreenquantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetgreenquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetgreenquantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetgreenquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetgreenquantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetindex(Value* pxl_wnd, double index) asm("_ZN4HPHP15f_pixelsetindexERKNS_6ObjectEd");

void fg1_pixelsetindex(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetindex(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetindex(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetindex(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetindex(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetindex(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetindex", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetmagenta(Value* pxl_wnd, double magenta) asm("_ZN4HPHP17f_pixelsetmagentaERKNS_6ObjectEd");

void fg1_pixelsetmagenta(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetmagenta(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetmagenta(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetmagenta(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetmagenta(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetmagenta(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetmagenta", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetmagentaquantum(Value* pxl_wnd, double magenta) asm("_ZN4HPHP24f_pixelsetmagentaquantumERKNS_6ObjectEd");

void fg1_pixelsetmagentaquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetmagentaquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetmagentaquantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetmagentaquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetmagentaquantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetmagentaquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetmagentaquantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetopacity(Value* pxl_wnd, double opacity) asm("_ZN4HPHP17f_pixelsetopacityERKNS_6ObjectEd");

void fg1_pixelsetopacity(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetopacity(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetopacity(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetopacity(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetopacity(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetopacity(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetopacity", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetopacityquantum(Value* pxl_wnd, double opacity) asm("_ZN4HPHP24f_pixelsetopacityquantumERKNS_6ObjectEd");

void fg1_pixelsetopacityquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetopacityquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetopacityquantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetopacityquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetopacityquantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetopacityquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetopacityquantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetquantumcolor(Value* pxl_wnd, double red, double green, double blue, double opacity) asm("_ZN4HPHP22f_pixelsetquantumcolorERKNS_6ObjectEdddd");

void fg1_pixelsetquantumcolor(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetquantumcolor(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-4);
    }
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-3);
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
  rv->m_type = KindOfNull;
  fh_pixelsetquantumcolor(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
}

TypedValue* fg_pixelsetquantumcolor(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfDouble) &&
        (args - 3)->m_type == KindOfDouble &&
        (args - 2)->m_type == KindOfDouble &&
        (args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetquantumcolor(&args[-0].m_data, (args[-1].m_data.dbl), (args[-2].m_data.dbl), (args[-3].m_data.dbl), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
    } else {
      fg1_pixelsetquantumcolor(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetquantumcolor", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetred(Value* pxl_wnd, double red) asm("_ZN4HPHP13f_pixelsetredERKNS_6ObjectEd");

void fg1_pixelsetred(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetred(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetred(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetred(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetred(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetred(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetred", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetredquantum(Value* pxl_wnd, double red) asm("_ZN4HPHP20f_pixelsetredquantumERKNS_6ObjectEd");

void fg1_pixelsetredquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetredquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetredquantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetredquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetredquantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetredquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetredquantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetyellow(Value* pxl_wnd, double yellow) asm("_ZN4HPHP16f_pixelsetyellowERKNS_6ObjectEd");

void fg1_pixelsetyellow(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetyellow(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetyellow(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetyellow(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetyellow(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetyellow(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetyellow", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelsetyellowquantum(Value* pxl_wnd, double yellow) asm("_ZN4HPHP23f_pixelsetyellowquantumERKNS_6ObjectEd");

void fg1_pixelsetyellowquantum(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetyellowquantum(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfDouble) {
    tvCastToDoubleInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_pixelsetyellowquantum(&args[-0].m_data, (args[-1].m_data.dbl));
}

TypedValue* fg_pixelsetyellowquantum(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfDouble &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelsetyellowquantum(&args[-0].m_data, (args[-1].m_data.dbl));
    } else {
      fg1_pixelsetyellowquantum(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetyellowquantum", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pixelgetiteratorexception(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP27f_pixelgetiteratorexceptionERKNS_6ObjectE");

void fg1_pixelgetiteratorexception(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetiteratorexception(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_pixelgetiteratorexception(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pixelgetiteratorexception(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_pixelgetiteratorexception(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pixelgetiteratorexception(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetiteratorexception", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pixelgetiteratorexceptionstring(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP33f_pixelgetiteratorexceptionstringERKNS_6ObjectE");

void fg1_pixelgetiteratorexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetiteratorexceptionstring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_pixelgetiteratorexceptionstring(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pixelgetiteratorexceptionstring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_pixelgetiteratorexceptionstring(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pixelgetiteratorexceptionstring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetiteratorexceptionstring", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pixelgetiteratorexceptiontype(Value* pxl_iter) asm("_ZN4HPHP31f_pixelgetiteratorexceptiontypeERKNS_6ObjectE");

void fg1_pixelgetiteratorexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetiteratorexceptiontype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pixelgetiteratorexceptiontype(&args[-0].m_data);
}

TypedValue* fg_pixelgetiteratorexceptiontype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pixelgetiteratorexceptiontype(&args[-0].m_data);
    } else {
      fg1_pixelgetiteratorexceptiontype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetiteratorexceptiontype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pixelgetnextiteratorrow(Value* _rv, Value* pxl_iter) asm("_ZN4HPHP25f_pixelgetnextiteratorrowERKNS_6ObjectE");

void fg1_pixelgetnextiteratorrow(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelgetnextiteratorrow(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_pixelgetnextiteratorrow(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pixelgetnextiteratorrow(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_pixelgetnextiteratorrow(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pixelgetnextiteratorrow(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelgetnextiteratorrow", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pixelresetiterator(Value* pxl_iter) asm("_ZN4HPHP20f_pixelresetiteratorERKNS_6ObjectE");

void fg1_pixelresetiterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelresetiterator(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_pixelresetiterator(&args[-0].m_data);
}

TypedValue* fg_pixelresetiterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_pixelresetiterator(&args[-0].m_data);
    } else {
      fg1_pixelresetiterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelresetiterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pixelsetiteratorrow(Value* pxl_iter, int row) asm("_ZN4HPHP21f_pixelsetiteratorrowERKNS_6ObjectEi");

void fg1_pixelsetiteratorrow(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsetiteratorrow(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_pixelsetiteratorrow(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_pixelsetiteratorrow(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_pixelsetiteratorrow(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_pixelsetiteratorrow(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsetiteratorrow", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pixelsynciterator(Value* pxl_iter) asm("_ZN4HPHP19f_pixelsynciteratorERKNS_6ObjectE");

void fg1_pixelsynciterator(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pixelsynciterator(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_pixelsynciterator(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_pixelsynciterator(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_pixelsynciterator(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_pixelsynciterator(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pixelsynciterator", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
