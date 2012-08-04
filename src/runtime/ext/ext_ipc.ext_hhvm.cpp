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
long long HPHP::f_ftok(HPHP::String const&, HPHP::String const&)
_ZN4HPHP6f_ftokERKNS_6StringES2_

(return value) => rax
pathname => rdi
proj => rsi
*/

long long fh_ftok(Value* pathname, Value* proj) asm("_ZN4HPHP6f_ftokERKNS_6StringES2_");

TypedValue * fg1_ftok(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_ftok(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (long long)fh_ftok((Value*)(args-0), (Value*)(args-1));
  return rv;
}

TypedValue* fg_ftok(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_ftok((Value*)(args-0), (Value*)(args-1));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ftok(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ftok", count, 2, 2, 1);
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
HPHP::Variant HPHP::f_msg_get_queue(long long, long long)
_ZN4HPHP15f_msg_get_queueExx

(return value) => rax
_rv => rdi
key => rsi
perms => rdx
*/

TypedValue* fh_msg_get_queue(TypedValue* _rv, long long key, long long perms) asm("_ZN4HPHP15f_msg_get_queueExx");

TypedValue * fg1_msg_get_queue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_msg_get_queue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_msg_get_queue((rv), (long long)(args[-0].m_data.num), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0666));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_msg_get_queue(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_msg_get_queue((&(rv)), (long long)(args[-0].m_data.num), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(0666));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_msg_get_queue(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("msg_get_queue", count, 1, 2, 1);
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
bool HPHP::f_msg_queue_exists(long long)
_ZN4HPHP18f_msg_queue_existsEx

(return value) => rax
key => rdi
*/

bool fh_msg_queue_exists(long long key) asm("_ZN4HPHP18f_msg_queue_existsEx");

TypedValue * fg1_msg_queue_exists(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_msg_queue_exists(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_msg_queue_exists((long long)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_msg_queue_exists(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_msg_queue_exists((long long)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_msg_queue_exists(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("msg_queue_exists", count, 1, 1, 1);
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
bool HPHP::f_msg_send(HPHP::Object const&, long long, HPHP::Variant const&, bool, bool, HPHP::VRefParamValue const&)
_ZN4HPHP10f_msg_sendERKNS_6ObjectExRKNS_7VariantEbbRKNS_14VRefParamValueE

(return value) => rax
queue => rdi
msgtype => rsi
message => rdx
serialize => rcx
blocking => r8
errorcode => r9
*/

bool fh_msg_send(Value* queue, long long msgtype, TypedValue* message, bool serialize, bool blocking, TypedValue* errorcode) asm("_ZN4HPHP10f_msg_sendERKNS_6ObjectExRKNS_7VariantEbbRKNS_14VRefParamValueE");

TypedValue * fg1_msg_send(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_msg_send(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
  VRefParamValue defVal5 = null;
  rv->m_data.num = (fh_msg_send((Value*)(args-0), (long long)(args[-1].m_data.num), (args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(true), (count > 5) ? (args-5) : (TypedValue*)(&defVal5))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_msg_send(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 3LL && count <= 6LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfBoolean) && (count <= 3 || (args-3)->m_type == KindOfBoolean) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        VRefParamValue defVal5 = null;
        rv.m_data.num = (fh_msg_send((Value*)(args-0), (long long)(args[-1].m_data.num), (args-2), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(true), (count > 5) ? (args-5) : (TypedValue*)(&defVal5))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_msg_send(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("msg_send", count, 3, 6, 1);
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
bool HPHP::f_msg_receive(HPHP::Object const&, long long, HPHP::VRefParamValue const&, long long, HPHP::VRefParamValue const&, bool, long long, HPHP::VRefParamValue const&)
_ZN4HPHP13f_msg_receiveERKNS_6ObjectExRKNS_14VRefParamValueExS5_bxS5_

(return value) => rax
queue => rdi
desiredmsgtype => rsi
msgtype => rdx
maxsize => rcx
message => r8
unserialize => r9
flags => st0
errorcode => st8
*/

bool fh_msg_receive(Value* queue, long long desiredmsgtype, TypedValue* msgtype, long long maxsize, TypedValue* message, bool unserialize, long long flags, TypedValue* errorcode) asm("_ZN4HPHP13f_msg_receiveERKNS_6ObjectExRKNS_14VRefParamValueExS5_bxS5_");

TypedValue * fg1_msg_receive(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_msg_receive(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
  VRefParamValue defVal7 = null;
  rv->m_data.num = (fh_msg_receive((Value*)(args-0), (long long)(args[-1].m_data.num), (args-2), (long long)(args[-3].m_data.num), (args-4), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(true), (count > 6) ? (long long)(args[-6].m_data.num) : (long long)(0), (count > 7) ? (args-7) : (TypedValue*)(&defVal7))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_msg_receive(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 5LL && count <= 8LL) {
      if ((count <= 6 || (args-6)->m_type == KindOfInt64) && (count <= 5 || (args-5)->m_type == KindOfBoolean) && (args-3)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        VRefParamValue defVal7 = null;
        rv.m_data.num = (fh_msg_receive((Value*)(args-0), (long long)(args[-1].m_data.num), (args-2), (long long)(args[-3].m_data.num), (args-4), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(true), (count > 6) ? (long long)(args[-6].m_data.num) : (long long)(0), (count > 7) ? (args-7) : (TypedValue*)(&defVal7))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_msg_receive(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("msg_receive", count, 5, 8, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_msg_remove_queue(HPHP::Object const&)
_ZN4HPHP18f_msg_remove_queueERKNS_6ObjectE

(return value) => rax
queue => rdi
*/

bool fh_msg_remove_queue(Value* queue) asm("_ZN4HPHP18f_msg_remove_queueERKNS_6ObjectE");

TypedValue * fg1_msg_remove_queue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_msg_remove_queue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_msg_remove_queue((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_msg_remove_queue(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_msg_remove_queue((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_msg_remove_queue(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("msg_remove_queue", count, 1, 1, 1);
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
bool HPHP::f_msg_set_queue(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP15f_msg_set_queueERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
queue => rdi
data => rsi
*/

bool fh_msg_set_queue(Value* queue, Value* data) asm("_ZN4HPHP15f_msg_set_queueERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_msg_set_queue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_msg_set_queue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_msg_set_queue((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_msg_set_queue(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_msg_set_queue((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_msg_set_queue(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("msg_set_queue", count, 2, 2, 1);
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
HPHP::Array HPHP::f_msg_stat_queue(HPHP::Object const&)
_ZN4HPHP16f_msg_stat_queueERKNS_6ObjectE

(return value) => rax
_rv => rdi
queue => rsi
*/

Value* fh_msg_stat_queue(Value* _rv, Value* queue) asm("_ZN4HPHP16f_msg_stat_queueERKNS_6ObjectE");

TypedValue * fg1_msg_stat_queue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_msg_stat_queue(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfArray;
  tvCastToObjectInPlace(args-0);
  fh_msg_stat_queue((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_msg_stat_queue(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        fh_msg_stat_queue((Value*)(&(rv)), (Value*)(args-0));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_msg_stat_queue(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("msg_stat_queue", count, 1, 1, 1);
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
bool HPHP::f_sem_acquire(HPHP::Object const&)
_ZN4HPHP13f_sem_acquireERKNS_6ObjectE

(return value) => rax
sem_identifier => rdi
*/

bool fh_sem_acquire(Value* sem_identifier) asm("_ZN4HPHP13f_sem_acquireERKNS_6ObjectE");

TypedValue * fg1_sem_acquire(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sem_acquire(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_sem_acquire((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_sem_acquire(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_sem_acquire((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sem_acquire(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sem_acquire", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_sem_get(long long, long long, long long, bool)
_ZN4HPHP9f_sem_getExxxb

(return value) => rax
_rv => rdi
key => rsi
max_acquire => rdx
perm => rcx
auto_release => r8
*/

TypedValue* fh_sem_get(TypedValue* _rv, long long key, long long max_acquire, long long perm, bool auto_release) asm("_ZN4HPHP9f_sem_getExxxb");

TypedValue * fg1_sem_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sem_get(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_sem_get((rv), (long long)(args[-0].m_data.num), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0666), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_sem_get(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_sem_get((&(rv)), (long long)(args[-0].m_data.num), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(1), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0666), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(true));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sem_get(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sem_get", count, 1, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_sem_release(HPHP::Object const&)
_ZN4HPHP13f_sem_releaseERKNS_6ObjectE

(return value) => rax
sem_identifier => rdi
*/

bool fh_sem_release(Value* sem_identifier) asm("_ZN4HPHP13f_sem_releaseERKNS_6ObjectE");

TypedValue * fg1_sem_release(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sem_release(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_sem_release((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_sem_release(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_sem_release((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sem_release(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sem_release", count, 1, 1, 1);
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
bool HPHP::f_sem_remove(HPHP::Object const&)
_ZN4HPHP12f_sem_removeERKNS_6ObjectE

(return value) => rax
sem_identifier => rdi
*/

bool fh_sem_remove(Value* sem_identifier) asm("_ZN4HPHP12f_sem_removeERKNS_6ObjectE");

TypedValue * fg1_sem_remove(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_sem_remove(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_sem_remove((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_sem_remove(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_sem_remove((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_sem_remove(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("sem_remove", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_shm_attach(long long, long long, long long)
_ZN4HPHP12f_shm_attachExxx

(return value) => rax
_rv => rdi
shm_key => rsi
shm_size => rdx
shm_flag => rcx
*/

TypedValue* fh_shm_attach(TypedValue* _rv, long long shm_key, long long shm_size, long long shm_flag) asm("_ZN4HPHP12f_shm_attachExxx");

TypedValue * fg1_shm_attach(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_shm_attach(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_shm_attach((rv), (long long)(args[-0].m_data.num), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(10000), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0666));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_shm_attach(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_shm_attach((&(rv)), (long long)(args[-0].m_data.num), (count > 1) ? (long long)(args[-1].m_data.num) : (long long)(10000), (count > 2) ? (long long)(args[-2].m_data.num) : (long long)(0666));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_shm_attach(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("shm_attach", count, 1, 3, 1);
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
bool HPHP::f_shm_detach(long long)
_ZN4HPHP12f_shm_detachEx

(return value) => rax
shm_identifier => rdi
*/

bool fh_shm_detach(long long shm_identifier) asm("_ZN4HPHP12f_shm_detachEx");

TypedValue * fg1_shm_detach(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_shm_detach(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_shm_detach((long long)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_shm_detach(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_shm_detach((long long)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_shm_detach(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("shm_detach", count, 1, 1, 1);
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
bool HPHP::f_shm_remove(long long)
_ZN4HPHP12f_shm_removeEx

(return value) => rax
shm_identifier => rdi
*/

bool fh_shm_remove(long long shm_identifier) asm("_ZN4HPHP12f_shm_removeEx");

TypedValue * fg1_shm_remove(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_shm_remove(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  rv->m_data.num = (fh_shm_remove((long long)(args[-0].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_shm_remove(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_shm_remove((long long)(args[-0].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_shm_remove(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("shm_remove", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_shm_get_var(long long, long long)
_ZN4HPHP13f_shm_get_varExx

(return value) => rax
_rv => rdi
shm_identifier => rsi
variable_key => rdx
*/

TypedValue* fh_shm_get_var(TypedValue* _rv, long long shm_identifier, long long variable_key) asm("_ZN4HPHP13f_shm_get_varExx");

TypedValue * fg1_shm_get_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_shm_get_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_shm_get_var((rv), (long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_shm_get_var(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        fh_shm_get_var((&(rv)), (long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_shm_get_var(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("shm_get_var", count, 2, 2, 1);
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
bool HPHP::f_shm_has_var(long long, long long)
_ZN4HPHP13f_shm_has_varExx

(return value) => rax
shm_identifier => rdi
variable_key => rsi
*/

bool fh_shm_has_var(long long shm_identifier, long long variable_key) asm("_ZN4HPHP13f_shm_has_varExx");

TypedValue * fg1_shm_has_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_shm_has_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_shm_has_var((long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_shm_has_var(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_shm_has_var((long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_shm_has_var(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("shm_has_var", count, 2, 2, 1);
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
bool HPHP::f_shm_put_var(long long, long long, HPHP::Variant const&)
_ZN4HPHP13f_shm_put_varExxRKNS_7VariantE

(return value) => rax
shm_identifier => rdi
variable_key => rsi
variable => rdx
*/

bool fh_shm_put_var(long long shm_identifier, long long variable_key, TypedValue* variable) asm("_ZN4HPHP13f_shm_put_varExxRKNS_7VariantE");

TypedValue * fg1_shm_put_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_shm_put_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_shm_put_var((long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num), (args-2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_shm_put_var(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_shm_put_var((long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num), (args-2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_shm_put_var(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("shm_put_var", count, 3, 3, 1);
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
bool HPHP::f_shm_remove_var(long long, long long)
_ZN4HPHP16f_shm_remove_varExx

(return value) => rax
shm_identifier => rdi
variable_key => rsi
*/

bool fh_shm_remove_var(long long shm_identifier, long long variable_key) asm("_ZN4HPHP16f_shm_remove_varExx");

TypedValue * fg1_shm_remove_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_shm_remove_var(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_shm_remove_var((long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_shm_remove_var(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_shm_remove_var((long long)(args[-0].m_data.num), (long long)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_shm_remove_var(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("shm_remove_var", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}




} // !HPHP

