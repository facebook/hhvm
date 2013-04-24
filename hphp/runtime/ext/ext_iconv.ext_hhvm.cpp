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
HPHP::Variant HPHP::f_iconv_mime_encode(HPHP::String const&, HPHP::String const&, HPHP::Variant const&)
_ZN4HPHP19f_iconv_mime_encodeERKNS_6StringES2_RKNS_7VariantE

(return value) => rax
_rv => rdi
field_name => rsi
field_value => rdx
preferences => rcx
*/

TypedValue* fh_iconv_mime_encode(TypedValue* _rv, Value* field_name, Value* field_value, TypedValue* preferences) asm("_ZN4HPHP19f_iconv_mime_encodeERKNS_6StringES2_RKNS_7VariantE");

TypedValue * fg1_iconv_mime_encode(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_mime_encode(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_iconv_mime_encode((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv_mime_encode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iconv_mime_encode((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (args-2) : (TypedValue*)(&null_variant));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_mime_encode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv_mime_encode", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iconv_mime_decode(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP19f_iconv_mime_decodeERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
encoded_string => rsi
mode => rdx
charset => rcx
*/

TypedValue* fh_iconv_mime_decode(TypedValue* _rv, Value* encoded_string, int mode, Value* charset) asm("_ZN4HPHP19f_iconv_mime_decodeERKNS_6StringEiS2_");

TypedValue * fg1_iconv_mime_decode(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_mime_decode(TypedValue* rv, ActRec* ar, int64_t count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_iconv_mime_decode((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv_mime_decode(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iconv_mime_decode((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_mime_decode(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv_mime_decode", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iconv_mime_decode_headers(HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP27f_iconv_mime_decode_headersERKNS_6StringEiS2_

(return value) => rax
_rv => rdi
encoded_headers => rsi
mode => rdx
charset => rcx
*/

TypedValue* fh_iconv_mime_decode_headers(TypedValue* _rv, Value* encoded_headers, int mode, Value* charset) asm("_ZN4HPHP27f_iconv_mime_decode_headersERKNS_6StringEiS2_");

TypedValue * fg1_iconv_mime_decode_headers(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_mime_decode_headers(TypedValue* rv, ActRec* ar, int64_t count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_iconv_mime_decode_headers((rv), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv_mime_decode_headers(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && (count <= 1 || (args-1)->m_type == KindOfInt64) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iconv_mime_decode_headers((&(rv)), &args[-0].m_data, (count > 1) ? (int)(args[-1].m_data.num) : (int)(0), (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_mime_decode_headers(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv_mime_decode_headers", count, 1, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iconv_get_encoding(HPHP::String const&)
_ZN4HPHP20f_iconv_get_encodingERKNS_6StringE

(return value) => rax
_rv => rdi
type => rsi
*/

TypedValue* fh_iconv_get_encoding(TypedValue* _rv, Value* type) asm("_ZN4HPHP20f_iconv_get_encodingERKNS_6StringE");

TypedValue * fg1_iconv_get_encoding(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_get_encoding(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  String defVal0 = "all";
  fh_iconv_get_encoding((rv), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv_get_encoding(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count <= 1LL) {
      if ((count <= 0 || IS_STRING_TYPE((args-0)->m_type))) {
        String defVal0 = "all";
        fh_iconv_get_encoding((&(rv)), (count > 0) ? &args[-0].m_data : (Value*)(&defVal0));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_get_encoding(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 1);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_toomany_arguments_nr("iconv_get_encoding", 1, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 1);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
bool HPHP::f_iconv_set_encoding(HPHP::String const&, HPHP::String const&)
_ZN4HPHP20f_iconv_set_encodingERKNS_6StringES2_

(return value) => rax
type => rdi
charset => rsi
*/

bool fh_iconv_set_encoding(Value* type, Value* charset) asm("_ZN4HPHP20f_iconv_set_encodingERKNS_6StringES2_");

TypedValue * fg1_iconv_set_encoding(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_set_encoding(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfBoolean;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_data.num = (fh_iconv_set_encoding(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
  return rv;
}

TypedValue* fg_iconv_set_encoding(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if (IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfBoolean;
        rv.m_data.num = (fh_iconv_set_encoding(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_set_encoding(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv_set_encoding", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iconv(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP7f_iconvERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
in_charset => rsi
out_charset => rdx
str => rcx
*/

TypedValue* fh_iconv(TypedValue* _rv, Value* in_charset, Value* out_charset, Value* str) asm("_ZN4HPHP7f_iconvERKNS_6StringES2_S2_");

TypedValue * fg1_iconv(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_iconv((rv), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 3LL) {
      if (IS_STRING_TYPE((args-2)->m_type) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iconv((&(rv)), &args[-0].m_data, &args[-1].m_data, &args[-2].m_data);
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv", count, 3, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iconv_strlen(HPHP::String const&, HPHP::String const&)
_ZN4HPHP14f_iconv_strlenERKNS_6StringES2_

(return value) => rax
_rv => rdi
str => rsi
charset => rdx
*/

TypedValue* fh_iconv_strlen(TypedValue* _rv, Value* str, Value* charset) asm("_ZN4HPHP14f_iconv_strlenERKNS_6StringES2_");

TypedValue * fg1_iconv_strlen(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_strlen(TypedValue* rv, ActRec* ar, int64_t count) {
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
  fh_iconv_strlen((rv), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv_strlen(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 1LL && count <= 2LL) {
      if ((count <= 1 || IS_STRING_TYPE((args-1)->m_type)) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iconv_strlen((&(rv)), &args[-0].m_data, (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_strlen(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv_strlen", count, 1, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iconv_strpos(HPHP::String const&, HPHP::String const&, int, HPHP::String const&)
_ZN4HPHP14f_iconv_strposERKNS_6StringES2_iS2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
offset => rcx
charset => r8
*/

TypedValue* fh_iconv_strpos(TypedValue* _rv, Value* haystack, Value* needle, int offset, Value* charset) asm("_ZN4HPHP14f_iconv_strposERKNS_6StringES2_iS2_");

TypedValue * fg1_iconv_strpos(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_strpos(TypedValue* rv, ActRec* ar, int64_t count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_iconv_strpos((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv_strpos(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || (args-2)->m_type == KindOfInt64) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iconv_strpos((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? (int)(args[-2].m_data.num) : (int)(0), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_strpos(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv_strpos", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iconv_strrpos(HPHP::String const&, HPHP::String const&, HPHP::String const&)
_ZN4HPHP15f_iconv_strrposERKNS_6StringES2_S2_

(return value) => rax
_rv => rdi
haystack => rsi
needle => rdx
charset => rcx
*/

TypedValue* fh_iconv_strrpos(TypedValue* _rv, Value* haystack, Value* needle, Value* charset) asm("_ZN4HPHP15f_iconv_strrposERKNS_6StringES2_S2_");

TypedValue * fg1_iconv_strrpos(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_strrpos(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
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
  fh_iconv_strrpos((rv), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv_strrpos(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 3LL) {
      if ((count <= 2 || IS_STRING_TYPE((args-2)->m_type)) && IS_STRING_TYPE((args-1)->m_type) && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iconv_strrpos((&(rv)), &args[-0].m_data, &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_strrpos(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 3);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv_strrpos", count, 2, 3, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 3);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::Variant HPHP::f_iconv_substr(HPHP::String const&, int, int, HPHP::String const&)
_ZN4HPHP14f_iconv_substrERKNS_6StringEiiS2_

(return value) => rax
_rv => rdi
str => rsi
offset => rdx
length => rcx
charset => r8
*/

TypedValue* fh_iconv_substr(TypedValue* _rv, Value* str, int offset, int length, Value* charset) asm("_ZN4HPHP14f_iconv_substrERKNS_6StringEiiS2_");

TypedValue * fg1_iconv_substr(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_iconv_substr(TypedValue* rv, ActRec* ar, int64_t count) {
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
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_iconv_substr((rv), &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_iconv_substr(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count >= 2LL && count <= 4LL) {
      if ((count <= 3 || IS_STRING_TYPE((args-3)->m_type)) && (count <= 2 || (args-2)->m_type == KindOfInt64) && (args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        fh_iconv_substr((&(rv)), &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? (int)(args[-2].m_data.num) : (int)(INT_MAX), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
        if (rv.m_type == KindOfUninit) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_iconv_substr(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 4);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("iconv_substr", count, 2, 4, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 4);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}



/*
HPHP::String HPHP::f_ob_iconv_handler(HPHP::String const&, int)
_ZN4HPHP18f_ob_iconv_handlerERKNS_6StringEi

(return value) => rax
_rv => rdi
contents => rsi
status => rdx
*/

Value* fh_ob_iconv_handler(Value* _rv, Value* contents, int status) asm("_ZN4HPHP18f_ob_iconv_handlerERKNS_6StringEi");

TypedValue * fg1_ob_iconv_handler(TypedValue* rv, ActRec* ar, int64_t count) __attribute__((noinline,cold));
TypedValue * fg1_ob_iconv_handler(TypedValue* rv, ActRec* ar, int64_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  rv->m_type = KindOfString;
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_ob_iconv_handler((&rv->m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  return rv;
}

TypedValue* fg_ob_iconv_handler(ActRec *ar) {
    TypedValue rv;
    int64_t count = ar->numArgs();
    TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
    if (count == 2LL) {
      if ((args-1)->m_type == KindOfInt64 && IS_STRING_TYPE((args-0)->m_type)) {
        rv.m_type = KindOfString;
        fh_ob_iconv_handler((&rv.m_data), &args[-0].m_data, (int)(args[-1].m_data.num));
        if (rv.m_data.num == 0LL) rv.m_type = KindOfNull;
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      } else {
        fg1_ob_iconv_handler(&rv, ar, count);
        frame_free_locals_no_this_inl(ar, 2);
        memcpy(&ar->m_r, &rv, sizeof(TypedValue));
        return &ar->m_r;
      }
    } else {
      throw_wrong_arguments_nr("ob_iconv_handler", count, 2, 2, 1);
    }
    rv.m_data.num = 0LL;
    rv.m_type = KindOfNull;
    frame_free_locals_no_this_inl(ar, 2);
    memcpy(&ar->m_r, &rv, sizeof(TypedValue));
    return &ar->m_r;
  return &ar->m_r;
}




} // !HPHP

