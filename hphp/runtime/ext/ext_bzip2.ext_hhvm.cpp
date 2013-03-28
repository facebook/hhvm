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
HPHP::Variant HPHP::f_bzclose(HPHP::Object const&)
_ZN4HPHP9f_bzcloseERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

TypedValue* fh_bzclose(TypedValue* _rv, Value* bz) asm("_ZN4HPHP9f_bzcloseERKNS_6ObjectE");

TypedValue * fg1_bzclose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzclose(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_bzclose((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzclose(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_bzclose((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzclose(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzclose", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_bzopen(HPHP::String const&, HPHP::String const&)
_ZN4HPHP8f_bzopenERKNS_6StringES2_

(return value) => rax
_rv => rdi
filename => rsi
mode => rdx
*/

TypedValue* fh_bzopen(TypedValue* _rv, Value* filename, Value* mode) asm("_ZN4HPHP8f_bzopenERKNS_6StringES2_");

TypedValue * fg1_bzopen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzopen(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_bzopen((rv), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzopen(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_bzopen((&(rv)), &args[-0].m_data, &args[-1].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzopen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzopen", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_bzread(HPHP::Object const&, int)
_ZN4HPHP8f_bzreadERKNS_6ObjectEi

(return value) => rax
_rv => rdi
bz => rsi
length => rdx
*/

TypedValue* fh_bzread(TypedValue* _rv, Value* bz, int length) asm("_ZN4HPHP8f_bzreadERKNS_6ObjectEi");

TypedValue * fg1_bzread(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzread(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_bzread((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(1024));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzread(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_bzread((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(1024));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzread(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzread", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_bzwrite(HPHP::Object const&, HPHP::String const&, int)
_ZN4HPHP9f_bzwriteERKNS_6ObjectERKNS_6StringEi

(return value) => rax
_rv => rdi
bz => rsi
data => rdx
length => rcx
*/

TypedValue* fh_bzwrite(TypedValue* _rv, Value* bz, Value* data, int length) asm("_ZN4HPHP9f_bzwriteERKNS_6ObjectERKNS_6StringEi");

TypedValue * fg1_bzwrite(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzwrite(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  fh_bzwrite((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzwrite(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_bzwrite((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzwrite(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzwrite", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_bzflush(HPHP::Object const&)
_ZN4HPHP9f_bzflushERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

TypedValue* fh_bzflush(TypedValue* _rv, Value* bz) asm("_ZN4HPHP9f_bzflushERKNS_6ObjectE");

TypedValue * fg1_bzflush(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzflush(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_bzflush((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzflush(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_bzflush((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzflush(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzflush", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_bzerrstr(HPHP::Object const&)
_ZN4HPHP10f_bzerrstrERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

Value* fh_bzerrstr(Value* _rv, Value* bz) asm("_ZN4HPHP10f_bzerrstrERKNS_6ObjectE");

TypedValue * fg1_bzerrstr(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzerrstr(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  tvCastToObjectInPlace(args-0);
  fh_bzerrstr((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzerrstr(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfString;
        fh_bzerrstr((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzerrstr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzerrstr", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_bzerror(HPHP::Object const&)
_ZN4HPHP9f_bzerrorERKNS_6ObjectE

(return value) => rax
_rv => rdi
bz => rsi
*/

TypedValue* fh_bzerror(TypedValue* _rv, Value* bz) asm("_ZN4HPHP9f_bzerrorERKNS_6ObjectE");

TypedValue * fg1_bzerror(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzerror(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_bzerror((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzerror(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_bzerror((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzerror(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzerror", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_bzerrno(HPHP::Object const&)
_ZN4HPHP9f_bzerrnoERKNS_6ObjectE

(return value) => rax
bz => rdi
*/

long fh_bzerrno(Value* bz) asm("_ZN4HPHP9f_bzerrnoERKNS_6ObjectE");

TypedValue * fg1_bzerrno(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzerrno(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (int64_t)fh_bzerrno(&args[-0].m_data);
  return rv;
}

TypedValue* fg_bzerrno(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_bzerrno(&args[-0].m_data);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzerrno(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzerrno", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_bzcompress(HPHP::String const&, int, int)
_ZN4HPHP12f_bzcompressERKNS_6StringEii

(return value) => rax
_rv => rdi
source => rsi
blocksize => rdx
workfactor => rcx
*/

TypedValue* fh_bzcompress(TypedValue* _rv, Value* source, int blocksize, int workfactor) asm("_ZN4HPHP12f_bzcompressERKNS_6StringEii");

TypedValue * fg1_bzcompress(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzcompress(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_bzcompress((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(4), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzcompress(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_bzcompress((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(4), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzcompress(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzcompress", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_bzdecompress(HPHP::String const&, int)
_ZN4HPHP14f_bzdecompressERKNS_6StringEi

(return value) => rax
_rv => rdi
source => rsi
small => rdx
*/

TypedValue* fh_bzdecompress(TypedValue* _rv, Value* source, int small) asm("_ZN4HPHP14f_bzdecompressERKNS_6StringEi");

TypedValue * fg1_bzdecompress(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_bzdecompress(TypedValue* rv, HPHP::VM::ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_bzdecompress((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_bzdecompress(HPHP::VM::ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_bzdecompress((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_bzdecompress(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("bzdecompress", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

