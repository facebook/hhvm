#include <runtime/ext_hhvm/ext_hhvm.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/array/array_init.h>
#include <runtime/ext/ext.h>
#include <runtime/vm/class.h>
#include <runtime/vm/runtime.h>
#include <runtime/vm/exception_gate.h>
#include <exception>

namespace HPHP {

/*
void HPHP::f_pcntl_exec(HPHP::String const&, HPHP::Array const&, HPHP::Array const&)
_ZN4HPHP12f_pcntl_execERKNS_6StringERKNS_5ArrayES5_

path => rdi
args => rsi
envs => rdx
*/

void fh_pcntl_exec(Value* path, Value* args, Value* envs) asm("_ZN4HPHP12f_pcntl_execERKNS_6StringERKNS_5ArrayES5_");

TypedValue * fg1_pcntl_exec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_exec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
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
  fh_pcntl_exec((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_array), (count > 2) ? (Value*)(args-2) : (Value*)(&null_array));
  return rv;
}

TypedValue* fg_pcntl_exec(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfArray) && (count <= 1 || (args-1)->m_type == KindOfArray) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_pcntl_exec((Value*)(args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&null_array), (count > 2) ? (Value*)(args-2) : (Value*)(&null_array));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_exec(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_exec", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
int HPHP::f_pcntl_fork()
_ZN4HPHP12f_pcntl_forkEv

(return value) => rax
*/

int fh_pcntl_fork() asm("_ZN4HPHP12f_pcntl_forkEv");

TypedValue* fg_pcntl_fork(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfInt64;
      rv.m_data.num = (long long)fh_pcntl_fork();
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("pcntl_fork", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_pcntl_getpriority(int, int)
_ZN4HPHP19f_pcntl_getpriorityEii

(return value) => rax
_rv => rdi
pid => rsi
process_identifier => rdx
*/

TypedValue* fh_pcntl_getpriority(TypedValue* _rv, int pid, int process_identifier) asm("_ZN4HPHP19f_pcntl_getpriorityEii");

TypedValue * fg1_pcntl_getpriority(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_getpriority(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_pcntl_getpriority((rv), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pcntl_getpriority(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (count <= 0 || (args-0)->m_type == KindOfInt64)) {
        fh_pcntl_getpriority((&(rv)), (count > 0) ? (int)(args[-0].m_data.num) : (int)(0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_getpriority(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("pcntl_getpriority", 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_pcntl_setpriority(int, int, int)
_ZN4HPHP19f_pcntl_setpriorityEiii

(return value) => rax
priority => rdi
pid => rsi
process_identifier => rdx
*/

bool fh_pcntl_setpriority(int priority, int pid, int process_identifier) asm("_ZN4HPHP19f_pcntl_setpriorityEiii");

TypedValue * fg1_pcntl_setpriority(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_setpriority(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_pcntl_setpriority((int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_pcntl_setpriority(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_pcntl_setpriority((int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_setpriority(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_setpriority", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_pcntl_signal(int, HPHP::Variant const&, bool)
_ZN4HPHP14f_pcntl_signalEiRKNS_7VariantEb

(return value) => rax
signo => rdi
handler => rsi
restart_syscalls => rdx
*/

bool fh_pcntl_signal(int signo, TypedValue* handler, bool restart_syscalls) asm("_ZN4HPHP14f_pcntl_signalEiRKNS_7VariantEb");

TypedValue * fg1_pcntl_signal(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_signal(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_pcntl_signal((int)(args[-0].m_data.num), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_pcntl_signal(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfBoolean) && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_pcntl_signal((int)(args[-0].m_data.num), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_signal(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_signal", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
int HPHP::f_pcntl_wait(HPHP::VRefParamValue const&, int)
_ZN4HPHP12f_pcntl_waitERKNS_14VRefParamValueEi

(return value) => rax
status => rdi
options => rsi
*/

int fh_pcntl_wait(TypedValue* status, int options) asm("_ZN4HPHP12f_pcntl_waitERKNS_14VRefParamValueEi");

TypedValue * fg1_pcntl_wait(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_wait(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (long long)fh_pcntl_wait((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  return rv;
}

TypedValue* fg_pcntl_wait(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_pcntl_wait((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_wait(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_wait", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
int HPHP::f_pcntl_waitpid(int, HPHP::VRefParamValue const&, int)
_ZN4HPHP15f_pcntl_waitpidEiRKNS_14VRefParamValueEi

(return value) => rax
pid => rdi
status => rsi
options => rdx
*/

int fh_pcntl_waitpid(int pid, TypedValue* status, int options) asm("_ZN4HPHP15f_pcntl_waitpidEiRKNS_14VRefParamValueEi");

TypedValue * fg1_pcntl_waitpid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pcntl_waitpid(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
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
  rv->m_data.num = (long long)fh_pcntl_waitpid((int)(args[-0].m_data.num), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  return rv;
}

TypedValue* fg_pcntl_waitpid(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_pcntl_waitpid((int)(args[-0].m_data.num), (args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pcntl_waitpid(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pcntl_waitpid", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_pcntl_signal_dispatch()
_ZN4HPHP23f_pcntl_signal_dispatchEv

(return value) => rax
*/

bool fh_pcntl_signal_dispatch() asm("_ZN4HPHP23f_pcntl_signal_dispatchEv");

TypedValue* fg_pcntl_signal_dispatch(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_pcntl_signal_dispatch()) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("pcntl_signal_dispatch", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::String HPHP::f_shell_exec(HPHP::String const&)
_ZN4HPHP12f_shell_execERKNS_6StringE

(return value) => rax
_rv => rdi
cmd => rsi
*/

Value* fh_shell_exec(Value* _rv, Value* cmd) asm("_ZN4HPHP12f_shell_execERKNS_6StringE");

TypedValue * fg1_shell_exec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_shell_exec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_shell_exec((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_shell_exec(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_shell_exec((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_shell_exec(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("shell_exec", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::String HPHP::f_exec(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP6f_execERKNS_6StringERKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
command => rsi
output => rdx
return_var => rcx
*/

Value* fh_exec(Value* _rv, Value* command, TypedValue* output, TypedValue* return_var) asm("_ZN4HPHP6f_execERKNS_6StringERKNS_14VRefParamValueES5_");

TypedValue * fg1_exec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_exec(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = null;
  VRefParamValue defVal2 = null;
  fh_exec((Value*)(rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_exec(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        VRefParamValue defVal1 = null;
        VRefParamValue defVal2 = null;
        fh_exec((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_exec(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("exec", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
void HPHP::f_passthru(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP10f_passthruERKNS_6StringERKNS_14VRefParamValueE

command => rdi
return_var => rsi
*/

void fh_passthru(Value* command, TypedValue* return_var) asm("_ZN4HPHP10f_passthruERKNS_6StringERKNS_14VRefParamValueE");

TypedValue * fg1_passthru(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_passthru(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = null;
  fh_passthru((Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  return rv;
}

TypedValue* fg_passthru(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        VRefParamValue defVal1 = null;
        fh_passthru((Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_passthru(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("passthru", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::String HPHP::f_system(HPHP::String const&, HPHP::VRefParamValue const&)
_ZN4HPHP8f_systemERKNS_6StringERKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
command => rsi
return_var => rdx
*/

Value* fh_system(Value* _rv, Value* command, TypedValue* return_var) asm("_ZN4HPHP8f_systemERKNS_6StringERKNS_14VRefParamValueE");

TypedValue * fg1_system(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_system(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  VRefParamValue defVal1 = null;
  fh_system((Value*)(rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_system(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        VRefParamValue defVal1 = null;
        fh_system((Value*)(&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_system(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("system", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_proc_open(HPHP::String const&, HPHP::Array const&, HPHP::VRefParamValue const&, HPHP::String const&, HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP11f_proc_openERKNS_6StringERKNS_5ArrayERKNS_14VRefParamValueES2_RKNS_7VariantESB_

(return value) => rax
_rv => rdi
cmd => rsi
descriptorspec => rdx
pipes => rcx
cwd => r8
env => r9
other_options => st0
*/

TypedValue* fh_proc_open(TypedValue* _rv, Value* cmd, Value* descriptorspec, TypedValue* pipes, Value* cwd, TypedValue* env, TypedValue* other_options) asm("_ZN4HPHP11f_proc_openERKNS_6StringERKNS_5ArrayERKNS_14VRefParamValueES2_RKNS_7VariantESB_");

TypedValue * fg1_proc_open(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_proc_open(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_proc_open((rv), (Value*)(args-0), (Value*)(args-1), (args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (args-4) : (TypedValue*)(&null_variant), (count > 5) ? (args-5) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_proc_open(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 6LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (args-1)->m_type == KindOfArray && IS_STRING_TYPE((args-0)->m_type)) {
        fh_proc_open((&(rv)), (Value*)(args-0), (Value*)(args-1), (args-2), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (args-4) : (TypedValue*)(&null_variant), (count > 5) ? (args-5) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_proc_open(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("proc_open", count, 3, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_proc_terminate(HPHP::Object const&, int)
_ZN4HPHP16f_proc_terminateERKNS_6ObjectEi

(return value) => rax
process => rdi
signal => rsi
*/

bool fh_proc_terminate(Value* process, int signal) asm("_ZN4HPHP16f_proc_terminateERKNS_6ObjectEi");

TypedValue * fg1_proc_terminate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_proc_terminate(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_proc_terminate((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_proc_terminate(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_proc_terminate((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_proc_terminate(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("proc_terminate", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
int HPHP::f_proc_close(HPHP::Object const&)
_ZN4HPHP12f_proc_closeERKNS_6ObjectE

(return value) => rax
process => rdi
*/

int fh_proc_close(Value* process) asm("_ZN4HPHP12f_proc_closeERKNS_6ObjectE");

TypedValue * fg1_proc_close(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_proc_close(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (long long)fh_proc_close((Value*)(args-0));
  return rv;
}

TypedValue* fg_proc_close(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_proc_close((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_proc_close(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("proc_close", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Array HPHP::f_proc_get_status(HPHP::Object const&)
_ZN4HPHP17f_proc_get_statusERKNS_6ObjectE

(return value) => rax
_rv => rdi
process => rsi
*/

Value* fh_proc_get_status(Value* _rv, Value* process) asm("_ZN4HPHP17f_proc_get_statusERKNS_6ObjectE");

TypedValue * fg1_proc_get_status(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_proc_get_status(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_proc_get_status((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_proc_get_status(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_proc_get_status((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_proc_get_status(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("proc_get_status", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_proc_nice(int)
_ZN4HPHP11f_proc_niceEi

(return value) => rax
increment => rdi
*/

bool fh_proc_nice(int increment) asm("_ZN4HPHP11f_proc_niceEi");

TypedValue * fg1_proc_nice(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_proc_nice(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_proc_nice((int)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_proc_nice(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_proc_nice((int)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_proc_nice(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("proc_nice", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::String HPHP::f_escapeshellarg(HPHP::String const&)
_ZN4HPHP16f_escapeshellargERKNS_6StringE

(return value) => rax
_rv => rdi
arg => rsi
*/

Value* fh_escapeshellarg(Value* _rv, Value* arg) asm("_ZN4HPHP16f_escapeshellargERKNS_6StringE");

TypedValue * fg1_escapeshellarg(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_escapeshellarg(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_escapeshellarg((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_escapeshellarg(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_escapeshellarg((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_escapeshellarg(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("escapeshellarg", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::String HPHP::f_escapeshellcmd(HPHP::String const&)
_ZN4HPHP16f_escapeshellcmdERKNS_6StringE

(return value) => rax
_rv => rdi
command => rsi
*/

Value* fh_escapeshellcmd(Value* _rv, Value* command) asm("_ZN4HPHP16f_escapeshellcmdERKNS_6StringE");

TypedValue * fg1_escapeshellcmd(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_escapeshellcmd(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_escapeshellcmd((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_escapeshellcmd(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_escapeshellcmd((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_escapeshellcmd(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("escapeshellcmd", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}




} // !HPHP

