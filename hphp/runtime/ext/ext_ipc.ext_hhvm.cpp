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

long fh_ftok(Value* pathname, Value* proj) asm("_ZN4HPHP6f_ftokERKNS_6StringES2_");

void fg1_ftok(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ftok(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_ftok(&args[-0].m_data, &args[-1].m_data);
}

TypedValue* fg_ftok(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_ftok(&args[-0].m_data, &args[-1].m_data);
    } else {
      fg1_ftok(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ftok", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_msg_get_queue(TypedValue* _rv, long key, long perms) asm("_ZN4HPHP15f_msg_get_queueEll");

void fg1_msg_get_queue(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_msg_get_queue(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_msg_get_queue(rv, (long)(args[-0].m_data.num), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0666));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_msg_get_queue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      fh_msg_get_queue(rv, (long)(args[-0].m_data.num), (count > 1) ? (long)(args[-1].m_data.num) : (long)(0666));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_msg_get_queue(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("msg_get_queue", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_msg_queue_exists(long key) asm("_ZN4HPHP18f_msg_queue_existsEl");

void fg1_msg_queue_exists(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_msg_queue_exists(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_msg_queue_exists((long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_msg_queue_exists(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_msg_queue_exists((long)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_msg_queue_exists(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("msg_queue_exists", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_msg_remove_queue(Value* queue) asm("_ZN4HPHP18f_msg_remove_queueERKNS_6ObjectE");

void fg1_msg_remove_queue(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_msg_remove_queue(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_msg_remove_queue(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_msg_remove_queue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_msg_remove_queue(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_msg_remove_queue(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("msg_remove_queue", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_msg_set_queue(Value* queue, Value* data) asm("_ZN4HPHP15f_msg_set_queueERKNS_6ObjectERKNS_5ArrayE");

void fg1_msg_set_queue(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_msg_set_queue(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_msg_set_queue(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_msg_set_queue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfArray &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_msg_set_queue(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_msg_set_queue(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("msg_set_queue", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_msg_stat_queue(Value* _rv, Value* queue) asm("_ZN4HPHP16f_msg_stat_queueERKNS_6ObjectE");

void fg1_msg_stat_queue(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_msg_stat_queue(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_msg_stat_queue(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_msg_stat_queue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_msg_stat_queue(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_msg_stat_queue(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("msg_stat_queue", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_msg_send(Value* queue, long msgtype, TypedValue* message, bool serialize, bool blocking, TypedValue* errorcode) asm("_ZN4HPHP10f_msg_sendERKNS_6ObjectElRKNS_7VariantEbbRKNS_14VRefParamValueE");

void fg1_msg_send(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_msg_send(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
  case 5:
    if ((args-4)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal5 = uninit_null();
  rv->m_data.num = (fh_msg_send(&args[-0].m_data, (long)(args[-1].m_data.num), (args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(true), (count > 5) ? (args-5) : (TypedValue*)(&defVal5))) ? 1LL : 0LL;
}

TypedValue* fg_msg_send(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 6) {
    if ((count <= 4 || (args - 4)->m_type == KindOfBoolean) &&
        (count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal5 = uninit_null();
      rv->m_data.num = (fh_msg_send(&args[-0].m_data, (long)(args[-1].m_data.num), (args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(true), (count > 5) ? (args-5) : (TypedValue*)(&defVal5))) ? 1LL : 0LL;
    } else {
      fg1_msg_send(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("msg_send", count, 3, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_msg_receive(Value* queue, long desiredmsgtype, TypedValue* msgtype, long maxsize, TypedValue* message, bool unserialize, long flags, TypedValue* errorcode) asm("_ZN4HPHP13f_msg_receiveERKNS_6ObjectElRKNS_14VRefParamValueElS5_blS5_");

void fg1_msg_receive(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_msg_receive(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 8
  case 7:
    if ((args-6)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-5);
    }
  case 5:
    break;
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal7 = uninit_null();
  rv->m_data.num = (fh_msg_receive(&args[-0].m_data, (long)(args[-1].m_data.num), (args-2), (long)(args[-3].m_data.num), (args-4), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(true), (count > 6) ? (long)(args[-6].m_data.num) : (long)(0), (count > 7) ? (args-7) : (TypedValue*)(&defVal7))) ? 1LL : 0LL;
}

TypedValue* fg_msg_receive(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 5 && count <= 8) {
    if ((count <= 6 || (args - 6)->m_type == KindOfInt64) &&
        (count <= 5 || (args - 5)->m_type == KindOfBoolean) &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal7 = uninit_null();
      rv->m_data.num = (fh_msg_receive(&args[-0].m_data, (long)(args[-1].m_data.num), (args-2), (long)(args[-3].m_data.num), (args-4), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(true), (count > 6) ? (long)(args[-6].m_data.num) : (long)(0), (count > 7) ? (args-7) : (TypedValue*)(&defVal7))) ? 1LL : 0LL;
    } else {
      fg1_msg_receive(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("msg_receive", count, 5, 8, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 8);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_sem_acquire(Value* sem_identifier) asm("_ZN4HPHP13f_sem_acquireERKNS_6ObjectE");

void fg1_sem_acquire(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sem_acquire(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_sem_acquire(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_sem_acquire(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_sem_acquire(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_sem_acquire(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sem_acquire", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_sem_release(Value* sem_identifier) asm("_ZN4HPHP13f_sem_releaseERKNS_6ObjectE");

void fg1_sem_release(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sem_release(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_sem_release(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_sem_release(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_sem_release(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_sem_release(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sem_release", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_sem_get(TypedValue* _rv, long key, long max_acquire, long perm, bool auto_release) asm("_ZN4HPHP9f_sem_getElllb");

void fg1_sem_get(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sem_get(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
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
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_sem_get(rv, (long)(args[-0].m_data.num), (count > 1) ? (long)(args[-1].m_data.num) : (long)(1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0666), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_sem_get(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfBoolean) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      fh_sem_get(rv, (long)(args[-0].m_data.num), (count > 1) ? (long)(args[-1].m_data.num) : (long)(1), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0666), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_sem_get(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sem_get", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_sem_remove(Value* sem_identifier) asm("_ZN4HPHP12f_sem_removeERKNS_6ObjectE");

void fg1_sem_remove(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_sem_remove(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_sem_remove(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_sem_remove(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_sem_remove(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_sem_remove(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("sem_remove", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_shm_attach(TypedValue* _rv, long shm_key, long shm_size, long shm_flag) asm("_ZN4HPHP12f_shm_attachElll");

void fg1_shm_attach(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_shm_attach(TypedValue* rv, ActRec* ar, int32_t count) {
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
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_shm_attach(rv, (long)(args[-0].m_data.num), (count > 1) ? (long)(args[-1].m_data.num) : (long)(10000), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0666));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_shm_attach(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      fh_shm_attach(rv, (long)(args[-0].m_data.num), (count > 1) ? (long)(args[-1].m_data.num) : (long)(10000), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0666));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_shm_attach(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("shm_attach", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_shm_detach(long shm_identifier) asm("_ZN4HPHP12f_shm_detachEl");

void fg1_shm_detach(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_shm_detach(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_shm_detach((long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_shm_detach(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_shm_detach((long)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_shm_detach(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("shm_detach", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_shm_remove(long shm_identifier) asm("_ZN4HPHP12f_shm_removeEl");

void fg1_shm_remove(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_shm_remove(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_shm_remove((long)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_shm_remove(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_shm_remove((long)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_shm_remove(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("shm_remove", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_shm_get_var(TypedValue* _rv, long shm_identifier, long variable_key) asm("_ZN4HPHP13f_shm_get_varEll");

void fg1_shm_get_var(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_shm_get_var(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_shm_get_var(rv, (long)(args[-0].m_data.num), (long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_shm_get_var(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      fh_shm_get_var(rv, (long)(args[-0].m_data.num), (long)(args[-1].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_shm_get_var(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("shm_get_var", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_shm_has_var(long shm_identifier, long variable_key) asm("_ZN4HPHP13f_shm_has_varEll");

void fg1_shm_has_var(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_shm_has_var(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_shm_has_var((long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_shm_has_var(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_shm_has_var((long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_shm_has_var(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("shm_has_var", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_shm_put_var(long shm_identifier, long variable_key, TypedValue* variable) asm("_ZN4HPHP13f_shm_put_varEllRKNS_7VariantE");

void fg1_shm_put_var(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_shm_put_var(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_shm_put_var((long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (args-2))) ? 1LL : 0LL;
}

TypedValue* fg_shm_put_var(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_shm_put_var((long)(args[-0].m_data.num), (long)(args[-1].m_data.num), (args-2))) ? 1LL : 0LL;
    } else {
      fg1_shm_put_var(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("shm_put_var", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_shm_remove_var(long shm_identifier, long variable_key) asm("_ZN4HPHP16f_shm_remove_varEll");

void fg1_shm_remove_var(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_shm_remove_var(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_shm_remove_var((long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_shm_remove_var(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_shm_remove_var((long)(args[-0].m_data.num), (long)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_shm_remove_var(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("shm_remove_var", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
