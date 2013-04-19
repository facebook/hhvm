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

TypedValue* fh_fopen(TypedValue* _rv, Value* filename, Value* mode, bool use_include_path, TypedValue* context) asm("_ZN4HPHP7f_fopenERKNS_6StringES2_bRKNS_7VariantE");

void fg1_fopen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fopen(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
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
  Variant defVal3;
  fh_fopen(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fopen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      Variant defVal3;
      fh_fopen(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fopen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fopen", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_popen(TypedValue* _rv, Value* command, Value* mode) asm("_ZN4HPHP7f_popenERKNS_6StringES2_");

void fg1_popen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_popen(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_popen(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_popen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_popen(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_popen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("popen", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fclose(Value* handle) asm("_ZN4HPHP8f_fcloseERKNS_6ObjectE");

void fg1_fclose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fclose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_fclose(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_fclose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_fclose(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_fclose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fclose", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_pclose(TypedValue* _rv, Value* handle) asm("_ZN4HPHP8f_pcloseERKNS_6ObjectE");

void fg1_pclose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pclose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_pclose(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_pclose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_pclose(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_pclose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pclose", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fseek(TypedValue* _rv, Value* handle, long offset, long whence) asm("_ZN4HPHP7f_fseekERKNS_6ObjectEll");

void fg1_fseek(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fseek(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_fseek(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(k_SEEK_SET));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fseek(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_fseek(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(k_SEEK_SET));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fseek(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fseek", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_rewind(Value* handle) asm("_ZN4HPHP8f_rewindERKNS_6ObjectE");

void fg1_rewind(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_rewind(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_rewind(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_rewind(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_rewind(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_rewind(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("rewind", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_ftell(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_ftellERKNS_6ObjectE");

void fg1_ftell(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ftell(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_ftell(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_ftell(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_ftell(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_ftell(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ftell", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_feof(Value* handle) asm("_ZN4HPHP6f_feofERKNS_6ObjectE");

void fg1_feof(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_feof(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_feof(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_feof(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_feof(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_feof(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("feof", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fstat(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_fstatERKNS_6ObjectE");

void fg1_fstat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fstat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_fstat(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fstat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_fstat(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fstat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fstat", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fread(TypedValue* _rv, Value* handle, long length) asm("_ZN4HPHP7f_freadERKNS_6ObjectEl");

void fg1_fread(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fread(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_fread(rv, &args[-0].m_data, (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fread(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_fread(rv, &args[-0].m_data, (long)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fread(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fread", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fgetc(TypedValue* _rv, Value* handle) asm("_ZN4HPHP7f_fgetcERKNS_6ObjectE");

void fg1_fgetc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fgetc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_fgetc(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fgetc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_fgetc(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fgetc(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fgetc", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fgets(TypedValue* _rv, Value* handle, long length) asm("_ZN4HPHP7f_fgetsERKNS_6ObjectEl");

void fg1_fgets(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fgets(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_fgets(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fgets(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      fh_fgets(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fgets(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fgets", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fgetss(TypedValue* _rv, Value* handle, long length, Value* allowable_tags) asm("_ZN4HPHP8f_fgetssERKNS_6ObjectElRKNS_6StringE");

void fg1_fgetss(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fgetss(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_fgetss(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fgetss(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      fh_fgetss(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fgetss(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fgetss", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fscanf(TypedValue* _rv, int64_t _argc, Value* handle, Value* format, Value* _argv) asm("_ZN4HPHP8f_fscanfEiRKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

void fg1_fscanf(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fscanf(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }

  Array extraArgs;
  {
    ArrayInit ai(count-2);
    for (int32_t i = 2; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-2);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-2, tvAsVariant(extraArg));
      } else {
        ai.set(i-2, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_fscanf(rv, count, &args[-0].m_data, &args[-1].m_data, (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fscanf(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {

      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int32_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_fscanf(rv, count, &args[-0].m_data, &args[-1].m_data, (Value*)(&extraArgs));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fscanf(rv, ar, count);
    }
  } else {
    throw_missing_arguments_nr("fscanf", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fpassthru(TypedValue* _rv, Value* handle) asm("_ZN4HPHP11f_fpassthruERKNS_6ObjectE");

void fg1_fpassthru(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fpassthru(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_fpassthru(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fpassthru(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_fpassthru(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fpassthru(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fpassthru", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fwrite(TypedValue* _rv, Value* handle, Value* data, long length) asm("_ZN4HPHP8f_fwriteERKNS_6ObjectERKNS_6StringEl");

void fg1_fwrite(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fwrite(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_fwrite(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fwrite(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_fwrite(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fwrite(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fwrite", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fputs(TypedValue* _rv, Value* handle, Value* data, long length) asm("_ZN4HPHP7f_fputsERKNS_6ObjectERKNS_6StringEl");

void fg1_fputs(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fputs(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_fputs(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fputs(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_fputs(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fputs(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fputs", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fprintf(TypedValue* _rv, int64_t _argc, Value* handle, Value* format, Value* _argv) asm("_ZN4HPHP9f_fprintfEiRKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

void fg1_fprintf(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fprintf(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }

  Array extraArgs;
  {
    ArrayInit ai(count-2);
    for (int32_t i = 2; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-2);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-2, tvAsVariant(extraArg));
      } else {
        ai.set(i-2, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  fh_fprintf(rv, count, &args[-0].m_data, &args[-1].m_data, (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fprintf(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {

      Array extraArgs;
      {
        ArrayInit ai(count-2);
        for (int32_t i = 2; i < count; ++i) {
          TypedValue* extraArg = ar->getExtraArg(i-2);
          if (tvIsStronglyBound(extraArg)) {
            ai.setRef(i-2, tvAsVariant(extraArg));
          } else {
            ai.set(i-2, tvAsVariant(extraArg));
          }
        }
        extraArgs = ai.create();
      }
      fh_fprintf(rv, count, &args[-0].m_data, &args[-1].m_data, (Value*)(&extraArgs));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fprintf(rv, ar, count);
    }
  } else {
    throw_missing_arguments_nr("fprintf", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_vfprintf(TypedValue* _rv, Value* handle, Value* format, Value* args) asm("_ZN4HPHP10f_vfprintfERKNS_6ObjectERKNS_6StringERKNS_5ArrayE");

void fg1_vfprintf(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_vfprintf(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_vfprintf(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_vfprintf(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfArray &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_vfprintf(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_vfprintf(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("vfprintf", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fflush(Value* handle) asm("_ZN4HPHP8f_fflushERKNS_6ObjectE");

void fg1_fflush(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fflush(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_fflush(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_fflush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_fflush(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_fflush(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fflush", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_ftruncate(Value* handle, long size) asm("_ZN4HPHP11f_ftruncateERKNS_6ObjectEl");

void fg1_ftruncate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ftruncate(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_ftruncate(&args[-0].m_data, (long)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_ftruncate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_ftruncate(&args[-0].m_data, (long)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_ftruncate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ftruncate", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_flock(Value* handle, int operation, TypedValue* wouldblock) asm("_ZN4HPHP7f_flockERKNS_6ObjectEiRKNS_14VRefParamValueE");

void fg1_flock(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_flock(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
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
  VRefParamValue defVal2 = uninit_null();
  rv->m_data.num = (fh_flock(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* fg_flock(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal2 = uninit_null();
      rv->m_data.num = (fh_flock(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
    } else {
      fg1_flock(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("flock", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fputcsv(TypedValue* _rv, Value* handle, Value* fields, Value* delimiter, Value* enclosure) asm("_ZN4HPHP9f_fputcsvERKNS_6ObjectERKNS_5ArrayERKNS_6StringES8_");

void fg1_fputcsv(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fputcsv(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
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
  String defVal2 = ",";
  String defVal3 = "\"";
  fh_fputcsv(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&defVal2), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fputcsv(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      String defVal2 = ",";
      String defVal3 = "\"";
      fh_fputcsv(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&defVal2), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fputcsv(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fputcsv", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fgetcsv(TypedValue* _rv, Value* handle, long length, Value* delimiter, Value* enclosure) asm("_ZN4HPHP9f_fgetcsvERKNS_6ObjectElRKNS_6StringES5_");

void fg1_fgetcsv(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fgetcsv(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  String defVal2 = ",";
  String defVal3 = "\"";
  fh_fgetcsv(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? &args[-2].m_data : (Value*)(&defVal2), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fgetcsv(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      String defVal2 = ",";
      String defVal3 = "\"";
      fh_fgetcsv(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? &args[-2].m_data : (Value*)(&defVal2), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fgetcsv(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fgetcsv", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_file_get_contents(TypedValue* _rv, Value* filename, bool use_include_path, TypedValue* context, long offset, long maxlen) asm("_ZN4HPHP19f_file_get_contentsERKNS_6StringEbRKNS_7VariantEll");

void fg1_file_get_contents(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_file_get_contents(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2;
  fh_file_get_contents(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (long)(args[-4].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_file_get_contents(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      Variant defVal2;
      fh_file_get_contents(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (long)(args[-4].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_file_get_contents(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("file_get_contents", count, 1, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_file_put_contents(TypedValue* _rv, Value* filename, TypedValue* data, int flags, TypedValue* context) asm("_ZN4HPHP19f_file_put_contentsERKNS_6StringERKNS_7VariantEiS5_");

void fg1_file_put_contents(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_file_put_contents(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal3;
  fh_file_put_contents(rv, &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_file_put_contents(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      Variant defVal3;
      fh_file_put_contents(rv, &args[-0].m_data, (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_file_put_contents(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("file_put_contents", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_file(TypedValue* _rv, Value* filename, int flags, TypedValue* context) asm("_ZN4HPHP6f_fileERKNS_6StringEiRKNS_7VariantE");

void fg1_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
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
  Variant defVal2;
  fh_file(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      Variant defVal2;
      fh_file(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("file", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_readfile(TypedValue* _rv, Value* filename, bool use_include_path, TypedValue* context) asm("_ZN4HPHP10f_readfileERKNS_6StringEbRKNS_7VariantE");

void fg1_readfile(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_readfile(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2;
  fh_readfile(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_readfile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      Variant defVal2;
      fh_readfile(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_readfile(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("readfile", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_move_uploaded_file(Value* filename, Value* destination) asm("_ZN4HPHP20f_move_uploaded_fileERKNS_6StringES2_");

void fg1_move_uploaded_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_move_uploaded_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_move_uploaded_file(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_move_uploaded_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_move_uploaded_file(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_move_uploaded_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("move_uploaded_file", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_parse_ini_file(TypedValue* _rv, Value* filename, bool process_sections, int scanner_mode) asm("_ZN4HPHP16f_parse_ini_fileERKNS_6StringEbi");

void fg1_parse_ini_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_parse_ini_file(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_parse_ini_file(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (int)(args[-2].m_data.num) : (int)(k_INI_SCANNER_NORMAL));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_parse_ini_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_parse_ini_file(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (int)(args[-2].m_data.num) : (int)(k_INI_SCANNER_NORMAL));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_parse_ini_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("parse_ini_file", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_file_exists(Value* filename) asm("_ZN4HPHP13f_file_existsERKNS_6StringE");

void fg1_file_exists(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_file_exists(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_file_exists(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_file_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_file_exists(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_file_exists(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("file_exists", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_parse_ini_string(TypedValue* _rv, Value* ini, bool process_sections, int scanner_mode) asm("_ZN4HPHP18f_parse_ini_stringERKNS_6StringEbi");

void fg1_parse_ini_string(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_parse_ini_string(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_parse_ini_string(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (int)(args[-2].m_data.num) : (int)(k_INI_SCANNER_NORMAL));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_parse_ini_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_parse_ini_string(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (int)(args[-2].m_data.num) : (int)(k_INI_SCANNER_NORMAL));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_parse_ini_string(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("parse_ini_string", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_parse_hdf_file(TypedValue* _rv, Value* filename) asm("_ZN4HPHP16f_parse_hdf_fileERKNS_6StringE");

void fg1_parse_hdf_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_parse_hdf_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_parse_hdf_file(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_parse_hdf_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_parse_hdf_file(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_parse_hdf_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("parse_hdf_file", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_parse_hdf_string(TypedValue* _rv, Value* input) asm("_ZN4HPHP18f_parse_hdf_stringERKNS_6StringE");

void fg1_parse_hdf_string(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_parse_hdf_string(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_parse_hdf_string(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_parse_hdf_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_parse_hdf_string(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_parse_hdf_string(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("parse_hdf_string", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_write_hdf_file(Value* data, Value* filename) asm("_ZN4HPHP16f_write_hdf_fileERKNS_5ArrayERKNS_6StringE");

void fg1_write_hdf_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_write_hdf_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_write_hdf_file(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_write_hdf_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfArray) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_write_hdf_file(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_write_hdf_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("write_hdf_file", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_write_hdf_string(Value* _rv, Value* data) asm("_ZN4HPHP18f_write_hdf_stringERKNS_5ArrayE");

void fg1_write_hdf_string(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_write_hdf_string(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  rv->m_type = KindOfString;
  fh_write_hdf_string(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_write_hdf_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfArray) {
      rv->m_type = KindOfString;
      fh_write_hdf_string(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_write_hdf_string(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("write_hdf_string", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_md5_file(TypedValue* _rv, Value* filename, bool raw_output) asm("_ZN4HPHP10f_md5_fileERKNS_6StringEb");

void fg1_md5_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_md5_file(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_md5_file(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_md5_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_md5_file(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_md5_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("md5_file", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_sha1_file(TypedValue* _rv, Value* filename, bool raw_output) asm("_ZN4HPHP11f_sha1_fileERKNS_6StringEb");

void fg1_sha1_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sha1_file(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_sha1_file(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_sha1_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_sha1_file(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_sha1_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sha1_file", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fileperms(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filepermsERKNS_6StringE");

void fg1_fileperms(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fileperms(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_fileperms(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fileperms(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fileperms(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fileperms(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fileperms", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fileinode(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileinodeERKNS_6StringE");

void fg1_fileinode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fileinode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_fileinode(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fileinode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fileinode(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fileinode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fileinode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_filesize(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_filesizeERKNS_6StringE");

void fg1_filesize(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_filesize(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_filesize(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_filesize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_filesize(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_filesize(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("filesize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fileowner(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileownerERKNS_6StringE");

void fg1_fileowner(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fileowner(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_fileowner(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fileowner(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fileowner(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fileowner(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fileowner", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_filegroup(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filegroupERKNS_6StringE");

void fg1_filegroup(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_filegroup(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_filegroup(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_filegroup(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_filegroup(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_filegroup(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("filegroup", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fileatime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_fileatimeERKNS_6StringE");

void fg1_fileatime(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fileatime(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_fileatime(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fileatime(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fileatime(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fileatime(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fileatime", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_filemtime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filemtimeERKNS_6StringE");

void fg1_filemtime(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_filemtime(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_filemtime(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_filemtime(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_filemtime(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_filemtime(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("filemtime", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_filectime(TypedValue* _rv, Value* filename) asm("_ZN4HPHP11f_filectimeERKNS_6StringE");

void fg1_filectime(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_filectime(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_filectime(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_filectime(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_filectime(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_filectime(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("filectime", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_filetype(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_filetypeERKNS_6StringE");

void fg1_filetype(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_filetype(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_filetype(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_filetype(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_filetype(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_filetype(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("filetype", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_linkinfo(TypedValue* _rv, Value* filename) asm("_ZN4HPHP10f_linkinfoERKNS_6StringE");

void fg1_linkinfo(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_linkinfo(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_linkinfo(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_linkinfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_linkinfo(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_linkinfo(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("linkinfo", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_writable(Value* filename) asm("_ZN4HPHP13f_is_writableERKNS_6StringE");

void fg1_is_writable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_writable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_writable(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_is_writable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_writable(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_is_writable(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_writable", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_writeable(Value* filename) asm("_ZN4HPHP14f_is_writeableERKNS_6StringE");

void fg1_is_writeable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_writeable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_writeable(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_is_writeable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_writeable(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_is_writeable(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_writeable", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_readable(Value* filename) asm("_ZN4HPHP13f_is_readableERKNS_6StringE");

void fg1_is_readable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_readable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_readable(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_is_readable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_readable(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_is_readable(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_readable", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_executable(Value* filename) asm("_ZN4HPHP15f_is_executableERKNS_6StringE");

void fg1_is_executable(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_executable(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_executable(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_is_executable(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_executable(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_is_executable(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_executable", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_file(Value* filename) asm("_ZN4HPHP9f_is_fileERKNS_6StringE");

void fg1_is_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_file(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_is_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_file(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_is_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_file", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_dir(Value* filename) asm("_ZN4HPHP8f_is_dirERKNS_6StringE");

void fg1_is_dir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_dir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_dir(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_is_dir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_dir(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_is_dir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_dir", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_link(Value* filename) asm("_ZN4HPHP9f_is_linkERKNS_6StringE");

void fg1_is_link(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_link(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_link(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_is_link(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_link(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_is_link(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_link", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_is_uploaded_file(Value* filename) asm("_ZN4HPHP18f_is_uploaded_fileERKNS_6StringE");

void fg1_is_uploaded_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_is_uploaded_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_is_uploaded_file(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_is_uploaded_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_is_uploaded_file(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_is_uploaded_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("is_uploaded_file", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_stat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP6f_statERKNS_6StringE");

void fg1_stat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_stat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_stat(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_stat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_stat(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_stat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("stat", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_lstat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP7f_lstatERKNS_6StringE");

void fg1_lstat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_lstat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_lstat(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_lstat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_lstat(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_lstat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("lstat", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_clearstatcache() asm("_ZN4HPHP16f_clearstatcacheEv");

TypedValue* fg_clearstatcache(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_clearstatcache();
  } else {
    throw_toomany_arguments_nr("clearstatcache", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_readlink(TypedValue* _rv, Value* path) asm("_ZN4HPHP10f_readlinkERKNS_6StringE");

void fg1_readlink(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_readlink(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_readlink(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_readlink(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_readlink(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_readlink(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("readlink", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_realpath(TypedValue* _rv, Value* path) asm("_ZN4HPHP10f_realpathERKNS_6StringE");

void fg1_realpath(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_realpath(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_realpath(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_realpath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_realpath(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_realpath(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("realpath", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_pathinfo(TypedValue* _rv, Value* path, int opt) asm("_ZN4HPHP10f_pathinfoERKNS_6StringEi");

void fg1_pathinfo(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pathinfo(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_pathinfo(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(15));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_pathinfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_pathinfo(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(15));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_pathinfo(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pathinfo", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_dirname(Value* _rv, Value* path) asm("_ZN4HPHP9f_dirnameERKNS_6StringE");

void fg1_dirname(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_dirname(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_dirname(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_dirname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_dirname(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_dirname(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("dirname", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_basename(Value* _rv, Value* path, Value* suffix) asm("_ZN4HPHP10f_basenameERKNS_6StringES2_");

void fg1_basename(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_basename(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_basename(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_basename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_basename(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_basename(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("basename", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_disk_free_space(TypedValue* _rv, Value* directory) asm("_ZN4HPHP17f_disk_free_spaceERKNS_6StringE");

void fg1_disk_free_space(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_disk_free_space(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_disk_free_space(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_disk_free_space(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_disk_free_space(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_disk_free_space(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("disk_free_space", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_diskfreespace(TypedValue* _rv, Value* directory) asm("_ZN4HPHP15f_diskfreespaceERKNS_6StringE");

void fg1_diskfreespace(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_diskfreespace(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_diskfreespace(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_diskfreespace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_diskfreespace(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_diskfreespace(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("diskfreespace", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_disk_total_space(TypedValue* _rv, Value* directory) asm("_ZN4HPHP18f_disk_total_spaceERKNS_6StringE");

void fg1_disk_total_space(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_disk_total_space(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_disk_total_space(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_disk_total_space(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_disk_total_space(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_disk_total_space(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("disk_total_space", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_chmod(Value* filename, long mode) asm("_ZN4HPHP7f_chmodERKNS_6StringEl");

void fg1_chmod(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_chmod(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_chmod(&args[-0].m_data, (long)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_chmod(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_chmod(&args[-0].m_data, (long)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_chmod(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("chmod", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_chown(Value* filename, TypedValue* user) asm("_ZN4HPHP7f_chownERKNS_6StringERKNS_7VariantE");

void fg1_chown(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_chown(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_chown(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_chown(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_chown(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_chown(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("chown", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_lchown(Value* filename, TypedValue* user) asm("_ZN4HPHP8f_lchownERKNS_6StringERKNS_7VariantE");

void fg1_lchown(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_lchown(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_lchown(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_lchown(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_lchown(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_lchown(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("lchown", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_chgrp(Value* filename, TypedValue* group) asm("_ZN4HPHP7f_chgrpERKNS_6StringERKNS_7VariantE");

void fg1_chgrp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_chgrp(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_chgrp(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_chgrp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_chgrp(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_chgrp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("chgrp", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_lchgrp(Value* filename, TypedValue* group) asm("_ZN4HPHP8f_lchgrpERKNS_6StringERKNS_7VariantE");

void fg1_lchgrp(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_lchgrp(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_lchgrp(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
}

TypedValue* fg_lchgrp(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_lchgrp(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
    } else {
      fg1_lchgrp(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("lchgrp", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_touch(Value* filename, long mtime, long atime) asm("_ZN4HPHP7f_touchERKNS_6StringEll");

void fg1_touch(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_touch(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_touch(&args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_touch(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_touch(&args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_touch(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("touch", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_copy(Value* source, Value* dest, TypedValue* context) asm("_ZN4HPHP6f_copyERKNS_6StringES2_RKNS_7VariantE");

void fg1_copy(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_copy(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  Variant defVal2;
  rv->m_data.num = (fh_copy(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* fg_copy(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      Variant defVal2;
      rv->m_data.num = (fh_copy(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
    } else {
      fg1_copy(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("copy", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_rename(Value* oldname, Value* newname, TypedValue* context) asm("_ZN4HPHP8f_renameERKNS_6StringES2_RKNS_7VariantE");

void fg1_rename(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_rename(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  Variant defVal2;
  rv->m_data.num = (fh_rename(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* fg_rename(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      Variant defVal2;
      rv->m_data.num = (fh_rename(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
    } else {
      fg1_rename(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("rename", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_umask(TypedValue* mask) asm("_ZN4HPHP7f_umaskERKNS_7VariantE");

TypedValue* fg_umask(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_umask((count > 0) ? (args-0) : (TypedValue*)(&null_variant));
  } else {
    throw_toomany_arguments_nr("umask", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_unlink(Value* filename, TypedValue* context) asm("_ZN4HPHP8f_unlinkERKNS_6StringERKNS_7VariantE");

void fg1_unlink(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_unlink(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  Variant defVal1;
  rv->m_data.num = (fh_unlink(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
}

TypedValue* fg_unlink(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      Variant defVal1;
      rv->m_data.num = (fh_unlink(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
    } else {
      fg1_unlink(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("unlink", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_link(Value* target, Value* link) asm("_ZN4HPHP6f_linkERKNS_6StringES2_");

void fg1_link(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_link(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_link(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_link(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_link(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_link(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("link", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_symlink(Value* target, Value* link) asm("_ZN4HPHP9f_symlinkERKNS_6StringES2_");

void fg1_symlink(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_symlink(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_symlink(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_symlink(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_symlink(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_symlink(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("symlink", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fnmatch(Value* pattern, Value* filename, int flags) asm("_ZN4HPHP9f_fnmatchERKNS_6StringES2_i");

void fg1_fnmatch(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fnmatch(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_fnmatch(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_fnmatch(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_fnmatch(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_fnmatch(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fnmatch", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_glob(TypedValue* _rv, Value* pattern, int flags) asm("_ZN4HPHP6f_globERKNS_6StringEi");

void fg1_glob(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_glob(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_glob(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_glob(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_glob(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_glob(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("glob", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_tempnam(TypedValue* _rv, Value* dir, Value* prefix) asm("_ZN4HPHP9f_tempnamERKNS_6StringES2_");

void fg1_tempnam(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_tempnam(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_tempnam(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_tempnam(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_tempnam(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_tempnam(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("tempnam", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_tmpfile(TypedValue* _rv) asm("_ZN4HPHP9f_tmpfileEv");

TypedValue* fg_tmpfile(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_tmpfile(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("tmpfile", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mkdir(Value* pathname, long mode, bool recursive, TypedValue* context) asm("_ZN4HPHP7f_mkdirERKNS_6StringElbRKNS_7VariantE");

void fg1_mkdir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mkdir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
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
  rv->m_type = KindOfBoolean;
  Variant defVal3;
  rv->m_data.num = (fh_mkdir(&args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0777), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (args-3) : (TypedValue*)(&defVal3))) ? 1LL : 0LL;
}

TypedValue* fg_mkdir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      Variant defVal3;
      rv->m_data.num = (fh_mkdir(&args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0777), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false), (count > 3) ? (args-3) : (TypedValue*)(&defVal3))) ? 1LL : 0LL;
    } else {
      fg1_mkdir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mkdir", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_rmdir(Value* dirname, TypedValue* context) asm("_ZN4HPHP7f_rmdirERKNS_6StringERKNS_7VariantE");

void fg1_rmdir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_rmdir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  Variant defVal1;
  rv->m_data.num = (fh_rmdir(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
}

TypedValue* fg_rmdir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      Variant defVal1;
      rv->m_data.num = (fh_rmdir(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
    } else {
      fg1_rmdir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("rmdir", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_getcwd(TypedValue* _rv) asm("_ZN4HPHP8f_getcwdEv");

TypedValue* fg_getcwd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_getcwd(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("getcwd", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_chdir(Value* directory) asm("_ZN4HPHP7f_chdirERKNS_6StringE");

void fg1_chdir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_chdir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_chdir(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_chdir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_chdir(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_chdir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("chdir", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_chroot(Value* directory) asm("_ZN4HPHP8f_chrootERKNS_6StringE");

void fg1_chroot(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_chroot(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_chroot(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_chroot(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_chroot(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_chroot(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("chroot", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_dir(TypedValue* _rv, Value* directory) asm("_ZN4HPHP5f_dirERKNS_6StringE");

void fg1_dir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_dir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_dir(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_dir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_dir(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_dir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("dir", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_opendir(TypedValue* _rv, Value* path, TypedValue* context) asm("_ZN4HPHP9f_opendirERKNS_6StringERKNS_7VariantE");

void fg1_opendir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_opendir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_opendir(rv, &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_opendir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      Variant defVal1;
      fh_opendir(rv, &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_opendir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("opendir", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_readdir(TypedValue* _rv, Value* dir_handle) asm("_ZN4HPHP9f_readdirERKNS_6ObjectE");

void fg1_readdir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_readdir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_readdir(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_readdir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_readdir(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_readdir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("readdir", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_rewinddir(Value* dir_handle) asm("_ZN4HPHP11f_rewinddirERKNS_6ObjectE");

void fg1_rewinddir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_rewinddir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_rewinddir(&args[-0].m_data);
}

TypedValue* fg_rewinddir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_rewinddir(&args[-0].m_data);
    } else {
      fg1_rewinddir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("rewinddir", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_scandir(TypedValue* _rv, Value* directory, bool descending, TypedValue* context) asm("_ZN4HPHP9f_scandirERKNS_6StringEbRKNS_7VariantE");

void fg1_scandir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_scandir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2;
  fh_scandir(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_scandir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      Variant defVal2;
      fh_scandir(rv, &args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(false), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_scandir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("scandir", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_closedir(Value* dir_handle) asm("_ZN4HPHP10f_closedirERKNS_6ObjectE");

void fg1_closedir(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_closedir(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_closedir(&args[-0].m_data);
}

TypedValue* fg_closedir(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_closedir(&args[-0].m_data);
    } else {
      fg1_closedir(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("closedir", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
