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

Value* fh_hphp_splfileinfo___construct(Value* _rv, Value* obj, Value* file_name) asm("_ZN4HPHP30f_hphp_splfileinfo___constructERKNS_6ObjectERKNS_6StringE");

void fg1_hphp_splfileinfo___construct(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo___construct(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_hphp_splfileinfo___construct(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_hphp_splfileinfo___construct(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo___construct(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo___construct", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileinfo_getatime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getatimeERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getatime(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getatime(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getatime(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileinfo_getatime(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getatime(&args[-0].m_data);
    } else {
      fg1_hphp_splfileinfo_getatime(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getatime", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_getbasename(Value* _rv, Value* obj, Value* suffix) asm("_ZN4HPHP30f_hphp_splfileinfo_getbasenameERKNS_6ObjectERKNS_6StringE");

void fg1_hphp_splfileinfo_getbasename(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getbasename(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_hphp_splfileinfo_getbasename(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_getbasename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileinfo_getbasename(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_getbasename(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getbasename", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileinfo_getctime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getctimeERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getctime(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getctime(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getctime(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileinfo_getctime(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getctime(&args[-0].m_data);
    } else {
      fg1_hphp_splfileinfo_getctime(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getctime", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_getfileinfo(Value* _rv, Value* obj, Value* class_name) asm("_ZN4HPHP30f_hphp_splfileinfo_getfileinfoERKNS_6ObjectERKNS_6StringE");

void fg1_hphp_splfileinfo_getfileinfo(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getfileinfo(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_hphp_splfileinfo_getfileinfo(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_getfileinfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_hphp_splfileinfo_getfileinfo(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_getfileinfo(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getfileinfo", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_getfilename(Value* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getfilenameERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getfilename(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getfilename(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_hphp_splfileinfo_getfilename(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_getfilename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileinfo_getfilename(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_getfilename(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getfilename", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileinfo_getgroup(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getgroupERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getgroup(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getgroup(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getgroup(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileinfo_getgroup(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getgroup(&args[-0].m_data);
    } else {
      fg1_hphp_splfileinfo_getgroup(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getgroup", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileinfo_getinode(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getinodeERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getinode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getinode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getinode(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileinfo_getinode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getinode(&args[-0].m_data);
    } else {
      fg1_hphp_splfileinfo_getinode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getinode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_getlinktarget(Value* _rv, Value* obj) asm("_ZN4HPHP32f_hphp_splfileinfo_getlinktargetERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getlinktarget(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getlinktarget(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_hphp_splfileinfo_getlinktarget(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_getlinktarget(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileinfo_getlinktarget(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_getlinktarget(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getlinktarget", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileinfo_getmtime(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getmtimeERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getmtime(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getmtime(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getmtime(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileinfo_getmtime(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getmtime(&args[-0].m_data);
    } else {
      fg1_hphp_splfileinfo_getmtime(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getmtime", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileinfo_getowner(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getownerERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getowner(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getowner(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getowner(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileinfo_getowner(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getowner(&args[-0].m_data);
    } else {
      fg1_hphp_splfileinfo_getowner(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getowner", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_getpath(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_getpathERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getpath(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getpath(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_hphp_splfileinfo_getpath(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_getpath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileinfo_getpath(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_getpath(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getpath", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_getpathinfo(Value* _rv, Value* obj, Value* class_name) asm("_ZN4HPHP30f_hphp_splfileinfo_getpathinfoERKNS_6ObjectERKNS_6StringE");

void fg1_hphp_splfileinfo_getpathinfo(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getpathinfo(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_hphp_splfileinfo_getpathinfo(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_getpathinfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_hphp_splfileinfo_getpathinfo(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_getpathinfo(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getpathinfo", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_getpathname(Value* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getpathnameERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getpathname(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getpathname(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_hphp_splfileinfo_getpathname(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_getpathname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileinfo_getpathname(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_getpathname(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getpathname", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileinfo_getperms(Value* obj) asm("_ZN4HPHP27f_hphp_splfileinfo_getpermsERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getperms(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getperms(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getperms(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileinfo_getperms(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getperms(&args[-0].m_data);
    } else {
      fg1_hphp_splfileinfo_getperms(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getperms", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_splfileinfo_getrealpath(TypedValue* _rv, Value* obj) asm("_ZN4HPHP30f_hphp_splfileinfo_getrealpathERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getrealpath(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getrealpath(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileinfo_getrealpath(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_getrealpath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_hphp_splfileinfo_getrealpath(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_getrealpath(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getrealpath", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileinfo_getsize(Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_getsizeERKNS_6ObjectE");

void fg1_hphp_splfileinfo_getsize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_getsize(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getsize(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileinfo_getsize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileinfo_getsize(&args[-0].m_data);
    } else {
      fg1_hphp_splfileinfo_getsize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_getsize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_gettype(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileinfo_gettypeERKNS_6ObjectE");

void fg1_hphp_splfileinfo_gettype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_gettype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_hphp_splfileinfo_gettype(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_gettype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileinfo_gettype(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_gettype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_gettype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileinfo_isdir(Value* obj) asm("_ZN4HPHP24f_hphp_splfileinfo_isdirERKNS_6ObjectE");

void fg1_hphp_splfileinfo_isdir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_isdir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileinfo_isdir(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileinfo_isdir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileinfo_isdir(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileinfo_isdir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_isdir", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileinfo_isexecutable(Value* obj) asm("_ZN4HPHP31f_hphp_splfileinfo_isexecutableERKNS_6ObjectE");

void fg1_hphp_splfileinfo_isexecutable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_isexecutable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileinfo_isexecutable(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileinfo_isexecutable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileinfo_isexecutable(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileinfo_isexecutable(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_isexecutable", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileinfo_isfile(Value* obj) asm("_ZN4HPHP25f_hphp_splfileinfo_isfileERKNS_6ObjectE");

void fg1_hphp_splfileinfo_isfile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_isfile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileinfo_isfile(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileinfo_isfile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileinfo_isfile(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileinfo_isfile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_isfile", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileinfo_islink(Value* obj) asm("_ZN4HPHP25f_hphp_splfileinfo_islinkERKNS_6ObjectE");

void fg1_hphp_splfileinfo_islink(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_islink(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileinfo_islink(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileinfo_islink(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileinfo_islink(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileinfo_islink(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_islink", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileinfo_isreadable(Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo_isreadableERKNS_6ObjectE");

void fg1_hphp_splfileinfo_isreadable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_isreadable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileinfo_isreadable(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileinfo_isreadable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileinfo_isreadable(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileinfo_isreadable(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_isreadable", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileinfo_iswritable(Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo_iswritableERKNS_6ObjectE");

void fg1_hphp_splfileinfo_iswritable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_iswritable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileinfo_iswritable(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileinfo_iswritable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileinfo_iswritable(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileinfo_iswritable(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_iswritable", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo_openfile(Value* _rv, Value* obj, Value* open_mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP27f_hphp_splfileinfo_openfileERKNS_6ObjectERKNS_6StringEbRKNS_7VariantE");

void fg1_hphp_splfileinfo_openfile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_openfile(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfObject;
  fh_hphp_splfileinfo_openfile(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num), (args-3));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo_openfile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 2)->m_type == KindOfBoolean &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_hphp_splfileinfo_openfile(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, (bool)(args[-2].m_data.num), (args-3));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo_openfile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_openfile", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_splfileinfo_setfileclass(Value* obj, Value* class_name) asm("_ZN4HPHP31f_hphp_splfileinfo_setfileclassERKNS_6ObjectERKNS_6StringE");

void fg1_hphp_splfileinfo_setfileclass(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_setfileclass(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_hphp_splfileinfo_setfileclass(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_hphp_splfileinfo_setfileclass(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_hphp_splfileinfo_setfileclass(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_hphp_splfileinfo_setfileclass(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_setfileclass", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_splfileinfo_setinfoclass(Value* obj, Value* class_name) asm("_ZN4HPHP31f_hphp_splfileinfo_setinfoclassERKNS_6ObjectERKNS_6StringE");

void fg1_hphp_splfileinfo_setinfoclass(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo_setinfoclass(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_hphp_splfileinfo_setinfoclass(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_hphp_splfileinfo_setinfoclass(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_hphp_splfileinfo_setinfoclass(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_hphp_splfileinfo_setinfoclass(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo_setinfoclass", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileinfo___tostring(Value* _rv, Value* obj) asm("_ZN4HPHP29f_hphp_splfileinfo___tostringERKNS_6ObjectE");

void fg1_hphp_splfileinfo___tostring(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileinfo___tostring(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_hphp_splfileinfo___tostring(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileinfo___tostring(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileinfo___tostring(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileinfo___tostring(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileinfo___tostring", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileobject___construct(Value* _rv, Value* obj, Value* filename, Value* open_mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP32f_hphp_splfileobject___constructERKNS_6ObjectERKNS_6StringES5_bRKNS_7VariantE");

void fg1_hphp_splfileobject___construct(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject___construct(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-3);
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
  rv->m_type = KindOfObject;
  fh_hphp_splfileobject___construct(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (bool)(args[-3].m_data.num), (args-4));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 3)->m_type == KindOfBoolean &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfObject;
      fh_hphp_splfileobject___construct(&(rv->m_data), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (bool)(args[-3].m_data.num), (args-4));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject___construct(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject___construct", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_splfileobject_current(TypedValue* _rv, Value* obj) asm("_ZN4HPHP28f_hphp_splfileobject_currentERKNS_6ObjectE");

void fg1_hphp_splfileobject_current(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_current(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_current(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject_current(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_hphp_splfileobject_current(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject_current(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_current", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileobject_eof(Value* obj) asm("_ZN4HPHP24f_hphp_splfileobject_eofERKNS_6ObjectE");

void fg1_hphp_splfileobject_eof(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_eof(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileobject_eof(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileobject_eof(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileobject_eof(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileobject_eof(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_eof", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileobject_fflush(Value* obj) asm("_ZN4HPHP27f_hphp_splfileobject_fflushERKNS_6ObjectE");

void fg1_hphp_splfileobject_fflush(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fflush(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileobject_fflush(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileobject_fflush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileobject_fflush(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileobject_fflush(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fflush", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileobject_fgetc(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fgetcERKNS_6ObjectE");

void fg1_hphp_splfileobject_fgetc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fgetc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_hphp_splfileobject_fgetc(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject_fgetc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileobject_fgetc(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject_fgetc(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fgetc", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_splfileobject_fgetcsv(TypedValue* _rv, Value* obj, Value* delimiter, Value* enclosure, Value* escape) asm("_ZN4HPHP28f_hphp_splfileobject_fgetcsvERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_hphp_splfileobject_fgetcsv(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fgetcsv(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_hphp_splfileobject_fgetcsv(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject_fgetcsv(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_hphp_splfileobject_fgetcsv(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject_fgetcsv(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fgetcsv", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileobject_fgets(Value* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fgetsERKNS_6ObjectE");

void fg1_hphp_splfileobject_fgets(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fgets(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfString;
  fh_hphp_splfileobject_fgets(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject_fgets(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileobject_fgets(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject_fgets(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fgets", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphp_splfileobject_fgetss(Value* _rv, Value* obj, Value* allowable_tags) asm("_ZN4HPHP27f_hphp_splfileobject_fgetssERKNS_6ObjectERKNS_6StringE");

void fg1_hphp_splfileobject_fgetss(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fgetss(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_hphp_splfileobject_fgetss(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject_fgetss(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_hphp_splfileobject_fgetss(&(rv->m_data), &args[-0].m_data, &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject_fgetss(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fgetss", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileobject_flock(Value* obj, TypedValue* wouldblock) asm("_ZN4HPHP26f_hphp_splfileobject_flockERKNS_6ObjectERKNS_14VRefParamValueE");

void fg1_hphp_splfileobject_flock(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_flock(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileobject_flock(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileobject_flock(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileobject_flock(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileobject_flock(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_flock", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileobject_fpassthru(Value* obj) asm("_ZN4HPHP30f_hphp_splfileobject_fpassthruERKNS_6ObjectE");

void fg1_hphp_splfileobject_fpassthru(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fpassthru(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_fpassthru(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileobject_fpassthru(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileobject_fpassthru(&args[-0].m_data);
    } else {
      fg1_hphp_splfileobject_fpassthru(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fpassthru", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_splfileobject_fscanf(TypedValue* _rv, long _argc, Value* obj, Value* format, TypedValue* _argv) asm("_ZN4HPHP27f_hphp_splfileobject_fscanfElRKNS_6ObjectERKNS_6StringERKNS_7VariantE");

void fg1_hphp_splfileobject_fscanf(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fscanf(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_hphp_splfileobject_fscanf(rv, (long)(args[-0].m_data.num), &args[-1].m_data, &args[-2].m_data, (args-3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject_fscanf(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfInt64) {
      fh_hphp_splfileobject_fscanf(rv, (long)(args[-0].m_data.num), &args[-1].m_data, &args[-2].m_data, (args-3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject_fscanf(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fscanf", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileobject_fseek(Value* obj, long offset, long whence) asm("_ZN4HPHP26f_hphp_splfileobject_fseekERKNS_6ObjectEll");

void fg1_hphp_splfileobject_fseek(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fseek(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_fseek(&args[-0].m_data, (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
}

TypedValue* fg_hphp_splfileobject_fseek(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileobject_fseek(&args[-0].m_data, (long)(args[-1].m_data.num), (long)(args[-2].m_data.num));
    } else {
      fg1_hphp_splfileobject_fseek(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fseek", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_splfileobject_fstat(TypedValue* _rv, Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_fstatERKNS_6ObjectE");

void fg1_hphp_splfileobject_fstat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fstat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_fstat(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject_fstat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_hphp_splfileobject_fstat(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject_fstat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fstat", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileobject_ftell(Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_ftellERKNS_6ObjectE");

void fg1_hphp_splfileobject_ftell(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_ftell(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_ftell(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileobject_ftell(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileobject_ftell(&args[-0].m_data);
    } else {
      fg1_hphp_splfileobject_ftell(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_ftell", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileobject_ftruncate(Value* obj, long size) asm("_ZN4HPHP30f_hphp_splfileobject_ftruncateERKNS_6ObjectEl");

void fg1_hphp_splfileobject_ftruncate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_ftruncate(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileobject_ftruncate(&args[-0].m_data, (long)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileobject_ftruncate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileobject_ftruncate(&args[-0].m_data, (long)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileobject_ftruncate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_ftruncate", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileobject_fwrite(Value* obj, Value* str, long length) asm("_ZN4HPHP27f_hphp_splfileobject_fwriteERKNS_6ObjectERKNS_6StringEl");

void fg1_hphp_splfileobject_fwrite(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_fwrite(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_fwrite(&args[-0].m_data, &args[-1].m_data, (long)(args[-2].m_data.num));
}

TypedValue* fg_hphp_splfileobject_fwrite(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileobject_fwrite(&args[-0].m_data, &args[-1].m_data, (long)(args[-2].m_data.num));
    } else {
      fg1_hphp_splfileobject_fwrite(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_fwrite", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphp_splfileobject_getcvscontrol(TypedValue* _rv, Value* obj) asm("_ZN4HPHP34f_hphp_splfileobject_getcvscontrolERKNS_6ObjectE");

void fg1_hphp_splfileobject_getcvscontrol(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_getcvscontrol(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_hphp_splfileobject_getcvscontrol(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphp_splfileobject_getcvscontrol(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_hphp_splfileobject_getcvscontrol(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphp_splfileobject_getcvscontrol(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_getcvscontrol", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileobject_getflags(Value* obj) asm("_ZN4HPHP29f_hphp_splfileobject_getflagsERKNS_6ObjectE");

void fg1_hphp_splfileobject_getflags(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_getflags(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_getflags(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileobject_getflags(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileobject_getflags(&args[-0].m_data);
    } else {
      fg1_hphp_splfileobject_getflags(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_getflags", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileobject_getmaxlinelen(Value* obj) asm("_ZN4HPHP34f_hphp_splfileobject_getmaxlinelenERKNS_6ObjectE");

void fg1_hphp_splfileobject_getmaxlinelen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_getmaxlinelen(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_getmaxlinelen(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileobject_getmaxlinelen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileobject_getmaxlinelen(&args[-0].m_data);
    } else {
      fg1_hphp_splfileobject_getmaxlinelen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_getmaxlinelen", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_hphp_splfileobject_key(Value* obj) asm("_ZN4HPHP24f_hphp_splfileobject_keyERKNS_6ObjectE");

void fg1_hphp_splfileobject_key(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_key(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_hphp_splfileobject_key(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileobject_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_hphp_splfileobject_key(&args[-0].m_data);
    } else {
      fg1_hphp_splfileobject_key(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_key", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_splfileobject_next(Value* obj) asm("_ZN4HPHP25f_hphp_splfileobject_nextERKNS_6ObjectE");

void fg1_hphp_splfileobject_next(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_next(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_hphp_splfileobject_next(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileobject_next(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_hphp_splfileobject_next(&args[-0].m_data);
    } else {
      fg1_hphp_splfileobject_next(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_next", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_splfileobject_rewind(Value* obj) asm("_ZN4HPHP27f_hphp_splfileobject_rewindERKNS_6ObjectE");

void fg1_hphp_splfileobject_rewind(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_rewind(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_hphp_splfileobject_rewind(&args[-0].m_data);
}

TypedValue* fg_hphp_splfileobject_rewind(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_hphp_splfileobject_rewind(&args[-0].m_data);
    } else {
      fg1_hphp_splfileobject_rewind(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_rewind", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_hphp_splfileobject_valid(Value* obj) asm("_ZN4HPHP26f_hphp_splfileobject_validERKNS_6ObjectE");

void fg1_hphp_splfileobject_valid(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_valid(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphp_splfileobject_valid(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphp_splfileobject_valid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphp_splfileobject_valid(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphp_splfileobject_valid(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_valid", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_splfileobject_seek(Value* obj, long line_pos) asm("_ZN4HPHP25f_hphp_splfileobject_seekERKNS_6ObjectEl");

void fg1_hphp_splfileobject_seek(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_seek(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_hphp_splfileobject_seek(&args[-0].m_data, (long)(args[-1].m_data.num));
}

TypedValue* fg_hphp_splfileobject_seek(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_hphp_splfileobject_seek(&args[-0].m_data, (long)(args[-1].m_data.num));
    } else {
      fg1_hphp_splfileobject_seek(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_seek", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_splfileobject_setcsvcontrol(Value* obj, Value* delimiter, Value* enclosure, Value* escape) asm("_ZN4HPHP34f_hphp_splfileobject_setcsvcontrolERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_hphp_splfileobject_setcsvcontrol(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_setcsvcontrol(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfNull;
  fh_hphp_splfileobject_setcsvcontrol(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
}

TypedValue* fg_hphp_splfileobject_setcsvcontrol(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_hphp_splfileobject_setcsvcontrol(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
    } else {
      fg1_hphp_splfileobject_setcsvcontrol(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_setcsvcontrol", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_splfileobject_setflags(Value* obj, long flags) asm("_ZN4HPHP29f_hphp_splfileobject_setflagsERKNS_6ObjectEl");

void fg1_hphp_splfileobject_setflags(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_setflags(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_hphp_splfileobject_setflags(&args[-0].m_data, (long)(args[-1].m_data.num));
}

TypedValue* fg_hphp_splfileobject_setflags(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_hphp_splfileobject_setflags(&args[-0].m_data, (long)(args[-1].m_data.num));
    } else {
      fg1_hphp_splfileobject_setflags(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_setflags", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphp_splfileobject_setmaxlinelen(Value* obj, long max_len) asm("_ZN4HPHP34f_hphp_splfileobject_setmaxlinelenERKNS_6ObjectEl");

void fg1_hphp_splfileobject_setmaxlinelen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphp_splfileobject_setmaxlinelen(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_hphp_splfileobject_setmaxlinelen(&args[-0].m_data, (long)(args[-1].m_data.num));
}

TypedValue* fg_hphp_splfileobject_setmaxlinelen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_hphp_splfileobject_setmaxlinelen(&args[-0].m_data, (long)(args[-1].m_data.num));
    } else {
      fg1_hphp_splfileobject_setmaxlinelen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphp_splfileobject_setmaxlinelen", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
