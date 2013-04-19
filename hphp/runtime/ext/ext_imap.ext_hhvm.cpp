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

TypedValue* fh_imap_8bit(TypedValue* _rv, Value* str) asm("_ZN4HPHP11f_imap_8bitERKNS_6StringE");

void fg1_imap_8bit(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_8bit(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imap_8bit(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_8bit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_8bit(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_8bit(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_8bit", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_alerts(TypedValue* _rv) asm("_ZN4HPHP13f_imap_alertsEv");

TypedValue* fg_imap_alerts(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_imap_alerts(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("imap_alerts", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_append(Value* imap_stream, Value* mailbox, Value* message, Value* options) asm("_ZN4HPHP13f_imap_appendERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_imap_append(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_append(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  String defVal3 = "";
  rv->m_data.num = (fh_imap_append(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&defVal3))) ? 1LL : 0LL;
}

TypedValue* fg_imap_append(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      String defVal3 = "";
      rv->m_data.num = (fh_imap_append(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&defVal3))) ? 1LL : 0LL;
    } else {
      fg1_imap_append(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_append", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_base64(TypedValue* _rv, Value* text) asm("_ZN4HPHP13f_imap_base64ERKNS_6StringE");

void fg1_imap_base64(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_base64(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imap_base64(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_base64(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_base64(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_base64(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_base64", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_binary(TypedValue* _rv, Value* str) asm("_ZN4HPHP13f_imap_binaryERKNS_6StringE");

void fg1_imap_binary(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_binary(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imap_binary(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_binary(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_binary(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_binary(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_binary", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_body(TypedValue* _rv, Value* imap_stream, long msg_number, long options) asm("_ZN4HPHP11f_imap_bodyERKNS_6ObjectEll");

void fg1_imap_body(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_body(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_body(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_body(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_body(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_body(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_body", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_bodystruct(TypedValue* _rv, Value* imap_stream, long msg_number, Value* section) asm("_ZN4HPHP17f_imap_bodystructERKNS_6ObjectElRKNS_6StringE");

void fg1_imap_bodystruct(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_bodystruct(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imap_bodystruct(rv, &args[-0].m_data, (long)(args[-1].m_data.num), &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_bodystruct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_bodystruct(rv, &args[-0].m_data, (long)(args[-1].m_data.num), &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_bodystruct(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_bodystruct", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_check(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP12f_imap_checkERKNS_6ObjectE");

void fg1_imap_check(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_check(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imap_check(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_check(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_imap_check(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_check(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_check", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_clearflag_full(Value* imap_stream, Value* sequence, Value* flag, long options) asm("_ZN4HPHP21f_imap_clearflag_fullERKNS_6ObjectERKNS_6StringES5_l");

void fg1_imap_clearflag_full(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_clearflag_full(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_clearflag_full(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_clearflag_full(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_clearflag_full(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_clearflag_full(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_clearflag_full", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_close(Value* imap_stream, long flag) asm("_ZN4HPHP12f_imap_closeERKNS_6ObjectEl");

void fg1_imap_close(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_close(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_close(&args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_close(&args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_close(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_close", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_createmailbox(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP20f_imap_createmailboxERKNS_6ObjectERKNS_6StringE");

void fg1_imap_createmailbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_createmailbox(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_createmailbox(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imap_createmailbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_createmailbox(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imap_createmailbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_createmailbox", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_delete(Value* imap_stream, Value* msg_number, long options) asm("_ZN4HPHP13f_imap_deleteERKNS_6ObjectERKNS_6StringEl");

void fg1_imap_delete(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_delete(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_delete(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_delete(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_delete(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_delete(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_delete", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_deletemailbox(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP20f_imap_deletemailboxERKNS_6ObjectERKNS_6StringE");

void fg1_imap_deletemailbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_deletemailbox(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_deletemailbox(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imap_deletemailbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_deletemailbox(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imap_deletemailbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_deletemailbox", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_errors(TypedValue* _rv) asm("_ZN4HPHP13f_imap_errorsEv");

TypedValue* fg_imap_errors(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_imap_errors(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("imap_errors", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_expunge(Value* imap_stream) asm("_ZN4HPHP14f_imap_expungeERKNS_6ObjectE");

void fg1_imap_expunge(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_expunge(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_expunge(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imap_expunge(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_expunge(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imap_expunge(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_expunge", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_fetch_overview(TypedValue* _rv, Value* imap_stream, Value* sequence, long options) asm("_ZN4HPHP21f_imap_fetch_overviewERKNS_6ObjectERKNS_6StringEl");

void fg1_imap_fetch_overview(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_fetch_overview(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_fetch_overview(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_fetch_overview(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_fetch_overview(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_fetch_overview(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_fetch_overview", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_fetchbody(TypedValue* _rv, Value* imap_stream, long msg_number, Value* section, long options) asm("_ZN4HPHP16f_imap_fetchbodyERKNS_6ObjectElRKNS_6StringEl");

void fg1_imap_fetchbody(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_fetchbody(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imap_fetchbody(rv, &args[-0].m_data, (long)(args[-1].m_data.num), &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_fetchbody(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_fetchbody(rv, &args[-0].m_data, (long)(args[-1].m_data.num), &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_fetchbody(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_fetchbody", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_fetchheader(TypedValue* _rv, Value* imap_stream, long msg_number, long options) asm("_ZN4HPHP18f_imap_fetchheaderERKNS_6ObjectEll");

void fg1_imap_fetchheader(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_fetchheader(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_fetchheader(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_fetchheader(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_fetchheader(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_fetchheader(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_fetchheader", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_fetchstructure(TypedValue* _rv, Value* imap_stream, long msg_number, long options) asm("_ZN4HPHP21f_imap_fetchstructureERKNS_6ObjectEll");

void fg1_imap_fetchstructure(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_fetchstructure(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_fetchstructure(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_fetchstructure(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_fetchstructure(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_fetchstructure(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_fetchstructure", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_gc(Value* imap_stream, long caches) asm("_ZN4HPHP9f_imap_gcERKNS_6ObjectEl");

void fg1_imap_gc(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_gc(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_gc(&args[-0].m_data, (long)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imap_gc(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_gc(&args[-0].m_data, (long)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imap_gc(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_gc", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_get_quota(TypedValue* _rv, Value* imap_stream, Value* quota_root) asm("_ZN4HPHP16f_imap_get_quotaERKNS_6ObjectERKNS_6StringE");

void fg1_imap_get_quota(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_get_quota(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imap_get_quota(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_get_quota(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_get_quota(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_get_quota(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_get_quota", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_get_quotaroot(TypedValue* _rv, Value* imap_stream, Value* quota_root) asm("_ZN4HPHP20f_imap_get_quotarootERKNS_6ObjectERKNS_6StringE");

void fg1_imap_get_quotaroot(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_get_quotaroot(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imap_get_quotaroot(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_get_quotaroot(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_get_quotaroot(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_get_quotaroot(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_get_quotaroot", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_getacl(TypedValue* _rv, Value* imap_stream, Value* mailbox) asm("_ZN4HPHP13f_imap_getaclERKNS_6ObjectERKNS_6StringE");

void fg1_imap_getacl(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_getacl(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imap_getacl(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_getacl(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_getacl(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_getacl(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_getacl", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_getmailboxes(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP19f_imap_getmailboxesERKNS_6ObjectERKNS_6StringES5_");

void fg1_imap_getmailboxes(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_getmailboxes(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_getmailboxes(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_getmailboxes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_getmailboxes(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_getmailboxes(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_getmailboxes", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_getsubscribed(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP20f_imap_getsubscribedERKNS_6ObjectERKNS_6StringES5_");

void fg1_imap_getsubscribed(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_getsubscribed(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_getsubscribed(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_getsubscribed(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_getsubscribed(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_getsubscribed(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_getsubscribed", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_header(TypedValue* _rv, Value* imap_stream, long msg_number, long fromlength, long subjectlength, Value* defaulthost) asm("_ZN4HPHP13f_imap_headerERKNS_6ObjectElllRKNS_6StringE");

void fg1_imap_header(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_header(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
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
  String defVal4 = "";
  fh_imap_header(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? &args[-4].m_data : (Value*)(&defVal4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_header(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      String defVal4 = "";
      fh_imap_header(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? &args[-4].m_data : (Value*)(&defVal4));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_header(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_header", count, 2, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_headerinfo(TypedValue* _rv, Value* imap_stream, long msg_number, long fromlength, long subjectlength, Value* defaulthost) asm("_ZN4HPHP17f_imap_headerinfoERKNS_6ObjectElllRKNS_6StringE");

void fg1_imap_headerinfo(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_headerinfo(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
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
  String defVal4 = "";
  fh_imap_headerinfo(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? &args[-4].m_data : (Value*)(&defVal4));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_headerinfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      String defVal4 = "";
      fh_imap_headerinfo(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? &args[-4].m_data : (Value*)(&defVal4));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_headerinfo(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_headerinfo", count, 2, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_headers(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP14f_imap_headersERKNS_6ObjectE");

void fg1_imap_headers(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_headers(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imap_headers(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_headers(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_imap_headers(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_headers(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_headers", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_last_error(TypedValue* _rv) asm("_ZN4HPHP17f_imap_last_errorEv");

TypedValue* fg_imap_last_error(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_imap_last_error(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("imap_last_error", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_list(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP11f_imap_listERKNS_6ObjectERKNS_6StringES5_");

void fg1_imap_list(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_list(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_list(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_list(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_list(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_list(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_list", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_listmailbox(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP18f_imap_listmailboxERKNS_6ObjectERKNS_6StringES5_");

void fg1_imap_listmailbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_listmailbox(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_listmailbox(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_listmailbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_listmailbox(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_listmailbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_listmailbox", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_listscan(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern, Value* content) asm("_ZN4HPHP15f_imap_listscanERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_imap_listscan(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_listscan(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_listscan(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_listscan(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_listscan(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_listscan(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_listscan", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_listsubscribed(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP21f_imap_listsubscribedERKNS_6ObjectERKNS_6StringES5_");

void fg1_imap_listsubscribed(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_listsubscribed(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_listsubscribed(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_listsubscribed(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_listsubscribed(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_listsubscribed(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_listsubscribed", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_lsub(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern) asm("_ZN4HPHP11f_imap_lsubERKNS_6ObjectERKNS_6StringES5_");

void fg1_imap_lsub(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_lsub(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_lsub(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_lsub(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_lsub(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_lsub(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_lsub", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_mail_compose(TypedValue* _rv, Value* envelope, Value* body) asm("_ZN4HPHP19f_imap_mail_composeERKNS_5ArrayES2_");

void fg1_imap_mail_compose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_mail_compose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-0);
  }
  fh_imap_mail_compose(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_mail_compose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfArray) {
      fh_imap_mail_compose(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_mail_compose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_mail_compose", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_mail_copy(Value* imap_stream, Value* msglist, Value* mailbox, long options) asm("_ZN4HPHP16f_imap_mail_copyERKNS_6ObjectERKNS_6StringES5_l");

void fg1_imap_mail_copy(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_mail_copy(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_mail_copy(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_mail_copy(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_mail_copy(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_mail_copy(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_mail_copy", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_mail_move(Value* imap_stream, Value* msglist, Value* mailbox, long options) asm("_ZN4HPHP16f_imap_mail_moveERKNS_6ObjectERKNS_6StringES5_l");

void fg1_imap_mail_move(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_mail_move(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_mail_move(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_mail_move(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_mail_move(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_mail_move(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_mail_move", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_mail(Value* to, Value* subject, Value* message, Value* additional_headers, Value* cc, Value* bcc, Value* rpath) asm("_ZN4HPHP11f_imap_mailERKNS_6StringES2_S2_S2_S2_S2_S2_");

void fg1_imap_mail(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_mail(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 7
    if (!IS_STRING_TYPE((args-6)->m_type)) {
      tvCastToStringInPlace(args-6);
    }
  case 6:
    if (!IS_STRING_TYPE((args-5)->m_type)) {
      tvCastToStringInPlace(args-5);
    }
  case 5:
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
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
  rv->m_type = KindOfBoolean;
  String defVal3 = "";
  String defVal4 = "";
  String defVal5 = "";
  String defVal6 = "";
  rv->m_data.num = (fh_imap_mail(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? &args[-4].m_data : (Value*)(&defVal4), (count > 5) ? &args[-5].m_data : (Value*)(&defVal5), (count > 6) ? &args[-6].m_data : (Value*)(&defVal6))) ? 1LL : 0LL;
}

TypedValue* fg_imap_mail(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 7) {
    if ((count <= 6 || IS_STRING_TYPE((args - 6)->m_type)) &&
        (count <= 5 || IS_STRING_TYPE((args - 5)->m_type)) &&
        (count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      String defVal3 = "";
      String defVal4 = "";
      String defVal5 = "";
      String defVal6 = "";
      rv->m_data.num = (fh_imap_mail(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? &args[-4].m_data : (Value*)(&defVal4), (count > 5) ? &args[-5].m_data : (Value*)(&defVal5), (count > 6) ? &args[-6].m_data : (Value*)(&defVal6))) ? 1LL : 0LL;
    } else {
      fg1_imap_mail(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_mail", count, 3, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_mailboxmsginfo(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP21f_imap_mailboxmsginfoERKNS_6ObjectE");

void fg1_imap_mailboxmsginfo(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_mailboxmsginfo(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imap_mailboxmsginfo(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_mailboxmsginfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_imap_mailboxmsginfo(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_mailboxmsginfo(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_mailboxmsginfo", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_mime_header_decode(TypedValue* _rv, Value* text) asm("_ZN4HPHP25f_imap_mime_header_decodeERKNS_6StringE");

void fg1_imap_mime_header_decode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_mime_header_decode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imap_mime_header_decode(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_mime_header_decode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_mime_header_decode(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_mime_header_decode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_mime_header_decode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_msgno(TypedValue* _rv, Value* imap_stream, long uid) asm("_ZN4HPHP12f_imap_msgnoERKNS_6ObjectEl");

void fg1_imap_msgno(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_msgno(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imap_msgno(rv, &args[-0].m_data, (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_msgno(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_msgno(rv, &args[-0].m_data, (long)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_msgno(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_msgno", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_num_msg(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP14f_imap_num_msgERKNS_6ObjectE");

void fg1_imap_num_msg(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_num_msg(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imap_num_msg(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_num_msg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_imap_num_msg(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_num_msg(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_num_msg", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_num_recent(TypedValue* _rv, Value* imap_stream) asm("_ZN4HPHP17f_imap_num_recentERKNS_6ObjectE");

void fg1_imap_num_recent(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_num_recent(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_imap_num_recent(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_num_recent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_imap_num_recent(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_num_recent(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_num_recent", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_open(TypedValue* _rv, Value* mailbox, Value* username, Value* password, long options, long retries) asm("_ZN4HPHP11f_imap_openERKNS_6StringES2_S2_ll");

void fg1_imap_open(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_open(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_open(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (long)(args[-4].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_open(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_open(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? (long)(args[-4].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_open(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_open", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_ping(Value* imap_stream) asm("_ZN4HPHP11f_imap_pingERKNS_6ObjectE");

void fg1_imap_ping(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_ping(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_ping(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imap_ping(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_ping(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imap_ping(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_ping", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_qprint(TypedValue* _rv, Value* str) asm("_ZN4HPHP13f_imap_qprintERKNS_6StringE");

void fg1_imap_qprint(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_qprint(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imap_qprint(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_qprint(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_qprint(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_qprint(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_qprint", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_renamemailbox(Value* imap_stream, Value* old_mbox, Value* new_mbox) asm("_ZN4HPHP20f_imap_renamemailboxERKNS_6ObjectERKNS_6StringES5_");

void fg1_imap_renamemailbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_renamemailbox(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_imap_renamemailbox(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imap_renamemailbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_renamemailbox(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imap_renamemailbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_renamemailbox", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_reopen(Value* imap_stream, Value* mailbox, long options, long retries) asm("_ZN4HPHP13f_imap_reopenERKNS_6ObjectERKNS_6StringEll");

void fg1_imap_reopen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_reopen(TypedValue* rv, ActRec* ar, int32_t count) {
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
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_reopen(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_reopen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_reopen(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_reopen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_reopen", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_rfc822_parse_adrlist(TypedValue* _rv, Value* address, Value* default_host) asm("_ZN4HPHP27f_imap_rfc822_parse_adrlistERKNS_6StringES2_");

void fg1_imap_rfc822_parse_adrlist(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_rfc822_parse_adrlist(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_imap_rfc822_parse_adrlist(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_rfc822_parse_adrlist(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_rfc822_parse_adrlist(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_rfc822_parse_adrlist(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_rfc822_parse_adrlist", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_rfc822_parse_headers(TypedValue* _rv, Value* headers, Value* defaulthost) asm("_ZN4HPHP27f_imap_rfc822_parse_headersERKNS_6StringES2_");

void fg1_imap_rfc822_parse_headers(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_rfc822_parse_headers(TypedValue* rv, ActRec* ar, int32_t count) {
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
  String defVal1 = "";
  fh_imap_rfc822_parse_headers(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_rfc822_parse_headers(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      String defVal1 = "";
      fh_imap_rfc822_parse_headers(rv, &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_rfc822_parse_headers(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_rfc822_parse_headers", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_rfc822_write_address(TypedValue* _rv, Value* mailbox, Value* host, Value* personal) asm("_ZN4HPHP27f_imap_rfc822_write_addressERKNS_6StringES2_S2_");

void fg1_imap_rfc822_write_address(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_rfc822_write_address(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_imap_rfc822_write_address(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_rfc822_write_address(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_rfc822_write_address(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_rfc822_write_address(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_rfc822_write_address", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_savebody(Value* imap_stream, TypedValue* file, long msg_number, Value* part_number, long options) asm("_ZN4HPHP15f_imap_savebodyERKNS_6ObjectERKNS_7VariantElRKNS_6StringEl");

void fg1_imap_savebody(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_savebody(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  String defVal3 = "";
  rv->m_data.num = (fh_imap_savebody(&args[-0].m_data, (args-1), (long)(args[-2].m_data.num), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (long)(args[-4].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_savebody(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      String defVal3 = "";
      rv->m_data.num = (fh_imap_savebody(&args[-0].m_data, (args-1), (long)(args[-2].m_data.num), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3), (count > 4) ? (long)(args[-4].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_savebody(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_savebody", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_scanmailbox(TypedValue* _rv, Value* imap_stream, Value* ref, Value* pattern, Value* content) asm("_ZN4HPHP18f_imap_scanmailboxERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_imap_scanmailbox(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_scanmailbox(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_scanmailbox(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_scanmailbox(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_scanmailbox(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_scanmailbox(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_scanmailbox", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_search(TypedValue* _rv, Value* imap_stream, Value* criteria, long options, Value* charset) asm("_ZN4HPHP13f_imap_searchERKNS_6ObjectERKNS_6StringElS5_");

void fg1_imap_search(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_search(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
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
  String defVal3 = "";
  fh_imap_search(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_search(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      String defVal3 = "";
      fh_imap_search(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_search(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_search", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_set_quota(Value* imap_stream, Value* quota_root, long quota_limit) asm("_ZN4HPHP16f_imap_set_quotaERKNS_6ObjectERKNS_6StringEl");

void fg1_imap_set_quota(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_set_quota(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_set_quota(&args[-0].m_data, &args[-1].m_data, (long)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_imap_set_quota(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_set_quota(&args[-0].m_data, &args[-1].m_data, (long)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_imap_set_quota(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_set_quota", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_setacl(Value* imap_stream, Value* mailbox, Value* id, Value* rights) asm("_ZN4HPHP13f_imap_setaclERKNS_6ObjectERKNS_6StringES5_S5_");

void fg1_imap_setacl(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_setacl(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_imap_setacl(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imap_setacl(ActRec* ar) {
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
      rv->m_data.num = (fh_imap_setacl(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, &args[-3].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imap_setacl(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_setacl", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_setflag_full(Value* imap_stream, Value* sequence, Value* flag, long options) asm("_ZN4HPHP19f_imap_setflag_fullERKNS_6ObjectERKNS_6StringES5_l");

void fg1_imap_setflag_full(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_setflag_full(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_setflag_full(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_setflag_full(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_setflag_full(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (long)(args[-3].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_setflag_full(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_setflag_full", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_sort(TypedValue* _rv, Value* imap_stream, long criteria, long reverse, long options, Value* search_criteria, Value* charset) asm("_ZN4HPHP11f_imap_sortERKNS_6ObjectElllRKNS_6StringES5_");

void fg1_imap_sort(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_sort(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if (!IS_STRING_TYPE((args-5)->m_type)) {
      tvCastToStringInPlace(args-5);
    }
  case 5:
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  String defVal4 = "";
  String defVal5 = "";
  fh_imap_sort(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (long)(args[-2].m_data.num), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? &args[-4].m_data : (Value*)(&defVal4), (count > 5) ? &args[-5].m_data : (Value*)(&defVal5));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_sort(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 6) {
    if ((count <= 5 || IS_STRING_TYPE((args - 5)->m_type)) &&
        (count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      String defVal4 = "";
      String defVal5 = "";
      fh_imap_sort(rv, &args[-0].m_data, (long)(args[-1].m_data.num), (long)(args[-2].m_data.num), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0), (count > 4) ? &args[-4].m_data : (Value*)(&defVal4), (count > 5) ? &args[-5].m_data : (Value*)(&defVal5));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_sort(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_sort", count, 3, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_status(TypedValue* _rv, Value* imap_stream, Value* mailbox, long options) asm("_ZN4HPHP13f_imap_statusERKNS_6ObjectERKNS_6StringEl");

void fg1_imap_status(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_status(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_status(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_status(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_status(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_status", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_subscribe(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP16f_imap_subscribeERKNS_6ObjectERKNS_6StringE");

void fg1_imap_subscribe(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_subscribe(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_subscribe(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imap_subscribe(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_subscribe(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imap_subscribe(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_subscribe", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_thread(TypedValue* _rv, Value* imap_stream, long options) asm("_ZN4HPHP13f_imap_threadERKNS_6ObjectEl");

void fg1_imap_thread(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_thread(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_thread(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_thread(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_thread(rv, &args[-0].m_data, (count > 1) ? (long)(args[-1].m_data.num) : (long)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_thread(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_thread", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_timeout(TypedValue* _rv, long timeout_type, long timeout) asm("_ZN4HPHP14f_imap_timeoutEll");

void fg1_imap_timeout(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_timeout(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_imap_timeout(rv, (long)(args[-0].m_data.num), (count > 1) ? (long)(args[-1].m_data.num) : (long)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_timeout(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      fh_imap_timeout(rv, (long)(args[-0].m_data.num), (count > 1) ? (long)(args[-1].m_data.num) : (long)(-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_timeout(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_timeout", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_uid(TypedValue* _rv, Value* imap_stream, long msg_number) asm("_ZN4HPHP10f_imap_uidERKNS_6ObjectEl");

void fg1_imap_uid(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_uid(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_imap_uid(rv, &args[-0].m_data, (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_uid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_imap_uid(rv, &args[-0].m_data, (long)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_uid(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_uid", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_undelete(Value* imap_stream, Value* msg_number, long flags) asm("_ZN4HPHP15f_imap_undeleteERKNS_6ObjectERKNS_6StringEl");

void fg1_imap_undelete(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_undelete(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_undelete(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
}

TypedValue* fg_imap_undelete(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_undelete(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (long)(args[-2].m_data.num) : (long)(0))) ? 1LL : 0LL;
    } else {
      fg1_imap_undelete(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_undelete", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_imap_unsubscribe(Value* imap_stream, Value* mailbox) asm("_ZN4HPHP18f_imap_unsubscribeERKNS_6ObjectERKNS_6StringE");

void fg1_imap_unsubscribe(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_unsubscribe(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_imap_unsubscribe(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_imap_unsubscribe(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_imap_unsubscribe(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_imap_unsubscribe(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_unsubscribe", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_utf7_decode(TypedValue* _rv, Value* text) asm("_ZN4HPHP18f_imap_utf7_decodeERKNS_6StringE");

void fg1_imap_utf7_decode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_utf7_decode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imap_utf7_decode(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_utf7_decode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_utf7_decode(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_utf7_decode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_utf7_decode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_utf7_encode(TypedValue* _rv, Value* data) asm("_ZN4HPHP18f_imap_utf7_encodeERKNS_6StringE");

void fg1_imap_utf7_encode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_utf7_encode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imap_utf7_encode(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_utf7_encode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_utf7_encode(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_utf7_encode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_utf7_encode", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_imap_utf8(TypedValue* _rv, Value* mime_encoded_text) asm("_ZN4HPHP11f_imap_utf8ERKNS_6StringE");

void fg1_imap_utf8(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_imap_utf8(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_imap_utf8(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_imap_utf8(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_imap_utf8(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_imap_utf8(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("imap_utf8", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
