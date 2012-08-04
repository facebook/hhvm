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
HPHP::Variant HPHP::f_fsockopen(HPHP::String const&, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, double)
_ZN4HPHP11f_fsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d

(return value) => rax
_rv => rdi
hostname => rsi
port => rdx
errnum => rcx
errstr => r8
timeout => xmm0
*/

TypedValue* fh_fsockopen(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr, double timeout) asm("_ZN4HPHP11f_fsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d");

TypedValue * fg1_fsockopen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_fsockopen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-4);
    }
  case 4:
  case 3:
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
  VRefParamValue defVal2 = null;
  VRefParamValue defVal3 = null;
  fh_fsockopen((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_fsockopen(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfDouble) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        VRefParamValue defVal2 = null;
        VRefParamValue defVal3 = null;
        fh_fsockopen((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_fsockopen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("fsockopen", count, 1, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_pfsockopen(HPHP::String const&, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, double)
_ZN4HPHP12f_pfsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d

(return value) => rax
_rv => rdi
hostname => rsi
port => rdx
errnum => rcx
errstr => r8
timeout => xmm0
*/

TypedValue* fh_pfsockopen(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr, double timeout) asm("_ZN4HPHP12f_pfsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d");

TypedValue * fg1_pfsockopen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_pfsockopen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-4);
    }
  case 4:
  case 3:
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
  VRefParamValue defVal2 = null;
  VRefParamValue defVal3 = null;
  fh_pfsockopen((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_pfsockopen(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfDouble) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        VRefParamValue defVal2 = null;
        VRefParamValue defVal3 = null;
        fh_pfsockopen((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_pfsockopen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("pfsockopen", count, 1, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_socket_create(int, int, int)
_ZN4HPHP15f_socket_createEiii

(return value) => rax
_rv => rdi
domain => rsi
type => rdx
protocol => rcx
*/

TypedValue* fh_socket_create(TypedValue* _rv, int domain, int type, int protocol) asm("_ZN4HPHP15f_socket_createEiii");

TypedValue * fg1_socket_create(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_create(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_socket_create((rv), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_create(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        fh_socket_create((&(rv)), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_create(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_create", count, 3, 3, 1);
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
HPHP::Variant HPHP::f_socket_create_listen(int, int)
_ZN4HPHP22f_socket_create_listenEii

(return value) => rax
_rv => rdi
port => rsi
backlog => rdx
*/

TypedValue* fh_socket_create_listen(TypedValue* _rv, int port, int backlog) asm("_ZN4HPHP22f_socket_create_listenEii");

TypedValue * fg1_socket_create_listen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_create_listen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_socket_create_listen((rv), (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(128));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_create_listen(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfInt64) {
        fh_socket_create_listen((&(rv)), (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(128));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_create_listen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_create_listen", count, 1, 2, 1);
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
bool HPHP::f_socket_create_pair(int, int, int, HPHP::VRefParamValue const&)
_ZN4HPHP20f_socket_create_pairEiiiRKNS_14VRefParamValueE

(return value) => rax
domain => rdi
type => rsi
protocol => rdx
fd => rcx
*/

bool fh_socket_create_pair(int domain, int type, int protocol, TypedValue* fd) asm("_ZN4HPHP20f_socket_create_pairEiiiRKNS_14VRefParamValueE");

TypedValue * fg1_socket_create_pair(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_create_pair(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (fh_socket_create_pair((int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args-3))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_create_pair(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_create_pair((int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args-3))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_create_pair(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_create_pair", count, 4, 4, 1);
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
HPHP::Variant HPHP::f_socket_get_option(HPHP::Object const&, int, int)
_ZN4HPHP19f_socket_get_optionERKNS_6ObjectEii

(return value) => rax
_rv => rdi
socket => rsi
level => rdx
optname => rcx
*/

TypedValue* fh_socket_get_option(TypedValue* _rv, Value* socket, int level, int optname) asm("_ZN4HPHP19f_socket_get_optionERKNS_6ObjectEii");

TypedValue * fg1_socket_get_option(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_get_option(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_socket_get_option((rv), (Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_get_option(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_socket_get_option((&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_get_option(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_get_option", count, 3, 3, 1);
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
bool HPHP::f_socket_getpeername(HPHP::Object const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP20f_socket_getpeernameERKNS_6ObjectERKNS_14VRefParamValueES5_

(return value) => rax
socket => rdi
address => rsi
port => rdx
*/

bool fh_socket_getpeername(Value* socket, TypedValue* address, TypedValue* port) asm("_ZN4HPHP20f_socket_getpeernameERKNS_6ObjectERKNS_14VRefParamValueES5_");

TypedValue * fg1_socket_getpeername(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_getpeername(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  VRefParamValue defVal2 = null;
  rv->m_data.num = (fh_socket_getpeername((Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_getpeername(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        VRefParamValue defVal2 = null;
        rv.m_data.num = (fh_socket_getpeername((Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_getpeername(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_getpeername", count, 2, 3, 1);
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
bool HPHP::f_socket_getsockname(HPHP::Object const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP20f_socket_getsocknameERKNS_6ObjectERKNS_14VRefParamValueES5_

(return value) => rax
socket => rdi
address => rsi
port => rdx
*/

bool fh_socket_getsockname(Value* socket, TypedValue* address, TypedValue* port) asm("_ZN4HPHP20f_socket_getsocknameERKNS_6ObjectERKNS_14VRefParamValueES5_");

TypedValue * fg1_socket_getsockname(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_getsockname(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  VRefParamValue defVal2 = null;
  rv->m_data.num = (fh_socket_getsockname((Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_getsockname(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        VRefParamValue defVal2 = null;
        rv.m_data.num = (fh_socket_getsockname((Value*)(args-0), (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_getsockname(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_getsockname", count, 2, 3, 1);
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
bool HPHP::f_socket_set_block(HPHP::Object const&)
_ZN4HPHP18f_socket_set_blockERKNS_6ObjectE

(return value) => rax
socket => rdi
*/

bool fh_socket_set_block(Value* socket) asm("_ZN4HPHP18f_socket_set_blockERKNS_6ObjectE");

TypedValue * fg1_socket_set_block(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_set_block(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_socket_set_block((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_set_block(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_set_block((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_set_block(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_set_block", count, 1, 1, 1);
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
bool HPHP::f_socket_set_nonblock(HPHP::Object const&)
_ZN4HPHP21f_socket_set_nonblockERKNS_6ObjectE

(return value) => rax
socket => rdi
*/

bool fh_socket_set_nonblock(Value* socket) asm("_ZN4HPHP21f_socket_set_nonblockERKNS_6ObjectE");

TypedValue * fg1_socket_set_nonblock(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_set_nonblock(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_socket_set_nonblock((Value*)(args-0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_set_nonblock(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_set_nonblock((Value*)(args-0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_set_nonblock(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_set_nonblock", count, 1, 1, 1);
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
bool HPHP::f_socket_set_option(HPHP::Object const&, int, int, HPHP::Variant const&)
_ZN4HPHP19f_socket_set_optionERKNS_6ObjectEiiRKNS_7VariantE

(return value) => rax
socket => rdi
level => rsi
optname => rdx
optval => rcx
*/

bool fh_socket_set_option(Value* socket, int level, int optname, TypedValue* optval) asm("_ZN4HPHP19f_socket_set_optionERKNS_6ObjectEiiRKNS_7VariantE");

TypedValue * fg1_socket_set_option(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_set_option(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_socket_set_option((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args-3))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_set_option(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_set_option((Value*)(args-0), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args-3))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_set_option(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_set_option", count, 4, 4, 1);
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
bool HPHP::f_socket_connect(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP16f_socket_connectERKNS_6ObjectERKNS_6StringEi

(return value) => rax
socket => rdi
address => rsi
port => rdx
*/

bool fh_socket_connect(Value* socket, Value* address, int port) asm("_ZN4HPHP16f_socket_connectERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_socket_connect(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_connect(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_socket_connect((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_connect(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_connect((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_connect(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_connect", count, 2, 3, 1);
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
bool HPHP::f_socket_bind(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP13f_socket_bindERKNS_6ObjectERKNS_6StringEi

(return value) => rax
socket => rdi
address => rsi
port => rdx
*/

bool fh_socket_bind(Value* socket, Value* address, int port) asm("_ZN4HPHP13f_socket_bindERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_socket_bind(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_bind(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_socket_bind((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_bind(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_bind((Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_bind(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_bind", count, 2, 3, 1);
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
bool HPHP::f_socket_listen(HPHP::Object const&, int)
_ZN4HPHP15f_socket_listenERKNS_6ObjectEi

(return value) => rax
socket => rdi
backlog => rsi
*/

bool fh_socket_listen(Value* socket, int backlog) asm("_ZN4HPHP15f_socket_listenERKNS_6ObjectEi");

TypedValue * fg1_socket_listen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_listen(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  rv->m_data.num = (fh_socket_listen((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_listen(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_listen((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_listen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_listen", count, 1, 2, 1);
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
HPHP::Variant HPHP::f_socket_select(HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP15f_socket_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi

(return value) => rax
_rv => rdi
read => rsi
write => rdx
except => rcx
vtv_sec => r8
tv_usec => r9
*/

TypedValue* fh_socket_select(TypedValue* _rv, TypedValue* read, TypedValue* write, TypedValue* except, TypedValue* vtv_sec, int tv_usec) asm("_ZN4HPHP15f_socket_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi");

TypedValue * fg1_socket_select(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_select(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-4);
  fh_socket_select((rv), (args-0), (args-1), (args-2), (args-3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_select(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfInt64)) {
        fh_socket_select((&(rv)), (args-0), (args-1), (args-2), (args-3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_select(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_select", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_socket_server(HPHP::String const&, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP15f_socket_serverERKNS_6StringEiRKNS_14VRefParamValueES5_

(return value) => rax
_rv => rdi
hostname => rsi
port => rdx
errnum => rcx
errstr => r8
*/

TypedValue* fh_socket_server(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr) asm("_ZN4HPHP15f_socket_serverERKNS_6StringEiRKNS_14VRefParamValueES5_");

TypedValue * fg1_socket_server(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_server(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
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
  VRefParamValue defVal2 = null;
  VRefParamValue defVal3 = null;
  fh_socket_server((rv), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_server(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 4LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        VRefParamValue defVal2 = null;
        VRefParamValue defVal3 = null;
        fh_socket_server((&(rv)), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_server(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_server", count, 1, 4, 1);
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
HPHP::Variant HPHP::f_socket_accept(HPHP::Object const&)
_ZN4HPHP15f_socket_acceptERKNS_6ObjectE

(return value) => rax
_rv => rdi
socket => rsi
*/

TypedValue* fh_socket_accept(TypedValue* _rv, Value* socket) asm("_ZN4HPHP15f_socket_acceptERKNS_6ObjectE");

TypedValue * fg1_socket_accept(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_accept(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_socket_accept((rv), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_accept(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_socket_accept((&(rv)), (Value*)(args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_accept(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_accept", count, 1, 1, 1);
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
HPHP::Variant HPHP::f_socket_read(HPHP::Object const&, int, int)
_ZN4HPHP13f_socket_readERKNS_6ObjectEii

(return value) => rax
_rv => rdi
socket => rsi
length => rdx
type => rcx
*/

TypedValue* fh_socket_read(TypedValue* _rv, Value* socket, int length, int type) asm("_ZN4HPHP13f_socket_readERKNS_6ObjectEii");

TypedValue * fg1_socket_read(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_read(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_socket_read((rv), (Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_read(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_socket_read((&(rv)), (Value*)(args-0), (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_read(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_read", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_socket_write(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP14f_socket_writeERKNS_6ObjectERKNS_6StringEi

(return value) => rax
_rv => rdi
socket => rsi
buffer => rdx
length => rcx
*/

TypedValue* fh_socket_write(TypedValue* _rv, Value* socket, Value* buffer, int length) asm("_ZN4HPHP14f_socket_writeERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_socket_write(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_write(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_socket_write((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_write(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_socket_write((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_write(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_write", count, 2, 3, 1);
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
HPHP::Variant HPHP::f_socket_send(HPHP::Object const&, HPHP::String const&, int, int)
_ZN4HPHP13f_socket_sendERKNS_6ObjectERKNS_6StringEii

(return value) => rax
_rv => rdi
socket => rsi
buf => rdx
len => rcx
flags => r8
*/

TypedValue* fh_socket_send(TypedValue* _rv, Value* socket, Value* buf, int len, int flags) asm("_ZN4HPHP13f_socket_sendERKNS_6ObjectERKNS_6StringEii");

TypedValue * fg1_socket_send(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_send(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
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
  fh_socket_send((rv), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_send(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_socket_send((&(rv)), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_send(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_send", count, 4, 4, 1);
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
HPHP::Variant HPHP::f_socket_sendto(HPHP::Object const&, HPHP::String const&, int, int, HPHP::String const&, int)
_ZN4HPHP15f_socket_sendtoERKNS_6ObjectERKNS_6StringEiiS5_i

(return value) => rax
_rv => rdi
socket => rsi
buf => rdx
len => rcx
flags => r8
addr => r9
port => st0
*/

TypedValue* fh_socket_sendto(TypedValue* _rv, Value* socket, Value* buf, int len, int flags, Value* addr, int port) asm("_ZN4HPHP15f_socket_sendtoERKNS_6ObjectERKNS_6StringEiiS5_i");

TypedValue * fg1_socket_sendto(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_sendto(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    break;
  }
  if (!IS_STRING_TYPE((args-4)->m_type)) {
    tvCastToStringInPlace(args-4);
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
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
  fh_socket_sendto((rv), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (Value*)(args-4), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_sendto(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 5LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && IS_STRING_TYPE((args-4)->m_type) && (args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_socket_sendto((&(rv)), (Value*)(args-0), (Value*)(args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (Value*)(args-4), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_sendto(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_sendto", count, 5, 6, 1);
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
HPHP::Variant HPHP::f_socket_recv(HPHP::Object const&, HPHP::VRefParamValue const&, int, int)
_ZN4HPHP13f_socket_recvERKNS_6ObjectERKNS_14VRefParamValueEii

(return value) => rax
_rv => rdi
socket => rsi
buf => rdx
len => rcx
flags => r8
*/

TypedValue* fh_socket_recv(TypedValue* _rv, Value* socket, TypedValue* buf, int len, int flags) asm("_ZN4HPHP13f_socket_recvERKNS_6ObjectERKNS_14VRefParamValueEii");

TypedValue * fg1_socket_recv(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_recv(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_socket_recv((rv), (Value*)(args-0), (args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_recv(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 4LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_socket_recv((&(rv)), (Value*)(args-0), (args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_recv(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_recv", count, 4, 4, 1);
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
HPHP::Variant HPHP::f_socket_recvfrom(HPHP::Object const&, HPHP::VRefParamValue const&, int, int, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&)
_ZN4HPHP17f_socket_recvfromERKNS_6ObjectERKNS_14VRefParamValueEiiS5_S5_

(return value) => rax
_rv => rdi
socket => rsi
buf => rdx
len => rcx
flags => r8
name => r9
port => st0
*/

TypedValue* fh_socket_recvfrom(TypedValue* _rv, Value* socket, TypedValue* buf, int len, int flags, TypedValue* name, TypedValue* port) asm("_ZN4HPHP17f_socket_recvfromERKNS_6ObjectERKNS_14VRefParamValueEiiS5_S5_");

TypedValue * fg1_socket_recvfrom(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_recvfrom(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
  case 5:
    break;
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  VRefParamValue defVal5 = 0;
  fh_socket_recvfrom((rv), (Value*)(args-0), (args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (args-4), (count > 5) ? (args-5) : (TypedValue*)(&defVal5));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_recvfrom(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 5LL && count <= 6LL) {
      if ((args-3)->m_type == KindOfInt64 && (args-2)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        VRefParamValue defVal5 = 0;
        fh_socket_recvfrom((&(rv)), (Value*)(args-0), (args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (args-4), (count > 5) ? (args-5) : (TypedValue*)(&defVal5));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_recvfrom(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_recvfrom", count, 5, 6, 1);
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
bool HPHP::f_socket_shutdown(HPHP::Object const&, int)
_ZN4HPHP17f_socket_shutdownERKNS_6ObjectEi

(return value) => rax
socket => rdi
how => rsi
*/

bool fh_socket_shutdown(Value* socket, int how) asm("_ZN4HPHP17f_socket_shutdownERKNS_6ObjectEi");

TypedValue * fg1_socket_shutdown(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_shutdown(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  rv->m_data.num = (fh_socket_shutdown((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_socket_shutdown(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_socket_shutdown((Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_shutdown(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_shutdown", count, 1, 2, 1);
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
void HPHP::f_socket_close(HPHP::Object const&)
_ZN4HPHP14f_socket_closeERKNS_6ObjectE

socket => rdi
*/

void fh_socket_close(Value* socket) asm("_ZN4HPHP14f_socket_closeERKNS_6ObjectE");

TypedValue * fg1_socket_close(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_close(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_socket_close((Value*)(args-0));
  return rv;
}

TypedValue* fg_socket_close(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_socket_close((Value*)(args-0));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_close(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_close", count, 1, 1, 1);
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
HPHP::String HPHP::f_socket_strerror(int)
_ZN4HPHP17f_socket_strerrorEi

(return value) => rax
_rv => rdi
errnum => rsi
*/

Value* fh_socket_strerror(Value* _rv, int errnum) asm("_ZN4HPHP17f_socket_strerrorEi");

TypedValue * fg1_socket_strerror(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_strerror(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  fh_socket_strerror((Value*)(rv), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_socket_strerror(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfInt64) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_socket_strerror((Value*)(&(rv)), (int)(args[-0].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_strerror(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("socket_strerror", count, 1, 1, 1);
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
long long HPHP::f_socket_last_error(HPHP::Object const&)
_ZN4HPHP19f_socket_last_errorERKNS_6ObjectE

(return value) => rax
socket => rdi
*/

long long fh_socket_last_error(Value* socket) asm("_ZN4HPHP19f_socket_last_errorERKNS_6ObjectE");

TypedValue * fg1_socket_last_error(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_last_error(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (long long)fh_socket_last_error((count > 0) ? (Value*)(args-0) : (Value*)(&null_object));
  return rv;
}

TypedValue* fg_socket_last_error(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfObject)) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (long long)fh_socket_last_error((count > 0) ? (Value*)(args-0) : (Value*)(&null_object));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_last_error(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("socket_last_error", 1, 1);
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
void HPHP::f_socket_clear_error(HPHP::Object const&)
_ZN4HPHP20f_socket_clear_errorERKNS_6ObjectE

socket => rdi
*/

void fh_socket_clear_error(Value* socket) asm("_ZN4HPHP20f_socket_clear_errorERKNS_6ObjectE");

TypedValue * fg1_socket_clear_error(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_socket_clear_error(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToObjectInPlace(args-0);
  fh_socket_clear_error((count > 0) ? (Value*)(args-0) : (Value*)(&null_object));
  return rv;
}

TypedValue* fg_socket_clear_error(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfObject)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_socket_clear_error((count > 0) ? (Value*)(args-0) : (Value*)(&null_object));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_socket_clear_error(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("socket_clear_error", 1, 1);
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
HPHP::Variant HPHP::f_getaddrinfo(HPHP::String const&, HPHP::String const&, int, int, int, int)
_ZN4HPHP13f_getaddrinfoERKNS_6StringES2_iiii

(return value) => rax
_rv => rdi
host => rsi
port => rdx
family => rcx
socktype => r8
protocol => r9
flags => st0
*/

TypedValue* fh_getaddrinfo(TypedValue* _rv, Value* host, Value* port, int family, int socktype, int protocol, int flags) asm("_ZN4HPHP13f_getaddrinfoERKNS_6StringES2_iiii");

TypedValue * fg1_getaddrinfo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_getaddrinfo(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
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
  fh_getaddrinfo((rv), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_getaddrinfo(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_getaddrinfo((&(rv)), (Value*)(args-0), (Value*)(args-1), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_getaddrinfo(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("getaddrinfo", count, 2, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}




} // !HPHP

