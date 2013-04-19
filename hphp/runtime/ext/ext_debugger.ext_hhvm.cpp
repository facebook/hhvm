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

bool fh_hphpd_install_user_command(Value* cmd, Value* clsname) asm("_ZN4HPHP28f_hphpd_install_user_commandERKNS_6StringES2_");

void fg1_hphpd_install_user_command(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphpd_install_user_command(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_hphpd_install_user_command(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_hphpd_install_user_command(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_hphpd_install_user_command(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_hphpd_install_user_command(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphpd_install_user_command", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_hphpd_get_user_commands(Value* _rv) asm("_ZN4HPHP25f_hphpd_get_user_commandsEv");

TypedValue* fg_hphpd_get_user_commands(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_hphpd_get_user_commands(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("hphpd_get_user_commands", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_hphpd_break(bool condition) asm("_ZN4HPHP13f_hphpd_breakEb");

void fg1_hphpd_break(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphpd_break(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_hphpd_break((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
}

TypedValue* fg_hphpd_break(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfNull;
      fh_hphpd_break((count > 0) ? (bool)(args[-0].m_data.num) : (bool)(true));
    } else {
      fg1_hphpd_break(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("hphpd_break", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphpd_get_client(TypedValue* _rv, Value* name) asm("_ZN4HPHP18f_hphpd_get_clientERKNS_6StringE");

void fg1_hphpd_get_client(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphpd_get_client(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  String defVal0 = uninit_null();
  fh_hphpd_get_client(rv, (count > 0) ? &args[-0].m_data : (Value*)(&defVal0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphpd_get_client(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      String defVal0 = uninit_null();
      fh_hphpd_get_client(rv, (count > 0) ? &args[-0].m_data : (Value*)(&defVal0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphpd_get_client(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("hphpd_get_client", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_hphpd_client_ctrl(TypedValue* _rv, Value* name, Value* op) asm("_ZN4HPHP19f_hphpd_client_ctrlERKNS_6StringES2_");

void fg1_hphpd_client_ctrl(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_hphpd_client_ctrl(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_hphpd_client_ctrl(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_hphpd_client_ctrl(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_hphpd_client_ctrl(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_hphpd_client_ctrl(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("hphpd_client_ctrl", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
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
void th_20DebuggerProxyCmdUser___construct(ObjectData* this_) asm("_ZN4HPHP22c_DebuggerProxyCmdUser13t___constructEv");

TypedValue* tg_20DebuggerProxyCmdUser___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_20DebuggerProxyCmdUser___construct((this_));
    } else {
      throw_toomany_arguments_nr("DebuggerProxyCmdUser::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerProxyCmdUser::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_20DebuggerProxyCmdUser_isLocal(ObjectData* this_) asm("_ZN4HPHP22c_DebuggerProxyCmdUser9t_islocalEv");

TypedValue* tg_20DebuggerProxyCmdUser_isLocal(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (th_20DebuggerProxyCmdUser_isLocal((this_))) ? 1LL : 0LL;
    } else {
      throw_toomany_arguments_nr("DebuggerProxyCmdUser::isLocal", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerProxyCmdUser::isLocal");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_20DebuggerProxyCmdUser_send(TypedValue* _rv, ObjectData* this_, Value* cmd) asm("_ZN4HPHP22c_DebuggerProxyCmdUser6t_sendERKNS_6ObjectE");

void tg1_20DebuggerProxyCmdUser_send(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_20DebuggerProxyCmdUser_send(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_20DebuggerProxyCmdUser_send(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_20DebuggerProxyCmdUser_send(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfObject) {
        th_20DebuggerProxyCmdUser_send(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_20DebuggerProxyCmdUser_send(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerProxyCmdUser::send", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerProxyCmdUser::send");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
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
void th_21DebuggerClientCmdUser___construct(ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser13t___constructEv");

TypedValue* tg_21DebuggerClientCmdUser___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_21DebuggerClientCmdUser___construct((this_));
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_quit(ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_quitEv");

TypedValue* tg_21DebuggerClientCmdUser_quit(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_21DebuggerClientCmdUser_quit((this_));
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::quit", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::quit");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_print(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser7t_printEiRKNS_6StringERKNS_5ArrayE");

void tg1_21DebuggerClientCmdUser_print(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_print(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;

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
  th_21DebuggerClientCmdUser_print((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
}

TypedValue* tg_21DebuggerClientCmdUser_print(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;

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
        th_21DebuggerClientCmdUser_print((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
      } else {
        tg1_21DebuggerClientCmdUser_print(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("DebuggerClientCmdUser::print", 1, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::print");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_help(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_helpEiRKNS_6StringERKNS_5ArrayE");

void tg1_21DebuggerClientCmdUser_help(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_help(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;

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
  th_21DebuggerClientCmdUser_help((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
}

TypedValue* tg_21DebuggerClientCmdUser_help(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;

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
        th_21DebuggerClientCmdUser_help((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
      } else {
        tg1_21DebuggerClientCmdUser_help(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("DebuggerClientCmdUser::help", 1, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::help");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_info(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_infoEiRKNS_6StringERKNS_5ArrayE");

void tg1_21DebuggerClientCmdUser_info(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_info(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;

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
  th_21DebuggerClientCmdUser_info((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
}

TypedValue* tg_21DebuggerClientCmdUser_info(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;

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
        th_21DebuggerClientCmdUser_info((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
      } else {
        tg1_21DebuggerClientCmdUser_info(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("DebuggerClientCmdUser::info", 1, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::info");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_output(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser8t_outputEiRKNS_6StringERKNS_5ArrayE");

void tg1_21DebuggerClientCmdUser_output(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_output(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;

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
  th_21DebuggerClientCmdUser_output((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
}

TypedValue* tg_21DebuggerClientCmdUser_output(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;

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
        th_21DebuggerClientCmdUser_output((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
      } else {
        tg1_21DebuggerClientCmdUser_output(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("DebuggerClientCmdUser::output", 1, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::output");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_error(ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser7t_errorEiRKNS_6StringERKNS_5ArrayE");

void tg1_21DebuggerClientCmdUser_error(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_error(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;

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
  th_21DebuggerClientCmdUser_error((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
}

TypedValue* tg_21DebuggerClientCmdUser_error(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;

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
        th_21DebuggerClientCmdUser_error((this_), count, &args[-0].m_data, (Value*)(&extraArgs));
      } else {
        tg1_21DebuggerClientCmdUser_error(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("DebuggerClientCmdUser::error", 1, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::error");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_code(ObjectData* this_, Value* source, int highlight_line, int start_line_no, int end_line_no) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_codeERKNS_6StringEiii");

void tg1_21DebuggerClientCmdUser_code(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_code(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
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
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  th_21DebuggerClientCmdUser_code((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
}

TypedValue* tg_21DebuggerClientCmdUser_code(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1 && count <= 4) {
      if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
          (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
          (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_21DebuggerClientCmdUser_code((this_), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
      } else {
        tg1_21DebuggerClientCmdUser_code(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::code", count, 1, 4, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::code");
  }
  frame_free_locals_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_21DebuggerClientCmdUser_ask(TypedValue* _rv, ObjectData* this_, int64_t _argc, Value* format, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser5t_askEiRKNS_6StringERKNS_5ArrayE");

void tg1_21DebuggerClientCmdUser_ask(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_ask(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);

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
  th_21DebuggerClientCmdUser_ask(rv, (this_), count, &args[-0].m_data, (Value*)(&extraArgs));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_21DebuggerClientCmdUser_ask(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {

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
        th_21DebuggerClientCmdUser_ask(rv, (this_), count, &args[-0].m_data, (Value*)(&extraArgs));
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_21DebuggerClientCmdUser_ask(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("DebuggerClientCmdUser::ask", 1, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::ask");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_21DebuggerClientCmdUser_wrap(Value* _rv, ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_wrapERKNS_6StringE");

void tg1_21DebuggerClientCmdUser_wrap(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_wrap(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  th_21DebuggerClientCmdUser_wrap(&(rv->m_data), (this_), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_21DebuggerClientCmdUser_wrap(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfString;
        th_21DebuggerClientCmdUser_wrap(&(rv->m_data), (this_), &args[-0].m_data);
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_21DebuggerClientCmdUser_wrap(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::wrap", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::wrap");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_helpTitle(ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser11t_helptitleERKNS_6StringE");

void tg1_21DebuggerClientCmdUser_helpTitle(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_helpTitle(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_21DebuggerClientCmdUser_helpTitle((this_), &args[-0].m_data);
}

TypedValue* tg_21DebuggerClientCmdUser_helpTitle(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_21DebuggerClientCmdUser_helpTitle((this_), &args[-0].m_data);
      } else {
        tg1_21DebuggerClientCmdUser_helpTitle(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::helpTitle", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::helpTitle");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_helpCmds(ObjectData* this_, int64_t _argc, Value* cmd, Value* desc, Value* _argv) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_helpcmdsEiRKNS_6StringES3_RKNS_5ArrayE");

void tg1_21DebuggerClientCmdUser_helpCmds(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_helpCmds(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfNull;

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
  th_21DebuggerClientCmdUser_helpCmds((this_), count, &args[-0].m_data, &args[-1].m_data, (Value*)(&extraArgs));
}

TypedValue* tg_21DebuggerClientCmdUser_helpCmds(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count >= 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;

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
        th_21DebuggerClientCmdUser_helpCmds((this_), count, &args[-0].m_data, &args[-1].m_data, (Value*)(&extraArgs));
      } else {
        tg1_21DebuggerClientCmdUser_helpCmds(rv, ar, count, this_);
      }
    } else {
      throw_missing_arguments_nr("DebuggerClientCmdUser::helpCmds", 2, count, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::helpCmds");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_helpBody(ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_helpbodyERKNS_6StringE");

void tg1_21DebuggerClientCmdUser_helpBody(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_helpBody(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_21DebuggerClientCmdUser_helpBody((this_), &args[-0].m_data);
}

TypedValue* tg_21DebuggerClientCmdUser_helpBody(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_21DebuggerClientCmdUser_helpBody((this_), &args[-0].m_data);
      } else {
        tg1_21DebuggerClientCmdUser_helpBody(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::helpBody", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::helpBody");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_helpSection(ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser13t_helpsectionERKNS_6StringE");

void tg1_21DebuggerClientCmdUser_helpSection(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_helpSection(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_21DebuggerClientCmdUser_helpSection((this_), &args[-0].m_data);
}

TypedValue* tg_21DebuggerClientCmdUser_helpSection(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_21DebuggerClientCmdUser_helpSection((this_), &args[-0].m_data);
      } else {
        tg1_21DebuggerClientCmdUser_helpSection(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::helpSection", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::helpSection");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_tutorial(ObjectData* this_, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_tutorialERKNS_6StringE");

void tg1_21DebuggerClientCmdUser_tutorial(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_tutorial(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  th_21DebuggerClientCmdUser_tutorial((this_), &args[-0].m_data);
}

TypedValue* tg_21DebuggerClientCmdUser_tutorial(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if (IS_STRING_TYPE((args - 0)->m_type)) {
        rv->m_type = KindOfNull;
        th_21DebuggerClientCmdUser_tutorial((this_), &args[-0].m_data);
      } else {
        tg1_21DebuggerClientCmdUser_tutorial(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::tutorial", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::tutorial");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_21DebuggerClientCmdUser_getCode(Value* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser9t_getcodeEv");

TypedValue* tg_21DebuggerClientCmdUser_getCode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_21DebuggerClientCmdUser_getCode(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::getCode", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::getCode");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_21DebuggerClientCmdUser_getCommand(Value* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser12t_getcommandEv");

TypedValue* tg_21DebuggerClientCmdUser_getCommand(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfString;
      th_21DebuggerClientCmdUser_getCommand(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::getCommand", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::getCommand");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool th_21DebuggerClientCmdUser_arg(ObjectData* this_, int index, Value* str) asm("_ZN4HPHP23c_DebuggerClientCmdUser5t_argEiRKNS_6StringE");

void tg1_21DebuggerClientCmdUser_arg(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_arg(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (th_21DebuggerClientCmdUser_arg((this_), (int)(args[-0].m_data.num), &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* tg_21DebuggerClientCmdUser_arg(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      if (IS_STRING_TYPE((args - 1)->m_type) &&
          (args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfBoolean;
        rv->m_data.num = (th_21DebuggerClientCmdUser_arg((this_), (int)(args[-0].m_data.num), &args[-1].m_data)) ? 1LL : 0LL;
      } else {
        tg1_21DebuggerClientCmdUser_arg(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::arg", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::arg");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_21DebuggerClientCmdUser_argCount(ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_argcountEv");

TypedValue* tg_21DebuggerClientCmdUser_argCount(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_21DebuggerClientCmdUser_argCount((this_));
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::argCount", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::argCount");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_21DebuggerClientCmdUser_argValue(Value* _rv, ObjectData* this_, int index) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_argvalueEi");

void tg1_21DebuggerClientCmdUser_argValue(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_argValue(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  th_21DebuggerClientCmdUser_argValue(&(rv->m_data), (this_), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_21DebuggerClientCmdUser_argValue(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfString;
        th_21DebuggerClientCmdUser_argValue(&(rv->m_data), (this_), (int)(args[-0].m_data.num));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_21DebuggerClientCmdUser_argValue(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::argValue", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::argValue");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_21DebuggerClientCmdUser_lineRest(Value* _rv, ObjectData* this_, int index) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_linerestEi");

void tg1_21DebuggerClientCmdUser_lineRest(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_lineRest(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  th_21DebuggerClientCmdUser_lineRest(&(rv->m_data), (this_), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* tg_21DebuggerClientCmdUser_lineRest(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfString;
        th_21DebuggerClientCmdUser_lineRest(&(rv->m_data), (this_), (int)(args[-0].m_data.num));
        if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
      } else {
        tg1_21DebuggerClientCmdUser_lineRest(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::lineRest", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::lineRest");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* th_21DebuggerClientCmdUser_args(Value* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_argsEv");

TypedValue* tg_21DebuggerClientCmdUser_args(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfArray;
      th_21DebuggerClientCmdUser_args(&(rv->m_data), (this_));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::args", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::args");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_21DebuggerClientCmdUser_send(TypedValue* _rv, ObjectData* this_, Value* cmd) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_sendERKNS_6ObjectE");

void tg1_21DebuggerClientCmdUser_send(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_send(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_21DebuggerClientCmdUser_send(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_21DebuggerClientCmdUser_send(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfObject) {
        th_21DebuggerClientCmdUser_send(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_21DebuggerClientCmdUser_send(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::send", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::send");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_21DebuggerClientCmdUser_xend(TypedValue* _rv, ObjectData* this_, Value* cmd) asm("_ZN4HPHP23c_DebuggerClientCmdUser6t_xendERKNS_6ObjectE");

void tg1_21DebuggerClientCmdUser_xend(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_xend(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  th_21DebuggerClientCmdUser_xend(rv, (this_), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* tg_21DebuggerClientCmdUser_xend(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfObject) {
        th_21DebuggerClientCmdUser_xend(rv, (this_), &args[-0].m_data);
        if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
      } else {
        tg1_21DebuggerClientCmdUser_xend(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::xend", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::xend");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_21DebuggerClientCmdUser_getCurrentLocation(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser20t_getcurrentlocationEv");

TypedValue* tg_21DebuggerClientCmdUser_getCurrentLocation(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_21DebuggerClientCmdUser_getCurrentLocation(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::getCurrentLocation", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::getCurrentLocation");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_21DebuggerClientCmdUser_getStackTrace(TypedValue* _rv, ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser15t_getstacktraceEv");

TypedValue* tg_21DebuggerClientCmdUser_getStackTrace(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      th_21DebuggerClientCmdUser_getStackTrace(rv, (this_));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::getStackTrace", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::getStackTrace");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_21DebuggerClientCmdUser_getFrame(ObjectData* this_) asm("_ZN4HPHP23c_DebuggerClientCmdUser10t_getframeEv");

TypedValue* tg_21DebuggerClientCmdUser_getFrame(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_21DebuggerClientCmdUser_getFrame((this_));
    } else {
      throw_toomany_arguments_nr("DebuggerClientCmdUser::getFrame", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::getFrame");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_printFrame(ObjectData* this_, int index) asm("_ZN4HPHP23c_DebuggerClientCmdUser12t_printframeEi");

void tg1_21DebuggerClientCmdUser_printFrame(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) __attribute__((noinline,cold));
void tg1_21DebuggerClientCmdUser_printFrame(TypedValue* rv, ActRec* ar, int32_t count, ObjectData* this_) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfNull;
  th_21DebuggerClientCmdUser_printFrame((this_), (int)(args[-0].m_data.num));
}

TypedValue* tg_21DebuggerClientCmdUser_printFrame(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      if ((args - 0)->m_type == KindOfInt64) {
        rv->m_type = KindOfNull;
        th_21DebuggerClientCmdUser_printFrame((this_), (int)(args[-0].m_data.num));
      } else {
        tg1_21DebuggerClientCmdUser_printFrame(rv, ar, count, this_);
      }
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::printFrame", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::printFrame");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void th_21DebuggerClientCmdUser_addCompletion(ObjectData* this_, TypedValue* list) asm("_ZN4HPHP23c_DebuggerClientCmdUser15t_addcompletionERKNS_7VariantE");

TypedValue* tg_21DebuggerClientCmdUser_addCompletion(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      rv->m_type = KindOfNull;
      th_21DebuggerClientCmdUser_addCompletion((this_), (args-0));
    } else {
      throw_wrong_arguments_nr("DebuggerClientCmdUser::addCompletion", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClientCmdUser::addCompletion");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
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
void th_14DebuggerClient___construct(ObjectData* this_) asm("_ZN4HPHP16c_DebuggerClient13t___constructEv");

TypedValue* tg_14DebuggerClient___construct(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfNull;
      th_14DebuggerClient___construct((this_));
    } else {
      throw_toomany_arguments_nr("DebuggerClient::__construct", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClient::__construct");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long th_14DebuggerClient_getState(ObjectData* this_) asm("_ZN4HPHP16c_DebuggerClient10t_getstateEv");

TypedValue* tg_14DebuggerClient_getState(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 0) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)th_14DebuggerClient_getState((this_));
    } else {
      throw_toomany_arguments_nr("DebuggerClient::getState", 0, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClient::getState");
  }
  frame_free_locals_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_14DebuggerClient_init(TypedValue* _rv, ObjectData* this_, TypedValue* options) asm("_ZN4HPHP16c_DebuggerClient6t_initERKNS_7VariantE");

TypedValue* tg_14DebuggerClient_init(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 1) {
      th_14DebuggerClient_init(rv, (this_), (args-0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("DebuggerClient::init", count, 1, 1, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClient::init");
  }
  frame_free_locals_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* th_14DebuggerClient_processCmd(TypedValue* _rv, ObjectData* this_, TypedValue* cmdName, TypedValue* args) asm("_ZN4HPHP16c_DebuggerClient12t_processcmdERKNS_7VariantES3_");

TypedValue* tg_14DebuggerClient_processCmd(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  ObjectData* this_ = (ar->hasThis() ? ar->getThis() : nullptr);
  if (this_) {
    if (count == 2) {
      th_14DebuggerClient_processCmd(rv, (this_), (args-0), (args-1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      throw_wrong_arguments_nr("DebuggerClient::processCmd", count, 2, 2, 1);
      rv->m_data.num = 0LL;
      rv->m_type = KindOfNull;
    }
  } else {
    throw_instance_method_fatal("DebuggerClient::processCmd");
  }
  frame_free_locals_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
