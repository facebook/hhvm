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

bool fh_dangling_server_proxy_old_request() asm("_ZN4HPHP35f_dangling_server_proxy_old_requestEv");

TypedValue* fg_dangling_server_proxy_old_request(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_dangling_server_proxy_old_request()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("dangling_server_proxy_old_request", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_dangling_server_proxy_new_request(Value* host) asm("_ZN4HPHP35f_dangling_server_proxy_new_requestERKNS_6StringE");

void fg1_dangling_server_proxy_new_request(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_dangling_server_proxy_new_request(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_dangling_server_proxy_new_request(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_dangling_server_proxy_new_request(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_dangling_server_proxy_new_request(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_dangling_server_proxy_new_request(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("dangling_server_proxy_new_request", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pagelet_server_is_enabled() asm("_ZN4HPHP27f_pagelet_server_is_enabledEv");

TypedValue* fg_pagelet_server_is_enabled(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_pagelet_server_is_enabled()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("pagelet_server_is_enabled", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pagelet_server_task_start(Value* _rv, Value* url, Value* headers, Value* post_data, Value* files) asm("_ZN4HPHP27f_pagelet_server_task_startERKNS_6StringERKNS_5ArrayES2_S5_");

void fg1_pagelet_server_task_start(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pagelet_server_task_start(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfObject;
  fh_pagelet_server_task_start(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_array), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_array));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pagelet_server_task_start(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfArray) &&
        (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (count <= 1 || (args - 1)->m_type == KindOfArray) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfObject;
      fh_pagelet_server_task_start(&(rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_array), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_array));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pagelet_server_task_start(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pagelet_server_task_start", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pagelet_server_task_status(Value* task) asm("_ZN4HPHP28f_pagelet_server_task_statusERKNS_6ObjectE");

void fg1_pagelet_server_task_status(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pagelet_server_task_status(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pagelet_server_task_status(&args[-0].m_data);
}

TypedValue* fg_pagelet_server_task_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pagelet_server_task_status(&args[-0].m_data);
    } else {
      fg1_pagelet_server_task_status(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pagelet_server_task_status", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_pagelet_server_task_result(Value* _rv, Value* task, TypedValue* headers, TypedValue* code, long timeout_ms) asm("_ZN4HPHP28f_pagelet_server_task_resultERKNS_6ObjectERKNS_14VRefParamValueES5_l");

void fg1_pagelet_server_task_result(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pagelet_server_task_result(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfString;
  fh_pagelet_server_task_result(&(rv->m_data), &args[-0].m_data, (args-1), (args-2), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_pagelet_server_task_result(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfString;
      fh_pagelet_server_task_result(&(rv->m_data), &args[-0].m_data, (args-1), (args-2), (count > 3) ? (long)(args[-3].m_data.num) : (long)(0));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_pagelet_server_task_result(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pagelet_server_task_result", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pagelet_server_flush() asm("_ZN4HPHP22f_pagelet_server_flushEv");

TypedValue* fg_pagelet_server_flush(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_pagelet_server_flush();
  } else {
    throw_toomany_arguments_nr("pagelet_server_flush", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xbox_send_message(Value* msg, TypedValue* ret, long timeout_ms, Value* host) asm("_ZN4HPHP19f_xbox_send_messageERKNS_6StringERKNS_14VRefParamValueElS2_");

void fg1_xbox_send_message(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xbox_send_message(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  String defVal3 = "localhost";
  rv->m_data.num = (fh_xbox_send_message(&args[-0].m_data, (args-1), (long)(args[-2].m_data.num), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3))) ? 1LL : 0LL;
}

TypedValue* fg_xbox_send_message(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (args - 2)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      String defVal3 = "localhost";
      rv->m_data.num = (fh_xbox_send_message(&args[-0].m_data, (args-1), (long)(args[-2].m_data.num), (count > 3) ? &args[-3].m_data : (Value*)(&defVal3))) ? 1LL : 0LL;
    } else {
      fg1_xbox_send_message(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xbox_send_message", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xbox_post_message(Value* msg, Value* host) asm("_ZN4HPHP19f_xbox_post_messageERKNS_6StringES2_");

void fg1_xbox_post_message(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xbox_post_message(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  String defVal1 = "localhost";
  rv->m_data.num = (fh_xbox_post_message(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1))) ? 1LL : 0LL;
}

TypedValue* fg_xbox_post_message(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      String defVal1 = "localhost";
      rv->m_data.num = (fh_xbox_post_message(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&defVal1))) ? 1LL : 0LL;
    } else {
      fg1_xbox_post_message(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xbox_post_message", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_xbox_task_start(Value* _rv, Value* message) asm("_ZN4HPHP17f_xbox_task_startERKNS_6StringE");

void fg1_xbox_task_start(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xbox_task_start(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfObject;
  fh_xbox_task_start(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_xbox_task_start(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfObject;
      fh_xbox_task_start(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_xbox_task_start(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xbox_task_start", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_xbox_task_status(Value* task) asm("_ZN4HPHP18f_xbox_task_statusERKNS_6ObjectE");

void fg1_xbox_task_status(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xbox_task_status(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_xbox_task_status(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_xbox_task_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_xbox_task_status(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_xbox_task_status(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xbox_task_status", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xbox_task_result(Value* task, long timeout_ms, TypedValue* ret) asm("_ZN4HPHP18f_xbox_task_resultERKNS_6ObjectElRKNS_14VRefParamValueE");

void fg1_xbox_task_result(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xbox_task_result(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_xbox_task_result(&args[-0].m_data, (long)(args[-1].m_data.num), (args-2));
}

TypedValue* fg_xbox_task_result(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_xbox_task_result(&args[-0].m_data, (long)(args[-1].m_data.num), (args-2));
    } else {
      fg1_xbox_task_result(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xbox_task_result", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_xbox_process_call_message(TypedValue* _rv, Value* msg) asm("_ZN4HPHP27f_xbox_process_call_messageERKNS_6StringE");

void fg1_xbox_process_call_message(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xbox_process_call_message(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_xbox_process_call_message(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_xbox_process_call_message(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_xbox_process_call_message(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_xbox_process_call_message(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xbox_process_call_message", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xbox_get_thread_timeout() asm("_ZN4HPHP25f_xbox_get_thread_timeoutEv");

TypedValue* fg_xbox_get_thread_timeout(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_xbox_get_thread_timeout();
  } else {
    throw_toomany_arguments_nr("xbox_get_thread_timeout", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_xbox_set_thread_timeout(int timeout) asm("_ZN4HPHP25f_xbox_set_thread_timeoutEi");

void fg1_xbox_set_thread_timeout(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_xbox_set_thread_timeout(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfNull;
  fh_xbox_set_thread_timeout((int)(args[-0].m_data.num));
}

TypedValue* fg_xbox_set_thread_timeout(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfNull;
      fh_xbox_set_thread_timeout((int)(args[-0].m_data.num));
    } else {
      fg1_xbox_set_thread_timeout(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("xbox_set_thread_timeout", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_xbox_schedule_thread_reset() asm("_ZN4HPHP28f_xbox_schedule_thread_resetEv");

TypedValue* fg_xbox_schedule_thread_reset(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_xbox_schedule_thread_reset();
  } else {
    throw_toomany_arguments_nr("xbox_schedule_thread_reset", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_xbox_get_thread_time() asm("_ZN4HPHP22f_xbox_get_thread_timeEv");

TypedValue* fg_xbox_get_thread_time(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_xbox_get_thread_time();
  } else {
    throw_toomany_arguments_nr("xbox_get_thread_time", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
