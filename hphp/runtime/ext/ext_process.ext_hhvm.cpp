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

long fh_pcntl_alarm(int seconds) asm("_ZN4HPHP13f_pcntl_alarmEi");

void fg1_pcntl_alarm(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_alarm(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pcntl_alarm((int)(args[-0].m_data.num));
}

TypedValue* fg_pcntl_alarm(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pcntl_alarm((int)(args[-0].m_data.num));
    } else {
      fg1_pcntl_alarm(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_alarm", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_pcntl_exec(Value* path, Value* args, Value* envs) asm("_ZN4HPHP12f_pcntl_execERKNS_6StringERKNS_5ArrayES5_");

void fg1_pcntl_exec(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_exec(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-2);
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
  rv->m_type = KindOfNull;
  fh_pcntl_exec(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_array), (count > 2) ? &args[-2].m_data : (Value*)(&null_array));
}

TypedValue* fg_pcntl_exec(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfArray) &&
        (count <= 1 || (args - 1)->m_type == KindOfArray) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_pcntl_exec(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_array), (count > 2) ? &args[-2].m_data : (Value*)(&null_array));
    } else {
      fg1_pcntl_exec(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_exec", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pcntl_fork() asm("_ZN4HPHP12f_pcntl_forkEv");

TypedValue* fg_pcntl_fork(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_pcntl_fork();
  } else {
    throw_toomany_arguments_nr("pcntl_fork", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_pcntl_getpriority(TypedValue* _rv, int pid, int process_identifier) asm("_ZN4HPHP19f_pcntl_getpriorityEii");

void fg1_pcntl_getpriority(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_getpriority(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-0);
    }
  case 0:
    break;
  }
  fh_pcntl_getpriority(rv, (count > 0) ? (int)(args[-0].m_data.num) : (int)(0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_pcntl_getpriority(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (count <= 0 || (args - 0)->m_type == KindOfInt64)) {
      fh_pcntl_getpriority(rv, (count > 0) ? (int)(args[-0].m_data.num) : (int)(0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_pcntl_getpriority(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("pcntl_getpriority", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pcntl_setpriority(int priority, int pid, int process_identifier) asm("_ZN4HPHP19f_pcntl_setpriorityEiii");

void fg1_pcntl_setpriority(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_setpriority(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_pcntl_setpriority((int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_pcntl_setpriority(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_pcntl_setpriority((int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_pcntl_setpriority(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_setpriority", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pcntl_signal_dispatch() asm("_ZN4HPHP23f_pcntl_signal_dispatchEv");

TypedValue* fg_pcntl_signal_dispatch(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_pcntl_signal_dispatch()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("pcntl_signal_dispatch", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pcntl_signal(int signo, TypedValue* handler, bool restart_syscalls) asm("_ZN4HPHP14f_pcntl_signalEiRKNS_7VariantEb");

void fg1_pcntl_signal(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_signal(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_pcntl_signal((int)(args[-0].m_data.num), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_pcntl_signal(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_pcntl_signal((int)(args[-0].m_data.num), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_pcntl_signal(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_signal", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pcntl_wait(TypedValue* status, int options) asm("_ZN4HPHP12f_pcntl_waitERKNS_14VRefParamValueEi");

void fg1_pcntl_wait(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_wait(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pcntl_wait((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
}

TypedValue* fg_pcntl_wait(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pcntl_wait((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
    } else {
      fg1_pcntl_wait(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_wait", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pcntl_waitpid(int pid, TypedValue* status, int options) asm("_ZN4HPHP15f_pcntl_waitpidEiRKNS_14VRefParamValueEi");

void fg1_pcntl_waitpid(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_waitpid(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pcntl_waitpid((int)(args[-0].m_data.num), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
}

TypedValue* fg_pcntl_waitpid(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pcntl_waitpid((int)(args[-0].m_data.num), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
    } else {
      fg1_pcntl_waitpid(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_waitpid", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pcntl_wexitstatus(int status) asm("_ZN4HPHP19f_pcntl_wexitstatusEi");

void fg1_pcntl_wexitstatus(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_wexitstatus(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pcntl_wexitstatus((int)(args[-0].m_data.num));
}

TypedValue* fg_pcntl_wexitstatus(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pcntl_wexitstatus((int)(args[-0].m_data.num));
    } else {
      fg1_pcntl_wexitstatus(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_wexitstatus", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pcntl_wifexited(int status) asm("_ZN4HPHP17f_pcntl_wifexitedEi");

void fg1_pcntl_wifexited(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_wifexited(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_pcntl_wifexited((int)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_pcntl_wifexited(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_pcntl_wifexited((int)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_pcntl_wifexited(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_wifexited", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pcntl_wifsignaled(int status) asm("_ZN4HPHP19f_pcntl_wifsignaledEi");

void fg1_pcntl_wifsignaled(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_wifsignaled(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_pcntl_wifsignaled((int)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_pcntl_wifsignaled(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_pcntl_wifsignaled((int)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_pcntl_wifsignaled(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_wifsignaled", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_pcntl_wifstopped(int status) asm("_ZN4HPHP18f_pcntl_wifstoppedEi");

void fg1_pcntl_wifstopped(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_wifstopped(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_pcntl_wifstopped((int)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_pcntl_wifstopped(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_pcntl_wifstopped((int)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_pcntl_wifstopped(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_wifstopped", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pcntl_wstopsig(int status) asm("_ZN4HPHP16f_pcntl_wstopsigEi");

void fg1_pcntl_wstopsig(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_wstopsig(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pcntl_wstopsig((int)(args[-0].m_data.num));
}

TypedValue* fg_pcntl_wstopsig(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pcntl_wstopsig((int)(args[-0].m_data.num));
    } else {
      fg1_pcntl_wstopsig(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_wstopsig", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_pcntl_wtermsig(int status) asm("_ZN4HPHP16f_pcntl_wtermsigEi");

void fg1_pcntl_wtermsig(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pcntl_wtermsig(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_pcntl_wtermsig((int)(args[-0].m_data.num));
}

TypedValue* fg_pcntl_wtermsig(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_pcntl_wtermsig((int)(args[-0].m_data.num));
    } else {
      fg1_pcntl_wtermsig(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pcntl_wtermsig", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_shell_exec(Value* _rv, Value* cmd) asm("_ZN4HPHP12f_shell_execERKNS_6StringE");

void fg1_shell_exec(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_shell_exec(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_shell_exec(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_shell_exec(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_shell_exec(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_shell_exec(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("shell_exec", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_exec(Value* _rv, Value* command, TypedValue* output, TypedValue* return_var) asm("_ZN4HPHP6f_execERKNS_6StringERKNS_14VRefParamValueES5_");

void fg1_exec(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_exec(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  VRefParamValue defVal1 = uninit_null();
  VRefParamValue defVal2 = uninit_null();
  fh_exec(&(rv->m_data), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_exec(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      VRefParamValue defVal1 = uninit_null();
      VRefParamValue defVal2 = uninit_null();
      fh_exec(&(rv->m_data), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_exec(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("exec", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_passthru(Value* command, TypedValue* return_var) asm("_ZN4HPHP10f_passthruERKNS_6StringERKNS_14VRefParamValueE");

void fg1_passthru(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_passthru(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  VRefParamValue defVal1 = uninit_null();
  fh_passthru(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
}

TypedValue* fg_passthru(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      VRefParamValue defVal1 = uninit_null();
      fh_passthru(&args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
    } else {
      fg1_passthru(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("passthru", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_system(Value* _rv, Value* command, TypedValue* return_var) asm("_ZN4HPHP8f_systemERKNS_6StringERKNS_14VRefParamValueE");

void fg1_system(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_system(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  VRefParamValue defVal1 = uninit_null();
  fh_system(&(rv->m_data), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_system(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      VRefParamValue defVal1 = uninit_null();
      fh_system(&(rv->m_data), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_system(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("system", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_proc_open(TypedValue* _rv, Value* cmd, Value* descriptorspec, TypedValue* pipes, Value* cwd, TypedValue* env, TypedValue* other_options) asm("_ZN4HPHP11f_proc_openERKNS_6StringERKNS_5ArrayERKNS_14VRefParamValueES2_RKNS_7VariantESB_");

void fg1_proc_open(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_proc_open(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
  case 5:
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_proc_open(rv, &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? (args-4) : (TypedValue*)(&null_variant), (count > 5) ? (args-5) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_proc_open(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 6) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (args - 1)->m_type == KindOfArray &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_proc_open(rv, &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? (args-4) : (TypedValue*)(&null_variant), (count > 5) ? (args-5) : (TypedValue*)(&null_variant));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_proc_open(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("proc_open", count, 3, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_proc_terminate(Value* process, int signal) asm("_ZN4HPHP16f_proc_terminateERKNS_6ObjectEi");

void fg1_proc_terminate(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_proc_terminate(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_proc_terminate(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_proc_terminate(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_proc_terminate(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_proc_terminate(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("proc_terminate", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_proc_close(Value* process) asm("_ZN4HPHP12f_proc_closeERKNS_6ObjectE");

void fg1_proc_close(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_proc_close(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_proc_close(&args[-0].m_data);
}

TypedValue* fg_proc_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_proc_close(&args[-0].m_data);
    } else {
      fg1_proc_close(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("proc_close", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_proc_get_status(Value* _rv, Value* process) asm("_ZN4HPHP17f_proc_get_statusERKNS_6ObjectE");

void fg1_proc_get_status(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_proc_get_status(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_proc_get_status(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_proc_get_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_proc_get_status(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_proc_get_status(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("proc_get_status", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_proc_nice(int increment) asm("_ZN4HPHP11f_proc_niceEi");

void fg1_proc_nice(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_proc_nice(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_proc_nice((int)(args[-0].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_proc_nice(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_proc_nice((int)(args[-0].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_proc_nice(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("proc_nice", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_escapeshellarg(Value* _rv, Value* arg) asm("_ZN4HPHP16f_escapeshellargERKNS_6StringE");

void fg1_escapeshellarg(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_escapeshellarg(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_escapeshellarg(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_escapeshellarg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_escapeshellarg(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_escapeshellarg(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("escapeshellarg", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_escapeshellcmd(Value* _rv, Value* command) asm("_ZN4HPHP16f_escapeshellcmdERKNS_6StringE");

void fg1_escapeshellcmd(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_escapeshellcmd(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_escapeshellcmd(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_escapeshellcmd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_escapeshellcmd(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_escapeshellcmd(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("escapeshellcmd", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
