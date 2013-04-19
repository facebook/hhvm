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

bool fh_mail(Value* to, Value* subject, Value* message, Value* additional_headers, Value* additional_parameters) asm("_ZN4HPHP6f_mailERKNS_6StringES2_S2_S2_S2_");

void fg1_mail(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mail(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mail(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_mail(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mail(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? &args[-3].m_data : (Value*)(&null_string), (count > 4) ? &args[-4].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_mail(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mail", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_ezmlm_hash(Value* addr) asm("_ZN4HPHP12f_ezmlm_hashERKNS_6StringE");

void fg1_ezmlm_hash(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_ezmlm_hash(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_ezmlm_hash(&args[-0].m_data);
}

TypedValue* fg_ezmlm_hash(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_ezmlm_hash(&args[-0].m_data);
    } else {
      fg1_ezmlm_hash(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("ezmlm_hash", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mailparse_msg_create(Value* _rv) asm("_ZN4HPHP22f_mailparse_msg_createEv");

TypedValue* fg_mailparse_msg_create(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    rv->m_type = KindOfObject;
    fh_mailparse_msg_create(&(rv->m_data));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("mailparse_msg_create", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mailparse_msg_free(Value* mimemail) asm("_ZN4HPHP20f_mailparse_msg_freeERKNS_6ObjectE");

void fg1_mailparse_msg_free(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_free(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mailparse_msg_free(&args[-0].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mailparse_msg_free(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mailparse_msg_free(&args[-0].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mailparse_msg_free(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_free", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mailparse_msg_parse_file(TypedValue* _rv, Value* filename) asm("_ZN4HPHP26f_mailparse_msg_parse_fileERKNS_6StringE");

void fg1_mailparse_msg_parse_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_parse_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_mailparse_msg_parse_file(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_msg_parse_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_mailparse_msg_parse_file(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_msg_parse_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_parse_file", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mailparse_msg_parse(Value* mimemail, Value* data) asm("_ZN4HPHP21f_mailparse_msg_parseERKNS_6ObjectERKNS_6StringE");

void fg1_mailparse_msg_parse(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_parse(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mailparse_msg_parse(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mailparse_msg_parse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mailparse_msg_parse(&args[-0].m_data, &args[-1].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mailparse_msg_parse(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_parse", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mailparse_msg_extract_part_file(TypedValue* _rv, Value* mimemail, TypedValue* filename, TypedValue* callbackfunc) asm("_ZN4HPHP33f_mailparse_msg_extract_part_fileERKNS_6ObjectERKNS_7VariantES5_");

void fg1_mailparse_msg_extract_part_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_extract_part_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  Variant defVal2 = "";
  fh_mailparse_msg_extract_part_file(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_msg_extract_part_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((args - 0)->m_type == KindOfObject) {
      Variant defVal2 = "";
      fh_mailparse_msg_extract_part_file(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_msg_extract_part_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_extract_part_file", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mailparse_msg_extract_whole_part_file(TypedValue* _rv, Value* mimemail, TypedValue* filename, TypedValue* callbackfunc) asm("_ZN4HPHP39f_mailparse_msg_extract_whole_part_fileERKNS_6ObjectERKNS_7VariantES5_");

void fg1_mailparse_msg_extract_whole_part_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_extract_whole_part_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  Variant defVal2 = "";
  fh_mailparse_msg_extract_whole_part_file(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_msg_extract_whole_part_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((args - 0)->m_type == KindOfObject) {
      Variant defVal2 = "";
      fh_mailparse_msg_extract_whole_part_file(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_msg_extract_whole_part_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_extract_whole_part_file", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mailparse_msg_extract_part(TypedValue* _rv, Value* mimemail, TypedValue* msgbody, TypedValue* callbackfunc) asm("_ZN4HPHP28f_mailparse_msg_extract_partERKNS_6ObjectERKNS_7VariantES5_");

void fg1_mailparse_msg_extract_part(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_extract_part(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  Variant defVal2 = "";
  fh_mailparse_msg_extract_part(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_msg_extract_part(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((args - 0)->m_type == KindOfObject) {
      Variant defVal2 = "";
      fh_mailparse_msg_extract_part(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&defVal2));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_msg_extract_part(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_extract_part", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mailparse_msg_get_part_data(Value* _rv, Value* mimemail) asm("_ZN4HPHP29f_mailparse_msg_get_part_dataERKNS_6ObjectE");

void fg1_mailparse_msg_get_part_data(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_get_part_data(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_mailparse_msg_get_part_data(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_msg_get_part_data(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_mailparse_msg_get_part_data(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_msg_get_part_data(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_get_part_data", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mailparse_msg_get_part(TypedValue* _rv, Value* mimemail, Value* mimesection) asm("_ZN4HPHP24f_mailparse_msg_get_partERKNS_6ObjectERKNS_6StringE");

void fg1_mailparse_msg_get_part(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_get_part(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  fh_mailparse_msg_get_part(rv, &args[-0].m_data, &args[-1].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_msg_get_part(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        (args - 0)->m_type == KindOfObject) {
      fh_mailparse_msg_get_part(rv, &args[-0].m_data, &args[-1].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_msg_get_part(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_get_part", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mailparse_msg_get_structure(Value* _rv, Value* mimemail) asm("_ZN4HPHP29f_mailparse_msg_get_structureERKNS_6ObjectE");

void fg1_mailparse_msg_get_structure(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_msg_get_structure(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_mailparse_msg_get_structure(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_msg_get_structure(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_mailparse_msg_get_structure(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_msg_get_structure(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_msg_get_structure", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_mailparse_rfc822_parse_addresses(Value* _rv, Value* addresses) asm("_ZN4HPHP34f_mailparse_rfc822_parse_addressesERKNS_6StringE");

void fg1_mailparse_rfc822_parse_addresses(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_rfc822_parse_addresses(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_mailparse_rfc822_parse_addresses(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_rfc822_parse_addresses(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfArray;
      fh_mailparse_rfc822_parse_addresses(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_rfc822_parse_addresses(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_rfc822_parse_addresses", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_mailparse_stream_encode(Value* sourcefp, Value* destfp, Value* encoding) asm("_ZN4HPHP25f_mailparse_stream_encodeERKNS_6ObjectES2_RKNS_6StringE");

void fg1_mailparse_stream_encode(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_stream_encode(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if ((args-1)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-1);
  }
  if ((args-0)->m_type != KindOfObject) {
    tvCastToObjectInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_mailparse_stream_encode(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_mailparse_stream_encode(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        (args - 1)->m_type == KindOfObject &&
        (args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_mailparse_stream_encode(&args[-0].m_data, &args[-1].m_data, &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_mailparse_stream_encode(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_stream_encode", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mailparse_uudecode_all(TypedValue* _rv, Value* fp) asm("_ZN4HPHP24f_mailparse_uudecode_allERKNS_6ObjectE");

void fg1_mailparse_uudecode_all(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_uudecode_all(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_mailparse_uudecode_all(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_uudecode_all(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_mailparse_uudecode_all(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_uudecode_all(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_uudecode_all", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_mailparse_determine_best_xfer_encoding(TypedValue* _rv, Value* fp) asm("_ZN4HPHP40f_mailparse_determine_best_xfer_encodingERKNS_6ObjectE");

void fg1_mailparse_determine_best_xfer_encoding(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_mailparse_determine_best_xfer_encoding(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  fh_mailparse_determine_best_xfer_encoding(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_mailparse_determine_best_xfer_encoding(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      fh_mailparse_determine_best_xfer_encoding(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_mailparse_determine_best_xfer_encoding(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("mailparse_determine_best_xfer_encoding", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
