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

TypedValue * fg1_mysql_connect(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_connect(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_mysql_connect((rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_connect(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 7LL) {
      if ((count <= 6 || (args-6)->m_type == KindOfInt64) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfBoolean) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_connect((&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (bool)(args[-3].m_data.num) : (bool)(false), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_pconnect(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_pconnect(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_mysql_pconnect((rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_pconnect(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_pconnect((&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(-1), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_connect_with_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_connect_with_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_mysql_connect_with_db((rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1), (count > 7) ? (int)(args[-7].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_connect_with_db(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 8LL) {
      if ((count <= 7 || (args-7)->m_type == KindOfInt64) && (count <= 6 || (args-6)->m_type == KindOfInt64) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfBoolean) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_connect_with_db((&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (bool)(args[-4].m_data.num) : (bool)(false), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1), (count > 7) ? (int)(args[-7].m_data.num) : (int)(-1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 8);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_pconnect_with_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_pconnect_with_db(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  fh_mysql_pconnect_with_db((rv), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_pconnect_with_db(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 7LL) {
      if ((count <= 6 || (args-6)->m_type == KindOfInt64) && (count <= 5 || (args-5)->m_type == KindOfInt64) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        fh_mysql_pconnect_with_db((&(rv)), (count > 0) ? (Value*)(args-0) : (Value*)(&null_string), (count > 1) ? (Value*)(args-1) : (Value*)(&null_string), (count > 2) ? (Value*)(args-2) : (Value*)(&null_string), (count > 3) ? (Value*)(args-3) : (Value*)(&null_string), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(-1), (count > 6) ? (int)(args[-6].m_data.num) : (int)(-1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 7);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::String HPHP::f_mysql_escape_string(HPHP::String const&)
_ZN4HPHP21f_mysql_escape_stringERKNS_6StringE

(return value) => rax
_rv => rdi
unescaped_string => rsi
*/

Value* fh_mysql_escape_string(Value* _rv, Value* unescaped_string) asm("_ZN4HPHP21f_mysql_escape_stringERKNS_6StringE");

TypedValue * fg1_mysql_escape_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_escape_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfString;
  tvCastToStringInPlace(args-0);
  fh_mysql_escape_string((Value*)(rv), (Value*)(args-0));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_escape_string(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv._count = 0;
        rv.m_type = KindOfString;
        fh_mysql_escape_string((Value*)(&(rv)), (Value*)(args-0));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_real_escape_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_real_escape_string(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_real_escape_string((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_real_escape_string(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_real_escape_string((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_errno(HPHP::Variant const&)
_ZN4HPHP13f_mysql_errnoERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_errno(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_errnoERKNS_7VariantE");

TypedValue* fg_mysql_errno(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_error(HPHP::Variant const&)
_ZN4HPHP13f_mysql_errorERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_error(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP13f_mysql_errorERKNS_7VariantE");

TypedValue* fg_mysql_error(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_warning_count(HPHP::Variant const&)
_ZN4HPHP21f_mysql_warning_countERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_warning_count(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP21f_mysql_warning_countERKNS_7VariantE");

TypedValue* fg_mysql_warning_count(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_mysql_set_timeout(int, HPHP::Variant const&)
_ZN4HPHP19f_mysql_set_timeoutEiRKNS_7VariantE

(return value) => rax
query_timeout_ms => rdi
link_identifier => rsi
*/

bool fh_mysql_set_timeout(int query_timeout_ms, TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_set_timeoutEiRKNS_7VariantE");

TypedValue * fg1_mysql_set_timeout(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_set_timeout(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-0);
  Variant defVal1;
  rv->m_data.num = (fh_mysql_set_timeout((count > 0) ? (int)(args[-0].m_data.num) : (int)(-1), (count > 1) ? (args-1) : (TypedValue*)(&defVal1))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mysql_set_timeout(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfInt64)) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_query((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_query(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_query((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_multi_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_multi_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_multi_query((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_multi_query(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_multi_query((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_mysql_next_result(HPHP::Variant const&)
_ZN4HPHP19f_mysql_next_resultERKNS_7VariantE

(return value) => rax
link_identifier => rdi
*/

bool fh_mysql_next_result(TypedValue* link_identifier) asm("_ZN4HPHP19f_mysql_next_resultERKNS_7VariantE");

TypedValue* fg_mysql_next_result(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_mysql_more_results(HPHP::Variant const&)
_ZN4HPHP20f_mysql_more_resultsERKNS_7VariantE

(return value) => rax
link_identifier => rdi
*/

bool fh_mysql_more_results(TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_more_resultsERKNS_7VariantE");

TypedValue* fg_mysql_more_results(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_fetch_result(HPHP::Variant const&)
_ZN4HPHP20f_mysql_fetch_resultERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_fetch_result(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP20f_mysql_fetch_resultERKNS_7VariantE");

TypedValue* fg_mysql_fetch_result(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_unbuffered_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_unbuffered_query(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_unbuffered_query((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_unbuffered_query(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_unbuffered_query((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_list_dbs(HPHP::Variant const&)
_ZN4HPHP16f_mysql_list_dbsERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_list_dbs(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP16f_mysql_list_dbsERKNS_7VariantE");

TypedValue* fg_mysql_list_dbs(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_list_tables(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_list_tables(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  Variant defVal1;
  fh_mysql_list_tables((rv), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_list_tables(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        Variant defVal1;
        fh_mysql_list_tables((&(rv)), (Value*)(args-0), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_list_processes(HPHP::Variant const&)
_ZN4HPHP22f_mysql_list_processesERKNS_7VariantE

(return value) => rax
_rv => rdi
link_identifier => rsi
*/

TypedValue* fh_mysql_list_processes(TypedValue* _rv, TypedValue* link_identifier) asm("_ZN4HPHP22f_mysql_list_processesERKNS_7VariantE");

TypedValue* fg_mysql_list_processes(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_num_fields(HPHP::Variant const&)
_ZN4HPHP18f_mysql_num_fieldsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_num_fields(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP18f_mysql_num_fieldsERKNS_7VariantE");

TypedValue* fg_mysql_num_fields(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_num_rows(HPHP::Variant const&)
_ZN4HPHP16f_mysql_num_rowsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_num_rows(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP16f_mysql_num_rowsERKNS_7VariantE");

TypedValue* fg_mysql_num_rows(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_free_result(HPHP::Variant const&)
_ZN4HPHP19f_mysql_free_resultERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_free_result(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP19f_mysql_free_resultERKNS_7VariantE");

TypedValue* fg_mysql_free_result(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_mysql_data_seek(HPHP::Variant const&, int)
_ZN4HPHP17f_mysql_data_seekERKNS_7VariantEi

(return value) => rax
result => rdi
row => rsi
*/

bool fh_mysql_data_seek(TypedValue* result, int row) asm("_ZN4HPHP17f_mysql_data_seekERKNS_7VariantEi");

TypedValue * fg1_mysql_data_seek(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_data_seek(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (fh_mysql_data_seek((args-0), (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mysql_data_seek(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_fetch_row(HPHP::Variant const&)
_ZN4HPHP17f_mysql_fetch_rowERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_row(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP17f_mysql_fetch_rowERKNS_7VariantE");

TypedValue* fg_mysql_fetch_row(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_fetch_assoc(HPHP::Variant const&)
_ZN4HPHP19f_mysql_fetch_assocERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_assoc(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP19f_mysql_fetch_assocERKNS_7VariantE");

TypedValue* fg_mysql_fetch_assoc(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_fetch_array(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_fetch_array(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_fetch_array((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(3));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_fetch_array(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
HPHP::Variant HPHP::f_mysql_fetch_lengths(HPHP::Variant const&)
_ZN4HPHP21f_mysql_fetch_lengthsERKNS_7VariantE

(return value) => rax
_rv => rdi
result => rsi
*/

TypedValue* fh_mysql_fetch_lengths(TypedValue* _rv, TypedValue* result) asm("_ZN4HPHP21f_mysql_fetch_lengthsERKNS_7VariantE");

TypedValue* fg_mysql_fetch_lengths(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_fetch_object(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_fetch_object(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
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
  Array defVal2 = null;
  fh_mysql_fetch_object((rv), (args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_fetch_object(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfArray) && (count <= 1 || IS_STRING_TYPE((args-1)->m_type))) {
        String defVal1 = "stdClass";
        Array defVal2 = null;
        fh_mysql_fetch_object((&(rv)), (args-0), (count > 1) ? (Value*)(args-1) : (Value*)(&defVal1), (count > 2) ? (Value*)(args-2) : (Value*)(&defVal2));
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_result(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_result(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_result((rv), (args-0), (int)(args[-1].m_data.num), (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_result(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_fetch_field(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_fetch_field(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_fetch_field((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(-1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_fetch_field(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}



/*
bool HPHP::f_mysql_field_seek(HPHP::Variant const&, int)
_ZN4HPHP18f_mysql_field_seekERKNS_7VariantEi

(return value) => rax
result => rdi
field => rsi
*/

bool fh_mysql_field_seek(TypedValue* result, int field) asm("_ZN4HPHP18f_mysql_field_seekERKNS_7VariantEi");

TypedValue * fg1_mysql_field_seek(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_seek(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->_count = 0;
  rv->m_type = KindOfBoolean;
  tvCastToInt64InPlace(args-1);
  rv->m_data.num = (fh_mysql_field_seek((args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_mysql_field_seek(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64)) {
        rv._count = 0;
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_field_name(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_name(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_name((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_name(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_field_table(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_table(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_table((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_table(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_field_len(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_len(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_len((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_len(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_field_type(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_type(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_type((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_type(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
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

TypedValue * fg1_mysql_field_flags(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) __attribute__((noinline,cold));
TypedValue * fg1_mysql_field_flags(TypedValue* rv, HPHP::VM::ActRec* ar, long long count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-1);
  fh_mysql_field_flags((rv), (args-0), (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_mysql_field_flags(HPHP::VM::ActRec *ar) {
  EXCEPTION_GATE_ENTER();
    TypedValue rv;
    long long count = ar->numArgs();
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
    rv._count = 0;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  EXCEPTION_GATE_RETURN(&ar->m_r);
}




} // !HPHP

