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
HPHP::Object HPHP::f_stream_context_create(HPHP::Array const&, HPHP::Array const&)
_ZN4HPHP23f_stream_context_createERKNS_5ArrayES2_

(return value) => rax
_rv => rdi
options => rsi
params => rdx
*/

Value* fh_stream_context_create(Value* _rv, Value* options, Value* params) asm("_ZN4HPHP23f_stream_context_createERKNS_5ArrayES2_");

TypedValue * fg1_stream_context_create(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_create(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-1);
    }
  case 1:
    if ((args-0)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-0);
    }
  case 0:
    break;
  }
  fh_stream_context_create((&rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_array), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_context_create(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfArray) && (count <= 0 || (args-0)->m_type == KindOfArray)) {
        rv.m_type = KindOfObject;
        fh_stream_context_create((&rv.m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_array), (count > 1) ? &args[-1].m_data : (Value*)(&null_array));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_create(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("stream_context_create", 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_stream_context_get_default(HPHP::Array const&)
_ZN4HPHP28f_stream_context_get_defaultERKNS_5ArrayE

(return value) => rax
_rv => rdi
options => rsi
*/

Value* fh_stream_context_get_default(Value* _rv, Value* options) asm("_ZN4HPHP28f_stream_context_get_defaultERKNS_5ArrayE");

TypedValue * fg1_stream_context_get_default(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_get_default(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToArrayInPlace(args-0);
  fh_stream_context_get_default((&rv->m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_array));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_context_get_default(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || (args-0)->m_type == KindOfArray)) {
        rv.m_type = KindOfObject;
        fh_stream_context_get_default((&rv.m_data), (count > 0) ? &args[-0].m_data : (Value*)(&null_array));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_get_default(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("stream_context_get_default", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_context_get_options(HPHP::Object const&)
_ZN4HPHP28f_stream_context_get_optionsERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream_or_context => rsi
*/

TypedValue* fh_stream_context_get_options(TypedValue* _rv, Value* stream_or_context) asm("_ZN4HPHP28f_stream_context_get_optionsERKNS_6ObjectE");

TypedValue * fg1_stream_context_get_options(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_get_options(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_stream_context_get_options((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_context_get_options(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_stream_context_get_options((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_get_options(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_context_get_options", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_context_set_option(HPHP::Object const&, HPHP::Variant const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP27f_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_

(return value) => rax
stream_or_context => rdi
wrapper => rsi
option => rdx
value => rcx
*/

bool fh_stream_context_set_option(Value* stream_or_context, TypedValue* wrapper, Value* option, TypedValue* value) asm("_ZN4HPHP27f_stream_context_set_optionERKNS_6ObjectERKNS_7VariantERKNS_6StringES5_");

TypedValue * fg1_stream_context_set_option(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_set_option(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 4
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_context_set_option(&args[-0].m_data, (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_context_set_option(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_context_set_option(&args[-0].m_data, (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_set_option(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_context_set_option", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_context_set_param(HPHP::Object const&, HPHP::Array const&)
_ZN4HPHP26f_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE

(return value) => rax
stream_or_context => rdi
params => rsi
*/

bool fh_stream_context_set_param(Value* stream_or_context, Value* params) asm("_ZN4HPHP26f_stream_context_set_paramERKNS_6ObjectERKNS_5ArrayE");

TypedValue * fg1_stream_context_set_param(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_context_set_param(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_context_set_param(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_context_set_param(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfArray && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_context_set_param(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_context_set_param(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_context_set_param", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_copy_to_stream(HPHP::Object const&, HPHP::Object const&, int, int)
_ZN4HPHP23f_stream_copy_to_streamERKNS_6ObjectES2_ii

(return value) => rax
_rv => rdi
source => rsi
dest => rdx
maxlength => rcx
offset => r8
*/

TypedValue* fh_stream_copy_to_stream(TypedValue* _rv, Value* source, Value* dest, int maxlength, int offset) asm("_ZN4HPHP23f_stream_copy_to_streamERKNS_6ObjectES2_ii");

TypedValue * fg1_stream_copy_to_stream(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_copy_to_stream(TypedValue* rv, ActRec* ar, int64_t count) {
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
    break;
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_copy_to_stream((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_copy_to_stream(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfInt64) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        fh_stream_copy_to_stream((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(-1), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_copy_to_stream(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_copy_to_stream", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_encoding(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP17f_stream_encodingERKNS_6ObjectERKNS_6StringE

(return value) => rax
stream => rdi
encoding => rsi
*/

bool fh_stream_encoding(Value* stream, Value* encoding) asm("_ZN4HPHP17f_stream_encodingERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_stream_encoding(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_encoding(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  switch (count) {
  default: // count >= 2
    if (!IS_STRING_TYPE((args-1)->m_type)) {
      tvCastToStringInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_encoding(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_encoding(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_encoding(&args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_encoding(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_encoding", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_stream_bucket_append(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP22f_stream_bucket_appendERKNS_6ObjectES2_

brigade => rdi
bucket => rsi
*/

void fh_stream_bucket_append(Value* brigade, Value* bucket) asm("_ZN4HPHP22f_stream_bucket_appendERKNS_6ObjectES2_");

TypedValue * fg1_stream_bucket_append(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_bucket_append(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_bucket_append(&args[-0].m_data, &args[-1].m_data);
  return rv;
}

TypedValue* fg_stream_bucket_append(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_stream_bucket_append(&args[-0].m_data, &args[-1].m_data);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_bucket_append(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_bucket_append", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
void HPHP::f_stream_bucket_prepend(HPHP::Object const&, HPHP::Object const&)
_ZN4HPHP23f_stream_bucket_prependERKNS_6ObjectES2_

brigade => rdi
bucket => rsi
*/

void fh_stream_bucket_prepend(Value* brigade, Value* bucket) asm("_ZN4HPHP23f_stream_bucket_prependERKNS_6ObjectES2_");

TypedValue * fg1_stream_bucket_prepend(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_bucket_prepend(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_data.num = 0LL;
  rv->m_type = KindOfNull;
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_bucket_prepend(&args[-0].m_data, &args[-1].m_data);
  return rv;
}

TypedValue* fg_stream_bucket_prepend(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfObject && (args-0)->m_type == KindOfObject) {
        rv.m_data.num = 0LL;
        rv.m_type = KindOfNull;
        fh_stream_bucket_prepend(&args[-0].m_data, &args[-1].m_data);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_bucket_prepend(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_bucket_prepend", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_stream_bucket_make_writeable(HPHP::Object const&)
_ZN4HPHP30f_stream_bucket_make_writeableERKNS_6ObjectE

(return value) => rax
_rv => rdi
brigade => rsi
*/

Value* fh_stream_bucket_make_writeable(Value* _rv, Value* brigade) asm("_ZN4HPHP30f_stream_bucket_make_writeableERKNS_6ObjectE");

TypedValue * fg1_stream_bucket_make_writeable(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_bucket_make_writeable(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  tvCastToObjectInPlace(args-0);
  fh_stream_bucket_make_writeable((&rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_bucket_make_writeable(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_stream_bucket_make_writeable((&rv.m_data), &args[-0].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_bucket_make_writeable(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_bucket_make_writeable", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_stream_bucket_new(HPHP::Object const&, HPHP::String const&)
_ZN4HPHP19f_stream_bucket_newERKNS_6ObjectERKNS_6StringE

(return value) => rax
_rv => rdi
stream => rsi
buffer => rdx
*/

Value* fh_stream_bucket_new(Value* _rv, Value* stream, Value* buffer) asm("_ZN4HPHP19f_stream_bucket_newERKNS_6ObjectERKNS_6StringE");

TypedValue * fg1_stream_bucket_new(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_bucket_new(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_bucket_new((&rv->m_data), &args[-0].m_data, &args[-1].m_data);
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_bucket_new(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_stream_bucket_new((&rv.m_data), &args[-0].m_data, &args[-1].m_data);
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_bucket_new(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_bucket_new", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_filter_register(HPHP::String const&, HPHP::String const&)
_ZN4HPHP24f_stream_filter_registerERKNS_6StringES2_

(return value) => rax
filtername => rdi
classname => rsi
*/

bool fh_stream_filter_register(Value* filtername, Value* classname) asm("_ZN4HPHP24f_stream_filter_registerERKNS_6StringES2_");

TypedValue * fg1_stream_filter_register(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_filter_register(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_filter_register(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_filter_register(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_filter_register(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_filter_register(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_filter_register", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_filter_remove(HPHP::Object const&)
_ZN4HPHP22f_stream_filter_removeERKNS_6ObjectE

(return value) => rax
stream_filter => rdi
*/

bool fh_stream_filter_remove(Value* stream_filter) asm("_ZN4HPHP22f_stream_filter_removeERKNS_6ObjectE");

TypedValue * fg1_stream_filter_remove(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_filter_remove(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToObjectInPlace(args-0);
  rv->m_data.num = (fh_stream_filter_remove(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_filter_remove(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_filter_remove(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_filter_remove(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_filter_remove", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_stream_filter_append(HPHP::Object const&, HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP22f_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
stream => rsi
filtername => rdx
read_write => rcx
params => r8
*/

Value* fh_stream_filter_append(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP22f_stream_filter_appendERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

TypedValue * fg1_stream_filter_append(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_filter_append(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 4
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_filter_append((&rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_filter_append(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_stream_filter_append((&rv.m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_filter_append(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_filter_append", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Object HPHP::f_stream_filter_prepend(HPHP::Object const&, HPHP::String const&, int, HPHP::Variant const&)
_ZN4HPHP23f_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE

(return value) => rax
_rv => rdi
stream => rsi
filtername => rdx
read_write => rcx
params => r8
*/

Value* fh_stream_filter_prepend(Value* _rv, Value* stream, Value* filtername, int read_write, TypedValue* params) asm("_ZN4HPHP23f_stream_filter_prependERKNS_6ObjectERKNS_6StringEiRKNS_7VariantE");

TypedValue * fg1_stream_filter_prepend(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_filter_prepend(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfObject;
  switch (count) {
  default: // count >= 4
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_filter_prepend((&rv->m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
  if (rv->m_data.num == 0LL)rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_filter_prepend(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfObject;
        fh_stream_filter_prepend((&rv.m_data), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_filter_prepend(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_filter_prepend", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_get_contents(HPHP::Object const&, int, int)
_ZN4HPHP21f_stream_get_contentsERKNS_6ObjectEii

(return value) => rax
_rv => rdi
handle => rsi
maxlen => rdx
offset => rcx
*/

TypedValue* fh_stream_get_contents(TypedValue* _rv, Value* handle, int maxlen, int offset) asm("_ZN4HPHP21f_stream_get_contentsERKNS_6ObjectEii");

TypedValue * fg1_stream_get_contents(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_get_contents(TypedValue* rv, ActRec* ar, int64_t count) {
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_get_contents((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_get_contents(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_stream_get_contents((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_get_contents(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_get_contents", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_stream_get_filters()
_ZN4HPHP20f_stream_get_filtersEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_filters(Value* _rv) asm("_ZN4HPHP20f_stream_get_filtersEv");

TypedValue* fg_stream_get_filters(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_stream_get_filters((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("stream_get_filters", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_get_line(HPHP::Object const&, int, HPHP::String const&)
_ZN4HPHP17f_stream_get_lineERKNS_6ObjectEiRKNS_6StringE

(return value) => rax
_rv => rdi
handle => rsi
length => rdx
ending => rcx
*/

TypedValue* fh_stream_get_line(TypedValue* _rv, Value* handle, int length, Value* ending) asm("_ZN4HPHP17f_stream_get_lineERKNS_6ObjectEiRKNS_6StringE");

TypedValue * fg1_stream_get_line(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_get_line(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    if ((args-1)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_get_line((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_get_line(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && (args-0)->m_type == KindOfObject) {
        fh_stream_get_line((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_get_line(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_get_line", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_get_meta_data(HPHP::Object const&)
_ZN4HPHP22f_stream_get_meta_dataERKNS_6ObjectE

(return value) => rax
_rv => rdi
stream => rsi
*/

TypedValue* fh_stream_get_meta_data(TypedValue* _rv, Value* stream) asm("_ZN4HPHP22f_stream_get_meta_dataERKNS_6ObjectE");

TypedValue * fg1_stream_get_meta_data(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_get_meta_data(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_stream_get_meta_data((rv), &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_get_meta_data(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if ((args-0)->m_type == KindOfObject) {
        fh_stream_get_meta_data((&(rv)), &args[-0].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_get_meta_data(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_get_meta_data", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_stream_get_transports()
_ZN4HPHP23f_stream_get_transportsEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_transports(Value* _rv) asm("_ZN4HPHP23f_stream_get_transportsEv");

TypedValue* fg_stream_get_transports(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_stream_get_transports((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("stream_get_transports", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Array HPHP::f_stream_get_wrappers()
_ZN4HPHP21f_stream_get_wrappersEv

(return value) => rax
_rv => rdi
*/

Value* fh_stream_get_wrappers(Value* _rv) asm("_ZN4HPHP21f_stream_get_wrappersEv");

TypedValue* fg_stream_get_wrappers(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 0LL) {
      rv.m_type = KindOfArray;
      fh_stream_get_wrappers((&rv.m_data));
      if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
      frame_free_locals_no_this_inl(ar, 0);
      memcpy(&ar->m_r, &rv, sizeof(TypedValue));
      return &ar->m_r;
    } else {
      throw_toomany_arguments_nr("stream_get_wrappers", 0, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 0);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_register_wrapper(HPHP::String const&, HPHP::String const&)
_ZN4HPHP25f_stream_register_wrapperERKNS_6StringES2_

(return value) => rax
protocol => rdi
classname => rsi
*/

bool fh_stream_register_wrapper(Value* protocol, Value* classname) asm("_ZN4HPHP25f_stream_register_wrapperERKNS_6StringES2_");

TypedValue * fg1_stream_register_wrapper(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_register_wrapper(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_register_wrapper(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_register_wrapper(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_register_wrapper(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_register_wrapper(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_register_wrapper", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_wrapper_register(HPHP::String const&, HPHP::String const&)
_ZN4HPHP25f_stream_wrapper_registerERKNS_6StringES2_

(return value) => rax
protocol => rdi
classname => rsi
*/

bool fh_stream_wrapper_register(Value* protocol, Value* classname) asm("_ZN4HPHP25f_stream_wrapper_registerERKNS_6StringES2_");

TypedValue * fg1_stream_wrapper_register(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_wrapper_register(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_wrapper_register(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_wrapper_register(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_wrapper_register(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_wrapper_register(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_wrapper_register", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_wrapper_restore(HPHP::String const&)
_ZN4HPHP24f_stream_wrapper_restoreERKNS_6StringE

(return value) => rax
protocol => rdi
*/

bool fh_stream_wrapper_restore(Value* protocol) asm("_ZN4HPHP24f_stream_wrapper_restoreERKNS_6StringE");

TypedValue * fg1_stream_wrapper_restore(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_wrapper_restore(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_stream_wrapper_restore(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_wrapper_restore(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_wrapper_restore(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_wrapper_restore(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_wrapper_restore", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_wrapper_unregister(HPHP::String const&)
_ZN4HPHP27f_stream_wrapper_unregisterERKNS_6StringE

(return value) => rax
protocol => rdi
*/

bool fh_stream_wrapper_unregister(Value* protocol) asm("_ZN4HPHP27f_stream_wrapper_unregisterERKNS_6StringE");

TypedValue * fg1_stream_wrapper_unregister(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_wrapper_unregister(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  tvCastToStringInPlace(args-0);
  rv->m_data.num = (fh_stream_wrapper_unregister(&args[-0].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_wrapper_unregister(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 1LL) {
      if (IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_wrapper_unregister(&args[-0].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_wrapper_unregister(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_wrapper_unregister", count, 1, 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_stream_resolve_include_path(HPHP::String const&, HPHP::Object const&)
_ZN4HPHP29f_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE

(return value) => rax
_rv => rdi
filename => rsi
context => rdx
*/

Value* fh_stream_resolve_include_path(Value* _rv, Value* filename, Value* context) asm("_ZN4HPHP29f_stream_resolve_include_pathERKNS_6StringERKNS_6ObjectE");

TypedValue * fg1_stream_resolve_include_path(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_resolve_include_path(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  switch (count) {
  default: // count >= 2
    if ((args-1)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-1);
    }
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_stream_resolve_include_path((&rv->m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_resolve_include_path(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfObject) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfString;
        fh_stream_resolve_include_path((&rv.m_data), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_object));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_resolve_include_path(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_resolve_include_path", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_select(HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, HPHP::Variant const&, int)
_ZN4HPHP15f_stream_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi

(return value) => rax
_rv => rdi
read => rsi
write => rdx
except => rcx
vtv_sec => r8
tv_usec => r9
*/

TypedValue* fh_stream_select(TypedValue* _rv, TypedValue* read, TypedValue* write, TypedValue* except, TypedValue* vtv_sec, int tv_usec) asm("_ZN4HPHP15f_stream_selectERKNS_14VRefParamValueES2_S2_RKNS_7VariantEi");

TypedValue * fg1_stream_select(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_select(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-4);
  fh_stream_select((rv), (args-0), (args-1), (args-2), (args-3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_select(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 4LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfInt64)) {
        fh_stream_select((&(rv)), (args-0), (args-1), (args-2), (args-3), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_select(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_select", count, 4, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_set_blocking(HPHP::Object const&, int)
_ZN4HPHP21f_stream_set_blockingERKNS_6ObjectEi

(return value) => rax
stream => rdi
mode => rsi
*/

bool fh_stream_set_blocking(Value* stream, int mode) asm("_ZN4HPHP21f_stream_set_blockingERKNS_6ObjectEi");

TypedValue * fg1_stream_set_blocking(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_set_blocking(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_set_blocking(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_set_blocking(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_set_blocking(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_set_blocking(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_set_blocking", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_set_timeout(HPHP::Object const&, int, int)
_ZN4HPHP20f_stream_set_timeoutERKNS_6ObjectEii

(return value) => rax
stream => rdi
seconds => rsi
microseconds => rdx
*/

bool fh_stream_set_timeout(Value* stream, int seconds, int microseconds) asm("_ZN4HPHP20f_stream_set_timeoutERKNS_6ObjectEii");

TypedValue * fg1_stream_set_timeout(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_set_timeout(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
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
  rv->m_data.num = (fh_stream_set_timeout(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_set_timeout(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_set_timeout(&args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_set_timeout(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_set_timeout", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_stream_set_write_buffer(HPHP::Object const&, int)
_ZN4HPHP25f_stream_set_write_bufferERKNS_6ObjectEi

(return value) => rax
stream => rdi
buffer => rsi
*/

long fh_stream_set_write_buffer(Value* stream, int buffer) asm("_ZN4HPHP25f_stream_set_write_bufferERKNS_6ObjectEi");

TypedValue * fg1_stream_set_write_buffer(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_set_write_buffer(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_stream_set_write_buffer(&args[-0].m_data, (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_stream_set_write_buffer(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_stream_set_write_buffer(&args[-0].m_data, (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_set_write_buffer(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_set_write_buffer", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
long HPHP::f_set_file_buffer(HPHP::Object const&, int)
_ZN4HPHP17f_set_file_bufferERKNS_6ObjectEi

(return value) => rax
stream => rdi
buffer => rsi
*/

long fh_set_file_buffer(Value* stream, int buffer) asm("_ZN4HPHP17f_set_file_bufferERKNS_6ObjectEi");

TypedValue * fg1_set_file_buffer(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_set_file_buffer(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfInt64;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (int64_t)fh_set_file_buffer(&args[-0].m_data, (int)(args[-1].m_data.num));
  return rv;
}

TypedValue* fg_set_file_buffer(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfInt64;
        rv.m_data.num = (int64_t)fh_set_file_buffer(&args[-0].m_data, (int)(args[-1].m_data.num));
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_set_file_buffer(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("set_file_buffer", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_socket_accept(HPHP::Object const&, double, HPHP::VRefParamValue const&)
_ZN4HPHP22f_stream_socket_acceptERKNS_6ObjectEdRKNS_14VRefParamValueE

(return value) => rax
_rv => rdi
server_socket => rsi
timeout => xmm0
peername => rdx
*/

TypedValue* fh_stream_socket_accept(TypedValue* _rv, Value* server_socket, double timeout, TypedValue* peername) asm("_ZN4HPHP22f_stream_socket_acceptERKNS_6ObjectEdRKNS_14VRefParamValueE");

TypedValue * fg1_stream_socket_accept(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_accept(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
  case 2:
    if ((args-1)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-1);
    }
  case 1:
    break;
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  VRefParamValue defVal2 = uninit_null();
  fh_stream_socket_accept((rv), &args[-0].m_data, (count > 1) ? (args[-1].m_data.dbl) : (double)(0.0), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_accept(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 1 || (args-1)->m_type == KindOfDouble) && (args-0)->m_type == KindOfObject) {
        VRefParamValue defVal2 = uninit_null();
        fh_stream_socket_accept((&(rv)), &args[-0].m_data, (count > 1) ? (args[-1].m_data.dbl) : (double)(0.0), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_accept(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_accept", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_socket_server(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, int, HPHP::Object const&)
_ZN4HPHP22f_stream_socket_serverERKNS_6StringERKNS_14VRefParamValueES5_iRKNS_6ObjectE

(return value) => rax
_rv => rdi
local_socket => rsi
errnum => rdx
errstr => rcx
flags => r8
context => r9
*/

TypedValue* fh_stream_socket_server(TypedValue* _rv, Value* local_socket, TypedValue* errnum, TypedValue* errstr, int flags, Value* context) asm("_ZN4HPHP22f_stream_socket_serverERKNS_6StringERKNS_14VRefParamValueES5_iRKNS_6ObjectE");

TypedValue * fg1_stream_socket_server(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_server(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if ((args-4)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
  case 2:
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal1 = uninit_null();
  VRefParamValue defVal2 = uninit_null();
  fh_stream_socket_server((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? &args[-4].m_data : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_server(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 5LL) {
      if ((count <= 4 || (args-4)->m_type == KindOfObject) && (count <= 3 || (args-3)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        VRefParamValue defVal1 = uninit_null();
        VRefParamValue defVal2 = uninit_null();
        fh_stream_socket_server((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? &args[-4].m_data : (Value*)(&null_object));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_server(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 5);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_server", count, 1, 5, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 5);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_socket_client(HPHP::String const&, HPHP::VRefParamValue const&, HPHP::VRefParamValue const&, double, int, HPHP::Object const&)
_ZN4HPHP22f_stream_socket_clientERKNS_6StringERKNS_14VRefParamValueES5_diRKNS_6ObjectE

(return value) => rax
_rv => rdi
remote_socket => rsi
errnum => rdx
errstr => rcx
timeout => xmm0
flags => r8
context => r9
*/

TypedValue* fh_stream_socket_client(TypedValue* _rv, Value* remote_socket, TypedValue* errnum, TypedValue* errstr, double timeout, int flags, Value* context) asm("_ZN4HPHP22f_stream_socket_clientERKNS_6StringERKNS_14VRefParamValueES5_diRKNS_6ObjectE");

TypedValue * fg1_stream_socket_client(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_client(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-5);
    }
  case 5:
    if ((args-4)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfDouble) {
      tvCastToDoubleInPlace(args-3);
    }
  case 3:
  case 2:
  case 1:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  VRefParamValue defVal1 = uninit_null();
  VRefParamValue defVal2 = uninit_null();
  fh_stream_socket_client((rv), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? &args[-5].m_data : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_client(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 6LL) {
      if ((count <= 5 || (args-5)->m_type == KindOfObject) && (count <= 4 || (args-4)->m_type == KindOfInt64) && (count <= 3 || (args-3)->m_type == KindOfDouble) && IS_STRING_TYPE((args-0)->m_type)) {
        VRefParamValue defVal1 = uninit_null();
        VRefParamValue defVal2 = uninit_null();
        fh_stream_socket_client((&(rv)), &args[-0].m_data, (count > 1) ? (args-1) : (TypedValue*)(&defVal1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2), (count > 3) ? (args[-3].m_data.dbl) : (double)(0.0), (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? &args[-5].m_data : (Value*)(&null_object));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_client(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 6);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_client", count, 1, 6, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 6);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_socket_enable_crypto(HPHP::Object const&, bool, int, HPHP::Object const&)
_ZN4HPHP29f_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_

(return value) => rax
_rv => rdi
stream => rsi
enable => rdx
crypto_type => rcx
session_stream => r8
*/

TypedValue* fh_stream_socket_enable_crypto(TypedValue* _rv, Value* stream, bool enable, int crypto_type, Value* session_stream) asm("_ZN4HPHP29f_stream_socket_enable_cryptoERKNS_6ObjectEbiS2_");

TypedValue * fg1_stream_socket_enable_crypto(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_enable_crypto(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfObject) {
      tvCastToObjectInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_socket_enable_crypto((rv), &args[-0].m_data, (bool)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_object));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_enable_crypto(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || (args-3)->m_type == KindOfObject) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        fh_stream_socket_enable_crypto((&(rv)), &args[-0].m_data, (bool)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_object));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_enable_crypto(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_enable_crypto", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_socket_get_name(HPHP::Object const&, bool)
_ZN4HPHP24f_stream_socket_get_nameERKNS_6ObjectEb

(return value) => rax
_rv => rdi
handle => rsi
want_peer => rdx
*/

TypedValue* fh_stream_socket_get_name(TypedValue* _rv, Value* handle, bool want_peer) asm("_ZN4HPHP24f_stream_socket_get_nameERKNS_6ObjectEb");

TypedValue * fg1_stream_socket_get_name(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_get_name(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-1)->m_type != KindOfBoolean) {
    tvCastToBooleanInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_socket_get_name((rv), &args[-0].m_data, (bool)(args[-1].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_get_name(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfBoolean && (args-0)->m_type == KindOfObject) {
        fh_stream_socket_get_name((&(rv)), &args[-0].m_data, (bool)(args[-1].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_get_name(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_get_name", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_socket_pair(int, int, int)
_ZN4HPHP20f_stream_socket_pairEiii

(return value) => rax
_rv => rdi
domain => rsi
type => rdx
protocol => rcx
*/

TypedValue* fh_stream_socket_pair(TypedValue* _rv, int domain, int type, int protocol) asm("_ZN4HPHP20f_stream_socket_pairEiii");

TypedValue * fg1_stream_socket_pair(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_pair(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_stream_socket_pair((rv), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_pair(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if ((args-2)->m_type == KindOfInt64 && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfInt64) {
        fh_stream_socket_pair((&(rv)), (int)(args[-0].m_data.num), (int)(args[-1].m_data.num), (int)(args[-2].m_data.num));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_pair(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_pair", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_socket_recvfrom(HPHP::Object const&, int, int, HPHP::String const&)
_ZN4HPHP24f_stream_socket_recvfromERKNS_6ObjectEiiRKNS_6StringE

(return value) => rax
_rv => rdi
socket => rsi
length => rdx
flags => rcx
address => r8
*/

TypedValue* fh_stream_socket_recvfrom(TypedValue* _rv, Value* socket, int length, int flags, Value* address) asm("_ZN4HPHP24f_stream_socket_recvfromERKNS_6ObjectEiiRKNS_6StringE");

TypedValue * fg1_stream_socket_recvfrom(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_recvfrom(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
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
  fh_stream_socket_recvfrom((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_recvfrom(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        fh_stream_socket_recvfrom((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_recvfrom(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_recvfrom", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_stream_socket_sendto(HPHP::Object const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP22f_stream_socket_sendtoERKNS_6ObjectERKNS_6StringEiS5_

(return value) => rax
_rv => rdi
socket => rsi
data => rdx
flags => rcx
address => r8
*/

TypedValue* fh_stream_socket_sendto(TypedValue* _rv, Value* socket, Value* data, int flags, Value* address) asm("_ZN4HPHP22f_stream_socket_sendtoERKNS_6ObjectERKNS_6StringEiS5_");

TypedValue * fg1_stream_socket_sendto(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_sendto(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
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
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_stream_socket_sendto((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_stream_socket_sendto(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && (args-0)->m_type == KindOfObject) {
        fh_stream_socket_sendto((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_sendto(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_sendto", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_stream_socket_shutdown(HPHP::Object const&, int)
_ZN4HPHP24f_stream_socket_shutdownERKNS_6ObjectEi

(return value) => rax
stream => rdi
how => rsi
*/

bool fh_stream_socket_shutdown(Value* stream, int how) asm("_ZN4HPHP24f_stream_socket_shutdownERKNS_6ObjectEi");

TypedValue * fg1_stream_socket_shutdown(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_stream_socket_shutdown(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_data.num = (fh_stream_socket_shutdown(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_stream_socket_shutdown(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && (args-0)->m_type == KindOfObject) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_stream_socket_shutdown(&args[-0].m_data, (int)(args[-1].m_data.num))) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_stream_socket_shutdown(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("stream_socket_shutdown", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

