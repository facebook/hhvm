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

bool fh_openssl_csr_export_to_file(TypedValue* csr, Value* outfilename, bool notext) asm("_ZN4HPHP28f_openssl_csr_export_to_fileERKNS_7VariantERKNS_6StringEb");

void fg1_openssl_csr_export_to_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_csr_export_to_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_csr_export_to_file((args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_csr_export_to_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_csr_export_to_file((args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_openssl_csr_export_to_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_csr_export_to_file", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_csr_export(TypedValue* csr, TypedValue* out, bool notext) asm("_ZN4HPHP20f_openssl_csr_exportERKNS_7VariantERKNS_14VRefParamValueEb");

void fg1_openssl_csr_export(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_csr_export(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-2);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_csr_export((args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_csr_export(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_csr_export((args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_openssl_csr_export(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_csr_export", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_csr_get_public_key(TypedValue* _rv, TypedValue* csr) asm("_ZN4HPHP28f_openssl_csr_get_public_keyERKNS_7VariantE");

TypedValue* fg_openssl_csr_get_public_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_openssl_csr_get_public_key(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("openssl_csr_get_public_key", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_csr_get_subject(TypedValue* _rv, TypedValue* csr, bool use_shortnames) asm("_ZN4HPHP25f_openssl_csr_get_subjectERKNS_7VariantEb");

void fg1_openssl_csr_get_subject(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_csr_get_subject(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_openssl_csr_get_subject(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_csr_get_subject(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      fh_openssl_csr_get_subject(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_csr_get_subject(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_csr_get_subject", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_csr_new(TypedValue* _rv, Value* dn, TypedValue* privkey, TypedValue* configargs, TypedValue* extraattribs) asm("_ZN4HPHP17f_openssl_csr_newERKNS_5ArrayERKNS_14VRefParamValueERKNS_7VariantES8_");

void fg1_openssl_csr_new(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_csr_new(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToArrayInPlace(args-0);
  fh_openssl_csr_new(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_csr_new(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((args - 0)->m_type == KindOfArray) {
      fh_openssl_csr_new(rv, &args[-0].m_data, (args-1), (count > 2) ? (args-2) : (TypedValue*)(&null_variant), (count > 3) ? (args-3) : (TypedValue*)(&null_variant));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_csr_new(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_csr_new", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_csr_sign(TypedValue* _rv, TypedValue* csr, TypedValue* cacert, TypedValue* priv_key, int days, TypedValue* configargs, int serial) asm("_ZN4HPHP18f_openssl_csr_signERKNS_7VariantES2_S2_iS2_i");

void fg1_openssl_csr_sign(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_csr_sign(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
  case 4:
    break;
  }
  if ((args-3)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-3);
  }
  fh_openssl_csr_sign(rv, (args-0), (args-1), (args-2), (int)(args[-3].m_data.num), (count > 4) ? (args-4) : (TypedValue*)(&null_variant), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_csr_sign(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 6) {
    if ((count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        (args - 3)->m_type == KindOfInt64) {
      fh_openssl_csr_sign(rv, (args-0), (args-1), (args-2), (int)(args[-3].m_data.num), (count > 4) ? (args-4) : (TypedValue*)(&null_variant), (count > 5) ? (int)(args[-5].m_data.num) : (int)(0));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_csr_sign(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_csr_sign", count, 4, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_error_string(TypedValue* _rv) asm("_ZN4HPHP22f_openssl_error_stringEv");

TypedValue* fg_openssl_error_string(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 0) {
    fh_openssl_error_string(rv);
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("openssl_error_string", 0, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 0);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_openssl_free_key(Value* key) asm("_ZN4HPHP18f_openssl_free_keyERKNS_6ObjectE");

void fg1_openssl_free_key(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_free_key(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_openssl_free_key(&args[-0].m_data);
}

TypedValue* fg_openssl_free_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_openssl_free_key(&args[-0].m_data);
    } else {
      fg1_openssl_free_key(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_free_key", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_openssl_pkey_free(Value* key) asm("_ZN4HPHP19f_openssl_pkey_freeERKNS_6ObjectE");

void fg1_openssl_pkey_free(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkey_free(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_openssl_pkey_free(&args[-0].m_data);
}

TypedValue* fg_openssl_pkey_free(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_openssl_pkey_free(&args[-0].m_data);
    } else {
      fg1_openssl_pkey_free(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkey_free", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_open(Value* sealed_data, TypedValue* open_data, Value* env_key, TypedValue* priv_key_id) asm("_ZN4HPHP14f_openssl_openERKNS_6StringERKNS_14VRefParamValueES2_RKNS_7VariantE");

void fg1_openssl_open(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_open(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_open(&args[-0].m_data, (args-1), &args[-2].m_data, (args-3))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_open(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_open(&args[-0].m_data, (args-1), &args[-2].m_data, (args-3))) ? 1LL : 0LL;
    } else {
      fg1_openssl_open(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_open", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_pkcs12_export_to_file(TypedValue* x509, Value* filename, TypedValue* priv_key, Value* pass, TypedValue* args) asm("_ZN4HPHP31f_openssl_pkcs12_export_to_fileERKNS_7VariantERKNS_6StringES2_S5_S2_");

void fg1_openssl_pkcs12_export_to_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkcs12_export_to_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
  case 4:
    break;
  }
  if (!IS_STRING_TYPE((args-3)->m_type)) {
    tvCastToStringInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_pkcs12_export_to_file((args-0), &args[-1].m_data, (args-2), &args[-3].m_data, (count > 4) ? (args-4) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_pkcs12_export_to_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if (IS_STRING_TYPE((args - 3)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_pkcs12_export_to_file((args-0), &args[-1].m_data, (args-2), &args[-3].m_data, (count > 4) ? (args-4) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
    } else {
      fg1_openssl_pkcs12_export_to_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkcs12_export_to_file", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_pkcs12_export(TypedValue* x509, TypedValue* out, TypedValue* priv_key, Value* pass, TypedValue* args) asm("_ZN4HPHP23f_openssl_pkcs12_exportERKNS_7VariantERKNS_14VRefParamValueES2_RKNS_6StringES2_");

void fg1_openssl_pkcs12_export(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkcs12_export(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-3);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_pkcs12_export((args-0), (args-1), (args-2), &args[-3].m_data, (count > 4) ? (args-4) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_pkcs12_export(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 5) {
    if (IS_STRING_TYPE((args - 3)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_pkcs12_export((args-0), (args-1), (args-2), &args[-3].m_data, (count > 4) ? (args-4) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
    } else {
      fg1_openssl_pkcs12_export(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkcs12_export", count, 4, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_pkcs12_read(Value* pkcs12, TypedValue* certs, Value* pass) asm("_ZN4HPHP21f_openssl_pkcs12_readERKNS_6StringERKNS_14VRefParamValueES2_");

void fg1_openssl_pkcs12_read(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkcs12_read(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (!IS_STRING_TYPE((args-2)->m_type)) {
    tvCastToStringInPlace(args-2);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_pkcs12_read(&args[-0].m_data, (args-1), &args[-2].m_data)) ? 1LL : 0LL;
}

TypedValue* fg_openssl_pkcs12_read(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 3) {
    if (IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_pkcs12_read(&args[-0].m_data, (args-1), &args[-2].m_data)) ? 1LL : 0LL;
    } else {
      fg1_openssl_pkcs12_read(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkcs12_read", count, 3, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_pkcs7_decrypt(Value* infilename, Value* outfilename, TypedValue* recipcert, TypedValue* recipkey) asm("_ZN4HPHP23f_openssl_pkcs7_decryptERKNS_6StringES2_RKNS_7VariantES5_");

void fg1_openssl_pkcs7_decrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkcs7_decrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_pkcs7_decrypt(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_pkcs7_decrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if (IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_pkcs7_decrypt(&args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
    } else {
      fg1_openssl_pkcs7_decrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkcs7_decrypt", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_pkcs7_encrypt(Value* infilename, Value* outfilename, TypedValue* recipcerts, Value* headers, int flags, int cipherid) asm("_ZN4HPHP23f_openssl_pkcs7_encryptERKNS_6StringES2_RKNS_7VariantERKNS_5ArrayEii");

void fg1_openssl_pkcs7_encrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkcs7_encrypt(TypedValue* rv, ActRec* ar, int32_t count) {
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
    break;
  }
  if ((args-3)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_pkcs7_encrypt(&args[-0].m_data, &args[-1].m_data, (args-2), &args[-3].m_data, (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(k_OPENSSL_CIPHER_RC2_40))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_pkcs7_encrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 4 && count <= 6) {
    if ((count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        (count <= 4 || (args - 4)->m_type == KindOfInt64) &&
        (args - 3)->m_type == KindOfArray &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_pkcs7_encrypt(&args[-0].m_data, &args[-1].m_data, (args-2), &args[-3].m_data, (count > 4) ? (int)(args[-4].m_data.num) : (int)(0), (count > 5) ? (int)(args[-5].m_data.num) : (int)(k_OPENSSL_CIPHER_RC2_40))) ? 1LL : 0LL;
    } else {
      fg1_openssl_pkcs7_encrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkcs7_encrypt", count, 4, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_pkcs7_sign(Value* infilename, Value* outfilename, TypedValue* signcert, TypedValue* privkey, TypedValue* headers, int flags, Value* extracerts) asm("_ZN4HPHP20f_openssl_pkcs7_signERKNS_6StringES2_RKNS_7VariantES5_S5_iS2_");

void fg1_openssl_pkcs7_sign(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkcs7_sign(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 7
    if (!IS_STRING_TYPE((args-6)->m_type)) {
      tvCastToStringInPlace(args-6);
    }
  case 6:
    if ((args-5)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-5);
    }
  case 5:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_pkcs7_sign(&args[-0].m_data, &args[-1].m_data, (args-2), (args-3), (args-4), (count > 5) ? (int)(args[-5].m_data.num) : (int)(k_PKCS7_DETACHED), (count > 6) ? &args[-6].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_pkcs7_sign(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 5 && count <= 7) {
    if ((count <= 6 || IS_STRING_TYPE((args - 6)->m_type)) &&
        (count <= 5 || (args - 5)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_pkcs7_sign(&args[-0].m_data, &args[-1].m_data, (args-2), (args-3), (args-4), (count > 5) ? (int)(args[-5].m_data.num) : (int)(k_PKCS7_DETACHED), (count > 6) ? &args[-6].m_data : (Value*)(&null_string))) ? 1LL : 0LL;
    } else {
      fg1_openssl_pkcs7_sign(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkcs7_sign", count, 5, 7, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 7);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_pkcs7_verify(TypedValue* _rv, Value* filename, int flags, Value* outfilename, Value* cainfo, Value* extracerts, Value* content) asm("_ZN4HPHP22f_openssl_pkcs7_verifyERKNS_6StringEiS2_RKNS_5ArrayES2_S2_");

void fg1_openssl_pkcs7_verify(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkcs7_verify(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 6
    if (!IS_STRING_TYPE((args-5)->m_type)) {
      tvCastToStringInPlace(args-5);
    }
  case 5:
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-3);
    }
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
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
  fh_openssl_pkcs7_verify(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_array), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_pkcs7_verify(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 6) {
    if ((count <= 5 || IS_STRING_TYPE((args - 5)->m_type)) &&
        (count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || (args - 3)->m_type == KindOfArray) &&
        (count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        (args - 1)->m_type == KindOfInt64 &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_openssl_pkcs7_verify(rv, &args[-0].m_data, (int)(args[-1].m_data.num), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? &args[-3].m_data : (Value*)(&null_array), (count > 4) ? &args[-4].m_data : (Value*)(&null_string), (count > 5) ? &args[-5].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_pkcs7_verify(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkcs7_verify", count, 2, 6, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 6);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_pkey_export_to_file(TypedValue* key, Value* outfilename, Value* passphrase, TypedValue* configargs) asm("_ZN4HPHP29f_openssl_pkey_export_to_fileERKNS_7VariantERKNS_6StringES5_S2_");

void fg1_openssl_pkey_export_to_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkey_export_to_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
  case 3:
    if (!IS_STRING_TYPE((args-2)->m_type)) {
      tvCastToStringInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_pkey_export_to_file((args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_pkey_export_to_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type)) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_pkey_export_to_file((args-0), &args[-1].m_data, (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
    } else {
      fg1_openssl_pkey_export_to_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkey_export_to_file", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_pkey_export(TypedValue* key, TypedValue* out, Value* passphrase, TypedValue* configargs) asm("_ZN4HPHP21f_openssl_pkey_exportERKNS_7VariantERKNS_14VRefParamValueERKNS_6StringES2_");

void fg1_openssl_pkey_export(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkey_export(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-2);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_pkey_export((args-0), (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_pkey_export(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 2 || IS_STRING_TYPE((args - 2)->m_type))) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_pkey_export((args-0), (args-1), (count > 2) ? &args[-2].m_data : (Value*)(&null_string), (count > 3) ? (args-3) : (TypedValue*)(&null_variant))) ? 1LL : 0LL;
    } else {
      fg1_openssl_pkey_export(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkey_export", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_openssl_pkey_get_details(Value* _rv, Value* key) asm("_ZN4HPHP26f_openssl_pkey_get_detailsERKNS_6ObjectE");

void fg1_openssl_pkey_get_details(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkey_get_details(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_openssl_pkey_get_details(&(rv->m_data), &args[-0].m_data);
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_pkey_get_details(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfArray;
      fh_openssl_pkey_get_details(&(rv->m_data), &args[-0].m_data);
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_pkey_get_details(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkey_get_details", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_pkey_get_private(TypedValue* _rv, TypedValue* key, Value* passphrase) asm("_ZN4HPHP26f_openssl_pkey_get_privateERKNS_7VariantERKNS_6StringE");

void fg1_openssl_pkey_get_private(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_pkey_get_private(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_openssl_pkey_get_private(rv, (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_pkey_get_private(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type))) {
      fh_openssl_pkey_get_private(rv, (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_pkey_get_private(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_pkey_get_private", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_get_privatekey(TypedValue* _rv, TypedValue* key, Value* passphrase) asm("_ZN4HPHP24f_openssl_get_privatekeyERKNS_7VariantERKNS_6StringE");

void fg1_openssl_get_privatekey(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_get_privatekey(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-1);
  fh_openssl_get_privatekey(rv, (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_get_privatekey(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || IS_STRING_TYPE((args - 1)->m_type))) {
      fh_openssl_get_privatekey(rv, (args-0), (count > 1) ? &args[-1].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_get_privatekey(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_get_privatekey", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_pkey_get_public(TypedValue* _rv, TypedValue* certificate) asm("_ZN4HPHP25f_openssl_pkey_get_publicERKNS_7VariantE");

TypedValue* fg_openssl_pkey_get_public(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_openssl_pkey_get_public(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("openssl_pkey_get_public", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_get_publickey(TypedValue* _rv, TypedValue* certificate) asm("_ZN4HPHP23f_openssl_get_publickeyERKNS_7VariantE");

TypedValue* fg_openssl_get_publickey(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_openssl_get_publickey(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("openssl_get_publickey", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_openssl_pkey_new(Value* _rv, TypedValue* configargs) asm("_ZN4HPHP18f_openssl_pkey_newERKNS_7VariantE");

TypedValue* fg_openssl_pkey_new(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    rv->m_type = KindOfObject;
    fh_openssl_pkey_new(&(rv->m_data), (count > 0) ? (args-0) : (TypedValue*)(&null_variant));
    if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
  } else {
    throw_toomany_arguments_nr("openssl_pkey_new", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_private_decrypt(Value* data, TypedValue* decrypted, TypedValue* key, int padding) asm("_ZN4HPHP25f_openssl_private_decryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

void fg1_openssl_private_decrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_private_decrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_private_decrypt(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_PKCS1_PADDING))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_private_decrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_private_decrypt(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_PKCS1_PADDING))) ? 1LL : 0LL;
    } else {
      fg1_openssl_private_decrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_private_decrypt", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_private_encrypt(Value* data, TypedValue* crypted, TypedValue* key, int padding) asm("_ZN4HPHP25f_openssl_private_encryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

void fg1_openssl_private_encrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_private_encrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_private_encrypt(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_PKCS1_PADDING))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_private_encrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_private_encrypt(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_PKCS1_PADDING))) ? 1LL : 0LL;
    } else {
      fg1_openssl_private_encrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_private_encrypt", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_public_decrypt(Value* data, TypedValue* decrypted, TypedValue* key, int padding) asm("_ZN4HPHP24f_openssl_public_decryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

void fg1_openssl_public_decrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_public_decrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_public_decrypt(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_PKCS1_PADDING))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_public_decrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_public_decrypt(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_PKCS1_PADDING))) ? 1LL : 0LL;
    } else {
      fg1_openssl_public_decrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_public_decrypt", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_public_encrypt(Value* data, TypedValue* crypted, TypedValue* key, int padding) asm("_ZN4HPHP24f_openssl_public_encryptERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

void fg1_openssl_public_encrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_public_encrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_public_encrypt(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_PKCS1_PADDING))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_public_encrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_public_encrypt(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_PKCS1_PADDING))) ? 1LL : 0LL;
    } else {
      fg1_openssl_public_encrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_public_encrypt", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_seal(TypedValue* _rv, Value* data, TypedValue* sealed_data, TypedValue* env_keys, Value* pub_key_ids) asm("_ZN4HPHP14f_openssl_sealERKNS_6StringERKNS_14VRefParamValueES5_RKNS_5ArrayE");

void fg1_openssl_seal(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_seal(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if ((args-3)->m_type != KindOfArray) {
    tvCastToArrayInPlace(args-3);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_openssl_seal(rv, &args[-0].m_data, (args-1), (args-2), &args[-3].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_seal(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 4) {
    if ((args - 3)->m_type == KindOfArray &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_openssl_seal(rv, &args[-0].m_data, (args-1), (args-2), &args[-3].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_seal(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_seal", count, 4, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_sign(Value* data, TypedValue* signature, TypedValue* priv_key_id, int signature_alg) asm("_ZN4HPHP14f_openssl_signERKNS_6StringERKNS_14VRefParamValueERKNS_7VariantEi");

void fg1_openssl_sign(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_sign(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_sign(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_ALGO_SHA1))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_sign(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_sign(&args[-0].m_data, (args-1), (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_ALGO_SHA1))) ? 1LL : 0LL;
    } else {
      fg1_openssl_sign(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_sign", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_verify(TypedValue* _rv, Value* data, Value* signature, TypedValue* pub_key_id, int signature_alg) asm("_ZN4HPHP16f_openssl_verifyERKNS_6StringES2_RKNS_7VariantEi");

void fg1_openssl_verify(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_verify(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
    }
  case 3:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  if (!IS_STRING_TYPE((args-0)->m_type)) {
    tvCastToStringInPlace(args-0);
  }
  fh_openssl_verify(rv, &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_ALGO_SHA1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_verify(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 4) {
    if ((count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_openssl_verify(rv, &args[-0].m_data, &args[-1].m_data, (args-2), (count > 3) ? (int)(args[-3].m_data.num) : (int)(k_OPENSSL_ALGO_SHA1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_verify(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_verify", count, 3, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_x509_check_private_key(TypedValue* cert, TypedValue* key) asm("_ZN4HPHP32f_openssl_x509_check_private_keyERKNS_7VariantES2_");

TypedValue* fg_openssl_x509_check_private_key(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 2) {
    rv->m_type = KindOfBoolean;
    rv->m_data.num = (fh_openssl_x509_check_private_key((args-0), (args-1))) ? 1LL : 0LL;
  } else {
    throw_wrong_arguments_nr("openssl_x509_check_private_key", count, 2, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

long fh_openssl_x509_checkpurpose(TypedValue* x509cert, int purpose, Value* cainfo, Value* untrustedfile) asm("_ZN4HPHP27f_openssl_x509_checkpurposeERKNS_7VariantEiRKNS_5ArrayERKNS_6StringE");

void fg1_openssl_x509_checkpurpose(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_x509_checkpurpose(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 4
    if (!IS_STRING_TYPE((args-3)->m_type)) {
      tvCastToStringInPlace(args-3);
    }
  case 3:
    if ((args-2)->m_type != KindOfArray) {
      tvCastToArrayInPlace(args-2);
    }
  case 2:
    break;
  }
  if ((args-1)->m_type != KindOfInt64) {
    tvCastToInt64InPlace(args-1);
  }
  rv->m_type = KindOfInt64;
  rv->m_data.num = (int64_t)fh_openssl_x509_checkpurpose((args-0), (int)(args[-1].m_data.num), (count > 2) ? &args[-2].m_data : (Value*)(&null_array), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
}

TypedValue* fg_openssl_x509_checkpurpose(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 4) {
    if ((count <= 3 || IS_STRING_TYPE((args - 3)->m_type)) &&
        (count <= 2 || (args - 2)->m_type == KindOfArray) &&
        (args - 1)->m_type == KindOfInt64) {
      rv->m_type = KindOfInt64;
      rv->m_data.num = (int64_t)fh_openssl_x509_checkpurpose((args-0), (int)(args[-1].m_data.num), (count > 2) ? &args[-2].m_data : (Value*)(&null_array), (count > 3) ? &args[-3].m_data : (Value*)(&null_string));
    } else {
      fg1_openssl_x509_checkpurpose(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_x509_checkpurpose", count, 2, 4, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 4);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_x509_export_to_file(TypedValue* x509, Value* outfilename, bool notext) asm("_ZN4HPHP29f_openssl_x509_export_to_fileERKNS_7VariantERKNS_6StringEb");

void fg1_openssl_x509_export_to_file(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_x509_export_to_file(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
    }
  case 2:
    break;
  }
  if (!IS_STRING_TYPE((args-1)->m_type)) {
    tvCastToStringInPlace(args-1);
  }
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_x509_export_to_file((args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_x509_export_to_file(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_x509_export_to_file((args-0), &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_openssl_x509_export_to_file(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_x509_export_to_file", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

bool fh_openssl_x509_export(TypedValue* x509, TypedValue* output, bool notext) asm("_ZN4HPHP21f_openssl_x509_exportERKNS_7VariantERKNS_14VRefParamValueEb");

void fg1_openssl_x509_export(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_x509_export(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-2);
  rv->m_type = KindOfBoolean;
  rv->m_data.num = (fh_openssl_x509_export((args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
}

TypedValue* fg_openssl_x509_export(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfBoolean;
      rv->m_data.num = (fh_openssl_x509_export((args-0), (args-1), (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(true))) ? 1LL : 0LL;
    } else {
      fg1_openssl_x509_export(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_x509_export", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

void fh_openssl_x509_free(Value* x509cert) asm("_ZN4HPHP19f_openssl_x509_freeERKNS_6ObjectE");

void fg1_openssl_x509_free(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_x509_free(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToObjectInPlace(args-0);
  rv->m_type = KindOfNull;
  fh_openssl_x509_free(&args[-0].m_data);
}

TypedValue* fg_openssl_x509_free(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if ((args - 0)->m_type == KindOfObject) {
      rv->m_type = KindOfNull;
      fh_openssl_x509_free(&args[-0].m_data);
    } else {
      fg1_openssl_x509_free(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_x509_free", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_x509_parse(TypedValue* _rv, TypedValue* x509cert, bool shortnames) asm("_ZN4HPHP20f_openssl_x509_parseERKNS_7VariantEb");

void fg1_openssl_x509_parse(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_x509_parse(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-1);
  fh_openssl_x509_parse(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_x509_parse(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((count <= 1 || (args - 1)->m_type == KindOfBoolean)) {
      fh_openssl_x509_parse(rv, (args-0), (count > 1) ? (bool)(args[-1].m_data.num) : (bool)(true));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_x509_parse(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_x509_parse", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_x509_read(TypedValue* _rv, TypedValue* x509certdata) asm("_ZN4HPHP19f_openssl_x509_readERKNS_7VariantE");

TypedValue* fg_openssl_x509_read(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    fh_openssl_x509_read(rv, (args-0));
    if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
  } else {
    throw_wrong_arguments_nr("openssl_x509_read", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_random_pseudo_bytes(TypedValue* _rv, int length, TypedValue* crypto_strong) asm("_ZN4HPHP29f_openssl_random_pseudo_bytesEiRKNS_14VRefParamValueE");

void fg1_openssl_random_pseudo_bytes(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_random_pseudo_bytes(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToInt64InPlace(args-0);
  VRefParamValue defVal1 = false;
  fh_openssl_random_pseudo_bytes(rv, (int)(args[-0].m_data.num), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_random_pseudo_bytes(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 1 && count <= 2) {
    if ((args - 0)->m_type == KindOfInt64) {
      VRefParamValue defVal1 = false;
      fh_openssl_random_pseudo_bytes(rv, (int)(args[-0].m_data.num), (count > 1) ? (args-1) : (TypedValue*)(&defVal1));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_random_pseudo_bytes(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_random_pseudo_bytes", count, 1, 2, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 2);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_cipher_iv_length(TypedValue* _rv, Value* method) asm("_ZN4HPHP26f_openssl_cipher_iv_lengthERKNS_6StringE");

void fg1_openssl_cipher_iv_length(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_cipher_iv_length(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToStringInPlace(args-0);
  fh_openssl_cipher_iv_length(rv, &args[-0].m_data);
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_cipher_iv_length(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count == 1) {
    if (IS_STRING_TYPE((args - 0)->m_type)) {
      fh_openssl_cipher_iv_length(rv, &args[-0].m_data);
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_cipher_iv_length(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_cipher_iv_length", count, 1, 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_encrypt(TypedValue* _rv, Value* data, Value* method, Value* password, int options, Value* iv) asm("_ZN4HPHP17f_openssl_encryptERKNS_6StringES2_S2_iS2_");

void fg1_openssl_encrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_encrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  fh_openssl_encrypt(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_encrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_openssl_encrypt(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_encrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_encrypt", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_decrypt(TypedValue* _rv, Value* data, Value* method, Value* password, int options, Value* iv) asm("_ZN4HPHP17f_openssl_decryptERKNS_6StringES2_S2_iS2_");

void fg1_openssl_decrypt(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_decrypt(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 5
    if (!IS_STRING_TYPE((args-4)->m_type)) {
      tvCastToStringInPlace(args-4);
    }
  case 4:
    if ((args-3)->m_type != KindOfInt64) {
      tvCastToInt64InPlace(args-3);
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
  fh_openssl_decrypt(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_decrypt(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 3 && count <= 5) {
    if ((count <= 4 || IS_STRING_TYPE((args - 4)->m_type)) &&
        (count <= 3 || (args - 3)->m_type == KindOfInt64) &&
        IS_STRING_TYPE((args - 2)->m_type) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_openssl_decrypt(rv, &args[-0].m_data, &args[-1].m_data, &args[-2].m_data, (count > 3) ? (int)(args[-3].m_data.num) : (int)(0), (count > 4) ? &args[-4].m_data : (Value*)(&null_string));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_decrypt(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_decrypt", count, 3, 5, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 5);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

TypedValue* fh_openssl_digest(TypedValue* _rv, Value* data, Value* method, bool raw_output) asm("_ZN4HPHP16f_openssl_digestERKNS_6StringES2_b");

void fg1_openssl_digest(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_digest(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  switch (count) {
  default: // count >= 3
    if ((args-2)->m_type != KindOfBoolean) {
      tvCastToBooleanInPlace(args-2);
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
  fh_openssl_digest(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
  if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_digest(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count >= 2 && count <= 3) {
    if ((count <= 2 || (args - 2)->m_type == KindOfBoolean) &&
        IS_STRING_TYPE((args - 1)->m_type) &&
        IS_STRING_TYPE((args - 0)->m_type)) {
      fh_openssl_digest(rv, &args[-0].m_data, &args[-1].m_data, (count > 2) ? (bool)(args[-2].m_data.num) : (bool)(false));
      if (rv->m_type == KindOfUninit) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_digest(rv, ar, count);
    }
  } else {
    throw_wrong_arguments_nr("openssl_digest", count, 2, 3, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 3);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_openssl_get_cipher_methods(Value* _rv, bool aliases) asm("_ZN4HPHP28f_openssl_get_cipher_methodsEb");

void fg1_openssl_get_cipher_methods(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_get_cipher_methods(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_openssl_get_cipher_methods(&(rv->m_data), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_get_cipher_methods(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfArray;
      fh_openssl_get_cipher_methods(&(rv->m_data), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_get_cipher_methods(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("openssl_get_cipher_methods", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

Value* fh_openssl_get_md_methods(Value* _rv, bool aliases) asm("_ZN4HPHP24f_openssl_get_md_methodsEb");

void fg1_openssl_get_md_methods(TypedValue* rv, ActRec* ar, int32_t count) __attribute__((noinline,cold));
void fg1_openssl_get_md_methods(TypedValue* rv, ActRec* ar, int32_t count) {
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  tvCastToBooleanInPlace(args-0);
  rv->m_type = KindOfArray;
  fh_openssl_get_md_methods(&(rv->m_data), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
  if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
}

TypedValue* fg_openssl_get_md_methods(ActRec* ar) {
  TypedValue rvSpace;
  TypedValue* rv = &rvSpace;
  int32_t count = ar->numArgs();
  TypedValue* args UNUSED = ((TypedValue*)ar) - 1;
  if (count <= 1) {
    if ((count <= 0 || (args - 0)->m_type == KindOfBoolean)) {
      rv->m_type = KindOfArray;
      fh_openssl_get_md_methods(&(rv->m_data), (count > 0) ? (bool)(args[-0].m_data.num) : (bool)(false));
      if (rv->m_data.num == 0LL) rv->m_type = KindOfNull;
    } else {
      fg1_openssl_get_md_methods(rv, ar, count);
    }
  } else {
    throw_toomany_arguments_nr("openssl_get_md_methods", 1, 1);
    rv->m_data.num = 0LL;
    rv->m_type = KindOfNull;
  }
  frame_free_locals_no_this_inl(ar, 1);
  memcpy(&ar->m_r, rv, sizeof(TypedValue));
  return &ar->m_r;
}

} // namespace HPHP
