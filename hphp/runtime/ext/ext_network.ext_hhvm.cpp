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

TypedValue* fh_gethostname(TypedValue* _rv) asm("_ZN4HPHP13f_gethostnameEv");

TypedValue* fg_gethostname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_gethostname(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("gethostname", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_gethostbyaddr(TypedValue* _rv, Value* ip_address) asm("_ZN4HPHP15f_gethostbyaddrERKNS_6StringE");

void fg1_gethostbyaddr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_gethostbyaddr(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_gethostbyaddr(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_gethostbyaddr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_gethostbyaddr(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_gethostbyaddr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("gethostbyaddr", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_gethostbyname(Value* _rv, Value* hostname) asm("_ZN4HPHP15f_gethostbynameERKNS_6StringE");

void fg1_gethostbyname(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_gethostbyname(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfString;
  fh_gethostbyname(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_gethostbyname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfString;
      fh_gethostbyname(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_gethostbyname(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("gethostbyname", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_gethostbynamel(TypedValue* _rv, Value* hostname) asm("_ZN4HPHP16f_gethostbynamelERKNS_6StringE");

void fg1_gethostbynamel(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_gethostbynamel(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_gethostbynamel(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_gethostbynamel(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_gethostbynamel(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_gethostbynamel(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("gethostbynamel", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_getprotobyname(TypedValue* _rv, Value* name) asm("_ZN4HPHP16f_getprotobynameERKNS_6StringE");

void fg1_getprotobyname(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_getprotobyname(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_getprotobyname(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_getprotobyname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_getprotobyname(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_getprotobyname(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("getprotobyname", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_getprotobynumber(TypedValue* _rv, int number) asm("_ZN4HPHP18f_getprotobynumberEi");

void fg1_getprotobynumber(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_getprotobynumber(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_getprotobynumber(rv, (int)(args[-0].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_getprotobynumber(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      fh_getprotobynumber(rv, (int)(args[-0].m_data.num));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_getprotobynumber(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("getprotobynumber", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_getservbyname(TypedValue* _rv, Value* service, Value* protocol) asm("_ZN4HPHP15f_getservbynameERKNS_6StringES2_");

void fg1_getservbyname(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_getservbyname(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_getservbyname(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_getservbyname(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_getservbyname(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_getservbyname(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("getservbyname", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_getservbyport(TypedValue* _rv, int port, Value* protocol) asm("_ZN4HPHP15f_getservbyportEiRKNS_6StringE");

void fg1_getservbyport(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_getservbyport(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  fh_getservbyport(rv, (int)(args[-0].m_data.num), &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_getservbyport(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfInt64) {
      fh_getservbyport(rv, (int)(args[-0].m_data.num), &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_getservbyport(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("getservbyport", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_inet_ntop(TypedValue* _rv, Value* in_addr) asm("_ZN4HPHP11f_inet_ntopERKNS_6StringE");

void fg1_inet_ntop(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_inet_ntop(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_inet_ntop(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_inet_ntop(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_inet_ntop(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_inet_ntop(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("inet_ntop", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_inet_pton(TypedValue* _rv, Value* address) asm("_ZN4HPHP11f_inet_ptonERKNS_6StringE");

void fg1_inet_pton(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_inet_pton(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_inet_pton(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_inet_pton(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_inet_pton(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_inet_pton(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("inet_pton", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_ip2long(TypedValue* _rv, Value* ip_address) asm("_ZN4HPHP9f_ip2longERKNS_6StringE");

void fg1_ip2long(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ip2long(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_ip2long(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_ip2long(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_ip2long(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_ip2long(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ip2long", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_long2ip(Value* _rv, int proper_address) asm("_ZN4HPHP9f_long2ipEi");

void fg1_long2ip(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_long2ip(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  rv->m_type = KindOfString;
  fh_long2ip(&(rv->m_data), (int)(args[-0].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_long2ip(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfString;
      fh_long2ip(&(rv->m_data), (int)(args[-0].m_data.num));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_long2ip(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("long2ip", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_dns_check_record(Value* host, Value* type) asm("_ZN4HPHP18f_dns_check_recordERKNS_6StringES2_");

void fg1_dns_check_record(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_dns_check_record(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_dns_check_record(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_dns_check_record(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_dns_check_record(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_dns_check_record(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("dns_check_record", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_checkdnsrr(Value* host, Value* type) asm("_ZN4HPHP12f_checkdnsrrERKNS_6StringES2_");

void fg1_checkdnsrr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_checkdnsrr(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_data.num = (fh_checkdnsrr(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_checkdnsrr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_checkdnsrr(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_checkdnsrr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("checkdnsrr", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_dns_get_record(TypedValue* _rv, Value* hostname, int type, TypedValue* authns, TypedValue* addtl) asm("_ZN4HPHP16f_dns_get_recordERKNS_6StringEiRKNS_14VRefParamValueES5_");

void fg1_dns_get_record(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_dns_get_record(TypedValue* rv, ActRec* ar, int32_t count) {
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
  fh_dns_get_record(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_dns_get_record(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 4) {
    if ((count <= 1 || (args - 1)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      VRefParamValue defVal2 = uninit_null();
      VRefParamValue defVal3 = uninit_null();
      fh_dns_get_record(rv, &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args-3) : (TypedValue*)(&defVal3));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_dns_get_record(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("dns_get_record", count, 1, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_dns_get_mx(Value* hostname, TypedValue* mxhosts, TypedValue* weights) asm("_ZN4HPHP12f_dns_get_mxERKNS_6StringERKNS_14VRefParamValueES5_");

void fg1_dns_get_mx(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_dns_get_mx(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal2 = uninit_null();
  rv->m_data.num = (fh_dns_get_mx(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* fg_dns_get_mx(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal2 = uninit_null();
      rv->m_data.num = (fh_dns_get_mx(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
    } else {
      fg1_dns_get_mx(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("dns_get_mx", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_getmxrr(Value* hostname, TypedValue* mxhosts, TypedValue* weight) asm("_ZN4HPHP9f_getmxrrERKNS_6StringERKNS_14VRefParamValueES5_");

void fg1_getmxrr(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_getmxrr(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfBoolean;
  VRefParamValue defVal2 = uninit_null();
  rv->m_data.num = (fh_getmxrr(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
}

TypedValue* fg_getmxrr(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      VRefParamValue defVal2 = uninit_null();
      rv->m_data.num = (fh_getmxrr(&args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2))) ? 1LL : 0LL;
    } else {
      fg1_getmxrr(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("getmxrr", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_socket_get_status(TypedValue* _rv, Value* stream) asm("_ZN4HPHP19f_socket_get_statusERKNS_6ObjectE");

void fg1_socket_get_status(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_get_status(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_socket_get_status(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_socket_get_status(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_socket_get_status(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_socket_get_status(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_get_status", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_set_blocking(Value* stream, int mode) asm("_ZN4HPHP21f_socket_set_blockingERKNS_6ObjectEi");

void fg1_socket_set_blocking(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_set_blocking(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_socket_set_blocking(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_socket_set_blocking(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if ((args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_set_blocking(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_socket_set_blocking(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_set_blocking", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_socket_set_timeout(Value* stream, int seconds, int microseconds) asm("_ZN4HPHP20f_socket_set_timeoutERKNS_6ObjectEii");

void fg1_socket_set_timeout(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_socket_set_timeout(TypedValue* rv, ActRec* ar, int32_t count) {
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
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_socket_set_timeout(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
}

TypedValue* fg_socket_set_timeout(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (args - 1)->m_type == KindOfInt64 &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_socket_set_timeout(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
    } else {
      fg1_socket_set_timeout(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("socket_set_timeout", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_header(Value* str, bool replace, int http_response_code) asm("_ZN4HPHP8f_headerERKNS_6StringEbi");

void fg1_header(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_header(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfNull;
  fh_header(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
}

TypedValue* fg_header(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || (args - 1)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfNull;
      fh_header(&args[-0].m_data, (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
    } else {
      fg1_header(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("header", count, 1, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_headers_sent(TypedValue* file, TypedValue* line) asm("_ZN4HPHP14f_headers_sentERKNS_14VRefParamValueES2_");

TypedValue* fg_headers_sent(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 2) {
    rv->m_type = KindOfBoolean;
    VRefParamValue defVal0 = uninit_null();
    VRefParamValue defVal1 = uninit_null();
    rv->m_data.num = (fh_headers_sent((count > 0) ? (args-0) : (TypedValue*)(&defVal0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("headers_sent", 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_http_response_code(TypedValue* _rv, int response_code) asm("_ZN4HPHP20f_http_response_codeEi");

void fg1_http_response_code(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_http_response_code(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  fh_http_response_code(rv, (count > 0) ? (int)(args[-0].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_http_response_code(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfInt64)) {
      fh_http_response_code(rv, (count > 0) ? (int)(args[-0].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_http_response_code(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("http_response_code", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_headers_list(Value* _rv) asm("_ZN4HPHP14f_headers_listEv");

TypedValue* fg_headers_list(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfArray;
    fh_headers_list(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("headers_list", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_header_register_callback(TypedValue* callback) asm("_ZN4HPHP26f_header_register_callbackERKNS_7VariantE");

TypedValue* fg_header_register_callback(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_header_register_callback((args-0))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("header_register_callback", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_header_remove(Value* name) asm("_ZN4HPHP15f_header_removeERKNS_6StringE");

void fg1_header_remove(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_header_remove(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_header_remove((count > 0) ? &args[-0].m_data : (Value*)(&null_string));
}

TypedValue* fg_header_remove(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || IS_STRING_TYPE((args - 0)->m_type))) {
      rv->m_type = KindOfNull;
      fh_header_remove((count > 0) ? &args[-0].m_data : (Value*)(&null_string));
    } else {
      fg1_header_remove(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("header_remove", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

int fh_get_http_request_size() asm("_ZN4HPHP23f_get_http_request_sizeEv");

TypedValue* fg_get_http_request_size(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfInt64;
    rv->m_data.num = (int64_t)fh_get_http_request_size();
  } else {
    throw_toomany_arguments_nr("get_http_request_size", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_setcookie(Value* name, Value* value, long expire, Value* path, Value* domain, bool secure, bool httponly) asm("_ZN4HPHP11f_setcookieERKNS_6StringES2_lS2_S2_bb");

void fg1_setcookie(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_setcookie(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 7
    if ((args-6)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-5);
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
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
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
  rv->m_data.num = (fh_setcookie(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(false), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_setcookie(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 7) {
    if ((count <= 6 || (args - 6)->m_type == KindOfBoolean) &&
        (count <= 5 || (args - 5)->m_type == KindOfBoolean) &&
        (count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_setcookie(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(false), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_setcookie(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("setcookie", count, 1, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_setrawcookie(Value* name, Value* value, long expire, Value* path, Value* domain, bool secure, bool httponly) asm("_ZN4HPHP14f_setrawcookieERKNS_6StringES2_lS2_S2_bb");

void fg1_setrawcookie(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_setrawcookie(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 7
    if ((args-6)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-5);
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
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
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
  rv->m_data.num = (fh_setrawcookie(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(false), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(false))) ? 1LL : 0LL;
}

TypedValue* fg_setrawcookie(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 7) {
    if ((count <= 6 || (args - 6)->m_type == KindOfBoolean) &&
        (count <= 5 || (args - 5)->m_type == KindOfBoolean) &&
        (count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfInt64) &&
        (count <= 1 || IS_STRING_TYPE((args - 1)->m_type)) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_setrawcookie(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? (long)(args[-2].m_data.num) : (long)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? (bool)(args[-5].m_data.num) : (bool)(false), (count > 6) ? (bool)(args[-6].m_data.num) : (bool)(false))) ? 1LL : 0LL;
    } else {
      fg1_setrawcookie(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("setrawcookie", count, 1, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_define_syslog_variables() asm("_ZN4HPHP25f_define_syslog_variablesEv");

TypedValue* fg_define_syslog_variables(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfNull;
    fh_define_syslog_variables();
  } else {
    throw_toomany_arguments_nr("define_syslog_variables", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openlog(Value* ident, int option, int facility) asm("_ZN4HPHP9f_openlogERKNS_6StringEii");

void fg1_openlog(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openlog(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-2)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-2);
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openlog(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
}

TypedValue* fg_openlog(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if ((args - 2)->m_type == KindOfInt64 &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openlog(&args[-0].m_data, (int)(args[-1].m_data.num), (int)(args[-2].m_data.num))) ? 1LL : 0LL;
    } else {
      fg1_openlog(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openlog", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_closelog() asm("_ZN4HPHP10f_closelogEv");

TypedValue* fg_closelog(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_closelog()) ? 1LL : 0LL;
  } else {
    throw_toomany_arguments_nr("closelog", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_syslog(int priority, Value* message) asm("_ZN4HPHP8f_syslogEiRKNS_6StringE");

void fg1_syslog(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_syslog(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_syslog((int)(args[-0].m_data.num), &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_syslog(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfInt64) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_syslog((int)(args[-0].m_data.num), &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_syslog(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("syslog", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
