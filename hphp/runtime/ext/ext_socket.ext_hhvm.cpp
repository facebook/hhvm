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

TypedValue* fh_socket_create(TypedValue* _rv, int domain, int type, int protocol) asm("_ZN4HPHP15f_socket_createEiii");

void fg1_socket_create(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_create(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_create(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_create(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      fh_socket_create(rv, (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_create(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_create", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_create_listen(TypedValue* _rv, int port, int backlog) asm("_ZN4HPHP22f_socket_create_listenEii");

void fg1_socket_create_listen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_create_listen(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_create_listen(rv, (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(128));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_create_listen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfInt64) {
      fh_socket_create_listen(rv, (int)(args[-0].m_data.num), (count > 1) ? (int)(args[-1].m_data.num) : (int)(128));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_create_listen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_create_listen", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_create_pair(int domain, int type, int protocol, TypedValue* fd) asm("_ZN4HPHP20f_socket_create_pairEiiiRKNS_14VRefParamValueE");

void fg1_socket_create_pair(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_create_pair(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_socket_create_pair((int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args-3))) ? 1LL : 0LL;
}

TypedValue* fg_socket_create_pair(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_create_pair((int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args-3))) ? 1LL : 0LL;
    } else {
      fg1_socket_create_pair(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_create_pair", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_get_option(TypedValue* _rv, Value* socket, int level, int optname) asm("_ZN4HPHP19f_socket_get_optionERKNS_6ObjectEii");

void fg1_socket_get_option(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_get_option(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_get_option(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_get_option(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_socket_get_option(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_get_option(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_get_option", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_getpeername(Value* socket, TypedValue* address, TypedValue* port) asm("_ZN4HPHP20f_socket_getpeernameERKNS_6ObjectERKNS_14VRefParamValueES5_");

void fg1_socket_getpeername(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_getpeername(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal2 = uninit_null();
  rv->m_data.num = (fh_socket_getpeername(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* fg_socket_getpeername(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal2 = uninit_null();
      rv->m_data.num = (fh_socket_getpeername(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
    } else {
      fg1_socket_getpeername(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_getpeername", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_getsockname(Value* socket, TypedValue* address, TypedValue* port) asm("_ZN4HPHP20f_socket_getsocknameERKNS_6ObjectERKNS_14VRefParamValueES5_");

void fg1_socket_getsockname(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_getsockname(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal2 = uninit_null();
  rv->m_data.num = (fh_socket_getsockname(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* fg_socket_getsockname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal2 = uninit_null();
      rv->m_data.num = (fh_socket_getsockname(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
    } else {
      fg1_socket_getsockname(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_getsockname", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_set_block(Value* socket) asm("_ZN4HPHP18f_socket_set_blockERKNS_6ObjectE");

void fg1_socket_set_block(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_set_block(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_socket_set_block(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_socket_set_block(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_set_block(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_socket_set_block(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_set_block", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_set_nonblock(Value* socket) asm("_ZN4HPHP21f_socket_set_nonblockERKNS_6ObjectE");

void fg1_socket_set_nonblock(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_set_nonblock(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_socket_set_nonblock(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_socket_set_nonblock(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_set_nonblock(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_socket_set_nonblock(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_set_nonblock", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_set_option(Value* socket, int level, int optname, TypedValue* optval) asm("_ZN4HPHP19f_socket_set_optionERKNS_6ObjectEiiRKNS_7VariantE");

void fg1_socket_set_option(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_set_option(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_socket_set_option(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args-3))) ? 1LL : 0LL;
}

TypedValue* fg_socket_set_option(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_set_option(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num), (args-3))) ? 1LL : 0LL;
    } else {
      fg1_socket_set_option(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_set_option", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_connect(Value* socket, Value* address, int port) asm("_ZN4HPHP16f_socket_connectERKNS_6ObjectERKNS_6StringEi");

void fg1_socket_connect(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_connect(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_socket_connect(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_socket_connect(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_connect(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_socket_connect(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_connect", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_bind(Value* socket, Value* address, int port) asm("_ZN4HPHP13f_socket_bindERKNS_6ObjectERKNS_6StringEi");

void fg1_socket_bind(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_bind(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_socket_bind(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_socket_bind(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_bind(&args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_socket_bind(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_bind", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_listen(Value* socket, int backlog) asm("_ZN4HPHP15f_socket_listenERKNS_6ObjectEi");

void fg1_socket_listen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_listen(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_socket_listen(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_socket_listen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_listen(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_socket_listen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_listen", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_select(TypedValue* _rv, TypedValue* read, TypedValue* write, TypedValue* except, TypedValue* vtv_sec, int tv_usec) asm("_ZN4HPHP15f_socket_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi");

void fg1_socket_select(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_select(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-4);
  fh_socket_select(rv, (args-0), (args-1), (args-2), (args-3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_select(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfInt64)) {
      fh_socket_select(rv, (args-0), (args-1), (args-2), (args-3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_select(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_select", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_server(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr) asm("_ZN4HPHP15f_socket_serverERKNS_6StringEiRKNS_14VRefParamValueES5_");

void fg1_socket_server(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_server(TypedValue* rv, ActRec* ar, int32_t count) {
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
  VRefParamValue defVal2 = uninit_null();
  VRefParamValue defVal3 = uninit_null();
  fh_socket_server(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_server(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal2 = uninit_null();
      VRefParamValue defVal3 = uninit_null();
      fh_socket_server(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_server(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_server", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_accept(TypedValue* _rv, Value* socket) asm("_ZN4HPHP15f_socket_acceptERKNS_6ObjectE");

void fg1_socket_accept(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_accept(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_socket_accept(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_accept(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_socket_accept(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_accept(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_accept", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_read(TypedValue* _rv, Value* socket, int length, int type) asm("_ZN4HPHP13f_socket_readERKNS_6ObjectEii");

void fg1_socket_read(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_read(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_read(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_read(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_socket_read(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_read(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_read", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_write(TypedValue* _rv, Value* socket, Value* buffer, int length) asm("_ZN4HPHP14f_socket_writeERKNS_6ObjectERKNS_6StringEi");

void fg1_socket_write(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_write(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_write(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_write(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_socket_write(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_write(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_write", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_send(TypedValue* _rv, Value* socket, Value* buf, int len, int flags) asm("_ZN4HPHP13f_socket_sendERKNS_6ObjectERKNS_6StringEii");

void fg1_socket_send(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_send(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_send(rv, &args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_send(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_socket_send(rv, &args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_send(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_send", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_sendto(TypedValue* _rv, Value* socket, Value* buf, int len, int flags, Value* addr, int port) asm("_ZN4HPHP15f_socket_sendtoERKNS_6ObjectERKNS_6StringEiiS5_i");

void fg1_socket_sendto(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_sendto(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_sendto(rv, &args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_sendto(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 5 && count <= 6) {
    if ((count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 4)->m_type) &&
        (args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_socket_sendto(rv, &args[-0].m_data, &args[-1].m_data, (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), &args[-4].m_data, (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_sendto(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_sendto", count, 5, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_recv(TypedValue* _rv, Value* socket, TypedValue* buf, int len, int flags) asm("_ZN4HPHP13f_socket_recvERKNS_6ObjectERKNS_14VRefParamValueEii");

void fg1_socket_recv(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_recv(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_recv(rv, &args[-0].m_data, (args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_recv(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      fh_socket_recv(rv, &args[-0].m_data, (args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_recv(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_recv", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_recvfrom(TypedValue* _rv, Value* socket, TypedValue* buf, int len, int flags, TypedValue* name, TypedValue* port) asm("_ZN4HPHP17f_socket_recvfromERKNS_6ObjectERKNS_14VRefParamValueEiiS5_S5_");

void fg1_socket_recvfrom(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_recvfrom(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_socket_recvfrom(rv, &args[-0].m_data, (args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (args-4), (count > 5) ? (args-5) : (TypedValue*)(&defVal5));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_recvfrom(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 5 && count <= 6) {
    if ((args - 3)->m_type == KindOfInt64 &&
        (args - 2)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      VRefParamValue defVal5 = 0;
      fh_socket_recvfrom(rv, &args[-0].m_data, (args-1), (int)(args[-2].m_data.num), (int)(args[-3].m_data.num), (args-4), (count > 5) ? (args-5) : (TypedValue*)(&defVal5));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_recvfrom(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_recvfrom", count, 5, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_shutdown(Value* socket, int how) asm("_ZN4HPHP17f_socket_shutdownERKNS_6ObjectEi");

void fg1_socket_shutdown(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_shutdown(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_socket_shutdown(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_socket_shutdown(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_shutdown(&args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_socket_shutdown(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_shutdown", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_socket_close(Value* socket) asm("_ZN4HPHP14f_socket_closeERKNS_6ObjectE");

void fg1_socket_close(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_close(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_socket_close(&args[-0].m_data);
}

TypedValue* fg_socket_close(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_socket_close(&args[-0].m_data);
    } else {
      fg1_socket_close(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_close", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_socket_strerror(Value* _rv, int errnum) asm("_ZN4HPHP17f_socket_strerrorEi");

void fg1_socket_strerror(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_strerror(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_socket_strerror(&(rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_strerror(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_socket_strerror(&(rv->m_data), (int)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_socket_strerror(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_strerror", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_socket_last_error(Value* socket) asm("_ZN4HPHP19f_socket_last_errorERKNS_6ObjectE");

void fg1_socket_last_error(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_last_error(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_socket_last_error((count > 0) ? &args[-0].m_data : (Value*)(&null_object));
}

TypedValue* fg_socket_last_error(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfObject)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_socket_last_error((count > 0) ? &args[-0].m_data : (Value*)(&null_object));
    } else {
      fg1_socket_last_error(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("socket_last_error", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_socket_clear_error(Value* socket) asm("_ZN4HPHP20f_socket_clear_errorERKNS_6ObjectE");

void fg1_socket_clear_error(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_clear_error(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_socket_clear_error((count > 0) ? &args[-0].m_data : (Value*)(&null_object));
}

TypedValue* fg_socket_clear_error(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfObject)) {
      rv->m_type = KindOfNull;
      fh_socket_clear_error((count > 0) ? &args[-0].m_data : (Value*)(&null_object));
    } else {
      fg1_socket_clear_error(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("socket_clear_error", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_fsockopen(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr, double timeout) asm("_ZN4HPHP11f_fsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d");

void fg1_fsockopen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_fsockopen(TypedValue* rv, ActRec* ar, int32_t count) {
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
  VRefParamValue defVal2 = uninit_null();
  VRefParamValue defVal3 = uninit_null();
  fh_fsockopen(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_fsockopen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfDouble) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal2 = uninit_null();
      VRefParamValue defVal3 = uninit_null();
      fh_fsockopen(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_fsockopen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("fsockopen", count, 1, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_pfsockopen(TypedValue* _rv, Value* hostname, int port, TypedValue* errnum, TypedValue* errstr, double timeout) asm("_ZN4HPHP12f_pfsockopenERKNS_6StringEiRKNS_14VRefParamValueES5_d");

void fg1_pfsockopen(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_pfsockopen(TypedValue* rv, ActRec* ar, int32_t count) {
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
  VRefParamValue defVal2 = uninit_null();
  VRefParamValue defVal3 = uninit_null();
  fh_pfsockopen(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_pfsockopen(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 5) {
    if ((count <= 4 || (args - 4)->m_type == KindOfDouble) &&
        (count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal2 = uninit_null();
      VRefParamValue defVal3 = uninit_null();
      fh_pfsockopen(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3), (count > 4) ? (args[-4].m_data.dbl) : (double)(0.0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_pfsockopen(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("pfsockopen", count, 1, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_getaddrinfo(TypedValue* _rv, Value* host, Value* port, int family, int socktype, int protocol, int flags) asm("_ZN4HPHP13f_getaddrinfoERKNS_6StringES2_iiii");

void fg1_getaddrinfo(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_getaddrinfo(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_getaddrinfo(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_getaddrinfo(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 6) {
    if ((count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_getaddrinfo(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_getaddrinfo(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("getaddrinfo", count, 2, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
