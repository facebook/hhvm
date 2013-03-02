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
#include <exception>

namespace HPHP {

/*
bool HPHP::f_hphpd_install_user_command(HPHP::String const&, HPHP::String const&)
_ZN4HPHP28f_hphpd_install_user_commandERKNS_6StringES2_

(return value) => rax
cmd => rdi
clsname => rsi
*/

bool fh_hphpd_install_user_command(Value* cmd, Value* clsname) asm("_ZN4HPHP28f_hphpd_install_user_commandERKNS_6StringES2_");

TypedValue * fg1_hphpd_install_user_command(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphpd_install_user_command(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_hphpd_install_user_command((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_hphpd_install_user_command(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_hphpd_install_user_command((Value*)(args-0), (Value*)(args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphpd_install_user_command(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphpd_install_user_command", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_hphpd_get_user_commands()
_ZN4HPHP25f_hphpd_get_user_commandsEv

(return value) => rax
_rv => rdi
*/

Value* fh_hphpd_get_user_commands(Value* _rv) asm("_ZN4HPHP25f_hphpd_get_user_commandsEv");

TypedValue* fg_hphpd_get_user_commands(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv._count = 0;
      rv.m_type = KindOfArray;
      fh_hphpd_get_user_commands((Value*)(&(rv)));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("hphpd_get_user_commands", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_hphpd_break(bool)
_ZN4HPHP13f_hphpd_breakEb

condition => rdi
*/

void fh_hphpd_break(bool condition) asm("_ZN4HPHP13f_hphpd_breakEb");

TypedValue * fg1_hphpd_break(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphpd_break(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToBooleanInPlace(args-0);
  fh_hphpd_break((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
  return rv;
}

TypedValue* fg_hphpd_break(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfBoolean)) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        fh_hphpd_break((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphpd_break(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("hphpd_break", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphpd_get_client(HPHP::String const&)
_ZN4HPHP18f_hphpd_get_clientERKNS_6StringE

(return value) => rax
_rv => rdi
name => rsi
*/

TypedValue* fh_hphpd_get_client(TypedValue* _rv, Value* name) asm("_ZN4HPHP18f_hphpd_get_clientERKNS_6StringE");

TypedValue * fg1_hphpd_get_client(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphpd_get_client(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  String defVal0 = null;
  fh_hphpd_get_client((rv), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphpd_get_client(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        String defVal0 = null;
        fh_hphpd_get_client((&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&defVal0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphpd_get_client(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("hphpd_get_client", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_hphpd_client_ctrl(HPHP::String const&, HPHP::String const&)
_ZN4HPHP19f_hphpd_client_ctrlERKNS_6StringES2_

(return value) => rax
_rv => rdi
name => rsi
op => rdx
*/

TypedValue* fh_hphpd_client_ctrl(TypedValue* _rv, Value* name, Value* op) asm("_ZN4HPHP19f_hphpd_client_ctrlERKNS_6StringES2_");

TypedValue * fg1_hphpd_client_ctrl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_hphpd_client_ctrl(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_hphpd_client_ctrl((rv), (Value*)(args-0), (Value*)(args-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_hphpd_client_ctrl(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_hphpd_client_ctrl((&(rv)), (Value*)(args-0), (Value*)(args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_hphpd_client_ctrl(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("hphpd_client_ctrl", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



HPHP::VM::Instance* new_DebuggerProxyCmdUser_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DebuggerProxyCmdUser) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DebuggerProxyCmdUser(cls);
  return inst;
}

IMPLEMENT_CLASS(DebuggerProxyCmdUser);
/*
void HPHP::c_DebuggerProxyCmdUser::t___construct()
_ZN4HPHP22c_DebuggerProxyCmdUser13t___constructEv

this_ => rdi
*/

void th_20DebuggerProxyCmdUser___construct(ObjectData* this_) asm("_ZN4HPHP22c_DebuggerProxyCmdUser13t___constructEv");

TypedValue* tg_20DebuggerProxyCmdUser___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_20DebuggerProxyCmdUser___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerProxyCmdUser::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerProxyCmdUser::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DebuggerProxyCmdUser::t_islocal()
_ZN4HPHP22c_DebuggerProxyCmdUser9t_islocalEv

(return value) => rax
this_ => rdi
*/

bool th_20DebuggerProxyCmdUser_isLocal(ObjectData* this_) asm("_ZN4HPHP22c_DebuggerProxyCmdUser9t_islocalEv");

TypedValue* tg_20DebuggerProxyCmdUser_isLocal(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (th_20DebuggerProxyCmdUser_isLocal((this_))) ? 1LL : 0LL;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerProxyCmdUser::isLocal", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerProxyCmdUser::isLocal");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DebuggerProxyCmdUser::t_send(HPHP::Object const&)
_ZN4HPHP22c_DebuggerProxyCmdUser6t_sendERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
cmd => rdx
*/

TypedValue* th_20DebuggerProxyCmdUser_send(TypedValue* _rv, ObjectData* this_, Value* cmd) asm("_ZN4HPHP22c_DebuggerProxyCmdUser6t_sendERKNS_6ObjectE");

TypedValue* tg1_20DebuggerProxyCmdUser_send(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_20DebuggerProxyCmdUser_send(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_20DebuggerProxyCmdUser_send((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_20DebuggerProxyCmdUser_send(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          th_20DebuggerProxyCmdUser_send((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_20DebuggerProxyCmdUser_send(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerProxyCmdUser::send", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerProxyCmdUser::send");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DebuggerClientCmdUser_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DebuggerClientCmdUser) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DebuggerClientCmdUser(cls);
  return inst;
}

IMPLEMENT_CLASS(DebuggerClientCmdUser);
/*
void HPHP::c_DebuggerClientCmdUser::t___construct()
_ZN4HPHP23c_DebuggerClientCmdUser13t___constructEv

this_ => rdi
*/

void th_21DebuggerClientCmdUser___construct(ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser13t___constructEv");

TypedValue* tg_21DebuggerClientCmdUser___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_21DebuggerClientCmdUser___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_quit()
_ZN4HPHP23c_DebuggerClientCmdUser6t_quitEv

this_ => rdi
*/

void th_21DebuggerClientCmdUser_quit(ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_quitEv");

TypedValue* tg_21DebuggerClientCmdUser_quit(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_21DebuggerClientCmdUser_quit((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::quit", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::quit");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_print(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23c_DebuggerClientCmdUser7t_printEiRKNS_6StringERKNS_5ArrayE

this_ => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

void th_21DebuggerClientCmdUser_print(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser7t_printEiRKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_21DebuggerClientCmdUser_print(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_print(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int64_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_21DebuggerClientCmdUser_print((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_print(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          Array extraArgs;
          {
            ArrayInit ai(count-1);
            for (int64_t i = 1; i < count; ++i) {
              TypedValue* extraArg = ar->getExtraArg(i-1);
              if (tvIsStronglyBound(extraArg)) {
                ai.setRef(i-1, tvAsVariant(extraArg));
              } else {
                ai.set(i-1, tvAsVariant(extraArg));
              }
            }
            extraArgs = ai.create();
          }
          th_21DebuggerClientCmdUser_print((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_print(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("DebuggerClientCmdUser::print", count+1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::print");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_help(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23c_DebuggerClientCmdUser6t_helpEiRKNS_6StringERKNS_5ArrayE

this_ => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

void th_21DebuggerClientCmdUser_help(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_helpEiRKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_21DebuggerClientCmdUser_help(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_help(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int64_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_21DebuggerClientCmdUser_help((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_help(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          Array extraArgs;
          {
            ArrayInit ai(count-1);
            for (int64_t i = 1; i < count; ++i) {
              TypedValue* extraArg = ar->getExtraArg(i-1);
              if (tvIsStronglyBound(extraArg)) {
                ai.setRef(i-1, tvAsVariant(extraArg));
              } else {
                ai.set(i-1, tvAsVariant(extraArg));
              }
            }
            extraArgs = ai.create();
          }
          th_21DebuggerClientCmdUser_help((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_help(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("DebuggerClientCmdUser::help", count+1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::help");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_info(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23c_DebuggerClientCmdUser6t_infoEiRKNS_6StringERKNS_5ArrayE

this_ => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

void th_21DebuggerClientCmdUser_info(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_infoEiRKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_21DebuggerClientCmdUser_info(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_info(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int64_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_21DebuggerClientCmdUser_info((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_info(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          Array extraArgs;
          {
            ArrayInit ai(count-1);
            for (int64_t i = 1; i < count; ++i) {
              TypedValue* extraArg = ar->getExtraArg(i-1);
              if (tvIsStronglyBound(extraArg)) {
                ai.setRef(i-1, tvAsVariant(extraArg));
              } else {
                ai.set(i-1, tvAsVariant(extraArg));
              }
            }
            extraArgs = ai.create();
          }
          th_21DebuggerClientCmdUser_info((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_info(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("DebuggerClientCmdUser::info", count+1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::info");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_output(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23c_DebuggerClientCmdUser8t_outputEiRKNS_6StringERKNS_5ArrayE

this_ => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

void th_21DebuggerClientCmdUser_output(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser8t_outputEiRKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_21DebuggerClientCmdUser_output(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_output(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int64_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_21DebuggerClientCmdUser_output((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_output(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          Array extraArgs;
          {
            ArrayInit ai(count-1);
            for (int64_t i = 1; i < count; ++i) {
              TypedValue* extraArg = ar->getExtraArg(i-1);
              if (tvIsStronglyBound(extraArg)) {
                ai.setRef(i-1, tvAsVariant(extraArg));
              } else {
                ai.set(i-1, tvAsVariant(extraArg));
              }
            }
            extraArgs = ai.create();
          }
          th_21DebuggerClientCmdUser_output((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_output(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("DebuggerClientCmdUser::output", count+1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::output");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_error(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23c_DebuggerClientCmdUser7t_errorEiRKNS_6StringERKNS_5ArrayE

this_ => rdi
_argc => rsi
format => rdx
_argv => rcx
*/

void th_21DebuggerClientCmdUser_error(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser7t_errorEiRKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_21DebuggerClientCmdUser_error(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_error(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int64_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_21DebuggerClientCmdUser_error((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_error(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          Array extraArgs;
          {
            ArrayInit ai(count-1);
            for (int64_t i = 1; i < count; ++i) {
              TypedValue* extraArg = ar->getExtraArg(i-1);
              if (tvIsStronglyBound(extraArg)) {
                ai.setRef(i-1, tvAsVariant(extraArg));
              } else {
                ai.set(i-1, tvAsVariant(extraArg));
              }
            }
            extraArgs = ai.create();
          }
          th_21DebuggerClientCmdUser_error((this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_error(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("DebuggerClientCmdUser::error", count+1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::error");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_code(HPHP::String const&, int, int, int)
_ZN4HPHP23c_DebuggerClientCmdUser6t_codeERKNS_6StringEiii

this_ => rdi
source => rsi
highlight_line => rdx
start_line_no => rcx
end_line_no => r8
*/

void th_21DebuggerClientCmdUser_code(ObjectData* this_, Value* source, int highlight_line, int start_line_no, int end_line_no) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_codeERKNS_6StringEiii");

TypedValue* tg1_21DebuggerClientCmdUser_code(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_code(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
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
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  th_21DebuggerClientCmdUser_code((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_code(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL && count <= 4LL) {
        if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_21DebuggerClientCmdUser_code((this_), (Value*)(args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_code(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 4);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::code", count, 1, 4, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::code");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DebuggerClientCmdUser::t_ask(int, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23c_DebuggerClientCmdUser5t_askEiRKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
this_ => rsi
_argc => rdx
format => rcx
_argv => r8
*/

TypedValue* th_21DebuggerClientCmdUser_ask(TypedValue* _rv, ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser5t_askEiRKNS_6StringERKNS_5ArrayE");

TypedValue* tg1_21DebuggerClientCmdUser_ask(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_ask(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Array extraArgs;
  {
    ArrayInit ai(count-1);
    for (int64_t i = 1; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-1);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-1, tvAsVariant(extraArg));
      } else {
        ai.set(i-1, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_21DebuggerClientCmdUser_ask((rv), (this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_ask(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          Array extraArgs;
          {
            ArrayInit ai(count-1);
            for (int64_t i = 1; i < count; ++i) {
              TypedValue* extraArg = ar->getExtraArg(i-1);
              if (tvIsStronglyBound(extraArg)) {
                ai.setRef(i-1, tvAsVariant(extraArg));
              } else {
                ai.set(i-1, tvAsVariant(extraArg));
              }
            }
            extraArgs = ai.create();
          }
          th_21DebuggerClientCmdUser_ask((&(rv)), (this_), (count), (Value*)(args-0), (Value*)(&extraArgs));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_ask(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("DebuggerClientCmdUser::ask", count+1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::ask");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_DebuggerClientCmdUser::t_wrap(HPHP::String const&)
_ZN4HPHP23c_DebuggerClientCmdUser6t_wrapERKNS_6StringE

(return value) => rax
_rv => rdi
this_ => rsi
str => rdx
*/

Value* th_21DebuggerClientCmdUser_wrap(Value* _rv, ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_wrapERKNS_6StringE");

TypedValue* tg1_21DebuggerClientCmdUser_wrap(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_wrap(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  th_21DebuggerClientCmdUser_wrap((Value*)(rv), (this_), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_wrap(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_21DebuggerClientCmdUser_wrap((Value*)(&(rv)), (this_), (Value*)(args-0));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_wrap(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::wrap", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::wrap");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_helptitle(HPHP::String const&)
_ZN4HPHP23c_DebuggerClientCmdUser11t_helptitleERKNS_6StringE

this_ => rdi
str => rsi
*/

void th_21DebuggerClientCmdUser_helpTitle(ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser11t_helptitleERKNS_6StringE");

TypedValue* tg1_21DebuggerClientCmdUser_helpTitle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_helpTitle(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_21DebuggerClientCmdUser_helpTitle((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_helpTitle(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_21DebuggerClientCmdUser_helpTitle((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_helpTitle(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::helpTitle", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::helpTitle");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_helpcmds(int, HPHP::String const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP23c_DebuggerClientCmdUser10t_helpcmdsEiRKNS_6StringES3_RKNS_5ArrayE

this_ => rdi
_argc => rsi
cmd => rdx
desc => rcx
_argv => r8
*/

void th_21DebuggerClientCmdUser_helpCmds(ObjectData* this_, int64_t _argc, Value* cmd, Value* desc, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_helpcmdsEiRKNS_6StringES3_RKNS_5ArrayE");

TypedValue* tg1_21DebuggerClientCmdUser_helpCmds(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_helpCmds(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Array extraArgs;
  {
    ArrayInit ai(count-2);
    for (int64_t i = 2; i < count; ++i) {
      TypedValue* extraArg = ar->getExtraArg(i-2);
      if (tvIsStronglyBound(extraArg)) {
        ai.setRef(i-2, tvAsVariant(extraArg));
      } else {
        ai.set(i-2, tvAsVariant(extraArg));
      }
    }
    extraArgs = ai.create();
  }
  th_21DebuggerClientCmdUser_helpCmds((this_), (count), (Value*)(args-0), (Value*)(args-1), (Value*)(&extraArgs));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_helpCmds(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count >= 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          Array extraArgs;
          {
            ArrayInit ai(count-2);
            for (int64_t i = 2; i < count; ++i) {
              TypedValue* extraArg = ar->getExtraArg(i-2);
              if (tvIsStronglyBound(extraArg)) {
                ai.setRef(i-2, tvAsVariant(extraArg));
              } else {
                ai.set(i-2, tvAsVariant(extraArg));
              }
            }
            extraArgs = ai.create();
          }
          th_21DebuggerClientCmdUser_helpCmds((this_), (count), (Value*)(args-0), (Value*)(args-1), (Value*)(&extraArgs));
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_helpCmds(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_missing_arguments_nr("DebuggerClientCmdUser::helpCmds", count+1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::helpCmds");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_helpbody(HPHP::String const&)
_ZN4HPHP23c_DebuggerClientCmdUser10t_helpbodyERKNS_6StringE

this_ => rdi
str => rsi
*/

void th_21DebuggerClientCmdUser_helpBody(ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_helpbodyERKNS_6StringE");

TypedValue* tg1_21DebuggerClientCmdUser_helpBody(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_helpBody(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_21DebuggerClientCmdUser_helpBody((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_helpBody(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_21DebuggerClientCmdUser_helpBody((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_helpBody(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::helpBody", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::helpBody");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_helpsection(HPHP::String const&)
_ZN4HPHP23c_DebuggerClientCmdUser13t_helpsectionERKNS_6StringE

this_ => rdi
str => rsi
*/

void th_21DebuggerClientCmdUser_helpSection(ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser13t_helpsectionERKNS_6StringE");

TypedValue* tg1_21DebuggerClientCmdUser_helpSection(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_helpSection(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_21DebuggerClientCmdUser_helpSection((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_helpSection(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_21DebuggerClientCmdUser_helpSection((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_helpSection(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::helpSection", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::helpSection");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_tutorial(HPHP::String const&)
_ZN4HPHP23c_DebuggerClientCmdUser10t_tutorialERKNS_6StringE

this_ => rdi
str => rsi
*/

void th_21DebuggerClientCmdUser_tutorial(ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_tutorialERKNS_6StringE");

TypedValue* tg1_21DebuggerClientCmdUser_tutorial(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_tutorial(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToStringInPlace(args-0);
  th_21DebuggerClientCmdUser_tutorial((this_), (Value*)(args-0));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_tutorial(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if (IS_STRING_TYPE((args-0)->m_type)) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_21DebuggerClientCmdUser_tutorial((this_), (Value*)(args-0));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_tutorial(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::tutorial", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::tutorial");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_DebuggerClientCmdUser::t_getcode()
_ZN4HPHP23c_DebuggerClientCmdUser9t_getcodeEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_21DebuggerClientCmdUser_getCode(Value* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser9t_getcodeEv");

TypedValue* tg_21DebuggerClientCmdUser_getCode(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_21DebuggerClientCmdUser_getCode((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::getCode", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::getCode");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_DebuggerClientCmdUser::t_getcommand()
_ZN4HPHP23c_DebuggerClientCmdUser12t_getcommandEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_21DebuggerClientCmdUser_getCommand(Value* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser12t_getcommandEv");

TypedValue* tg_21DebuggerClientCmdUser_getCommand(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfString;
        th_21DebuggerClientCmdUser_getCommand((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::getCommand", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::getCommand");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
bool HPHP::c_DebuggerClientCmdUser::t_arg(int, HPHP::String const&)
_ZN4HPHP23c_DebuggerClientCmdUser5t_argEiRKNS_6StringE

(return value) => rax
this_ => rdi
index => rsi
str => rdx
*/

bool th_21DebuggerClientCmdUser_arg(ObjectData* this_, int index, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser5t_argEiRKNS_6StringE");

TypedValue* tg1_21DebuggerClientCmdUser_arg(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_arg(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_data.num = (th_21DebuggerClientCmdUser_arg((this_), (int)(args[-0].m_data.num), (Value*)(args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_arg(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfBoolean;
          rv.m_data.num = (th_21DebuggerClientCmdUser_arg((this_), (int)(args[-0].m_data.num), (Value*)(args-1))) ? 1LL : 0LL;
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_arg(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 2);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::arg", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::arg");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_DebuggerClientCmdUser::t_argcount()
_ZN4HPHP23c_DebuggerClientCmdUser10t_argcountEv

(return value) => rax
this_ => rdi
*/

long th_21DebuggerClientCmdUser_argCount(ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_argcountEv");

TypedValue* tg_21DebuggerClientCmdUser_argCount(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_21DebuggerClientCmdUser_argCount((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::argCount", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::argCount");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_DebuggerClientCmdUser::t_argvalue(int)
_ZN4HPHP23c_DebuggerClientCmdUser10t_argvalueEi

(return value) => rax
_rv => rdi
this_ => rsi
index => rdx
*/

Value* th_21DebuggerClientCmdUser_argValue(Value* _rv, ObjectData* this_, int index) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_argvalueEi");

TypedValue* tg1_21DebuggerClientCmdUser_argValue(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_argValue(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  th_21DebuggerClientCmdUser_argValue((Value*)(rv), (this_), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_argValue(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_21DebuggerClientCmdUser_argValue((Value*)(&(rv)), (this_), (int)(args[-0].m_data.num));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_argValue(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::argValue", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::argValue");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::String HPHP::c_DebuggerClientCmdUser::t_linerest(int)
_ZN4HPHP23c_DebuggerClientCmdUser10t_linerestEi

(return value) => rax
_rv => rdi
this_ => rsi
index => rdx
*/

Value* th_21DebuggerClientCmdUser_lineRest(Value* _rv, ObjectData* this_, int index) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_linerestEi");

TypedValue* tg1_21DebuggerClientCmdUser_lineRest(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_lineRest(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToInt64InPlace(args-0);
  th_21DebuggerClientCmdUser_lineRest((Value*)(rv), (this_), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_lineRest(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv._count = 0;
          rv.m_type = KindOfString;
          th_21DebuggerClientCmdUser_lineRest((Value*)(&(rv)), (this_), (int)(args[-0].m_data.num));
          if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_lineRest(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::lineRest", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::lineRest");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Array HPHP::c_DebuggerClientCmdUser::t_args()
_ZN4HPHP23c_DebuggerClientCmdUser6t_argsEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

Value* th_21DebuggerClientCmdUser_args(Value* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_argsEv");

TypedValue* tg_21DebuggerClientCmdUser_args(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfArray;
        th_21DebuggerClientCmdUser_args((Value*)(&(rv)), (this_));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::args", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::args");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DebuggerClientCmdUser::t_send(HPHP::Object const&)
_ZN4HPHP23c_DebuggerClientCmdUser6t_sendERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
cmd => rdx
*/

TypedValue* th_21DebuggerClientCmdUser_send(TypedValue* _rv, ObjectData* this_, Value* cmd) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_sendERKNS_6ObjectE");

TypedValue* tg1_21DebuggerClientCmdUser_send(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_send(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_21DebuggerClientCmdUser_send((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_send(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          th_21DebuggerClientCmdUser_send((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_send(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::send", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::send");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DebuggerClientCmdUser::t_xend(HPHP::Object const&)
_ZN4HPHP23c_DebuggerClientCmdUser6t_xendERKNS_6ObjectE

(return value) => rax
_rv => rdi
this_ => rsi
cmd => rdx
*/

TypedValue* th_21DebuggerClientCmdUser_xend(TypedValue* _rv, ObjectData* this_, Value* cmd) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_xendERKNS_6ObjectE");

TypedValue* tg1_21DebuggerClientCmdUser_xend(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_xend(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_21DebuggerClientCmdUser_xend((rv), (this_), (Value*)(args-0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_xend(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfObject) {
          th_21DebuggerClientCmdUser_xend((&(rv)), (this_), (Value*)(args-0));
          if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_xend(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::xend", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::xend");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DebuggerClientCmdUser::t_getcurrentlocation()
_ZN4HPHP23c_DebuggerClientCmdUser20t_getcurrentlocationEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_21DebuggerClientCmdUser_getCurrentLocation(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser20t_getcurrentlocationEv");

TypedValue* tg_21DebuggerClientCmdUser_getCurrentLocation(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_21DebuggerClientCmdUser_getCurrentLocation((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::getCurrentLocation", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::getCurrentLocation");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DebuggerClientCmdUser::t_getstacktrace()
_ZN4HPHP23c_DebuggerClientCmdUser15t_getstacktraceEv

(return value) => rax
_rv => rdi
this_ => rsi
*/

TypedValue* th_21DebuggerClientCmdUser_getStackTrace(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser15t_getstacktraceEv");

TypedValue* tg_21DebuggerClientCmdUser_getStackTrace(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        th_21DebuggerClientCmdUser_getStackTrace((&(rv)), (this_));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::getStackTrace", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::getStackTrace");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_DebuggerClientCmdUser::t_getframe()
_ZN4HPHP23c_DebuggerClientCmdUser10t_getframeEv

(return value) => rax
this_ => rdi
*/

long th_21DebuggerClientCmdUser_getFrame(ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_getframeEv");

TypedValue* tg_21DebuggerClientCmdUser_getFrame(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_21DebuggerClientCmdUser_getFrame((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClientCmdUser::getFrame", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::getFrame");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_printframe(int)
_ZN4HPHP23c_DebuggerClientCmdUser12t_printframeEi

this_ => rdi
index => rsi
*/

void th_21DebuggerClientCmdUser_printFrame(ObjectData* this_, int index) asm("_ZN4HPHP23c_DebuggerClientCmdUser12t_printframeEi");

TypedValue* tg1_21DebuggerClientCmdUser_printFrame(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) __attribute__((noinline,cold));
TypedValue* tg1_21DebuggerClientCmdUser_printFrame(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->_count = 0;
  rv->m_type = KindOfNull;
  tvCastToInt64InPlace(args-0);
  th_21DebuggerClientCmdUser_printFrame((this_), (int)(args[-0].m_data.num));
  return rv;
}

TypedValue* tg_21DebuggerClientCmdUser_printFrame(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        if ((args-0)->m_type == KindOfInt64) {
          rv.m_data.num = 0LL;
          rv._count = 0;
          rv.m_type = KindOfNull;
          th_21DebuggerClientCmdUser_printFrame((this_), (int)(args[-0].m_data.num));
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        } else {
          tg1_21DebuggerClientCmdUser_printFrame(&rv, ar, count , this_);
          frame_free_locals_inl(ar, 1);
          memcpy(&ar->m_r, &rv, sizeof(TypedValue));
          return &ar->m_r;
        }
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::printFrame", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::printFrame");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
void HPHP::c_DebuggerClientCmdUser::t_addcompletion(HPHP::Variant const&)
_ZN4HPHP23c_DebuggerClientCmdUser15t_addcompletionERKNS_7VariantE

this_ => rdi
list => rsi
*/

void th_21DebuggerClientCmdUser_addCompletion(ObjectData* this_, TypedValue* list) asm("_ZN4HPHP23c_DebuggerClientCmdUser15t_addcompletionERKNS_7VariantE");

TypedValue* tg_21DebuggerClientCmdUser_addCompletion(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_21DebuggerClientCmdUser_addCompletion((this_), (args-0));
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DebuggerClientCmdUser::addCompletion", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClientCmdUser::addCompletion");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

HPHP::VM::Instance* new_DebuggerClient_Instance(HPHP::VM::Class* cls) {
  size_t nProps = cls->numDeclProperties();
  size_t builtinPropSize = sizeof(c_DebuggerClient) - sizeof(ObjectData);
  size_t size = HPHP::VM::Instance::sizeForNProps(nProps) + builtinPropSize;
  HPHP::VM::Instance *inst = (HPHP::VM::Instance*)ALLOCOBJSZ(size);
  new ((void *)inst) c_DebuggerClient(cls);
  return inst;
}

IMPLEMENT_CLASS_NO_DEFAULT_SWEEP(DebuggerClient);
/*
void HPHP::c_DebuggerClient::t___construct()
_ZN4HPHP16c_DebuggerClient13t___constructEv

this_ => rdi
*/

void th_14DebuggerClient___construct(ObjectData* this_) asm("_ZN4HPHP16c_DebuggerClient13t___constructEv");

TypedValue* tg_14DebuggerClient___construct(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv.m_data.num = 0LL;
        rv._count = 0;
        rv.m_type = KindOfNull;
        th_14DebuggerClient___construct((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClient::__construct", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClient::__construct");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
long HPHP::c_DebuggerClient::t_getstate()
_ZN4HPHP16c_DebuggerClient10t_getstateEv

(return value) => rax
this_ => rdi
*/

long th_14DebuggerClient_getState(ObjectData* this_) asm("_ZN4HPHP16c_DebuggerClient10t_getstateEv");

TypedValue* tg_14DebuggerClient_getState(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 0LL) {
        rv._count = 0;
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)th_14DebuggerClient_getState((this_));
        frame_free_locals_inl(ar, 0);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_toomany_arguments_nr("DebuggerClient::getState", 0, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClient::getState");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DebuggerClient::t_init(HPHP::Variant const&)
_ZN4HPHP16c_DebuggerClient6t_initERKNS_7VariantE

(return value) => rax
_rv => rdi
this_ => rsi
options => rdx
*/

TypedValue* th_14DebuggerClient_init(TypedValue* _rv, ObjectData* this_, TypedValue* options) asm("_ZN4HPHP16c_DebuggerClient6t_initERKNS_7VariantE");

TypedValue* tg_14DebuggerClient_init(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 1LL) {
        th_14DebuggerClient_init((&(rv)), (this_), (args-0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DebuggerClient::init", count, 1, 1, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClient::init");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}

/*
HPHP::Variant HPHP::c_DebuggerClient::t_processcmd(HPHP::Variant const&, HPHP::Variant const&)
_ZN4HPHP16c_DebuggerClient12t_processcmdERKNS_7VariantES3_

(return value) => rax
_rv => rdi
this_ => rsi
cmdName => rdx
args => rcx
*/

TypedValue* th_14DebuggerClient_processCmd(TypedValue* _rv, ObjectData* this_, TypedValue* cmdName, TypedValue* args) asm("_ZN4HPHP16c_DebuggerClient12t_processcmdERKNS_7VariantES3_");

TypedValue* tg_14DebuggerClient_processCmd(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    ObjectData* this_ = (ar->hasThis() ? ar->getThis() : NULL);
    if (this_) {
      if (count == 2LL) {
        th_14DebuggerClient_processCmd((&(rv)), (this_), (args-0), (args-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        throw_wrong_arguments_nr("DebuggerClient::processCmd", count, 2, 2, 1);
      }
    } else {
      throw_instance_method_fatal("DebuggerClient::processCmd");
    }
    rv.m_data.num = 0LL;
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}


} // !HPHP

