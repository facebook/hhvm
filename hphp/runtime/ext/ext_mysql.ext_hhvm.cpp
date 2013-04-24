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
HPHP::Variant HPHP::f_mysql_connect(HPHP::String const&, HPHP::String const&, HPHP::String const&, bool, int, int, int)
_ZN4HPHP15f_mysql_connectERKNS_6StringES2_S2_biii

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
new_link => r8
client_flags => r9
connect_timeout_ms => st0
query_timeout_ms => st8
*/

TypedValue* fh_mysql_connect(TypedValue* _rv, Value* server, Value* username, Value* password, bool new_link, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP15f_mysql_connectERKNS_6StringES2_S2_biii");

TypedValue * fg1_mysql_connect(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_connect(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 7
    if ((args-6)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_mysql_connect((rv), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_connect(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 7LL) {
      if ((count <= 6 || (args-6)->m_type == KindOfInt64) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_connect((&(rv)), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_connect(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mysql_connect", 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_async_connect_start(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP27f_mysql_async_connect_startERKNS_6StringES2_S2_S2_

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
database => r8
*/

TypedValue* fh_mysql_async_connect_start(TypedValue* _rv, Value* server, Value* username, Value* password, Value* database) asm("_ZN4HPHP27f_mysql_async_connect_startERKNS_6StringES2_S2_S2_");

TypedValue * fg1_mysql_async_connect_start(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_async_connect_start(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_mysql_async_connect_start((rv), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_async_connect_start(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_async_connect_start((&(rv)), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_async_connect_start(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mysql_async_connect_start", 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mysql_async_connect_completed(HPHP::Variant const&)
_ZN4HPHP31f_mysql_async_connect_completedERKNS_7VariantE

(return value) => rax
link_identifier => rdi
*/

bool fh_mysql_async_connect_completed(TypedValue* link_identifier) asm("_ZN4HPHP31f_mysql_async_connect_completedERKNS_7VariantE");

TypedValue* fg_mysql_async_connect_completed(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_mysql_async_connect_completed((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_async_connect_completed", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mysql_async_query_start(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP25f_mysql_async_query_startERKNS_6StringERKNS_7VariantE

(return value) => rax
query => rdi
link_identifier => rsi
*/

bool fh_mysql_async_query_start(Value* query, TypedValue* link_identifier) asm("_ZN4HPHP25f_mysql_async_query_startERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_async_query_start(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_async_query_start(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_mysql_async_query_start(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mysql_async_query_start(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mysql_async_query_start(&args[-0].m_data, (args-1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_async_query_start(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_async_query_start", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_async_query_result(HPHP::Variant const&)
_ZN4HPHP26f_mysql_async_query_resultERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_async_query_result(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP26f_mysql_async_query_resultERKNS_7VariantE");

TypedValue* fg_mysql_async_query_result(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_mysql_async_query_result((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_async_query_result", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mysql_async_query_completed(HPHP::Variant const&)
_ZN4HPHP29f_mysql_async_query_completedERKNS_7VariantE

(return value) => rax
result => rdi
*/

bool fh_mysql_async_query_completed(TypedValue* result) asm("_ZN4HPHP29f_mysql_async_query_completedERKNS_7VariantE");

TypedValue* fg_mysql_async_query_completed(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfBoolean;
      rv.m_data.num = (fh_mysql_async_query_completed((args-0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_async_query_completed", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_async_fetch_array(HPHP::Variant const&, int)
_ZN4HPHP25f_mysql_async_fetch_arrayERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
result_type => rdx
*/

TypedValue* fh_mysql_async_fetch_array(TypedValue* _rv, TypedValue* result, int result_type) asm("_ZN4HPHP25f_mysql_async_fetch_arrayERKNS_7VariantEi");

TypedValue * fg1_mysql_async_fetch_array(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_async_fetch_array(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_async_fetch_array((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_async_fetch_array(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_mysql_async_fetch_array((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_async_fetch_array(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_async_fetch_array", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_async_wait_actionable(HPHP::Variant const&, double)
_ZN4HPHP29f_mysql_async_wait_actionableERKNS_7VariantEd

(return value) => rax
_rv => rdi
items => rsi
timeout => xmm0
*/

TypedValue* fh_mysql_async_wait_actionable(TypedValue* _rv, TypedValue* items, double timeout) asm("_ZN4HPHP29f_mysql_async_wait_actionableERKNS_7VariantEd");

TypedValue * fg1_mysql_async_wait_actionable(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_async_wait_actionable(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToDoubleInPlace(args-1);
  fh_mysql_async_wait_actionable((rv), (args-0), (args[-1].m_data.dbl));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_async_wait_actionable(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfDouble) {
        fh_mysql_async_wait_actionable((&(rv)), (args-0), (args[-1].m_data.dbl));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_async_wait_actionable(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_async_wait_actionable", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_mysql_async_status(HPHP::Variant const&)
_ZN4HPHP20f_mysql_async_statusERKNS_7VariantE

(return value) => rax
link_identifier => rdi
*/

long fh_mysql_async_status(TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_async_statusERKNS_7VariantE");

TypedValue* fg_mysql_async_status(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      rv.m_type = KindOfInt64;
      rv.m_data.num = (int64_t)fh_mysql_async_status((args-0));
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_async_status", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_pconnect(HPHP::String const&, HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP16f_mysql_pconnectERKNS_6StringES2_S2_iii

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
client_flags => r8
connect_timeout_ms => r9
query_timeout_ms => st0
*/

TypedValue* fh_mysql_pconnect(TypedValue* _rv, Value* server, Value* username, Value* password, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP16f_mysql_pconnectERKNS_6StringES2_S2_iii");

TypedValue * fg1_mysql_pconnect(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_pconnect(TypedValue* rv, ActRec* ar, int64_t count) {
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
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_mysql_pconnect((rv), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_pconnect(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_pconnect((&(rv)), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_pconnect(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mysql_pconnect", 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_connect_with_db(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, bool, int, int, int)
_ZN4HPHP23f_mysql_connect_with_dbERKNS_6StringES2_S2_S2_biii

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
database => r8
new_link => r9
client_flags => st0
connect_timeout_ms => st8
query_timeout_ms => st16
*/

TypedValue* fh_mysql_connect_with_db(TypedValue* _rv, Value* server, Value* username, Value* password, Value* database, bool new_link, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP23f_mysql_connect_with_dbERKNS_6StringES2_S2_S2_biii");

TypedValue * fg1_mysql_connect_with_db(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_connect_with_db(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 8
    if ((args-7)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-7);
    }
  case 7:
    if ((args-6)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_mysql_connect_with_db((rv), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1), (count > 7) ? (int)(args[-7].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_connect_with_db(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 8LL) {
      if ((count <= 7 || (args-7)->m_type == KindOfInt64) && (count <= 6 || (args-6)->m_type == KindOfInt64) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfBoolean) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_connect_with_db((&(rv)), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1), (count > 7) ? (int)(args[-7].m_data.num) : (int)(-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_connect_with_db(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 8);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mysql_connect_with_db", 8, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_pconnect_with_db(HPHP::String const&, HPHP::String const&, HPHP::String const&, HPHP::String const&, int, int, int)
_ZN4HPHP24f_mysql_pconnect_with_dbERKNS_6StringES2_S2_S2_iii

(return value) => rax
_rv => rdi
server => rsi
username => rdx
password => rcx
database => r8
client_flags => r9
connect_timeout_ms => st0
query_timeout_ms => st8
*/

TypedValue* fh_mysql_pconnect_with_db(TypedValue* _rv, Value* server, Value* username, Value* password, Value* database, int client_flags, int connect_timeout_ms, int query_timeout_ms) asm("_ZN4HPHP24f_mysql_pconnect_with_dbERKNS_6StringES2_S2_S2_iii");

TypedValue * fg1_mysql_pconnect_with_db(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_pconnect_with_db(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 7
    if ((args-6)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    if (!IS_STRING_TYPE((args-0)->m_type)) {
      tvCastToStringInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_mysql_pconnect_with_db((rv), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_pconnect_with_db(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 7LL) {
      if ((count <= 6 || (args-6)->m_type == KindOfInt64) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_pconnect_with_db((&(rv)), (count > 0) ? &args[-0].m_data : (Value*)(&null_string), (count > 1) ? &args[-1].m_data : (Value*)(&null_string), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_pconnect_with_db(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 7);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mysql_pconnect_with_db", 7, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_set_charset(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_mysql_set_charsetERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
charset => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_set_charset(TypedValue* _rv, Value* charset, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_set_charsetERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_set_charset(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_set_charset(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_set_charset((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_set_charset(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_set_charset((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_set_charset(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_set_charset", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_ping(HPHP::Variant const&)
_ZN4HPHP12f_mysql_pingERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_ping(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP12f_mysql_pingERKNS_7VariantE");

TypedValue* fg_mysql_ping(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_ping((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_ping", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_mysql_escape_string(HPHP::String const&)
_ZN4HPHP21f_mysql_escape_stringERKNS_6StringE

(return value) => rax
_rv => rdi
unescaped_string => rsi
*/

Value* fh_mysql_escape_string(Value* _rv, Value* unescaped_string) asm("_ZN4HPHP21f_mysql_escape_stringERKNS_6StringE");

TypedValue * fg1_mysql_escape_string(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_escape_string(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_mysql_escape_string((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_escape_string(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfString;
        fh_mysql_escape_string((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_escape_string(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_escape_string", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_real_escape_string(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP26f_mysql_real_escape_stringERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
unescaped_string => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_real_escape_string(TypedValue* _rv, Value* unescaped_string, TypedValue* link_identifier) asm("_ZN4HPHP26f_mysql_real_escape_stringERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_real_escape_string(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_real_escape_string(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_real_escape_string((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_real_escape_string(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_real_escape_string((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_real_escape_string(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_real_escape_string", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_client_encoding(HPHP::Variant const&)
_ZN4HPHP23f_mysql_client_encodingERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_client_encoding(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP23f_mysql_client_encodingERKNS_7VariantE");

TypedValue* fg_mysql_client_encoding(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_client_encoding((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_client_encoding", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_close(HPHP::Variant const&)
_ZN4HPHP13f_mysql_closeERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_close(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_closeERKNS_7VariantE");

TypedValue* fg_mysql_close(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_close((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_close", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_errno(HPHP::Variant const&)
_ZN4HPHP13f_mysql_errnoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_errno(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_errnoERKNS_7VariantE");

TypedValue* fg_mysql_errno(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_errno((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_errno", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_error(HPHP::Variant const&)
_ZN4HPHP13f_mysql_errorERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_error(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_errorERKNS_7VariantE");

TypedValue* fg_mysql_error(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_error((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_error", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_warning_count(HPHP::Variant const&)
_ZN4HPHP21f_mysql_warning_countERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_warning_count(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP21f_mysql_warning_countERKNS_7VariantE");

TypedValue* fg_mysql_warning_count(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_warning_count((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_warning_count", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_mysql_get_client_info()
_ZN4HPHP23f_mysql_get_client_infoEv

(return value) => rax
_rv => rdi
*/

Value* fh_mysql_get_client_info(Value* _rv) asm("_ZN4HPHP23f_mysql_get_client_infoEv");

TypedValue* fg_mysql_get_client_info(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfString;
      fh_mysql_get_client_info((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_get_client_info", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_get_host_info(HPHP::Variant const&)
_ZN4HPHP21f_mysql_get_host_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_host_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP21f_mysql_get_host_infoERKNS_7VariantE");

TypedValue* fg_mysql_get_host_info(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_get_host_info((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_get_host_info", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_get_proto_info(HPHP::Variant const&)
_ZN4HPHP22f_mysql_get_proto_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_proto_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP22f_mysql_get_proto_infoERKNS_7VariantE");

TypedValue* fg_mysql_get_proto_info(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_get_proto_info((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_get_proto_info", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_get_server_info(HPHP::Variant const&)
_ZN4HPHP23f_mysql_get_server_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_get_server_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP23f_mysql_get_server_infoERKNS_7VariantE");

TypedValue* fg_mysql_get_server_info(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_get_server_info((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_get_server_info", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_info(HPHP::Variant const&)
_ZN4HPHP12f_mysql_infoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_info(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP12f_mysql_infoERKNS_7VariantE");

TypedValue* fg_mysql_info(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_info((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_info", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_insert_id(HPHP::Variant const&)
_ZN4HPHP17f_mysql_insert_idERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_insert_id(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP17f_mysql_insert_idERKNS_7VariantE");

TypedValue* fg_mysql_insert_id(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_insert_id((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_insert_id", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_stat(HPHP::Variant const&)
_ZN4HPHP12f_mysql_statERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_stat(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP12f_mysql_statERKNS_7VariantE");

TypedValue* fg_mysql_stat(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_stat((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_stat", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_thread_id(HPHP::Variant const&)
_ZN4HPHP17f_mysql_thread_idERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_thread_id(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP17f_mysql_thread_idERKNS_7VariantE");

TypedValue* fg_mysql_thread_id(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_thread_id((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_thread_id", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_create_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP17f_mysql_create_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_create_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP17f_mysql_create_dbERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_create_db(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_create_db(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_create_db((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_create_db(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_create_db((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_create_db(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_create_db", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_select_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP17f_mysql_select_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_select_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP17f_mysql_select_dbERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_select_db(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_select_db(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_select_db((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_select_db(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_select_db((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_select_db(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_select_db", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_drop_db(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP15f_mysql_drop_dbERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
db => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_drop_db(TypedValue* _rv, Value* db, TypedValue* link_identifier) asm("_ZN4HPHP15f_mysql_drop_dbERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_drop_db(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_drop_db(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_drop_db((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_drop_db(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_drop_db((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_drop_db(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_drop_db", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_affected_rows(HPHP::Variant const&)
_ZN4HPHP21f_mysql_affected_rowsERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_affected_rows(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP21f_mysql_affected_rowsERKNS_7VariantE");

TypedValue* fg_mysql_affected_rows(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_affected_rows((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_affected_rows", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mysql_set_timeout(int, HPHP::Variant const&)
_ZN4HPHP19f_mysql_set_timeoutEiRKNS_7VariantE

(return value) => rax
query_timeout_ms => rdi
link_identifier => rsi
*/

bool fh_mysql_set_timeout(int query_timeout_ms, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_set_timeoutEiRKNS_7VariantE");

TypedValue * fg1_mysql_set_timeout(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_set_timeout(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  Variant defVal1;
  rv->m_data.num = (fh_mysql_set_timeout((count > 0) ? (int)(args[-0].m_data.num) : (int)(-1), (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mysql_set_timeout(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv.m_type = KindOfBoolean;
        Variant defVal1;
        rv.m_data.num = (fh_mysql_set_timeout((count > 0) ? (int)(args[-0].m_data.num) : (int)(-1), (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_set_timeout(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("mysql_set_timeout", 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_query(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP13f_mysql_queryERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
query => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_queryERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_query(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_query(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_query((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_query(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_query((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_query(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_query", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_multi_query(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_mysql_multi_queryERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
query => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_multi_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_multi_queryERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_multi_query(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_multi_query(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_multi_query((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_multi_query(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_multi_query((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_multi_query(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_multi_query", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mysql_next_result(HPHP::Variant const&)
_ZN4HPHP19f_mysql_next_resultERKNS_7VariantE

(return value) => rax
link_identifier => rdi
*/

bool fh_mysql_next_result(TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_next_resultERKNS_7VariantE");

TypedValue* fg_mysql_next_result(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv.m_type = KindOfBoolean;
      Variant defVal0;
      rv.m_data.num = (fh_mysql_next_result((count > 0) ? (args-0) : (TypedValue*)(&defVal0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_next_result", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mysql_more_results(HPHP::Variant const&)
_ZN4HPHP20f_mysql_more_resultsERKNS_7VariantE

(return value) => rax
link_identifier => rdi
*/

bool fh_mysql_more_results(TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_more_resultsERKNS_7VariantE");

TypedValue* fg_mysql_more_results(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv.m_type = KindOfBoolean;
      Variant defVal0;
      rv.m_data.num = (fh_mysql_more_results((count > 0) ? (args-0) : (TypedValue*)(&defVal0))) ? 1LL : 0LL;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_more_results", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_fetch_result(HPHP::Variant const&)
_ZN4HPHP20f_mysql_fetch_resultERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_fetch_result(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_fetch_resultERKNS_7VariantE");

TypedValue* fg_mysql_fetch_result(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_fetch_result((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_fetch_result", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_unbuffered_query(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP24f_mysql_unbuffered_queryERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
query => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_unbuffered_query(TypedValue* _rv, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP24f_mysql_unbuffered_queryERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_unbuffered_query(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_unbuffered_query(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_unbuffered_query((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_unbuffered_query(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_unbuffered_query((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_unbuffered_query(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_unbuffered_query", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_db_query(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP16f_mysql_db_queryERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
database => rsi
query => rdx
link_identifier => rcx
*/

TypedValue* fh_mysql_db_query(TypedValue* _rv, Value* database, Value* query, TypedValue* link_identifier) asm("_ZN4HPHP16f_mysql_db_queryERKNS_6StringES2_RKNS_7VariantE");

TypedValue * fg1_mysql_db_query(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_db_query(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2;
  fh_mysql_db_query((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_db_query(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal2;
        fh_mysql_db_query((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_db_query(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_db_query", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_list_dbs(HPHP::Variant const&)
_ZN4HPHP16f_mysql_list_dbsERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_list_dbs(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP16f_mysql_list_dbsERKNS_7VariantE");

TypedValue* fg_mysql_list_dbs(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_list_dbs((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_list_dbs", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_list_tables(HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_mysql_list_tablesERKNS_6StringERKNS_7VariantE

(return value) => rax
_rv => rdi
database => rsi
link_identifier => rdx
*/

TypedValue* fh_mysql_list_tables(TypedValue* _rv, Value* database, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_list_tablesERKNS_6StringERKNS_7VariantE");

TypedValue * fg1_mysql_list_tables(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_list_tables(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_list_tables((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_list_tables(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_list_tables((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_list_tables(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_list_tables", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_list_fields(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_mysql_list_fieldsERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
database_name => rsi
table_name => rdx
link_identifier => rcx
*/

TypedValue* fh_mysql_list_fields(TypedValue* _rv, Value* database_name, Value* table_name, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_list_fieldsERKNS_6StringES2_RKNS_7VariantE");

TypedValue * fg1_mysql_list_fields(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_list_fields(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  Variant defVal2;
  fh_mysql_list_fields((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_list_fields(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal2;
        fh_mysql_list_fields((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_list_fields(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_list_fields", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_list_processes(HPHP::Variant const&)
_ZN4HPHP22f_mysql_list_processesERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_list_processes(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP22f_mysql_list_processesERKNS_7VariantE");

TypedValue* fg_mysql_list_processes(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      Variant defVal0;
      fh_mysql_list_processes((&(rv)), (count > 0) ? (args-0) : (TypedValue*)(&defVal0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("mysql_list_processes", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_db_name(HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP15f_mysql_db_nameERKNS_7VariantEiS2_

(return value) => rax
_rv => rdi
result => rsi
row => rdx
field => rcx
*/

TypedValue* fh_mysql_db_name(TypedValue* _rv, TypedValue* result, int row, TypedValue* field) asm("_ZN4HPHP15f_mysql_db_nameERKNS_7VariantEiS2_");

TypedValue * fg1_mysql_db_name(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_db_name(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_db_name((rv), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_db_name(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_mysql_db_name((&(rv)), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_db_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_db_name", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_tablename(HPHP::Variant const&, int)
_ZN4HPHP17f_mysql_tablenameERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
i => rdx
*/

TypedValue* fh_mysql_tablename(TypedValue* _rv, TypedValue* result, int i) asm("_ZN4HPHP17f_mysql_tablenameERKNS_7VariantEi");

TypedValue * fg1_mysql_tablename(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_tablename(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_tablename((rv), (args-0), (int)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_tablename(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_mysql_tablename((&(rv)), (args-0), (int)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_tablename(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_tablename", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_num_fields(HPHP::Variant const&)
_ZN4HPHP18f_mysql_num_fieldsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_num_fields(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP18f_mysql_num_fieldsERKNS_7VariantE");

TypedValue* fg_mysql_num_fields(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_mysql_num_fields((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_num_fields", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_num_rows(HPHP::Variant const&)
_ZN4HPHP16f_mysql_num_rowsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_num_rows(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP16f_mysql_num_rowsERKNS_7VariantE");

TypedValue* fg_mysql_num_rows(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_mysql_num_rows((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_num_rows", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_free_result(HPHP::Variant const&)
_ZN4HPHP19f_mysql_free_resultERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_free_result(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP19f_mysql_free_resultERKNS_7VariantE");

TypedValue* fg_mysql_free_result(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_mysql_free_result((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_free_result", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mysql_data_seek(HPHP::Variant const&, int)
_ZN4HPHP17f_mysql_data_seekERKNS_7VariantEi

(return value) => rax
result => rdi
row => rsi
*/

bool fh_mysql_data_seek(TypedValue* result, int row) asm("_ZN4HPHP17f_mysql_data_seekERKNS_7VariantEi");

TypedValue * fg1_mysql_data_seek(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_data_seek(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (fh_mysql_data_seek((args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mysql_data_seek(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mysql_data_seek((args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_data_seek(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_data_seek", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_fetch_row(HPHP::Variant const&)
_ZN4HPHP17f_mysql_fetch_rowERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_row(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP17f_mysql_fetch_rowERKNS_7VariantE");

TypedValue* fg_mysql_fetch_row(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_mysql_fetch_row((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_fetch_row", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_fetch_assoc(HPHP::Variant const&)
_ZN4HPHP19f_mysql_fetch_assocERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_assoc(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP19f_mysql_fetch_assocERKNS_7VariantE");

TypedValue* fg_mysql_fetch_assoc(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_mysql_fetch_assoc((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_fetch_assoc", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_fetch_array(HPHP::Variant const&, int)
_ZN4HPHP19f_mysql_fetch_arrayERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
result_type => rdx
*/

TypedValue* fh_mysql_fetch_array(TypedValue* _rv, TypedValue* result, int result_type) asm("_ZN4HPHP19f_mysql_fetch_arrayERKNS_7VariantEi");

TypedValue * fg1_mysql_fetch_array(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_fetch_array(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_fetch_array((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_fetch_array(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_mysql_fetch_array((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(3));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_fetch_array(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_fetch_array", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_fetch_lengths(HPHP::Variant const&)
_ZN4HPHP21f_mysql_fetch_lengthsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_lengths(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP21f_mysql_fetch_lengthsERKNS_7VariantE");

TypedValue* fg_mysql_fetch_lengths(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      fh_mysql_fetch_lengths((&(rv)), (args-0));
      if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 1);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_wrong_arguments_nr("mysql_fetch_lengths", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_fetch_object(HPHP::Variant const&, HPHP::String const&, HPHP::Array const&)
_ZN4HPHP20f_mysql_fetch_objectERKNS_7VariantERKNS_6StringERKNS_5ArrayE

(return value) => rax
_rv => rdi
result => rsi
class_name => rdx
params => rcx
*/

TypedValue* fh_mysql_fetch_object(TypedValue* _rv, TypedValue* result, Value* class_name, Value* params) asm("_ZN4HPHP20f_mysql_fetch_objectERKNS_7VariantERKNS_6StringERKNS_5ArrayE");

TypedValue * fg1_mysql_fetch_object(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_fetch_object(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-2);
    }
  case 2:
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  String defVal1 = "stdClass";
  Array defVal2 = uninit_null();
  fh_mysql_fetch_object((rv), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? &args[-2].m_data : (Value*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_fetch_object(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfArray) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type))) {
        String defVal1 = "stdClass";
        Array defVal2 = uninit_null();
        fh_mysql_fetch_object((&(rv)), (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&defVal1), (count > 2) ? &args[-2].m_data : (Value*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_fetch_object(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_fetch_object", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_result(HPHP::Variant const&, int, HPHP::Variant const&)
_ZN4HPHP14f_mysql_resultERKNS_7VariantEiS2_

(return value) => rax
_rv => rdi
result => rsi
row => rdx
field => rcx
*/

TypedValue* fh_mysql_result(TypedValue* _rv, TypedValue* result, int row, TypedValue* field) asm("_ZN4HPHP14f_mysql_resultERKNS_7VariantEiS2_");

TypedValue * fg1_mysql_result(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_result(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_result((rv), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_result(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((args-1)->m_type == KindOfInt64) {
        fh_mysql_result((&(rv)), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_result(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_result", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_fetch_field(HPHP::Variant const&, int)
_ZN4HPHP19f_mysql_fetch_fieldERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_fetch_field(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_fetch_fieldERKNS_7VariantEi");

TypedValue * fg1_mysql_fetch_field(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_fetch_field(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_fetch_field((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_fetch_field(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_mysql_fetch_field((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_fetch_field(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_fetch_field", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_mysql_field_seek(HPHP::Variant const&, int)
_ZN4HPHP18f_mysql_field_seekERKNS_7VariantEi

(return value) => rax
result => rdi
field => rsi
*/

bool fh_mysql_field_seek(TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_seekERKNS_7VariantEi");

TypedValue * fg1_mysql_field_seek(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_seek(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (fh_mysql_field_seek((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mysql_field_seek(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_mysql_field_seek((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_field_seek(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_field_seek", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_field_name(HPHP::Variant const&, int)
_ZN4HPHP18f_mysql_field_nameERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_name(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_nameERKNS_7VariantEi");

TypedValue * fg1_mysql_field_name(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_name(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_name((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_name(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_mysql_field_name((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_field_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_field_name", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_field_table(HPHP::Variant const&, int)
_ZN4HPHP19f_mysql_field_tableERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_table(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_field_tableERKNS_7VariantEi");

TypedValue * fg1_mysql_field_table(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_table(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_table((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_table(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_mysql_field_table((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_field_table(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_field_table", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_field_len(HPHP::Variant const&, int)
_ZN4HPHP17f_mysql_field_lenERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_len(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP17f_mysql_field_lenERKNS_7VariantEi");

TypedValue * fg1_mysql_field_len(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_len(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_len((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_len(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_mysql_field_len((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_field_len(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_field_len", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_field_type(HPHP::Variant const&, int)
_ZN4HPHP18f_mysql_field_typeERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_type(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_typeERKNS_7VariantEi");

TypedValue * fg1_mysql_field_type(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_type(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_type((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_type(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_mysql_field_type((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_field_type(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_field_type", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_mysql_field_flags(HPHP::Variant const&, int)
_ZN4HPHP19f_mysql_field_flagsERKNS_7VariantEi

(return value) => rax
_rv => rdi
result => rsi
field => rdx
*/

TypedValue* fh_mysql_field_flags(TypedValue* _rv, TypedValue* result, int field) asm("_ZN4HPHP19f_mysql_field_flagsERKNS_7VariantEi");

TypedValue * fg1_mysql_field_flags(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_flags(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_flags((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_flags(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        fh_mysql_field_flags((&(rv)), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_mysql_field_flags(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("mysql_field_flags", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

