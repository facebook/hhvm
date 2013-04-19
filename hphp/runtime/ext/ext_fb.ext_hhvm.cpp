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

TypedValue* fh_fb_thrift_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP21f_fb_thrift_serializeERKNS_7VariantE");

TypedValue* fg_fb_thrift_serialize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_fb_thrift_serialize(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("fb_thrift_serialize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_thrift_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP23f_fb_thrift_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

TypedValue* fg_fb_thrift_unserialize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    VRefParamValue defVal2 = null_variant;
    fh_fb_thrift_unserialize(rv, (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("fb_thrift_unserialize", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP14f_fb_serializeERKNS_7VariantE");

TypedValue* fg_fb_serialize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_fb_serialize(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("fb_serialize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP16f_fb_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

TypedValue* fg_fb_unserialize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    VRefParamValue defVal2 = null_variant;
    fh_fb_unserialize(rv, (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("fb_unserialize", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_compact_serialize(TypedValue* _rv, TypedValue* thing) asm("_ZN4HPHP22f_fb_compact_serializeERKNS_7VariantE");

TypedValue* fg_fb_compact_serialize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_fb_compact_serialize(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("fb_compact_serialize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_compact_unserialize(TypedValue* _rv, TypedValue* thing, TypedValue* success, TypedValue* errcode) asm("_ZN4HPHP24f_fb_compact_unserializeERKNS_7VariantERKNS_14VRefParamValueES5_");

TypedValue* fg_fb_compact_unserialize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    VRefParamValue defVal2 = null_variant;
    fh_fb_compact_unserialize(rv, (args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("fb_compact_unserialize", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_fb_load_local_databases(Value* servers) asm("_ZN4HPHP25f_fb_load_local_databasesERKNS_5ArrayE");

void fg1_fb_load_local_databases(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_load_local_databases(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_fb_load_local_databases(&args[-0].m_data);
}

TypedValue* fg_fb_load_local_databases(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfArray) {
      rv->m_type = KindOfNull;
      fh_fb_load_local_databases(&args[-0].m_data);
    } else {
      fg1_fb_load_local_databases(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_load_local_databases", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_fb_parallel_query(Value* _rv, Value* sql_map, int max_thread, bool combine_result, bool retry_query_on_fail, int connect_timeout, int read_timeout, bool timeout_in_ms) asm("_ZN4HPHP19f_fb_parallel_queryERKNS_5ArrayEibbiib");

void fg1_fb_parallel_query(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_parallel_query(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 7
    if ((args-6)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
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
  if ((args-0)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-0);
  }
  rv->m_type = KindOfArray;
  fh_fb_parallel_query(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(50), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_parallel_query(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 7) {
    if ((count <= 6 || (args - 6)->m_type == KindOfBoolean) &&
        (count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfArray) {
      rv->m_type = KindOfArray;
      fh_fb_parallel_query(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(50), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(false));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_fb_parallel_query(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_parallel_query", count, 1, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_fb_crossall_query(Value* _rv, Value* sql, int max_thread, bool retry_query_on_fail, int connect_timeout, int read_timeout, bool timeout_in_ms) asm("_ZN4HPHP19f_fb_crossall_queryERKNS_6StringEibiib");

void fg1_fb_crossall_query(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_crossall_query(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-5);
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
  rv->m_type = KindOfArray;
  fh_fb_crossall_query(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(50), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (int)(args[-3].m_data.num) : (int)(-1), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_crossall_query(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 6) {
    if ((count <= 5 || (args - 5)->m_type == KindOfBoolean) &&
        (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_fb_crossall_query(&(rv->m_data), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(50), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true), (count > 3) ? (int)(args[-3].m_data.num) : (int)(-1), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(false));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_fb_crossall_query(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_crossall_query", count, 1, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fb_utf8ize(TypedValue* input) asm("_ZN4HPHP12f_fb_utf8izeERKNS_14VRefParamValueE");

TypedValue* fg_fb_utf8ize(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_fb_utf8ize((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("fb_utf8ize", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_fb_utf8_strlen(Value* input) asm("_ZN4HPHP16f_fb_utf8_strlenERKNS_6StringE");

void fg1_fb_utf8_strlen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_utf8_strlen(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_fb_utf8_strlen(&args[-0].m_data);
}

TypedValue* fg_fb_utf8_strlen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_fb_utf8_strlen(&args[-0].m_data);
    } else {
      fg1_fb_utf8_strlen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_utf8_strlen", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_fb_utf8_strlen_deprecated(Value* input) asm("_ZN4HPHP27f_fb_utf8_strlen_deprecatedERKNS_6StringE");

void fg1_fb_utf8_strlen_deprecated(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_utf8_strlen_deprecated(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_fb_utf8_strlen_deprecated(&args[-0].m_data);
}

TypedValue* fg_fb_utf8_strlen_deprecated(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_fb_utf8_strlen_deprecated(&args[-0].m_data);
    } else {
      fg1_fb_utf8_strlen_deprecated(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_utf8_strlen_deprecated", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_utf8_substr(TypedValue* _rv, Value* str, int start, int length) asm("_ZN4HPHP16f_fb_utf8_substrERKNS_6StringEii");

void fg1_fb_utf8_substr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_utf8_substr(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_fb_utf8_substr(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_utf8_substr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fb_utf8_substr(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fb_utf8_substr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_utf8_substr", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fb_could_include(Value* file) asm("_ZN4HPHP18f_fb_could_includeERKNS_6StringE");

void fg1_fb_could_include(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_could_include(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_fb_could_include(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_fb_could_include(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_fb_could_include(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_fb_could_include(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_could_include", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fb_intercept(Value* name, TypedValue* handler, TypedValue* data) asm("_ZN4HPHP14f_fb_interceptERKNS_6StringERKNS_7VariantES5_");

void fg1_fb_intercept(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_intercept(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_fb_intercept(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
}

TypedValue* fg_fb_intercept(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_fb_intercept(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
    } else {
      fg1_fb_intercept(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_intercept", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_stubout_intercept_handler(TypedValue* _rv, Value* name, TypedValue* obj, Value* params, TypedValue* data, TypedValue* done) asm("_ZN4HPHP30f_fb_stubout_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE");

void fg1_fb_stubout_intercept_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_stubout_intercept_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_fb_stubout_intercept_handler(rv, &args[-0].m_data, (args-1), &args[-2].m_data, (args-3), (args-4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_stubout_intercept_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 2)->m_type == KindOfArray &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fb_stubout_intercept_handler(rv, &args[-0].m_data, (args-1), &args[-2].m_data, (args-3), (args-4));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fb_stubout_intercept_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_stubout_intercept_handler", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_rpc_intercept_handler(TypedValue* _rv, Value* name, TypedValue* obj, Value* params, TypedValue* data, TypedValue* done) asm("_ZN4HPHP26f_fb_rpc_intercept_handlerERKNS_6StringERKNS_7VariantERKNS_5ArrayES5_RKNS_14VRefParamValueE");

void fg1_fb_rpc_intercept_handler(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_rpc_intercept_handler(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_fb_rpc_intercept_handler(rv, &args[-0].m_data, (args-1), &args[-2].m_data, (args-3), (args-4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_rpc_intercept_handler(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 2)->m_type == KindOfArray &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fb_rpc_intercept_handler(rv, &args[-0].m_data, (args-1), &args[-2].m_data, (args-3), (args-4));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fb_rpc_intercept_handler(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_rpc_intercept_handler", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_fb_renamed_functions(Value* names) asm("_ZN4HPHP22f_fb_renamed_functionsERKNS_5ArrayE");

void fg1_fb_renamed_functions(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_renamed_functions(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_fb_renamed_functions(&args[-0].m_data);
}

TypedValue* fg_fb_renamed_functions(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfArray) {
      rv->m_type = KindOfNull;
      fh_fb_renamed_functions(&args[-0].m_data);
    } else {
      fg1_fb_renamed_functions(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_renamed_functions", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fb_rename_function(Value* orig_func_name, Value* new_func_name) asm("_ZN4HPHP20f_fb_rename_functionERKNS_6StringES2_");

void fg1_fb_rename_function(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_rename_function(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_fb_rename_function(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_fb_rename_function(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_fb_rename_function(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_fb_rename_function(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_rename_function", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fb_autoload_map(TypedValue* map, Value* root) asm("_ZN4HPHP17f_fb_autoload_mapERKNS_7VariantERKNS_6StringE");

void fg1_fb_autoload_map(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_autoload_map(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_fb_autoload_map((args-0), &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_fb_autoload_map(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_fb_autoload_map((args-0), &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_fb_autoload_map(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_autoload_map", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_fb_call_user_func_safe(Value* _rv, int64_t _argc, TypedValue* function, Value* _argv) asm("_ZN4HPHP24f_fb_call_user_func_safeEiRKNS_7VariantERKNS_5ArrayE");

TypedValue* fg_fb_call_user_func_safe(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1) {
    rv->m_type = KindOfArray;

    Array extraArgs;
    {
      ArrayInit ai(count-1);
      for (int32_t i = 1; i < count; ++i) {
        TypedValue* extraArg = ar->getExtraArg(i-1);
        if (tvIsStronglyBound(extraArg)) {
          ai.setRef(i-1, tvAsVariant(extraArg));
        } else {
          ai.set(i-1, tvAsVariant(extraArg));
        }
      }
      extraArgs = ai.create();
    }
    fh_fb_call_user_func_safe(&(rv->m_data), count, (args-0), (Value*)(&extraArgs));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("fb_call_user_func_safe", 1, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_fb_call_user_func_array_safe(Value* _rv, TypedValue* function, Value* params) asm("_ZN4HPHP30f_fb_call_user_func_array_safeERKNS_7VariantERKNS_5ArrayE");

void fg1_fb_call_user_func_array_safe(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_call_user_func_array_safe(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-1);
  rv->m_type = KindOfArray;
  fh_fb_call_user_func_array_safe(&(rv->m_data), (args-0), &args[-1].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_call_user_func_array_safe(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray) {
      rv->m_type = KindOfArray;
      fh_fb_call_user_func_array_safe(&(rv->m_data), (args-0), &args[-1].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_fb_call_user_func_array_safe(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_call_user_func_array_safe", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_call_user_func_safe_return(TypedValue* _rv, int64_t _argc, TypedValue* function, TypedValue* def, Value* _argv) asm("_ZN4HPHP31f_fb_call_user_func_safe_returnEiRKNS_7VariantES2_RKNS_5ArrayE");

TypedValue* fg_fb_call_user_func_safe_return(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2) {

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
    fh_fb_call_user_func_safe_return(rv, count, (args-0), (args-1), (Value*)(&extraArgs));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_missing_arguments_nr("fb_call_user_func_safe_return", 2, count, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_get_code_coverage(TypedValue* _rv, bool flush) asm("_ZN4HPHP22f_fb_get_code_coverageEb");

void fg1_fb_get_code_coverage(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_get_code_coverage(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  fh_fb_get_code_coverage(rv, (bool)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_get_code_coverage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfBoolean) {
      fh_fb_get_code_coverage(rv, (bool)(args[-0].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fb_get_code_coverage(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_get_code_coverage", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_fb_enable_code_coverage() asm("_ZN4HPHP25f_fb_enable_code_coverageEv");

TypedValue* fg_fb_enable_code_coverage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_fb_enable_code_coverage();
  } else {
    throw_toomany_arguments_nr("fb_enable_code_coverage", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_disable_code_coverage(TypedValue* _rv) asm("_ZN4HPHP26f_fb_disable_code_coverageEv");

TypedValue* fg_fb_disable_code_coverage(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_fb_disable_code_coverage(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("fb_disable_code_coverage", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_fb_output_compression(bool new_value) asm("_ZN4HPHP23f_fb_output_compressionEb");

void fg1_fb_output_compression(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_output_compression(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_fb_output_compression((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_fb_output_compression(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfBoolean) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_fb_output_compression((bool)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_fb_output_compression(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_output_compression", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_fb_set_exit_callback(TypedValue* function) asm("_ZN4HPHP22f_fb_set_exit_callbackERKNS_7VariantE");

TypedValue* fg_fb_set_exit_callback(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfNull;
    fh_fb_set_exit_callback((args-0));
  } else {
    throw_wrong_arguments_nr("fb_set_exit_callback", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_fb_get_flush_stat(Value* _rv) asm("_ZN4HPHP19f_fb_get_flush_statEv");

TypedValue* fg_fb_get_flush_stat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_fb_get_flush_stat(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("fb_get_flush_stat", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_fb_get_last_flush_size() asm("_ZN4HPHP24f_fb_get_last_flush_sizeEv");

TypedValue* fg_fb_get_last_flush_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_fb_get_last_flush_size();
  } else {
    throw_toomany_arguments_nr("fb_get_last_flush_size", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_lazy_stat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP14f_fb_lazy_statERKNS_6StringE");

void fg1_fb_lazy_stat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_lazy_stat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_fb_lazy_stat(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_lazy_stat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fb_lazy_stat(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fb_lazy_stat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_lazy_stat", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_lazy_lstat(TypedValue* _rv, Value* filename) asm("_ZN4HPHP15f_fb_lazy_lstatERKNS_6StringE");

void fg1_fb_lazy_lstat(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_lazy_lstat(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_fb_lazy_lstat(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_lazy_lstat(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_fb_lazy_lstat(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fb_lazy_lstat(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_lazy_lstat", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_fb_lazy_realpath(Value* _rv, Value* filename) asm("_ZN4HPHP18f_fb_lazy_realpathERKNS_6StringE");

void fg1_fb_lazy_realpath(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_lazy_realpath(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_fb_lazy_realpath(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_fb_lazy_realpath(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_fb_lazy_realpath(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_fb_lazy_realpath(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_lazy_realpath", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_fb_gc_collect_cycles(Value* _rv) asm("_ZN4HPHP22f_fb_gc_collect_cyclesEv");

TypedValue* fg_fb_gc_collect_cycles(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfString;
    fh_fb_gc_collect_cycles(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("fb_gc_collect_cycles", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_fb_gc_detect_cycles(Value* filename) asm("_ZN4HPHP21f_fb_gc_detect_cyclesERKNS_6StringE");

void fg1_fb_gc_detect_cycles(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fb_gc_detect_cycles(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_fb_gc_detect_cycles(&args[-0].m_data);
}

TypedValue* fg_fb_gc_detect_cycles(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_fb_gc_detect_cycles(&args[-0].m_data);
    } else {
      fg1_fb_gc_detect_cycles(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fb_gc_detect_cycles", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fb_const_fetch(TypedValue* _rv, TypedValue* key) asm("_ZN4HPHP16f_fb_const_fetchERKNS_7VariantE");

TypedValue* fg_fb_const_fetch(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_fb_const_fetch(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("fb_const_fetch", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
