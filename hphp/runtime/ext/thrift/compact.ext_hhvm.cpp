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

int fh_thrift_protocol_set_compact_version(int version) asm("_ZN4HPHP37f_thrift_protocol_set_compact_versionEi");

void fg1_thrift_protocol_set_compact_version(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_thrift_protocol_set_compact_version(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_thrift_protocol_set_compact_version((int)(args[-0].m_data.num));
}

TypedValue* fg_thrift_protocol_set_compact_version(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_thrift_protocol_set_compact_version((int)(args[-0].m_data.num));
    } else {
      fg1_thrift_protocol_set_compact_version(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("thrift_protocol_set_compact_version", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_thrift_protocol_write_compact(Value* transportobj, Value* method_name, long msgtype, Value* request_struct, int seqid) asm("_ZN4HPHP31f_thrift_protocol_write_compactERKNS_6ObjectERKNS_6StringElS2_i");

void fg1_thrift_protocol_write_compact(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_thrift_protocol_write_compact(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-4)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-4);
  }
  if ((args-3)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_thrift_protocol_write_compact(&args[-0].m_data, &args[-1].m_data, (long)(args[-2].m_data.num), &args[-3].m_data, (int)(args[-4].m_data.num));
}

TypedValue* fg_thrift_protocol_write_compact(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 5) {
    if ((args - 4)->m_type == KindOfInt64 &&
        (args - 3)->m_type == KindOfObject &&
        (args - 2)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_thrift_protocol_write_compact(&args[-0].m_data, &args[-1].m_data, (long)(args[-2].m_data.num), &args[-3].m_data, (int)(args[-4].m_data.num));
    } else {
      fg1_thrift_protocol_write_compact(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("thrift_protocol_write_compact", count, 5, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_thrift_protocol_read_compact(TypedValue* _rv, Value* transportobj, Value* obj_typename) asm("_ZN4HPHP30f_thrift_protocol_read_compactERKNS_6ObjectERKNS_6StringE");

void fg1_thrift_protocol_read_compact(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_thrift_protocol_read_compact(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_thrift_protocol_read_compact(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_thrift_protocol_read_compact(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_thrift_protocol_read_compact(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_thrift_protocol_read_compact(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("thrift_protocol_read_compact", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
