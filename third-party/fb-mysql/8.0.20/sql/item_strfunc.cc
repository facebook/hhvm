/*
   Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/**
  @file sql/item_strfunc.cc

  @brief
  This file defines all string Items (e.g. CONCAT).
*/

#include "sql/item_strfunc.h"

#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <zlib.h>
#include <algorithm>
#include <atomic>
#include <cmath>  // std::isfinite
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "base64.h"  // base64_encode_max_arg_length
#include "decimal.h"
#include "m_string.h"
#include "my_aes.h"  // MY_AES_IV_SIZE
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_dir.h"  // For my_stat
#include "my_io.h"
#include "my_macros.h"
#include "my_md5.h"  // MD5_HASH_SIZE
#include "my_md5_size.h"
#include "my_rnd.h"  // my_rand_buffer
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_systime.h"
#include "myisampack.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_password_policy.h"
#include "mysqld_error.h"
#include "mysys_err.h"
#include "password.h"  // my_make_scrambled_password
#include "sha1.h"      // SHA1_HASH_SIZE
#include "sha2.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_password_policy
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"              // current_thd
#include "sql/dd/dd_event.h"              // dd::get_old_interval_type
#include "sql/dd/dd_table.h"              // is_encrypted
#include "sql/dd/info_schema/metadata.h"  // dd::info_schema::get_I_S_view...
#include "sql/dd/info_schema/table_stats.h"
#include "sql/dd/info_schema/tablespace_stats.h"
#include "sql/dd/properties.h"  // dd::Properties
#include "sql/dd/string_type.h"
#include "sql/dd/types/event.h"  // dd::Event::enum_interval_field
#include "sql/dd_sql_view.h"     // push_view_warning_or_error
#include "sql/derror.h"          // ER_THD
#include "sql/error_handler.h"   // Internal_error_handler
#include "sql/events.h"          // Events::reconstruct_interval_expression
#include "sql/filesort.h"
#include "sql/handler.h"
#include "sql/my_decimal.h"
#include "sql/mysqld.h"                             // binary_keyword etc
#include "sql/resourcegroups/resource_group_mgr.h"  // num_vcpus
#include "sql/rpl_gtid.h"
#include "sql/sort_param.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_locale.h"  // my_locale_by_name
#include "sql/sql_show.h"    // grant_types
#include "sql/strfunc.h"     // hexchar_to_int
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/val_int_compare.h"  // Integer_value
#include "template_utils.h"
#include "typelib.h"

using std::max;
using std::min;

static void report_conversion_error(const CHARSET_INFO *to_cs, const char *from,
                                    size_t from_length,
                                    const CHARSET_INFO *from_cs) {
  char printable_buff[32];
  convert_to_printable(printable_buff, sizeof(printable_buff), from,
                       from_length, from_cs, 6);
  const char *from_name =
      native_strcasecmp(from_cs->csname, "utf8") ? from_cs->csname : "utf8mb3";
  const char *to_name =
      native_strcasecmp(to_cs->csname, "utf8") ? to_cs->csname : "utf8mb3";

  my_error(ER_CANNOT_CONVERT_STRING, MYF(0), printable_buff, from_name,
           to_name);
}

/**
  Convert and/or validate input_string according to charset 'to_cs'.

  If input_string needs conversion to 'to_cs' then do the conversion,
  and verify the result.
  Otherwise, if input_string has charset my_charset_bin, then verify
  that it contains a valid string according to 'to_cs'.

  Will call my_error() in case conversion/validation fails.

  @param input_string        string to be converted/validated.
  @param to_cs               result character set
  @param output_string [out] output result variable

  @return nullptr in case of error, otherwise pointer to result.
 */
static String *convert_or_validate_string(String *input_string,
                                          const CHARSET_INFO *to_cs,
                                          String *output_string) {
  String *retval = input_string;
  if (input_string->needs_conversion(to_cs)) {
    uint errors = 0;
    output_string->copy(input_string->ptr(), input_string->length(),
                        input_string->charset(), to_cs, &errors);
    if (errors) {
      report_conversion_error(to_cs, input_string->ptr(),
                              input_string->length(), input_string->charset());
      return nullptr;
    }
    retval = output_string;
  } else if (to_cs != &my_charset_bin &&
             input_string->charset() == &my_charset_bin) {
    if (!input_string->is_valid_string(to_cs)) {
      report_conversion_error(to_cs, input_string->ptr(),
                              input_string->length(), input_string->charset());
      return nullptr;
    }
  }
  return retval;
}

/*
  For the Items which have only val_str_ascii() method
  and don't have their own "native" val_str(),
  we provide a "wrapper" method to convert from ASCII
  to Item character set when it's necessary.
  Conversion happens only in case of "tricky" Item character set (e.g. UCS2).
  Normally conversion does not happen, and val_str_ascii() is immediately
  returned instead.
*/
String *Item_str_func::val_str_from_val_str_ascii(String *str, String *str2) {
  DBUG_ASSERT(fixed == 1);

  if (!(collation.collation->state & MY_CS_NONASCII)) {
    String *res = val_str_ascii(str);
    if (res) res->set_charset(collation.collation);
    return res;
  }

  DBUG_ASSERT(str != str2);

  uint errors;
  String *res = val_str_ascii(str);
  if (!res) return nullptr;

  if ((null_value = str2->copy(res->ptr(), res->length(), &my_charset_latin1,
                               collation.collation, &errors)))
    return nullptr;

  return str2;
}

bool Item_str_func::fix_fields(THD *thd, Item **ref) {
  bool res = Item_func::fix_fields(thd, ref);
  /*
    In Item_str_func::check_well_formed_result() we may set null_value
    flag on the same condition as in test() below.
  */
  maybe_null = (maybe_null || thd->is_strict_sql_mode());
  return res;
}

my_decimal *Item_str_func::val_decimal(my_decimal *decimal_value) {
  DBUG_ASSERT(fixed == 1);
  char buff[64];
  String *res, tmp(buff, sizeof(buff), &my_charset_bin);
  res = val_str(&tmp);
  if (!res) return nullptr;
  (void)str2my_decimal(E_DEC_FATAL_ERROR, res->ptr(), res->length(),
                       res->charset(), decimal_value);
  return decimal_value;
}

double Item_str_func::val_real() {
  DBUG_ASSERT(fixed == 1);
  int err_not_used;
  const char *end_not_used;
  char buff[64];
  String *res, tmp(buff, sizeof(buff), &my_charset_bin);
  res = val_str(&tmp);
  return res ? my_strntod(res->charset(), res->ptr(), res->length(),
                          &end_not_used, &err_not_used)
             : 0.0;
}

longlong Item_str_func::val_int() {
  DBUG_ASSERT(fixed == 1);
  int err;
  char buff[22];
  String *res, tmp(buff, sizeof(buff), &my_charset_bin);
  res = val_str(&tmp);
  return (res ? my_strntoll(res->charset(), res->ptr(), res->length(), 10, NULL,
                            &err)
              : (longlong)0);
}

String *Item_func_md5::val_str_ascii(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *sptr = args[0]->val_str(str);
  str->set_charset(&my_charset_bin);
  if (sptr) {
    uchar digest[MD5_HASH_SIZE] = {0};

    null_value = false;
    int retval = compute_md5_hash((char *)digest, sptr->ptr(), sptr->length());
    if (retval == 1) {
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_SSL_FIPS_MODE_ERROR,
                          ER_THD(current_thd, ER_SSL_FIPS_MODE_ERROR),
                          "FIPS mode ON/STRICT: MD5 digest is not supported.");
    }
    if (str->alloc(32))  // Ensure that memory is free
    {
      null_value = true;
      return nullptr;
    }
    array_to_hex(str->ptr(), digest, MD5_HASH_SIZE);
    str->length((uint)32);
    return str;
  }
  null_value = true;
  return nullptr;
}

/*
  The MD5()/SHA() functions treat their parameter as being a case sensitive.
  Thus we set binary collation on it so different instances of MD5() will be
  compared properly.
*/
static CHARSET_INFO *get_checksum_charset(const char *csname) {
  CHARSET_INFO *cs = get_charset_by_csname(csname, MY_CS_BINSORT, MYF(0));
  if (!cs) {
    // Charset has no binary collation: use my_charset_bin.
    cs = &my_charset_bin;
  }
  return cs;
}

bool Item_func_md5::resolve_type(THD *) {
  CHARSET_INFO *cs = get_checksum_charset(args[0]->collation.collation->csname);
  args[0]->collation.set(cs, DERIVATION_COERCIBLE);
  set_data_type_string(32, default_charset());
  return false;
}

String *Item_func_sha::val_str_ascii(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *sptr = args[0]->val_str(str);
  str->set_charset(&my_charset_bin);
  if (sptr) /* If we got value different from NULL */
  {
    /* Temporary buffer to store 160bit digest */
    uint8 digest[SHA1_HASH_SIZE];
    compute_sha1_hash(digest, sptr->ptr(), sptr->length());
    /* Ensure that memory is free */
    if (!(str->alloc(SHA1_HASH_SIZE * 2))) {
      array_to_hex(str->ptr(), digest, SHA1_HASH_SIZE);
      str->length((uint)SHA1_HASH_SIZE * 2);
      null_value = false;
      return str;
    }
  }
  null_value = true;
  return nullptr;
}

bool Item_func_sha::resolve_type(THD *) {
  CHARSET_INFO *cs = get_checksum_charset(args[0]->collation.collation->csname);
  args[0]->collation.set(cs, DERIVATION_COERCIBLE);
  // size of hex representation of hash
  set_data_type_string(SHA1_HASH_SIZE * 2, default_charset());
  return false;
}

/*
  SHA2(str, hash_length)
  The second argument indicates the desired bit length of the
  result, which must have a value of 224, 256, 384, 512, or 0
  (which is equivalent to 256).
*/
String *Item_func_sha2::val_str_ascii(String *str) {
  DBUG_ASSERT(fixed == 1);
  unsigned char digest_buf[SHA512_DIGEST_LENGTH];
  uint digest_length = 0;

  String *input_string = args[0]->val_str(str);
  str->set_charset(&my_charset_bin);

  if (input_string == nullptr) {
    null_value = true;
    return (String *)nullptr;
  }

  null_value = args[0]->null_value;
  if (null_value) return nullptr;

  const unsigned char *input_ptr =
      pointer_cast<const unsigned char *>(input_string->ptr());
  size_t input_len = input_string->length();

  longlong hash_length = args[1]->val_int();
  null_value = args[1]->null_value;
  // Give error message in switch below.
  if (null_value) hash_length = -1;

  switch (hash_length) {
#ifndef OPENSSL_NO_SHA512
    case 512:
      digest_length = SHA512_DIGEST_LENGTH;
      (void)SHA_EVP512(input_ptr, input_len, digest_buf);
      break;
    case 384:
      digest_length = SHA384_DIGEST_LENGTH;
      (void)SHA_EVP384(input_ptr, input_len, digest_buf);
      break;
#endif
#ifndef OPENSSL_NO_SHA256
    case 224:
      digest_length = SHA224_DIGEST_LENGTH;
      (void)SHA_EVP224(input_ptr, input_len, digest_buf);
      break;
    case 256:
    case 0:  // SHA-256 is the default
      digest_length = SHA256_DIGEST_LENGTH;
      (void)SHA_EVP256(input_ptr, input_len, digest_buf);
      break;
#endif
    default:
      // For const values we have already warned in resolve_type().
      if (!args[1]->const_item())
        push_warning_printf(
            current_thd, Sql_condition::SL_WARNING,
            ER_WRONG_PARAMETERS_TO_NATIVE_FCT,
            ER_THD(current_thd, ER_WRONG_PARAMETERS_TO_NATIVE_FCT), "sha2");
      null_value = true;
      return nullptr;
  }

  /*
    Since we're subverting the usual String methods, we must make sure that
    the destination has space for the bytes we're about to write.
  */
  str->mem_realloc(digest_length * 2 + 1); /* Each byte as two nybbles */

  /* Convert the large number to a string-hex representation. */
  array_to_hex(str->ptr(), digest_buf, digest_length);

  /* We poked raw bytes in.  We must inform the the String of its length. */
  str->length(digest_length * 2); /* Each byte as two nybbles */

  null_value = false;
  return str;
}

bool Item_func_sha2::resolve_type(THD *thd) {
  maybe_null = true;
  longlong sha_variant;
  if (args[1]->const_item()) {
    sha_variant = args[1]->val_int();
    // Give error message in switch below.
    if (args[1]->null_value) sha_variant = -1;
  } else {
    sha_variant = 512;
  }

  switch (sha_variant) {
#ifndef OPENSSL_NO_SHA512
    case 512:
      set_data_type_string(SHA512_DIGEST_LENGTH * 2, default_charset());
      break;
    case 384:
      set_data_type_string(SHA384_DIGEST_LENGTH * 2, default_charset());
      break;
#endif
#ifndef OPENSSL_NO_SHA256
    case 256:
    case 0:  // SHA-256 is the default
      set_data_type_string(SHA256_DIGEST_LENGTH * 2, default_charset());
      break;
#endif
    case 224:
      set_data_type_string(SHA224_DIGEST_LENGTH * 2, default_charset());
      break;
    default:
      set_data_type_string(SHA256_DIGEST_LENGTH * 2, default_charset());
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_WRONG_PARAMETERS_TO_NATIVE_FCT,
          ER_THD(thd, ER_WRONG_PARAMETERS_TO_NATIVE_FCT), "sha2");
  }

  CHARSET_INFO *cs = get_checksum_charset(args[0]->collation.collation->csname);
  args[0]->collation.set(cs, DERIVATION_COERCIBLE);
  return false;
}

/* Implementation of AES encryption routines */

/** helper class to process an IV argument to aes_encrypt/aes_decrypt */
class iv_argument {
  char iv_buff[MY_AES_IV_SIZE + 1];  // +1 to cater for the terminating NULL
  String tmp_iv_value;

 public:
  iv_argument() : tmp_iv_value(iv_buff, sizeof(iv_buff), system_charset_info) {}

  /**
    Validate the arguments and retrieve the IV value.

    Processes a 3d optional IV argument to an Item_func function.
    Contains all the necessary stack buffers etc.

    @param aes_opmode  the encryption mode
    @param arg_count   number of parameters passed to the function
    @param args        array of arguments passed to the function
    @param func_name   the name of the function (for errors)
    @param thd         the current thread (for errors)
    @param [out] error_generated  set to true if error was generated.

    @return a pointer to the retrived validated IV or NULL
  */
  const unsigned char *retrieve_iv_ptr(enum my_aes_opmode aes_opmode,
                                       uint arg_count, Item **args,
                                       const char *func_name, THD *thd,
                                       bool *error_generated) {
    const unsigned char *iv_str = nullptr;

    *error_generated = false;

    if (my_aes_needs_iv(aes_opmode)) {
      /* we only enforce the need for IV */
      if (arg_count == 3) {
        String *iv = args[2]->val_str(&tmp_iv_value);
        if (!iv || iv->length() < MY_AES_IV_SIZE) {
          my_error(ER_AES_INVALID_IV, MYF(0), func_name,
                   (long long)MY_AES_IV_SIZE);
          *error_generated = true;
          return nullptr;
        }
        iv_str = (unsigned char *)iv->ptr();
      } else {
        my_error(ER_WRONG_PARAMCOUNT_TO_NATIVE_FCT, MYF(0), func_name);
        *error_generated = true;
        return nullptr;
      }
    } else {
      if (arg_count == 3) {
        push_warning_printf(thd, Sql_condition::SL_WARNING, WARN_OPTION_IGNORED,
                            ER_THD(thd, WARN_OPTION_IGNORED), "IV");
      }
    }
    return iv_str;
  }
};

bool Item_func_aes_encrypt::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  /* Unsafe for SBR since result depends on a session variable */
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  /* Not safe to cache either */
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

String *Item_func_aes_encrypt::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  char key_buff[80];
  String tmp_key_value(key_buff, sizeof(key_buff), system_charset_info);
  String *sptr, *key;
  int aes_length;
  THD *thd = current_thd;
  ulong aes_opmode;
  iv_argument iv_arg;
  DBUG_TRACE;

  sptr = args[0]->val_str(str);            // String to encrypt
  key = args[1]->val_str(&tmp_key_value);  // key
  aes_opmode = thd->variables.my_aes_mode;

  DBUG_ASSERT(aes_opmode <= MY_AES_END);

  if (sptr && key)  // we need both arguments to be not NULL
  {
    const unsigned char *iv_str =
        iv_arg.retrieve_iv_ptr((enum my_aes_opmode)aes_opmode, arg_count, args,
                               func_name(), thd, &null_value);
    if (null_value) return nullptr;

    // Calculate result length
    aes_length =
        my_aes_get_size(sptr->length(), (enum my_aes_opmode)aes_opmode);

    tmp_value.set_charset(&my_charset_bin);
    if (!tmp_value.alloc(aes_length))  // Ensure that memory is free
    {
      // finally encrypt directly to allocated buffer.
      if (my_aes_encrypt((unsigned char *)sptr->ptr(), sptr->length(),
                         (unsigned char *)tmp_value.ptr(),
                         (unsigned char *)key->ptr(), key->length(),
                         (enum my_aes_opmode)aes_opmode,
                         iv_str) == aes_length) {
        // We got the expected result length
        tmp_value.length(static_cast<size_t>(aes_length));
        return &tmp_value;
      }
    }
  }
  null_value = true;
  return nullptr;
}

bool Item_func_aes_encrypt::resolve_type(THD *thd) {
  ulong aes_opmode = thd->variables.my_aes_mode;
  DBUG_ASSERT(aes_opmode <= MY_AES_END);

  set_data_type_string((uint)my_aes_get_size(args[0]->max_length,
                                             (enum my_aes_opmode)aes_opmode));
  return false;
}

bool Item_func_aes_decrypt::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  /* Unsafe for SBR since result depends on a session variable */
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  /* Not safe to cache either */
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

String *Item_func_aes_decrypt::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  char key_buff[80];
  String tmp_key_value(key_buff, sizeof(key_buff), system_charset_info);
  String *sptr, *key;
  THD *thd = current_thd;
  ulong aes_opmode;
  iv_argument iv_arg;
  DBUG_TRACE;

  sptr = args[0]->val_str(str);            // String to decrypt
  key = args[1]->val_str(&tmp_key_value);  // Key
  aes_opmode = thd->variables.my_aes_mode;
  DBUG_ASSERT(aes_opmode <= MY_AES_END);

  if (sptr && key)  // Need to have both arguments not NULL
  {
    const unsigned char *iv_str =
        iv_arg.retrieve_iv_ptr((enum my_aes_opmode)aes_opmode, arg_count, args,
                               func_name(), thd, &null_value);
    if (null_value) return nullptr;
    str_value.set_charset(&my_charset_bin);
    if (!str_value.alloc(sptr->length()))  // Ensure that memory is free
    {
      // finally decrypt directly to allocated buffer.
      int length;
      length = my_aes_decrypt((unsigned char *)sptr->ptr(), sptr->length(),
                              (unsigned char *)str_value.ptr(),
                              (unsigned char *)key->ptr(), key->length(),
                              (enum my_aes_opmode)aes_opmode, iv_str);
      if (length >= 0)  // if we got correct data data
      {
        str_value.length((uint)length);
        return &str_value;
      }
    }
  }
  // Bad parameters. No memory or bad data will all go here
  null_value = true;
  return nullptr;
}

bool Item_func_aes_decrypt::resolve_type(THD *) {
  set_data_type_string(args[0]->max_char_length());
  maybe_null = true;
  return false;
}

bool Item_func_random_bytes::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;

  /* it is unsafe for SBR since it uses crypto random from the ssl library */
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  /* Not safe to cache either */
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_RAND);
  return false;
}

/*
  Artificially limited to 1k to avoid excessive memory usage.
  The SSL lib supports up to INT_MAX.
*/
const ulonglong Item_func_random_bytes::MAX_RANDOM_BYTES_BUFFER = 1024ULL;

bool Item_func_random_bytes::resolve_type(THD *) {
  set_data_type_string(MAX_RANDOM_BYTES_BUFFER, &my_charset_bin);
  return false;
}

String *Item_func_random_bytes::val_str(String *) {
  DBUG_ASSERT(fixed == 1);
  ulonglong n_bytes = args[0]->val_uint();
  null_value = args[0]->null_value;

  if (null_value) return nullptr;

  str_value.set_charset(&my_charset_bin);

  if (n_bytes == 0 || n_bytes > MAX_RANDOM_BYTES_BUFFER) {
    my_error(ER_DATA_OUT_OF_RANGE, MYF(0), "length", func_name());
    null_value = true;
    return nullptr;
  }

  if (str_value.alloc(n_bytes)) {
    my_error(ER_OUTOFMEMORY, n_bytes);
    null_value = true;
    return nullptr;
  }

  str_value.set_charset(&my_charset_bin);

  if (my_rand_buffer((unsigned char *)str_value.ptr(), n_bytes)) {
    my_error(ER_ERROR_WHEN_EXECUTING_COMMAND, MYF(0), func_name(),
             "SSL library can't generate random bytes");
    null_value = true;
    return nullptr;
  }

  str_value.length(n_bytes);
  return &str_value;
}

bool Item_func_to_base64::resolve_type(THD *thd) {
  maybe_null = args[0]->maybe_null;
  collation.set(default_charset(), DERIVATION_COERCIBLE, MY_REPERTOIRE_ASCII);
  if (args[0]->max_length > (uint)base64_encode_max_arg_length()) {
    maybe_null = true;
    set_data_type_string((ulonglong)base64_encode_max_arg_length());
  } else {
    uint64 length = base64_needed_encoded_length((uint64)args[0]->max_length);
    DBUG_ASSERT(length > 0);
    set_data_type_string((ulonglong)length - 1);
    maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
  }
  return false;
}

String *Item_func_to_base64::val_str_ascii(String *str) {
  String *res = args[0]->val_str(str);
  bool too_long = false;
  uint64 length;
  if (!res || res->length() > (uint)base64_encode_max_arg_length() ||
      (too_long =
           ((length = base64_needed_encoded_length((uint64)res->length())) >
            current_thd->variables.max_allowed_packet)) ||
      tmp_value.alloc((uint)length)) {
    null_value = true;  // NULL input, too long input, or OOM.
    if (too_long) {
      return push_packet_overflow_warning(current_thd, func_name());
    }
    return nullptr;
  }
  base64_encode(res->ptr(), (int)res->length(), tmp_value.ptr());
  DBUG_ASSERT(length > 0);
  tmp_value.length((uint)length - 1);  // Without trailing '\0'
  null_value = false;
  return &tmp_value;
}

bool Item_func_from_base64::resolve_type(THD *) {
  if (args[0]->max_length > (uint)base64_decode_max_arg_length()) {
    set_data_type_string(ulonglong(base64_decode_max_arg_length()));
  } else {
    uint64 length = base64_needed_decoded_length((uint64)args[0]->max_length);
    set_data_type_string(ulonglong(length));
  }
  maybe_null = true;  // Can be NULL, e.g. in case of badly formed input string
  return false;
}

String *Item_func_from_base64::val_str(String *str) {
  String *res = args[0]->val_str_ascii(str);
  bool too_long = false;
  int64 length;
  const char *end_ptr;

  if (!res || res->length() > (uint)base64_decode_max_arg_length() ||
      (too_long = ((uint64)(length = base64_needed_decoded_length(
                                (uint64)res->length())) >
                   current_thd->variables.max_allowed_packet)) ||
      tmp_value.alloc((uint)length) ||
      (length = base64_decode(res->ptr(), (uint64)res->length(),
                              tmp_value.ptr(), &end_ptr, 0)) < 0 ||
      end_ptr < res->ptr() + res->length()) {
    null_value =
        true;  // NULL input, too long input, OOM, or badly formed input
    if (too_long) {
      return push_packet_overflow_warning(current_thd, func_name());
    }
    return nullptr;
  }
  tmp_value.length((uint)length);
  null_value = false;
  return &tmp_value;
}

namespace {

/**
  Because it's not possible to disentangle the state of the parser from the
  THD, we have to destructively modify the current THD object in order to
  parse. This class backs up and restores members that are modified in
  Item_func_statement_digest::val_str_ascii. It also sports its own
  Query_arena and LEX objects, which are used during parsing.
*/
class Thd_parse_modifier {
 public:
  Thd_parse_modifier(THD *thd, uchar *token_buffer)
      : m_thd(thd),
        m_arena(&m_mem_root, Query_arena::STMT_REGULAR_EXECUTION),
        m_backed_up_lex(thd->lex),
        m_saved_parser_state(thd->m_parser_state),
        m_saved_digest(thd->m_digest),
        m_cs(thd->variables.character_set_client) {
    thd->m_digest = &m_digest_state;
    m_digest_state.reset(token_buffer, get_max_digest_length());
    m_arena.set_query_arena(*thd);
    thd->lex = &m_lex;
    lex_start(thd);
  }

  ~Thd_parse_modifier() {
    lex_end(&m_lex);
    m_thd->lex = m_backed_up_lex;
    m_thd->set_query_arena(m_arena);
    m_thd->m_parser_state = m_saved_parser_state;
    m_thd->m_digest = m_saved_digest;
    m_thd->variables.character_set_client = m_cs;
    m_thd->update_charset();
  }

 private:
  THD *m_thd;
  MEM_ROOT m_mem_root;
  Query_arena m_arena;
  LEX *m_backed_up_lex;
  LEX m_lex;
  sql_digest_state m_digest_state;
  Parser_state *m_saved_parser_state;
  sql_digest_state *m_saved_digest;
  const CHARSET_INFO *m_cs;
};

/**
  Error handler that wraps parse error messages, removes details and silences
  warnings.

  We don't want statement_digest() to raise warnings about deprecated syntax
  or semantic problems. This is likely not interesting to the
  caller. Therefore this handler issues a blanket silencing of all warnings.

  The reason we want to anonymize parse errors is to avoid leaking information
  in error messages that may be unintentionally visible to users of an
  application. For instance an application may in error insert an expression
  instead of a string:

    SELECT statement_digest( (SELECT * FROM( SELECT user() ) t) );

  The parser would normally raise an error saying:

    You have an error in your SQL syntax; /.../ near 'root@localhost'

  thus leaking data from the `user` table. Therefore, the errors are in this
  not disclosed.
*/
class Parse_error_anonymizer : public Internal_error_handler {
 public:
  Parse_error_anonymizer(THD *thd, Item *arg) : m_thd(thd), m_arg(arg) {
    thd->push_internal_handler(this);
  }

  bool handle_condition(THD *, uint, const char *,
                        Sql_condition::enum_severity_level *level,
                        const char *message) override {
    // Silence warnings.
    if (*level == Sql_condition::SL_WARNING) return true;

    // We pretend we're not here if already inside a call to handle_condition().
    if (is_handling) return false;

    is_handling = true;

    if (m_arg->basic_const_item())
      // Ok, it's a literal, we can print the whole error message.
      my_error(ER_PARSE_ERROR_IN_DIGEST_FN, MYF(0), message);
    else
      // The argument is an expression, potentially from malicious use, let's
      // not disclose anything.
      my_error(ER_UNDISCLOSED_PARSE_ERROR_IN_DIGEST_FN, MYF(0));

    is_handling = false;

    return true;
  }

  ~Parse_error_anonymizer() override { m_thd->pop_internal_handler(); }

 private:
  THD *m_thd;
  Item *m_arg;

  /// This avoids infinte recursion through my_error().
  bool is_handling = false;
};

/**
  Parses a string and fills the token buffer.

  The parser function THD::sql_parser() is called directly instead of
  parse_sql(), as the latter assumes that it is called with the intent to record
  the statement in performance_schema and later execute it, neither of which is
  called for here. In fact we hardly need the parser to calculate a digest,
  since it is calculated from the token stream. There are only some corner cases
  where `NULL` is sometimes a literal and sometimes an operator, as in
  `IS NULL`, `IS NOT NULL`.

  @param thd Session object used by the parser.

  @param statement_expr The expression that evaluates to something that
  can be parsed. Needed for error messages in case we don't want to disclose
  what it evaluates to.

  @param statement_string The non-NULL string resulting from evaluating
  statement_expr. The caller is preferred to do this as this function doesn't
  deal with NULL values.

  @retval true Error.
  @retval false All went well, the digest information is in THD::m_digest.
*/
bool parse(THD *thd, Item *statement_expr, String *statement_string) {
  // The lexer can't handle non-zero-length strings starting with NUL and we
  // can't return NULL for them because this function is declared
  // nonnullable.
  if (statement_string->length() > 0 && (*statement_string)[0] == '\0')
    statement_string->length(0);

  const CHARSET_INFO *cs = statement_string->charset();
  thd->variables.character_set_client = cs;
  thd->update_charset();

  Parser_state ps;

  // The lexer needs null-terminated strings, despite boasting the below
  // interface. Hence the use of c_ptr_safe().
  if (ps.init(thd, statement_string->c_ptr_safe(), statement_string->length()))
    return true;

  ps.m_lip.multi_statements = false;
  ps.m_lip.m_digest = thd->m_digest;
  ps.m_lip.m_digest->m_digest_storage.m_charset_number = cs->number;

  thd->m_parser_state = &ps;

  {
    Parse_error_anonymizer pea(thd, statement_expr);
    if (thd->sql_parser()) {
      return true;
    }
  }

  return false;
}

}  // namespace

bool Item_func_statement_digest::resolve_type(THD *thd) {
  set_data_type_string(DIGEST_HASH_TO_STRING_LENGTH, default_charset());
  m_token_buffer = static_cast<uchar *>(thd->alloc(get_max_digest_length()));
  if (m_token_buffer == nullptr) return true;
  return false;
}

/**
  Implementation of the STATEMENT_DIGEST() native function.

  @param buf A String object that we can write to.

  @return The same string object, or nullptr in case of error or null return.
*/
String *Item_func_statement_digest::val_str_ascii(String *buf) {
  DBUG_TRACE;

  String *statement_string = args[0]->val_str(buf);

  // This function is non-nullable, meaning it doesn't return NULL, unless the
  // argument is NULL.
  if (statement_string == nullptr) return null_return_str();
  null_value = false;

  uchar digest[DIGEST_HASH_SIZE];
  {
    THD *thd = current_thd;
    Thd_parse_modifier thd_mod(thd, m_token_buffer);

    if (parse(thd, args[0], statement_string)) return error_str();
    compute_digest_hash(&thd->m_digest->m_digest_storage, digest);
  }

  if (buf->reserve(DIGEST_HASH_TO_STRING_LENGTH)) return error_str();
  buf->length(DIGEST_HASH_TO_STRING_LENGTH);
  DIGEST_HASH_TO_STRING(digest, buf->c_ptr_quick());
  return buf;
}

bool Item_func_statement_digest_text::resolve_type(THD *thd) {
  set_data_type_string(MAX_BLOB_WIDTH, args[0]->collation);
  m_token_buffer = static_cast<uchar *>(thd->alloc(get_max_digest_length()));
  if (m_token_buffer == nullptr) return true;
  return false;
}

String *Item_func_statement_digest_text::val_str(String *buf) {
  DBUG_TRACE;

  String *statement_string = args[0]->val_str(buf);

  // This function is non-nullable, meaning it doesn't return NULL, unless the
  // argument is NULL.
  if (statement_string == nullptr) return null_return_str();
  null_value = false;

  THD *thd = current_thd;
  Thd_parse_modifier thd_mod(thd, m_token_buffer);

  if (parse(thd, args[0], statement_string)) return error_str();

  compute_digest_text(&thd->m_digest->m_digest_storage, buf);

  return buf;
}

/**
  Concatenate args with the following premises:
  If only one arg (which is ok), return value of arg;
*/

String *Item_func_concat::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res;

  THD *thd = current_thd;
  null_value = false;
  tmp_value.length(0);
  for (uint i = 0; i < arg_count; ++i) {
    if (!(res = args[i]->val_str(str))) {
      if (thd->is_error()) return error_str();

      DBUG_ASSERT(maybe_null);
      null_value = true;
      return nullptr;
    }
    if (res->length() + tmp_value.length() >
        thd->variables.max_allowed_packet) {
      return push_packet_overflow_warning(thd, func_name());
    }
    if (tmp_value.append(*res)) return error_str();
  }
  res = &tmp_value;
  res->set_charset(collation.collation);
  return res;
}

bool Item_func_concat::resolve_type(THD *thd) {
  ulonglong char_length = 0;

  if (agg_arg_charsets_for_string_result(collation, args, arg_count))
    return true;

  for (uint i = 0; i < arg_count; i++)
    char_length += args[i]->max_char_length();

  set_data_type_string(char_length);
  maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
  return false;
}

/**
  concat with separator. First arg is the separator
  concat_ws takes at least two arguments.
*/

String *Item_func_concat_ws::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  char tmp_str_buff[10];
  String tmp_sep_str(tmp_str_buff, sizeof(tmp_str_buff), default_charset_info);
  String *sep_str, *res = nullptr, *res2;
  uint i;

  THD *thd = current_thd;
  null_value = false;
  if (!(sep_str = args[0]->val_str(&tmp_sep_str))) return error_str();
  tmp_value.length(0);

  // Skip until non-null argument is found.
  // If not, return the empty string
  for (i = 1; i < arg_count; i++)
    if ((res = args[i]->val_str(str))) {
      break;
    }

  if (i == arg_count) return make_empty_result();

  if (tmp_value.append(*res)) return error_str();

  for (i++; i < arg_count; i++) {
    if (!(res2 = args[i]->val_str(str))) continue;  // Skip NULL

    if (tmp_value.length() + sep_str->length() + res2->length() >
        thd->variables.max_allowed_packet) {
      return push_packet_overflow_warning(thd, func_name());
    }
    if (tmp_value.append(*sep_str)) return error_str();
    if (tmp_value.append(*res2)) return error_str();
  }
  res = &tmp_value;
  res->set_charset(collation.collation);
  return res;
}

bool Item_func_concat_ws::resolve_type(THD *thd) {
  ulonglong char_length;

  if (agg_arg_charsets_for_string_result(collation, args, arg_count))
    return true;

  DBUG_ASSERT(arg_count >= 2);
  char_length = (ulonglong)args[0]->max_char_length() * (arg_count - 2);
  for (uint i = 1; i < arg_count; i++)
    char_length += args[i]->max_char_length();

  set_data_type_string(char_length);
  maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
  return false;
}

String *Item_func_reverse::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(str);
  const char *ptr, *end;
  char *tmp;

  if ((null_value = args[0]->null_value)) return nullptr;
  /* An empty string is a special case as the string pointer may be null */
  if (!res->length()) return make_empty_result();
  if (tmp_value.alloced_length() < res->length() &&
      tmp_value.mem_realloc(res->length())) {
    null_value = true;
    return nullptr;
  }
  tmp_value.length(res->length());
  tmp_value.set_charset(res->charset());
  ptr = res->ptr();
  end = ptr + res->length();
  tmp = tmp_value.ptr() + tmp_value.length();
  if (use_mb(res->charset())) {
    uint32 l;
    while (ptr < end) {
      if ((l = my_ismbchar(res->charset(), ptr, end))) {
        tmp -= l;
        DBUG_ASSERT(tmp >= tmp_value.ptr());
        memcpy(tmp, ptr, l);
        ptr += l;
      } else
        *--tmp = *ptr++;
    }
  } else {
    while (ptr < end) *--tmp = *ptr++;
  }
  return &tmp_value;
}

bool Item_func_reverse::resolve_type(THD *) {
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;
  DBUG_ASSERT(collation.collation != nullptr);
  set_data_type_string(args[0]->max_char_length());
  return false;
}

/**
  Replace all occurences of string2 in string1 with string3.
*/

String *Item_func_replace::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);

  String *res1 = args[0]->val_str(str);
  if ((null_value = args[0]->null_value)) return nullptr;
  String *res2 = args[1]->val_str(&tmp_value);
  if ((null_value = args[1]->null_value)) return nullptr;
  String *res3 = args[2]->val_str(&tmp_value2);
  if ((null_value = args[2]->null_value)) return nullptr;

  res1->set_charset(collation.collation);
  if (res1->length() == 0 || res2->length() == 0) return res1;

  tmp_value_res.length(0);
  tmp_value_res.set_charset(collation.collation);
  String *result = &tmp_value_res;

  StringBuffer<STRING_BUFFER_USUAL_SIZE> res2_converted(nullptr);
  StringBuffer<STRING_BUFFER_USUAL_SIZE> res3_converted(nullptr);

  res2 = convert_or_validate_string(res2, collation.collation, &res2_converted);
  if (res2 == nullptr) return error_str();

  res3 = convert_or_validate_string(res3, collation.collation, &res3_converted);
  if (res3 == nullptr) return error_str();

  THD *thd = current_thd;
  const unsigned long max_size = thd->variables.max_allowed_packet;

  const char *search = res2->ptr();
  const size_t from_length = res2->length();
  const char *search_end = search + from_length;
  const size_t to_length = res3->length();
  const char *ptr = res1->ptr();
  const char *strend = res1->ptr() + res1->length();
  while (ptr < strend) {
    if (ptr + from_length <= strend && std::equal(search, search_end, ptr)) {
      if (to_length > from_length &&
          result->length() + (to_length - from_length) + (strend - ptr) >
              max_size) {
        return push_packet_overflow_warning(thd, func_name());
      }
      if (result->append(*res3)) return error_str();
      ptr += from_length;
    } else {
      bool err = false;
      uint32 l = use_mb(res1->charset())
                     ? my_ismbchar(res1->charset(), ptr, strend)
                     : 0;
      if (l != 0)
        while (l-- > 0) err |= result->append(*ptr++);
      else
        err = result->append(*ptr++);

      if (err) return error_str();
    }
  }
  return result;
}

bool Item_func_replace::resolve_type(THD *thd) {
  ulonglong char_length = args[0]->max_char_length();
  int diff = (int)(args[2]->max_char_length() - args[1]->max_char_length());
  if (diff > 0 && args[1]->max_char_length()) {  // Calculate of maxreplaces
    ulonglong max_substrs = char_length / args[1]->max_char_length();
    char_length += max_substrs * (uint)diff;
  }

  // We let the first argument (only) determine the character set of the result.
  // REPLACE(str, from_str, to_str)
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;
  set_data_type_string(char_length);
  maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
  return false;
}

String *Item_func_insert::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res, *res2;
  longlong start, length, orig_len; /* must be longlong to avoid truncation */

  null_value = false;
  res = args[0]->val_str(str);
  res2 = args[3]->val_str(&tmp_value);
  start = args[1]->val_int();
  length = args[2]->val_int();

  if (args[0]->null_value || args[1]->null_value || args[2]->null_value ||
      args[3]->null_value) {
    DBUG_ASSERT(maybe_null);
    return error_str(); /* purecov: inspected */
  }
  orig_len = static_cast<longlong>(res->length());

  if ((start < 1) || (start > orig_len))
    return res;  // Wrong param; skip insert

  --start;  // Internal start from '0'

  if ((length < 0) || (length > orig_len)) length = orig_len;

  /*
    There is one exception not handled (intentionaly) by the character set
    aggregation code. If one string is strong side and is binary, and
    another one is weak side and is a multi-byte character string,
    then we need to operate on the second string in terms on bytes when
    calling ::numchars() and ::charpos(), rather than in terms of characters.
    Lets substitute its character set to binary.
  */
  if (collation.collation == &my_charset_bin) {
    res->set_charset(&my_charset_bin);
    res2->set_charset(&my_charset_bin);
  }

  /* start and length are now sufficiently valid to pass to charpos function */
  start = res->charpos((int)start);
  length = res->charpos((int)length, (uint32)start);

  /* Re-testing with corrected params */
  if (start > orig_len)
    return res; /* purecov: inspected */  // Wrong param; skip insert
  if (length > orig_len - start) length = orig_len - start;

  if ((ulonglong)(orig_len - length + res2->length()) >
      (ulonglong)current_thd->variables.max_allowed_packet) {
    return push_packet_overflow_warning(current_thd, func_name());
  }
  if (res->uses_buffer_owned_by(str)) {
    if (tmp_value_res.alloc(orig_len) || tmp_value_res.copy(*res))
      return error_str();
    res = &tmp_value_res;
  } else
    res = copy_if_not_alloced(str, res, orig_len);

  res->replace((uint32)start, (uint32)length, *res2);
  return res;
}

bool Item_func_insert::resolve_type(THD *thd) {
  // Handle character set for args[0] and args[3].
  if (agg_arg_charsets_for_string_result(collation, args, 2, 3)) return true;
  ulonglong length = ulonglong{args[0]->max_char_length()} +
                     ulonglong{args[3]->max_char_length()};
  set_data_type_string(length);
  maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
  return false;
}

String *Item_str_conv::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res;
  if (!(res = args[0]->val_str(str))) {
    null_value = true; /* purecov: inspected */
    return nullptr;    /* purecov: inspected */
  }
  null_value = false;
  if (multiply == 1) {
    size_t len;
    if (res->uses_buffer_owned_by(str)) {
      if (tmp_value.copy(*res)) return error_str();
      res = &tmp_value;
    } else
      res = copy_if_not_alloced(str, res, res->length());

    len = converter(collation.collation, res->ptr(), res->length(), res->ptr(),
                    res->length());
    DBUG_ASSERT(len <= res->length());
    res->length(len);
  } else {
    size_t len = res->length() * multiply;
    tmp_value.alloc(len);
    tmp_value.set_charset(collation.collation);
    len = converter(collation.collation, res->ptr(), res->length(),
                    tmp_value.ptr(), len);
    tmp_value.length(len);
    res = &tmp_value;
  }
  return res;
}

bool Item_func_lower::resolve_type(THD *) {
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;

  DBUG_ASSERT(collation.collation != nullptr);
  multiply = collation.collation->casedn_multiply;
  converter = collation.collation->cset->casedn;
  set_data_type_string(args[0]->max_char_length() * multiply);
  return false;
}

bool Item_func_upper::resolve_type(THD *) {
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;

  DBUG_ASSERT(collation.collation != nullptr);
  multiply = collation.collation->caseup_multiply;
  converter = collation.collation->cset->caseup;
  set_data_type_string(args[0]->max_char_length() * multiply);
  return false;
}

String *Item_func_left::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(str);

  /* must be longlong to avoid truncation */
  longlong length = args[1]->val_int();
  size_t char_pos;

  if ((null_value = (args[0]->null_value || args[1]->null_value)))
    return nullptr;

  /* if "unsigned_flag" is set, we have a *huge* positive number. */
  if ((length <= 0) && (!args[1]->unsigned_flag)) return make_empty_result();
  if ((res->length() <= (ulonglong)length) ||
      (res->length() <= (char_pos = res->charpos((int)length))))
    return res;

  tmp_value.set(*res, 0, char_pos);
  return &tmp_value;
}

void Item_str_func::left_right_max_length() {
  uint32 char_length = args[0]->max_char_length();
  if (args[1]->const_item()) {
    longlong length = args[1]->val_int();
    if (args[1]->null_value) goto end;

    Integer_value length_val(length, args[1]->unsigned_flag);
    if (length_val.is_negative())
      char_length = 0;
    else if (length_val <= Integer_value(INT_MAX32, false))
      char_length = min(char_length, static_cast<uint32>(length));
  }

end:
  set_data_type_string(char_length);
}

String *Item_str_func::push_packet_overflow_warning(THD *thd,
                                                    const char *func) {
  push_warning_printf(thd, Sql_condition::SL_WARNING,
                      ER_WARN_ALLOWED_PACKET_OVERFLOWED,
                      ER_THD(thd, ER_WARN_ALLOWED_PACKET_OVERFLOWED), func,
                      thd->variables.max_allowed_packet);
  DBUG_ASSERT(maybe_null);
  return error_str();
}

bool Item_func_left::resolve_type(THD *) {
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;
  DBUG_ASSERT(collation.collation != nullptr);
  left_right_max_length();
  return false;
}

String *Item_func_right::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(str);
  /* must be longlong to avoid truncation */
  longlong length = args[1]->val_int();

  if ((null_value = (args[0]->null_value || args[1]->null_value)))
    return nullptr; /* purecov: inspected */

  /* if "unsigned_flag" is set, we have a *huge* positive number. */
  if ((length <= 0) && (!args[1]->unsigned_flag))
    return make_empty_result(); /* purecov: inspected */

  if (res->length() <= (ulonglong)length) return res; /* purecov: inspected */

  size_t start = res->numchars();
  if (start <= (uint)length) return res;
  start = res->charpos(start - (uint)length);
  tmp_value.set(*res, start, res->length() - start);
  return &tmp_value;
}

bool Item_func_right::resolve_type(THD *) {
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;

  DBUG_ASSERT(collation.collation != nullptr);
  left_right_max_length();
  return false;
}

String *Item_func_substr::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(str);
  /* must be longlong to avoid truncation */
  longlong start = args[1]->val_int();
  /* Assumes that the maximum length of a String is < INT_MAX32. */
  /* Limit so that code sees out-of-bound value properly. */
  longlong length = arg_count == 3 ? args[2]->val_int() : INT_MAX32;
  longlong tmp_length;

  if ((null_value = (args[0]->null_value || args[1]->null_value ||
                     (arg_count == 3 && args[2]->null_value))))
    return nullptr; /* purecov: inspected */

  /* Negative or zero length, will return empty string. */
  if ((arg_count == 3) && (length <= 0) &&
      (length == 0 || !args[2]->unsigned_flag))
    return make_empty_result();

  /* Assumes that the maximum length of a String is < INT_MAX32. */
  /* Set here so that rest of code sees out-of-bound value as such. */
  if ((length <= 0) || (length > INT_MAX32)) length = INT_MAX32;

  /* if "unsigned_flag" is set, we have a *huge* positive number. */
  /* Assumes that the maximum length of a String is < INT_MAX32. */
  if ((!args[1]->unsigned_flag && (start < INT_MIN32 || start > INT_MAX32)) ||
      (args[1]->unsigned_flag && ((ulonglong)start > INT_MAX32)))
    return make_empty_result();

  start = ((start < 0) ? res->numchars() + start : start - 1);
  start = res->charpos((int)start);
  if ((start < 0) || (start + 1 > static_cast<longlong>(res->length())))
    return make_empty_result();

  length = res->charpos((int)length, (uint32)start);
  tmp_length = static_cast<longlong>(res->length()) - start;
  length = min(length, tmp_length);

  if (!start && (longlong)res->length() == length) return res;
  tmp_value.set(*res, (uint32)start, (uint32)length);
  return &tmp_value;
}

bool Item_func_substr::resolve_type(THD *) {
  uint32 max_char_length = args[0]->max_char_length();

  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;

  DBUG_ASSERT(collation.collation != nullptr);
  if (args[1]->const_item()) {
    longlong start = args[1]->val_int();
    if (args[1]->null_value) goto end;
    Integer_value start_val(start, args[1]->unsigned_flag);
    if (Integer_value(INT_MIN32, false) < start_val &&
        start_val <= Integer_value(INT_MAX32, false)) {
      if (start < 0)
        max_char_length = static_cast<uint32>(-start) > max_char_length
                              ? 0
                              : static_cast<uint>(-start);
      else
        max_char_length -= min(static_cast<uint32>(start - 1), max_char_length);
    }
  }
  if (arg_count == 3 && args[2]->const_item()) {
    longlong length = args[2]->val_int();
    if (args[2]->null_value) goto end;
    Integer_value length_val(length, args[2]->unsigned_flag);
    if (length_val.is_negative())
      max_char_length = 0;
    else if (length_val <= Integer_value(INT_MAX, false))
      max_char_length = min(max_char_length, static_cast<uint32>(length));
  }

end:
  set_data_type_string(max_char_length);
  return false;
}

bool Item_func_substr_index::resolve_type(THD *) {
  // We let the first argument (only) determine the character set of the result.
  // SUBSTRING_INDEX(str, delim, count)
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;
  set_data_type_string(args[0]->max_char_length());
  return false;
}

String *Item_func_substr_index::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  char buff[MAX_FIELD_WIDTH];
  String tmp(buff, sizeof(buff), system_charset_info);
  String *res = args[0]->val_str(str);
  String *delimiter = args[1]->val_str(&tmp);
  const longlong count = args[2]->val_int();
  int offset;

  if (args[0]->null_value || args[1]->null_value ||
      args[2]->null_value) {  // string and/or delim are null
    null_value = true;
    return nullptr;
  }
  null_value = false;
  size_t delimiter_length = delimiter->length();
  if (!res->length() || !delimiter_length || !count)
    return make_empty_result();  // Wrong parameters

  res->set_charset(collation.collation);

  StringBuffer<STRING_BUFFER_USUAL_SIZE> delimiter_converted(nullptr);
  delimiter = convert_or_validate_string(delimiter, collation.collation,
                                         &delimiter_converted);
  if (delimiter == nullptr) return error_str();
  delimiter_length = delimiter->length();

  Integer_value count_val(count, args[2]->unsigned_flag);

  // Assumes that the maximum length of a String < INT_MAX32
  if (Integer_value(INT_MAX32, false) < count_val ||
      count_val < Integer_value(INT_MIN32, false))
    return res;

  if (use_mb(res->charset())) {
    const char *ptr = res->ptr();
    const char *strend = ptr + res->length();
    const char *end = strend - delimiter_length + 1;
    const char *search = delimiter->ptr();
    const char *search_end = search + delimiter_length;
    longlong nnn = 0;
    longlong ccc = count;
    // A single pass for positive, two passes for negative count.
    for (int pass = (count_val.is_negative() ? 0 : 1); pass < 2; ++pass) {
      while (ptr < end) {
        if (*ptr == *search) {
          const char *i = ptr + 1;
          const char *j = search + 1;
          while (j != search_end) {
            if (*i++ != *j++) goto skip;
          }
          if (pass == 0)
            ++nnn;
          else if (--ccc == 0)
            break;
          ptr += delimiter_length;
          continue;
        }
      skip:
        ptr += max(1U, my_ismbchar(res->charset(), ptr, strend));
      } /* either not found or got total number when count<0 */

      if (pass == 0) /* count < 0 */
      {
        ccc += nnn + 1;
        if (ccc <= 0) return res; /* not found, return original string */
        ptr = res->ptr();
      } else {
        if (ccc != 0) return res;    /* Not found, return original string */
        if (count_val.is_negative()) /* return right part */
        {
          ptr += delimiter_length;
          tmp_value.set(*res, (ptr - res->ptr()), (strend - ptr));
        } else /* return left part */
        {
          tmp_value.set(*res, 0, (ptr - res->ptr()));
        }
      }
    }
  } else {
    if (count_val.is_negative()) {
      /*
        Negative index, start counting at the end
      */
      longlong count_ll = count;
      for (offset = res->length(); offset;) {
        /*
          this call will result in finding the position pointing to one
          address space less than where the found substring is located
          in res
        */
        if ((offset = res->strrstr(*delimiter, offset)) < 0)
          return res;  // Didn't find, return org string
        /*
          At this point, we've searched for the substring
          the number of times as supplied by the index value
        */
        if (++count_ll == 0) {
          offset += delimiter_length;
          tmp_value.set(*res, offset, res->length() - offset);
          break;
        }
      }
      if (count_ll != 0) return res;  // Didn't find, return org string
    } else {                          // start counting from the beginning
      ulonglong count_ull = count_val.val_unsigned();
      for (offset = 0;; offset += delimiter_length) {
        if ((offset = res->strstr(*delimiter, offset)) < 0)
          return res;  // Didn't find, return org string
        if (--count_ull == 0) {
          tmp_value.set(*res, 0, offset);
          break;
        }
      }
    }
  }
  return (&tmp_value);
}

String *Item_func_trim::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);

  String *res = args[0]->val_str(str);
  if ((null_value = args[0]->null_value)) return nullptr;

  char buff[MAX_FIELD_WIDTH];
  String tmp(buff, sizeof(buff), system_charset_info);
  String *remove_str = &remove;  // Default value.

  StringBuffer<STRING_BUFFER_USUAL_SIZE> remove_converted(nullptr);

  if (arg_count == 2) {
    remove_str = args[1]->val_str(&tmp);
    if ((null_value = args[1]->null_value)) return nullptr;
    remove_str = convert_or_validate_string(remove_str, collation.collation,
                                            &remove_converted);
    if (remove_str == nullptr) return error_str();
  }

  const size_t remove_length = remove_str->length();
  if (remove_length == 0 || remove_length > res->length()) return res;

  const char *ptr = res->ptr();
  const char *end = ptr + res->length();
  const char *const r_ptr = remove_str->ptr();

  if (use_mb(res->charset())) {
    if (m_trim_leading) {
      while (ptr + remove_length <= end) {
        uint num_bytes = 0;
        while (num_bytes < remove_length) {
          uint len;
          if ((len = my_ismbchar(res->charset(), ptr + num_bytes, end)))
            num_bytes += len;
          else
            ++num_bytes;
        }
        if (num_bytes != remove_length) break;
        if (memcmp(ptr, r_ptr, remove_length)) break;
        ptr += remove_length;
      }
    }
    if (m_trim_trailing) {
      // Optimize a common case, removing 0x20
      if (remove_length == 1) {
        const char *save_ptr = ptr;
        const char *new_end = ptr;
        const char chr = (*remove_str)[0];
        while (ptr < end) {
          uint32 l;
          if ((l = my_ismbchar(res->charset(), ptr, end))) {
            ptr += l;
            new_end = ptr;
          } else if (*ptr++ != chr)
            new_end = ptr;
        }
        end = new_end;
        ptr = save_ptr;
      } else {
        bool found;
        const char *save_ptr = ptr;
        do {
          found = false;
          while (ptr + remove_length < end) {
            uint32 l;
            if ((l = my_ismbchar(res->charset(), ptr, end)))
              ptr += l;
            else
              ++ptr;
          }
          if (ptr + remove_length == end &&
              !memcmp(ptr, r_ptr, remove_length)) {
            end -= remove_length;
            found = true;
          }
          ptr = save_ptr;
        } while (found);
      }
    }
  } else {
    if (m_trim_leading) {
      while (ptr + remove_length <= end && !memcmp(ptr, r_ptr, remove_length))
        ptr += remove_length;
    }
    if (m_trim_trailing) {
      while (ptr + remove_length <= end &&
             !memcmp(end - remove_length, r_ptr, remove_length))
        end -= remove_length;
    }
  }
  if (ptr == res->ptr() && end == ptr + res->length()) return res;
  tmp_value.set(*res, static_cast<uint>(ptr - res->ptr()),
                static_cast<uint>(end - ptr));
  return &tmp_value;
}

bool Item_func_trim::resolve_type(THD *) {
  // The parser swaps arguments, so args[0] is FROM str.
  // We let the first argument (only) determine the character set of the
  // result.
  // TRIM([{BOTH | LEADING | TRAILING} [remstr] FROM] str)
  // TRIM([remstr FROM] str)
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;

  if (arg_count == 1) {
    DBUG_ASSERT(collation.collation != nullptr);
    remove.set_charset(collation.collation);
    remove.set_ascii(" ", 1);
  }
  set_data_type_string(args[0]->max_char_length());
  return false;
}

/*
  We need a separate function for print(), in order to do correct printing.
  The function func_name() is also used e.g. by Item_func::eq() to
  distinguish between different functions, and we do not want
  trim(leading) to match trim(trailing) for eq()
 */
static const char *trim_func_name(Item_func_trim::TRIM_MODE mode) {
  switch (mode) {
    case Item_func_trim::TRIM_BOTH_DEFAULT:
    case Item_func_trim::TRIM_BOTH:
    case Item_func_trim::TRIM_LEADING:
    case Item_func_trim::TRIM_TRAILING:
      return "trim";
    case Item_func_trim::TRIM_LTRIM:
      return "ltrim";
    case Item_func_trim::TRIM_RTRIM:
      return "rtrim";
  }
  return nullptr;
}

void Item_func_trim::print(const THD *thd, String *str,
                           enum_query_type query_type) const {
  str->append(trim_func_name(m_trim_mode));
  str->append('(');
  const char *mode_name;
  switch (m_trim_mode) {
    case TRIM_BOTH:
      mode_name = "both ";
      break;
    case TRIM_LEADING:
      mode_name = "leading ";
      break;
    case TRIM_TRAILING:
      mode_name = "trailing ";
      break;
    default:
      mode_name = nullptr;
      break;
  }
  if (mode_name) {
    str->append(mode_name);
  }
  if (arg_count == 2) {
    args[1]->print(thd, str, query_type);
    str->append(STRING_WITH_LEN(" from "));
  }
  args[0]->print(thd, str, query_type);
  str->append(')');
}

Item *Item_func_sysconst::safe_charset_converter(THD *thd,
                                                 const CHARSET_INFO *tocs) {
  uint conv_errors;
  String tmp, cstr, *ostr = val_str(&tmp);
  if (null_value) {
    Item *null_item = new Item_null(fully_qualified_func_name());
    null_item->collation.set(tocs);
    return null_item;
  }
  cstr.copy(ostr->ptr(), ostr->length(), ostr->charset(), tocs, &conv_errors);
  if (conv_errors != 0) return nullptr;

  char *ptr = thd->strmake(cstr.ptr(), cstr.length());
  if (ptr == nullptr) return nullptr;
  auto conv = new Item_static_string_func(fully_qualified_func_name(), ptr,
                                          cstr.length(), cstr.charset(),
                                          collation.derivation);
  if (conv == nullptr) return nullptr;
  conv->mark_result_as_const();
  return conv;
}

bool Item_func_database::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;

  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

String *Item_func_database::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  THD *thd = current_thd;
  if (thd->db().str == nullptr) {
    null_value = true;
    return nullptr;
  } else
    str->copy(thd->db().str, thd->db().length, system_charset_info);
  return str;
}

/**
  @note USER() is replicated correctly if binlog_format=ROW or (as of
  BUG#28086) binlog_format=MIXED, but is incorrectly replicated to ''
  if binlog_format=STATEMENT.
*/
bool Item_func_user::init(const char *user, const char *host) {
  DBUG_ASSERT(fixed == 1);
  DBUG_ASSERT(host != nullptr);
  // For system threads (e.g. replication SQL thread) user may be empty
  if (user) {
    const CHARSET_INFO *cs = str_value.charset();
    size_t res_length = (strlen(user) + strlen(host) + 2) * cs->mbmaxlen;

    if (str_value.alloc((uint)res_length)) {
      null_value = true;
      return true;
    }

    res_length = cs->cset->snprintf(cs, str_value.ptr(), res_length, "%s@%s",
                                    user, host);
    str_value.length((uint)res_length);
    str_value.mark_as_const();
  }
  return false;
}

bool Item_func_user::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;

  LEX *lex = pc->thd->lex;
  lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  lex->safe_to_cache_query = false;
  return false;
}

bool Item_func_user::fix_fields(THD *thd, Item **ref) {
  return (Item_func_sysconst::fix_fields(thd, ref) ||
          init(thd->m_main_security_ctx.user().str,
               thd->m_main_security_ctx.host_or_ip().str));
}

bool Item_func_current_user::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;

  context = pc->thd->lex->current_context();
  return false;
}

bool Item_func_current_user::fix_fields(THD *thd, Item **ref) {
  if (Item_func_sysconst::fix_fields(thd, ref)) return true;

  Security_context *ctx =
      (context->security_ctx ? context->security_ctx : thd->security_context());
  return init(ctx->priv_user().str, ctx->priv_host().str);
}

bool Item_func_soundex::resolve_type(THD *) {
  uint32 char_length = args[0]->max_char_length();
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;
  DBUG_ASSERT(collation.collation != nullptr);
  char_length = max(char_length, 4U);
  set_data_type_string(char_length);
  tmp_value.set_charset(collation.collation);
  return false;
}

/**
  If alpha, map input letter to soundex code.
  If not alpha and remove_garbage is set then skip to next char
  else return 0
*/

static int soundex_toupper(int ch) {
  return (ch >= 'a' && ch <= 'z') ? ch - 'a' + 'A' : ch;
}

/* ABCDEFGHIJKLMNOPQRSTUVWXYZ */
/* :::::::::::::::::::::::::: */
static const char *soundex_map = "01230120022455012623010202";

static char get_scode(int wc) {
  int ch = soundex_toupper(wc);
  if (ch < 'A' || ch > 'Z') {
    // Thread extended alfa (country spec)
    return '0';  // as vokal
  }
  return (soundex_map[ch - 'A']);
}

static bool my_uni_isalpha(int wc) {
  /*
    Return true for all Basic Latin letters: a..z A..Z.
    Return true for all Unicode characters with code higher than U+00C0:
    - characters between 'z' and U+00C0 are controls and punctuations.
    - "U+00C0 LATIN CAPITAL LETTER A WITH GRAVE" is the first letter after 'z'.
  */
  return (wc >= 'a' && wc <= 'z') || (wc >= 'A' && wc <= 'Z') || (wc >= 0xC0);
}

String *Item_func_soundex::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(str);
  char last_ch, ch;
  const CHARSET_INFO *cs = collation.collation;
  my_wc_t wc;
  uint nchars;
  int rc;

  if ((null_value = args[0]->null_value))
    return nullptr; /* purecov: inspected */

  if (tmp_value.alloc(
          max(res->length(), static_cast<size_t>(4 * cs->mbminlen))))
    return str; /* purecov: inspected */
  char *to = tmp_value.ptr();
  char *to_end = to + tmp_value.alloced_length();
  const char *from = res->ptr(), *end = from + res->length();

  for (;;) /* Skip pre-space */
  {
    if ((rc = cs->cset->mb_wc(cs, &wc, pointer_cast<const uchar *>(from),
                              pointer_cast<const uchar *>(end))) <= 0)
      return make_empty_result(); /* EOL or invalid byte sequence */

    if (rc == 1 && cs->ctype) {
      /* Single byte letter found */
      if (my_isalpha(cs, *from)) {
        last_ch = get_scode(*from);        // Code of the first letter
        *to++ = soundex_toupper(*from++);  // Copy first letter
        break;
      }
      from++;
    } else {
      from += rc;
      if (my_uni_isalpha(wc)) {
        /* Multibyte letter found */
        wc = soundex_toupper(wc);
        last_ch = get_scode(wc);  // Code of the first letter
        if ((rc = cs->cset->wc_mb(cs, wc, pointer_cast<uchar *>(to),
                                  pointer_cast<uchar *>(to_end))) <= 0) {
          /* Extra safety - should not really happen */
          DBUG_ASSERT(false);
          return make_empty_result();
        }
        to += rc;
        break;
      }
    }
  }

  /*
     last_ch is now set to the first 'double-letter' check.
     loop on input letters until end of input
  */
  for (nchars = 1;;) {
    if ((rc = cs->cset->mb_wc(cs, &wc, pointer_cast<const uchar *>(from),
                              pointer_cast<const uchar *>(end))) <= 0)
      break; /* EOL or invalid byte sequence */

    if (rc == 1 && cs->ctype) {
      if (!my_isalpha(cs, *from++)) continue;
    } else {
      from += rc;
      if (!my_uni_isalpha(wc)) continue;
    }

    ch = get_scode(wc);
    if ((ch != '0') && (ch != last_ch))  // if not skipped or double
    {
      // letter, copy to output
      if ((rc = cs->cset->wc_mb(cs, (my_wc_t)ch, (uchar *)to,
                                (uchar *)to_end)) <= 0) {
        // Extra safety - should not really happen
        DBUG_ASSERT(false);
        break;
      }
      to += rc;
      nchars++;
      last_ch = ch;  // save code of last input letter
    }                // for next double-letter check
  }

  /* Pad up to 4 characters with DIGIT ZERO, if the string is shorter */
  if (nchars < 4) {
    uint nbytes = (4 - nchars) * cs->mbminlen;
    cs->cset->fill(cs, to, nbytes, '0');
    to += nbytes;
  }

  tmp_value.length((uint)(to - tmp_value.ptr()));
  return &tmp_value;
}

/**
  Change a number to format '3,333,333,333.000'.

  This should be 'internationalized' sometimes.
*/

const int FORMAT_MAX_DECIMALS = 30;

MY_LOCALE *Item_func_format::get_locale(Item *) {
  DBUG_ASSERT(arg_count == 3);
  THD *thd = current_thd;
  String tmp, *locale_name = args[2]->val_str_ascii(&tmp);
  MY_LOCALE *lc;
  if (!locale_name || !(lc = my_locale_by_name(thd, locale_name->ptr(),
                                               locale_name->length()))) {
    push_warning_printf(thd, Sql_condition::SL_WARNING, ER_UNKNOWN_LOCALE,
                        ER_THD(thd, ER_UNKNOWN_LOCALE),
                        locale_name ? locale_name->c_ptr_safe() : "NULL");
    lc = &my_locale_en_US;
  }
  return lc;
}

bool Item_func_format::resolve_type(THD *) {
  uint32 char_length = args[0]->max_char_length();
  uint32 max_sep_count = (char_length / 3) + (decimals ? 1 : 0) + /*sign*/ 1;
  set_data_type_string(char_length + max_sep_count + decimals,
                       default_charset());
  if (arg_count == 3)
    locale = args[2]->basic_const_item() ? get_locale(args[2]) : nullptr;
  else
    locale = &my_locale_en_US; /* Two arguments */
  return reject_geometry_args(arg_count, args, this);
}

/**
  @todo
  This needs to be fixed for multi-byte character set where numbers
  are stored in more than one byte
*/

String *Item_func_format::val_str_ascii(String *str) {
  size_t str_length;
  /* Number of decimal digits */
  int dec;
  /* Number of characters used to represent the decimals, including '.' */
  uint32 dec_length;
  MY_LOCALE *lc;
  DBUG_ASSERT(fixed == 1);

  dec = (int)args[1]->val_int();
  if (args[1]->null_value) {
    null_value = true;
    return nullptr;
  }

  lc = locale ? locale : get_locale(args[2]);

  dec = set_zone(dec, 0, FORMAT_MAX_DECIMALS);
  dec_length = dec ? dec + 1 : 0;
  null_value = false;

  if (args[0]->result_type() == DECIMAL_RESULT ||
      args[0]->result_type() == INT_RESULT) {
    my_decimal dec_val, rnd_dec, *res;
    res = args[0]->val_decimal(&dec_val);
    if ((null_value = args[0]->null_value))
      return nullptr; /* purecov: inspected */
    my_decimal_round(E_DEC_FATAL_ERROR, res, dec, false, &rnd_dec);
    my_decimal2string(E_DEC_FATAL_ERROR, &rnd_dec, str);
    str_length = str->length();
  } else {
    double nr = args[0]->val_real();
    if ((null_value = args[0]->null_value))
      return nullptr; /* purecov: inspected */
    nr = my_double_round(nr, (longlong)dec, false, false);
    str->set_real(nr, dec, &my_charset_numeric);
    if (!std::isfinite(nr)) return str;
    str_length = str->length();
  }
  /* We need this test to handle 'nan' and short values */
  if (lc->grouping[0] > 0 && str_length >= dec_length + 1 + lc->grouping[0]) {
    /* We need space for ',' between each group of digits as well. */
    char buf[2 * FLOATING_POINT_BUFFER + 2] = {0};
    int count;
    const char *grouping = lc->grouping;
    char sign_length = *str->ptr() == '-' ? 1 : 0;
    const char *src = str->ptr() + str_length - dec_length - 1;
    const char *src_begin = str->ptr() + sign_length;
    char *dst = buf + 2 * FLOATING_POINT_BUFFER;
    char *start_dst = dst;

    /* Put the fractional part */
    if (dec) {
      dst -= (dec + 1);
      *dst = lc->decimal_point;
      memcpy(dst + 1, src + 2, dec);
    }

    /* Put the integer part with grouping */
    for (count = *grouping; src >= src_begin; count--) {
      /*
        When *grouping==0x80 (which means "end of grouping")
        count will be initialized to -1 and
        we'll never get into this "if" anymore.
      */
      if (count == 0) {
        *--dst = lc->thousand_sep;
        if (grouping[1]) grouping++;
        count = *grouping;
      }
      DBUG_ASSERT(dst > buf);
      *--dst = *src--;
    }

    if (sign_length) /* Put '-' */
      *--dst = *str->ptr();

    /* Put the rest of the integer part without grouping */
    size_t result_length = start_dst - dst;
    str->copy(dst, result_length, &my_charset_latin1);
  } else if (dec_length && lc->decimal_point != '.') {
    /*
      For short values without thousands (<1000)
      replace decimal point to localized value.
    */
    DBUG_ASSERT(dec_length <= str_length);
    (*str)[str_length - dec_length] = lc->decimal_point;
  }
  return str;
}

void Item_func_format::print(const THD *thd, String *str,
                             enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("format("));
  args[0]->print(thd, str, query_type);
  str->append(',');
  args[1]->print(thd, str, query_type);
  if (arg_count > 2) {
    str->append(',');
    args[2]->print(thd, str, query_type);
  }
  str->append(')');
}

bool Item_func_elt::resolve_type(THD *) {
  uint32 char_length = 0;
  decimals = 0;

  if (agg_arg_charsets_for_string_result(collation, args + 1, arg_count - 1))
    return true;

  for (uint i = 1; i < arg_count; i++) {
    char_length = max(char_length, args[i]->max_char_length());
    decimals = max(decimals, args[i]->decimals);
  }
  set_data_type_string(char_length);
  maybe_null = true;  // NULL if wrong first arg
  return false;
}

double Item_func_elt::val_real() {
  DBUG_ASSERT(fixed == 1);
  uint tmp;
  null_value = true;
  if ((tmp = (uint)args[0]->val_int()) == 0 || args[0]->null_value ||
      tmp >= arg_count)
    return 0.0;
  double result = args[tmp]->val_real();
  null_value = args[tmp]->null_value;
  return result;
}

longlong Item_func_elt::val_int() {
  DBUG_ASSERT(fixed == 1);
  uint tmp;
  null_value = true;
  if ((tmp = (uint)args[0]->val_int()) == 0 || args[0]->null_value ||
      tmp >= arg_count)
    return 0;

  longlong result = args[tmp]->val_int();
  null_value = args[tmp]->null_value;
  return result;
}

String *Item_func_elt::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  uint tmp;
  null_value = true;
  if ((tmp = (uint)args[0]->val_int()) == 0 || args[0]->null_value ||
      tmp >= arg_count)
    return nullptr;

  String *result = args[tmp]->val_str(str);
  if (result) result->set_charset(collation.collation);
  null_value = args[tmp]->null_value;
  return result;
}

bool Item_func_make_set::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  /*
    We have to itemize() the "item" before the super::itemize() call there since
    this reflects the "natural" order of former semantic action code execution
    in the original parser:
  */
  return item->itemize(pc, &item) || super::itemize(pc, res);
}

void Item_func_make_set::split_sum_func(THD *thd, Ref_item_array ref_item_array,
                                        List<Item> &fields) {
  item->split_sum_func2(thd, ref_item_array, fields, &item, true);
  Item_str_func::split_sum_func(thd, ref_item_array, fields);
}

bool Item_func_make_set::resolve_type(THD *) {
  uint32 char_length = arg_count - 1; /* Separators */

  if (agg_arg_charsets_for_string_result(collation, args, arg_count))
    return true;

  for (uint i = 0; i < arg_count; i++)
    char_length += args[i]->max_char_length();
  set_data_type_string(char_length);
  used_tables_cache |= item->used_tables();
  not_null_tables_cache &= item->not_null_tables();
  add_accum_properties(item);

  return false;
}

void Item_func_make_set::update_used_tables() {
  Item_func::update_used_tables();
  item->update_used_tables();
  used_tables_cache |= item->used_tables();
  not_null_tables_cache |= item->not_null_tables();
  add_accum_properties(item);
}

String *Item_func_make_set::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  ulonglong bits;
  bool first_found = false;
  Item **ptr = args;
  String *result = nullptr;

  bits = item->val_int();
  if ((null_value = item->null_value)) return nullptr;

  if (arg_count < 64) bits &= ((ulonglong)1 << arg_count) - 1;

  for (; bits; bits >>= 1, ptr++) {
    if (bits & 1) {
      String *res = (*ptr)->val_str(str);
      if (res)  // Skip nulls
      {
        if (!first_found) {  // First argument
          first_found = true;
          if (res != str)
            result = res;  // Use original string
          else {
            if (tmp_str.copy(*res))  // Don't use 'str'
              return make_empty_result();
            result = &tmp_str;
          }
        } else {
          if (result != &tmp_str) {  // Copy data to tmp_str
            if (tmp_str.alloc((result != nullptr ? result->length() : 0) +
                              res->length() + 1) ||
                tmp_str.copy(*result))
              return make_empty_result();
            result = &tmp_str;
          }
          if (tmp_str.append(STRING_WITH_LEN(","), &my_charset_bin) ||
              tmp_str.append(*res))
            return make_empty_result();
        }
      }
    }
  }
  if (result == nullptr) return make_empty_result();
  return result;
}

Item *Item_func_make_set::transform(Item_transformer transformer, uchar *arg) {
  Item *new_item = item->transform(transformer, arg);
  if (!new_item) return nullptr;

  /*
    THD::change_item_tree() should be called only if the tree was
    really transformed, i.e. when a new item has been created.
    Otherwise we'll be allocating a lot of unnecessary memory for
    change records at each execution.
  */
  if (item != new_item) current_thd->change_item_tree(&item, new_item);
  return Item_str_func::transform(transformer, arg);
}

void Item_func_make_set::print(const THD *thd, String *str,
                               enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("make_set("));
  item->print(thd, str, query_type);
  if (arg_count) {
    str->append(',');
    print_args(thd, str, 0, query_type);
  }
  str->append(')');
}

String *Item_func_char::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  null_value = false;
  str->length(0);
  str->set_charset(collation.collation);
  for (uint i = 0; i < arg_count; i++) {
    int32 num = (int32)args[i]->val_int();
    if (!args[i]->null_value) {
      char tmp[4];
      if (num & 0xFF000000L) {
        mi_int4store(tmp, num);
        str->append(tmp, 4, &my_charset_bin);
      } else if (num & 0xFF0000L) {
        mi_int3store(tmp, num);
        str->append(tmp, 3, &my_charset_bin);
      } else if (num & 0xFF00L) {
        mi_int2store(tmp, num);
        str->append(tmp, 2, &my_charset_bin);
      } else {
        tmp[0] = (char)num;
        str->append(tmp, 1, &my_charset_bin);
      }
    }
  }
  str->mem_realloc(str->length());  // Add end 0 (for Purify)
  String *res = check_well_formed_result(str,
                                         false,  // send warning
                                         true);  // truncate
  if (!res) null_value = true;

  return res;
}

inline String *alloc_buffer(String *res, String *str, String *tmp_value,
                            size_t length) {
  if (res->alloced_length() < length) {
    if (str->alloced_length() >= length) {
      (void)str->copy(*res);
      str->length(length);
      return str;
    }
    if (tmp_value->alloc(length)) return nullptr;
    (void)tmp_value->copy(*res);
    tmp_value->length(length);
    return tmp_value;
  }
  res->length(length);
  return res;
}

bool Item_func_repeat::resolve_type(THD *thd) {
  if (agg_arg_charsets_for_string_result(collation, args, 1)) return true;
  DBUG_ASSERT(collation.collation != nullptr);
  if (args[1]->const_item()) {
    /* must be longlong to avoid truncation */
    longlong count = args[1]->val_int();
    if (args[1]->null_value) goto end;

    // If count is less than 1, returns an empty string.
    Integer_value count_val(count, args[1]->unsigned_flag);
    if (count_val.is_negative()) count = 0;

    unsigned long long count_ull = static_cast<unsigned long long>(count);

    /* Assumes that the maximum length of a String is < INT_MAX32. */
    /* Set here so that rest of code sees out-of-bound value as such. */
    if (count_ull > INT_MAX32) count_ull = INT_MAX32;

    ulonglong char_length =
        static_cast<ulonglong>(args[0]->max_char_length()) * count_ull;
    set_data_type_string(char_length);
    maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
    return false;
  }

end:
  set_data_type_string(uint32(MAX_BLOB_WIDTH));
  maybe_null = true;
  return false;
}

/**
  Item_func_repeat::str is carefully written to avoid reallocs
  as much as possible at the cost of a local buffer
*/

String *Item_func_repeat::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  size_t length, tot_length;
  char *to;
  /* must be longlong to avoid truncation */
  longlong count = args[1]->val_int();
  String *res = args[0]->val_str(str);

  if (args[0]->null_value || args[1]->null_value)
    return error_str();  // string and/or delim are null
  null_value = false;

  if (count <= 0 && (count == 0 || !args[1]->unsigned_flag))
    return make_empty_result();

  // Avoid looping, concatenating the empty string.
  if (res->length() == 0) return res;

  /* Assumes that the maximum length of a String is < INT_MAX32. */
  /* Bounds check on count:  If this is triggered, we will error. */
  if ((ulonglong)count > INT_MAX32) count = INT_MAX32;
  if (count == 1)  // To avoid reallocs
    return res;
  length = res->length();
  // Safe length check
  if (length > current_thd->variables.max_allowed_packet / (uint)count) {
    return push_packet_overflow_warning(current_thd, func_name());
  }
  tot_length = length * (uint)count;
  if (res->uses_buffer_owned_by(str)) {
    if (tmp_value.alloc(tot_length) || tmp_value.copy(*res)) return error_str();
    tmp_value.length(tot_length);
    res = &tmp_value;
  } else if (!(res = alloc_buffer(res, str, &tmp_value, tot_length)))
    return error_str();

  to = res->ptr() + length;
  while (--count) {
    memcpy(to, res->ptr(), length);
    to += length;
  }
  return res;
}

bool Item_func_space::resolve_type(THD *thd) {
  collation.set(default_charset(), DERIVATION_COERCIBLE, MY_REPERTOIRE_ASCII);
  if (args[0]->const_item()) {
    /* must be longlong to avoid truncation */
    longlong count = args[0]->val_int();
    if (args[0]->null_value) goto end;
    /*
     Assumes that the maximum length of a String is < INT_MAX32.
     Set here so that rest of code sees out-of-bound value as such.
    */
    if (count > INT_MAX32) count = INT_MAX32;
    set_data_type_string(ulonglong(count));
    maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
    return false;
  }

end:
  set_data_type_string(uint32(MAX_BLOB_WIDTH));
  maybe_null = true;
  return false;
}

String *Item_func_space::val_str(String *str) {
  uint tot_length;
  longlong count = args[0]->val_int();
  const CHARSET_INFO *cs = collation.collation;

  if (args[0]->null_value) return error_str();  // string and/or delim are null
  null_value = false;

  if (count <= 0 && (count == 0 || !args[0]->unsigned_flag))
    return make_empty_result();
  /*
   Assumes that the maximum length of a String is < INT_MAX32.
   Bounds check on count:  If this is triggered, we will error.
  */
  if ((ulonglong)count > INT_MAX32) count = INT_MAX32;

  // Safe length check
  tot_length = (uint)count * cs->mbminlen;
  if (tot_length > current_thd->variables.max_allowed_packet) {
    return push_packet_overflow_warning(current_thd, func_name());
  }

  if (str->alloc(tot_length)) return error_str();
  str->length(tot_length);
  str->set_charset(cs);
  cs->cset->fill(cs, str->ptr(), tot_length, ' ');
  return str;
}

bool Item_func_rpad::resolve_type(THD *thd) {
  // Handle character set for args[0] and args[2].
  if (agg_arg_charsets_for_string_result(collation, &args[0], 2, 2))
    return true;
  if (args[1]->const_item()) {
    ulonglong char_length = args[1]->val_uint();
    if (args[1]->null_value) goto end;
    DBUG_ASSERT(collation.collation->mbmaxlen > 0);
    /* Assumes that the maximum length of a String is < INT_MAX32. */
    /* Set here so that rest of code sees out-of-bound value as such. */
    if (char_length > INT_MAX32) char_length = INT_MAX32;
    set_data_type_string(char_length);
    maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
    return false;
  }

end:
  set_data_type_string(uint32(MAX_BLOB_WIDTH));
  maybe_null = true;
  return false;
}

String *Item_func_rpad::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  char *to;
  /* must be longlong to avoid truncation */
  longlong count = args[1]->val_int();
  /* Avoid modifying this string as it may affect args[0] */
  String *res = args[0]->val_str(str);
  String *rpad = args[2]->val_str(&rpad_str);

  if ((null_value =
           (args[0]->null_value || args[1]->null_value || args[2]->null_value)))
    return nullptr;

  if ((count < 0) && !args[1]->unsigned_flag) {
    return null_return_str();
  }

  if (!res || !rpad) {
    /* purecov: begin deadcode */
    DBUG_ASSERT(false);
    null_value = true;
    return nullptr;
    /* purecov: end */
  }

  /* Assumes that the maximum length of a String is < INT_MAX32. */
  /* Set here so that rest of code sees out-of-bound value as such. */
  if ((ulonglong)count > INT_MAX32) count = INT_MAX32;
  /*
    There is one exception not handled (intentionaly) by the character set
    aggregation code. If one string is strong side and is binary, and
    another one is weak side and is a multi-byte character string,
    then we need to operate on the second string in terms on bytes when
    calling ::numchars() and ::charpos(), rather than in terms of characters.
    Lets substitute its character set to binary.
  */
  if (collation.collation == &my_charset_bin) {
    res->set_charset(&my_charset_bin);
    rpad->set_charset(&my_charset_bin);
  }

  if (use_mb(rpad->charset())) {
    // This will chop off any trailing illegal characters from rpad.
    String *well_formed_pad =
        args[2]->check_well_formed_result(rpad,
                                          false,  // send warning
                                          true);  // truncate
    if (!well_formed_pad) {
      DBUG_ASSERT(current_thd->is_error());
      return error_str();
    }
  }

  const size_t res_char_length = res->numchars();

  if (count <=
      static_cast<longlong>(res_char_length)) {  // String to pad is big enough
    int res_charpos = res->charpos((int)count);
    if (tmp_value.alloc(res_charpos)) return nullptr;
    (void)tmp_value.copy(*res);
    tmp_value.length(res_charpos);  // Shorten result if longer
    return &tmp_value;
  }
  const size_t pad_char_length = rpad->numchars();

  // Must be ulonglong to avoid overflow
  const ulonglong byte_count = count * collation.collation->mbmaxlen;
  if (byte_count > current_thd->variables.max_allowed_packet) {
    return push_packet_overflow_warning(current_thd, func_name());
  }
  if (!pad_char_length) return make_empty_result();
  /* Must be done before alloc_buffer */
  const size_t res_byte_length = res->length();
  /*
    alloc_buffer() doesn't modify 'res' because 'res' is guaranteed too short
    at this stage.
  */
  if (!(res = alloc_buffer(res, str, &tmp_value,
                           static_cast<size_t>(byte_count)))) {
    return error_str();
  }

  to = res->ptr() + res_byte_length;
  const char *ptr_pad = rpad->ptr();
  const size_t pad_byte_length = rpad->length();
  count -= res_char_length;
  for (; (uint32)count > pad_char_length; count -= pad_char_length) {
    memcpy(to, ptr_pad, pad_byte_length);
    to += pad_byte_length;
  }
  if (count) {
    const size_t pad_charpos = rpad->charpos((int)count);
    memcpy(to, ptr_pad, pad_charpos);
    to += pad_charpos;
  }
  res->length((uint)(to - res->ptr()));
  return (res);
}

bool Item_func_lpad::resolve_type(THD *thd) {
  // Handle character set for args[0] and args[2].
  if (agg_arg_charsets_for_string_result(collation, &args[0], 2, 2))
    return true;

  if (args[1]->const_item()) {
    ulonglong char_length = args[1]->val_uint();
    if (args[1]->null_value) goto end;
    DBUG_ASSERT(collation.collation->mbmaxlen > 0);
    /* Assumes that the maximum length of a String is < INT_MAX32. */
    /* Set here so that rest of code sees out-of-bound value as such. */
    if (char_length > INT_MAX32) char_length = INT_MAX32;
    set_data_type_string(char_length);
    maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
    return false;
  }

end:
  set_data_type_string(uint32(MAX_BLOB_WIDTH));
  maybe_null = true;
  return false;
}

bool Item_func_uuid_to_bin::resolve_type(THD *) {
  set_data_type_string(uint32(binary_log::Uuid::BYTE_LENGTH), &my_charset_bin);
  maybe_null = true;
  return false;
}

String *Item_func_uuid_to_bin::val_str(String *str) {
  DBUG_ASSERT(fixed && (arg_count == 1 || arg_count == 2));
  null_value = true;

  String *res = args[0]->val_str(str);
  if (!res || args[0]->null_value) return nullptr;

  if (binary_log::Uuid::parse(res->ptr(), res->length(), m_bin_buf)) goto err;

  /*
    If there is a second argument which is true, it means
    that the uuid is version 1 which has the time-low part at the beginning
    of the uuid. So in order to make it index-friendly the time-low
    will be swapped with the time-high and the time-mid groups.
    Time-high has length 4, time-mid and time-low have length 2.
    (time-low)-(time-mid)-(time-high) => (time-high)-(time-mid)-(time-low)
  */
  if (arg_count == 2 && args[1]->val_bool()) {
    std::swap_ranges(&m_bin_buf[4], &m_bin_buf[4] + 2, &m_bin_buf[6]);
    std::swap_ranges(&m_bin_buf[0], &m_bin_buf[0] + 4, &m_bin_buf[4]);
  }

  null_value = false;
  str->set(reinterpret_cast<char *>(m_bin_buf), binary_log::Uuid::BYTE_LENGTH,
           &my_charset_bin);
  return str;

err:
  ErrConvString err(res);
  my_error(ER_WRONG_VALUE_FOR_TYPE, MYF(0), "string", err.ptr(), func_name());

  return nullptr;
}

bool Item_func_bin_to_uuid::resolve_type(THD *) {
  set_data_type_string(uint32(binary_log::Uuid::TEXT_LENGTH),
                       default_charset());
  maybe_null = true;
  return false;
}

String *Item_func_bin_to_uuid::val_str_ascii(String *str) {
  DBUG_ASSERT(fixed && (arg_count == 1 || arg_count == 2));
  null_value = true;

  String *res = args[0]->val_str(str);
  if (!res || args[0]->null_value) return nullptr;

  if (res->length() != binary_log::Uuid::BYTE_LENGTH) goto err;

  /*
    If there is a second argument which is true,
    the time-mid and time-high parts of uuid needs to be replaced
    by time-low as they were previously shuffled to become index-friendly.
    Time-high has length 4, time-mid and time-low have length 2.
    (time-high)-(time-mid)-(time-low) => (time-low)-(time-mid)-(time-high)
  */
  if (arg_count == 2 && args[1]->val_bool()) {
    uchar rearranged[binary_log::Uuid::BYTE_LENGTH];
    // The first 4 bytes are restored to "time-low".
    std::copy_n(&res->ptr()[4], 4, &rearranged[0]);
    // Bytes starting with 4th will be restored to "time-mid".
    std::copy_n(&res->ptr()[2], 2, &rearranged[4]);
    // Bytes starting with 6th will be restored to "time-high".
    std::copy_n(&res->ptr()[0], 2, &rearranged[6]);
    // The last 8 bytes were not changed so we just copy them.
    std::copy_n(&res->ptr()[8], 8, &rearranged[8]);
    binary_log::Uuid::to_string(rearranged, m_text_buf);
  } else
    binary_log::Uuid::to_string(reinterpret_cast<const uchar *>(res->ptr()),
                                m_text_buf);

  null_value = false;
  str->set(m_text_buf, binary_log::Uuid::TEXT_LENGTH, default_charset());
  return str;

err:
  ErrConvString err(res);
  my_error(ER_WRONG_VALUE_FOR_TYPE, MYF(0), "string", err.ptr(), func_name());

  return nullptr;
}

longlong Item_func_is_uuid::val_int() {
  DBUG_ASSERT(fixed && arg_count == 1);
  null_value = true;

  String buffer;
  String *arg_str = args[0]->val_str(&buffer);

  if (!arg_str) return 0;

  null_value = false;
  return binary_log::Uuid::is_valid(arg_str->ptr(), arg_str->length());
}

String *Item_func_lpad::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  size_t res_char_length, pad_char_length;
  /* must be longlong to avoid truncation */
  longlong count = args[1]->val_int();
  size_t byte_count;
  /* Avoid modifying this string as it may affect args[0] */
  String *res = args[0]->val_str(&tmp_value);
  String *pad = args[2]->val_str(&lpad_str);

  if ((null_value =
           (args[0]->null_value || args[1]->null_value || args[2]->null_value)))
    return nullptr;

  if (count < 0 && !args[1]->unsigned_flag) {
    return null_return_str();
  }

  if (!res || !pad) {
    /* purecov: begin deadcode */
    DBUG_ASSERT(false);
    null_value = true;
    return nullptr;
    /* purecov: end */
  }

  /* Assumes that the maximum length of a String is < INT_MAX32. */
  /* Set here so that rest of code sees out-of-bound value as such. */
  if ((ulonglong)count > INT_MAX32) count = INT_MAX32;

  /*
    There is one exception not handled (intentionaly) by the character set
    aggregation code. If one string is strong side and is binary, and
    another one is weak side and is a multi-byte character string,
    then we need to operate on the second string in terms on bytes when
    calling ::numchars() and ::charpos(), rather than in terms of characters.
    Lets substitute its character set to binary.
  */
  if (collation.collation == &my_charset_bin) {
    res->set_charset(&my_charset_bin);
    pad->set_charset(&my_charset_bin);
  }

  if (use_mb(pad->charset())) {
    // This will chop off any trailing illegal characters from pad.
    String *well_formed_pad =
        args[2]->check_well_formed_result(pad,
                                          false,  // send warning
                                          true);  // truncate
    if (!well_formed_pad) {
      DBUG_ASSERT(current_thd->is_error());
      return error_str();
    }
  }

  res_char_length = res->numchars();

  if (count <= static_cast<longlong>(res_char_length)) {
    int res_charpos = res->charpos((int)count);
    if (tmp_value.alloc(res_charpos)) return nullptr;
    (void)tmp_value.copy(*res);
    tmp_value.length(res_charpos);  // Shorten result if longer
    return &tmp_value;
  }

  pad_char_length = pad->numchars();
  byte_count = count * collation.collation->mbmaxlen;

  if (byte_count > current_thd->variables.max_allowed_packet) {
    return push_packet_overflow_warning(current_thd, func_name());
  }

  if (!pad_char_length) return make_empty_result();
  if (str->alloc(byte_count)) {
    my_error(ER_DA_OOM, MYF(0));
    return error_str();
  }

  str->length(0);
  str->set_charset(collation.collation);
  count -= res_char_length;
  while (count >= static_cast<longlong>(pad_char_length)) {
    str->append(*pad);
    count -= pad_char_length;
  }
  if (count > 0)
    str->append(pad->ptr(), pad->charpos((int)count), collation.collation);

  str->append(*res);
  null_value = false;
  return str;
}

bool Item_func_conv::resolve_type(THD *) {
  set_data_type_string(CONV_MAX_LENGTH, default_charset());
  maybe_null = true;
  return reject_geometry_args(arg_count, args, this);
}

String *Item_func_conv::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(str);
  const char *endptr;
  longlong dec;
  int from_base = (int)args[1]->val_int();
  int to_base = (int)args[2]->val_int();
  int err;

  // Note that abs(INT_MIN) is undefined.
  if (args[0]->null_value || args[1]->null_value || args[2]->null_value ||
      from_base == INT_MIN || to_base == INT_MIN || abs(to_base) > 36 ||
      abs(to_base) < 2 || abs(from_base) > 36 || abs(from_base) < 2 ||
      !(res->length())) {
    null_value = true;
    return nullptr;
  }
  null_value = false;
  unsigned_flag = !(from_base < 0);

  if (args[0]->data_type() == MYSQL_TYPE_BIT ||
      args[0]->type() == VARBIN_ITEM) {
    /*
     Special case: The string representation of BIT doesn't resemble the
     decimal representation, so we shouldn't change it to string and then to
     decimal.
     The same is true for hexadecimal and bit literals.
    */
    dec = args[0]->val_int();
  } else {
    if (from_base < 0)
      dec = my_strntoll(res->charset(), res->ptr(), res->length(), -from_base,
                        &endptr, &err);
    else
      dec = (longlong)my_strntoull(res->charset(), res->ptr(), res->length(),
                                   from_base, &endptr, &err);
    if (err) {
      /*
        If we got an overflow from my_strntoull, and the input was negative,
        then return 0 rather than ~0
        This is in order to be consistent with
          CAST(<large negative value>, unsigned)
        which returns zero.
       */
      if (from_base > 0) {
        my_decimal res_as_dec;
        res_as_dec.sign(false);
        str2my_decimal(E_DEC_OK, res->ptr(), res->length(), res->charset(),
                       &res_as_dec);
        if (res_as_dec.sign()) dec = 0;
      }
      ErrConvString err_str(res);
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_TRUNCATED_WRONG_VALUE,
                          ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE),
                          "DECIMAL", err_str.ptr());
    }
  }

  char ans[CONV_MAX_LENGTH + 1U];
  char *ptr = longlong2str(dec, ans, to_base);
  if (ptr == nullptr || str->copy(ans, ptr - ans, default_charset())) {
    return error_str();
  }
  return str;
}

String *Item_func_conv_charset::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  if (use_cached_value) return null_value ? nullptr : &str_value;
  String *arg = args[0]->val_str(str);
  uint dummy_errors;
  if (!arg) {
    null_value = true;
    return nullptr;
  }
  null_value = tmp_value.copy(arg->ptr(), arg->length(), arg->charset(),
                              conv_charset, &dummy_errors);
  return null_value ? nullptr
                    : check_well_formed_result(&tmp_value,
                                               false,  // send warning
                                               true);  // truncate
}

bool Item_func_conv_charset::resolve_type(THD *) {
  collation.set(conv_charset, DERIVATION_IMPLICIT);
  set_data_type_string(args[0]->max_char_length());
  return false;
}

void Item_func_conv_charset::print(const THD *thd, String *str,
                                   enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("convert("));
  args[0]->print(thd, str, query_type);
  str->append(STRING_WITH_LEN(" using "));
  str->append(conv_charset->csname);
  str->append(')');
}

bool Item_func_set_collation::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  THD *thd = pc->thd;
  args[1] = new (pc->mem_root) Item_string(
      collation_string.str, collation_string.length, thd->charset());
  if (args[1] == nullptr) return true;

  return super::itemize(pc, res);
}

String *Item_func_set_collation::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  str = args[0]->val_str(str);
  if ((null_value = args[0]->null_value)) return nullptr;
  str->set_charset(collation.collation);
  return str;
}

bool Item_func_set_collation::resolve_type(THD *) {
  CHARSET_INFO *set_collation;
  const char *colname;
  String tmp, *str = args[1]->val_str(&tmp);
  colname = str->c_ptr();
  if (colname == binary_keyword)
    set_collation = get_charset_by_csname(args[0]->collation.collation->csname,
                                          MY_CS_BINSORT, MYF(0));
  else {
    if (!(set_collation = mysqld_collation_get_by_name(colname))) return true;
  }

  if (!set_collation ||
      (!my_charset_same(args[0]->collation.collation, set_collation) &&
       args[0]->collation.derivation != DERIVATION_NUMERIC)) {
    my_error(ER_COLLATION_CHARSET_MISMATCH, MYF(0), colname,
             args[0]->collation.collation->csname);
    return true;
  }
  collation.set(set_collation, DERIVATION_EXPLICIT,
                args[0]->collation.repertoire);
  set_data_type_string(args[0]->max_char_length());
  return false;
}

bool Item_func_set_collation::eq(const Item *item, bool binary_cmp) const {
  /* Assume we don't have rtti */
  if (this == item) return true;
  if (item->type() != FUNC_ITEM) return false;
  const Item_func *item_func = down_cast<const Item_func *>(item);
  if (arg_count != item_func->arg_count || functype() != item_func->functype())
    return false;
  const Item_func_set_collation *item_func_sc =
      down_cast<const Item_func_set_collation *>(item);
  if (collation.collation != item_func_sc->collation.collation) return false;
  for (uint i = 0; i < arg_count; i++)
    if (!args[i]->eq(item_func_sc->args[i], binary_cmp)) return false;
  return true;
}

void Item_func_set_collation::print(const THD *thd, String *str,
                                    enum_query_type query_type) const {
  str->append('(');
  args[0]->print(thd, str, query_type);
  str->append(STRING_WITH_LEN(" collate "));
  DBUG_ASSERT(args[1]->basic_const_item() &&
              args[1]->type() == Item::STRING_ITEM);
  String tmp;
  args[1]->val_str(&tmp)->print(str);
  str->append(')');
}

String *Item_func_charset::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  uint dummy_errors;

  const CHARSET_INFO *cs = args[0]->charset_for_protocol();
  null_value = false;
  str->copy(cs->csname, strlen(cs->csname), &my_charset_latin1,
            collation.collation, &dummy_errors);
  return str;
}

String *Item_func_collation::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  uint dummy_errors;
  const CHARSET_INFO *cs = args[0]->charset_for_protocol();

  null_value = false;
  str->copy(cs->name, strlen(cs->name), &my_charset_latin1, collation.collation,
            &dummy_errors);
  return str;
}

bool Item_func_weight_string::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (as_binary) {
    if (args[0]->itemize(pc, &args[0])) return true;
    args[0] = new (pc->mem_root)
        Item_typecast_char(args[0], num_codepoints, &my_charset_bin);
    if (args[0] == nullptr) return true;
  }
  return super::itemize(pc, res);
}

void Item_func_weight_string::print(const THD *thd, String *str,
                                    enum_query_type query_type) const {
  str->append(func_name());
  str->append('(');
  args[0]->print(thd, str, query_type);
  if (num_codepoints && !as_binary) {
    str->append(" as char");
    str->append_parenthesized(num_codepoints);
  }
  str->append(')');
}

bool Item_func_weight_string::resolve_type(THD *) {
  const CHARSET_INFO *cs = args[0]->collation.collation;
  collation.set(&my_charset_bin, args[0]->collation.derivation);
  flags = my_strxfrm_flag_normalize(flags);
  field = args[0]->type() == FIELD_ITEM && args[0]->is_temporal()
              ? ((Item_field *)(args[0]))->field
              : (Field *)nullptr;
  /*
    Use result_length if it was given explicitly in constructor,
    otherwise calculate max_length using argument's max_length
    and "num_codepoints".

    FIXME: Shouldn't this use strxfrm_multiply instead of, or in
    addition to, mbmaxlen? Do we take into account things like
    UCA level separators at all?
  */
  set_data_type_string(
      field ? field->pack_length()
            : result_length ? result_length
                            : cs->mbmaxlen * max(args[0]->max_char_length(),
                                                 num_codepoints));
  maybe_null = true;
  return false;
}

bool Item_func_weight_string::eq(const Item *item, bool binary_cmp) const {
  if (this == item) return true;
  if (item->type() != FUNC_ITEM) return false;

  const Item_func *func_item = down_cast<const Item_func *>(item);
  if (functype() != func_item->functype() ||
      strcmp(func_name(), func_item->func_name()) != 0)
    return false;

  const Item_func_weight_string *wstr =
      down_cast<const Item_func_weight_string *>(item);
  if (num_codepoints != wstr->num_codepoints || flags != wstr->flags)
    return false;

  if (!args[0]->eq(wstr->args[0], binary_cmp)) return false;
  return true;
}

/* Return a weight_string according to collation */
String *Item_func_weight_string::val_str(String *str) {
  String *input = nullptr;
  const CHARSET_INFO *cs = args[0]->collation.collation;
  size_t output_buf_size, output_length;
  bool rounded_up = false;
  DBUG_ASSERT(fixed == 1);

  // Ask filesort what type it would sort this as. Currently, we support strings
  // and integers (the latter include temporal types).
  st_sort_field sortorder;
  sortorder.item = args[0];
  sortlength(current_thd, &sortorder, /*s_length=*/1);
  if (sortorder.result_type == INT_RESULT) {
    longlong value = get_int_sort_key_for_item(args[0]);
    if (args[0]->maybe_null && args[0]->null_value) return error_str();
    if (tmp_value.alloc(sortorder.length)) return error_str();
    copy_native_longlong(pointer_cast<uchar *>(tmp_value.ptr()),
                         sortorder.length, value, args[0]->unsigned_flag);
    tmp_value.length(sortorder.length);
    null_value = false;
    return &tmp_value;
  }

  if (sortorder.result_type != STRING_RESULT ||
      !(input = args[0]->val_str(str)))
    return error_str();

  /*
    Use result_length if it was given in constructor
    explicitly, otherwise calculate result length
    from argument and "num_codepoints".
  */
  output_buf_size =
      field ? field->pack_length()
            : result_length
                  ? result_length
                  : cs->coll->strnxfrmlen(
                        cs, cs->mbmaxlen *
                                max<size_t>(input->length(), num_codepoints));

  /*
    my_strnxfrm() with an odd number of bytes is illegal for some collations;
    ask for one more and then truncate ourselves instead.
  */
  if ((output_buf_size % 2) == 1) {
    ++output_buf_size;
    rounded_up = true;
  }

  if (output_buf_size > current_thd->variables.max_allowed_packet) {
    return push_packet_overflow_warning(current_thd, func_name());
  }

  if (tmp_value.alloc(output_buf_size)) return error_str();

  if (field) {
    output_length = field->pack_length();
    field->make_sort_key((uchar *)tmp_value.ptr(), output_buf_size);
  } else {
    size_t input_length = input->length();
    size_t used_num_codepoints = num_codepoints;
    if (num_codepoints) {
      // Truncate the string to the requested number of code points.
      input_length =
          min(input_length,
              cs->cset->charpos(cs, input->ptr(), input->ptr() + input_length,
                                num_codepoints));
    } else {
      /*
        Give in exactly the right number of code points, so that we
        do not get any excess trailing space from PAD SPACE collations.
      */
      used_num_codepoints =
          cs->cset->numchars(cs, input->ptr(), input->ptr() + input_length);
    }
    output_length = cs->coll->strnxfrm(
        cs, (uchar *)tmp_value.ptr(), output_buf_size, used_num_codepoints,
        (const uchar *)input->ptr(), input_length, flags);
  }
  DBUG_ASSERT(output_length <= output_buf_size);

  if (rounded_up && output_length == output_buf_size) --output_length;

  tmp_value.length(output_length);
  null_value = false;
  return &tmp_value;
}

String *Item_func_hex::val_str_ascii(String *str) {
  String *res;
  DBUG_ASSERT(fixed == 1);
  if (args[0]->result_type() != STRING_RESULT) {
    /* Return hex of signed longlong value */
    longlong dec = args[0]->val_int();

    if ((null_value = args[0]->null_value)) return nullptr;

    char ans[65], *ptr;
    if (!(ptr = longlong2str(dec, ans, 16)) ||
        str->copy(ans, (uint32)(ptr - ans), &my_charset_numeric))
      return make_empty_result();  // End of memory
    return str;
  }

  /* Convert given string to a hex string, character by character */
  res = args[0]->val_str(str);
  if (!res || tmp_value.alloc(res->length() * 2 + 1)) {
    null_value = true;
    return nullptr;
  }
  null_value = false;
  tmp_value.length(res->length() * 2);
  tmp_value.set_charset(&my_charset_latin1);

  octet2hex(tmp_value.ptr(), res->ptr(), res->length());
  return &tmp_value;
}

/** Convert given hex string to a binary string. */

String *Item_func_unhex::val_str(String *str) {
  const char *from, *end;
  char *to;
  String *res;
  size_t length;
  null_value = true;
  DBUG_ASSERT(fixed == 1);

  res = args[0]->val_str(str);
  // For a NULL input value return NULL without any warning
  if (args[0]->null_value) return nullptr;
  if (!res || tmp_value.alloc(length = (1 + res->length()) / 2)) goto err;

  from = res->ptr();
  tmp_value.length(length);
  to = tmp_value.ptr();
  if (res->length() % 2) {
    int hex_char = hexchar_to_int(*from++);
    if (hex_char == -1) goto err;
    *to++ = static_cast<char>(hex_char);
  }
  for (end = res->ptr() + res->length(); from < end; from += 2, to++) {
    int hex_char = hexchar_to_int(from[0]);
    if (hex_char == -1) goto err;
    *to = static_cast<char>(hex_char << 4);
    hex_char = hexchar_to_int(from[1]);
    if (hex_char == -1) goto err;
    *to |= hex_char;
  }
  null_value = false;
  return &tmp_value;

err:
  char buf[256];
  String err(buf, sizeof(buf), system_charset_info);
  err.length(0);
  args[0]->print(current_thd, &err, QT_NO_DATA_EXPANSION);
  push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                      ER_WRONG_VALUE_FOR_TYPE,
                      ER_THD(current_thd, ER_WRONG_VALUE_FOR_TYPE), "string",
                      err.c_ptr_safe(), func_name());

  return nullptr;
}

#ifndef DBUG_OFF
String *Item_func_like_range::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  longlong nbytes = args[1]->val_int();
  String *res = args[0]->val_str(str);
  size_t min_len, max_len;
  const CHARSET_INFO *cs = collation.collation;

  if (!res || args[0]->null_value || args[1]->null_value || nbytes < 0 ||
      nbytes > MAX_BLOB_WIDTH || min_str.alloc(nbytes) || max_str.alloc(nbytes))
    goto err;
  null_value = false;

  if (cs->coll->like_range(cs, res->ptr(), res->length(), '\\', '_', '%',
                           nbytes, min_str.ptr(), max_str.ptr(), &min_len,
                           &max_len))
    goto err;

  min_str.set_charset(collation.collation);
  max_str.set_charset(collation.collation);
  min_str.length(min_len);
  max_str.length(max_len);

  return is_min ? &min_str : &max_str;

err:
  null_value = true;
  return nullptr;
}
#endif

bool Item_typecast_char::eq(const Item *item, bool binary_cmp) const {
  if (this == item) return true;
  if (item->type() != FUNC_ITEM) return false;

  const Item_func *func_item = down_cast<const Item_func *>(item);
  if (functype() != func_item->functype() ||
      strcmp(func_name(), func_item->func_name()))
    return false;

  const Item_typecast_char *cast = down_cast<const Item_typecast_char *>(item);
  if (cast_length != cast->cast_length || cast_cs != cast->cast_cs)
    return false;

  if (!args[0]->eq(cast->args[0], binary_cmp)) return false;
  return true;
}

void Item_typecast_char::print(const THD *thd, String *str,
                               enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("cast("));
  args[0]->print(thd, str, query_type);
  str->append(STRING_WITH_LEN(" as char"));
  if (cast_length >= 0) str->append_parenthesized(cast_length);
  if (cast_cs) {
    str->append(STRING_WITH_LEN(" charset "));
    str->append(cast_cs->csname);
  }
  str->append(')');
}

String *Item_typecast_char::val_str(String *str) {
  DBUG_ASSERT(fixed);
  THD *thd = current_thd;
  uint32 length;

  if (cast_length >= 0 &&
      static_cast<ulonglong>(cast_length) > thd->variables.max_allowed_packet) {
    return push_packet_overflow_warning(
        thd, cast_cs == &my_charset_bin ? "cast_as_binary" : func_name());
  }

  String *res = args[0]->val_str(str);
  if (res == nullptr) {
    null_value = true;
    return nullptr;
  }
  /*
    Convert character set if differ
    If it is a literal string, we must also take a copy.
  */
  if (charset_conversion || res->alloced_length() == 0) {
    uint dummy_err;
    if (tmp_value.copy(res->ptr(), res->length(), from_cs, cast_cs, &dummy_err))
      return error_str();
    res = &tmp_value;
  }

  res->set_charset(cast_cs);

  /*
    Cut the tail if cast with length
    and the result is longer than cast length, e.g.
    CAST('string' AS CHAR(1))
  */
  if (cast_length >= 0) {
    if (res->length() > (length = (uint32)res->charpos(
                             cast_length))) {  // Safe even if const arg
      char char_type[40];
      snprintf(char_type, sizeof(char_type), "%s(%lu)",
               cast_cs == &my_charset_bin ? "BINARY" : "CHAR", (ulong)length);

      if (!res->alloced_length()) {  // Don't change const str
        DBUG_ASSERT(res != &tmp_value);
        tmp_value = *res;  // Not malloced string
        res = &tmp_value;
      }
      ErrConvString err(res);
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
          ER_THD(thd, ER_TRUNCATED_WRONG_VALUE), char_type, err.ptr());
      res->length(length);
    } else if (cast_cs == &my_charset_bin &&
               res->length() < static_cast<ulonglong>(cast_length)) {
      if (res->alloced_length() < static_cast<ulonglong>(cast_length)) {
        if (res == &tmp_value) {
          if (tmp_value.reserve(cast_length - res->length()))
            return error_str();
        } else {
          if (tmp_value.reserve(cast_length)) return error_str();
          tmp_value.copy(*res);
          res = &tmp_value;
        }
      }
      memset(res->ptr() + res->length(), 0, cast_length - res->length());
      res->length(cast_length);
    }
  }
  null_value = false;
  return res;
}

bool Item_typecast_char::resolve_type(THD *thd) {
  /*
    If we convert between two ASCII compatible character sets and the
    argument repertoire is MY_REPERTOIRE_ASCII then from_cs is set to cast_cs.
    This allows just to take over the args[0]->val_str() result
    and thus avoid unnecessary character set conversion.
  */
  from_cs = args[0]->collation.repertoire == MY_REPERTOIRE_ASCII &&
                    my_charset_is_ascii_based(cast_cs) &&
                    my_charset_is_ascii_based(args[0]->collation.collation)
                ? cast_cs
                : args[0]->collation.collation;

  collation.set(cast_cs, DERIVATION_IMPLICIT);
  set_data_type_string((uint32)(cast_length >= 0
                                    ? cast_length
                                    : cast_cs == &my_charset_bin
                                          ? args[0]->max_length
                                          : args[0]->max_char_length()));
  maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);

  /*
     We always force character set conversion if cast_cs
     is a multi-byte character set. It garantees that the
     result of CAST is a well-formed string.
     For single-byte character sets we allow just to copy from the argument.
     A single-byte character sets string is always well-formed.
  */
  charset_conversion =
      (cast_cs->mbmaxlen > 1) ||
      (!my_charset_same(from_cs, cast_cs) && from_cs != &my_charset_bin &&
       cast_cs != &my_charset_bin);
  return false;
}

bool Item_load_file::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->set_uncacheable(pc->select, UNCACHEABLE_SIDEEFFECT);
  return false;
}

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

String *Item_load_file::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *file_name;
  File file;
  MY_STAT stat_info;
  char path[FN_REFLEN];
  uchar buf[4096];

  if (!(file_name = args[0]->val_str(str)) ||
      !(current_thd->security_context()->check_access(FILE_ACL))) {
    DBUG_ASSERT(maybe_null);
    return error_str();
  }

  (void)fn_format(path, file_name->c_ptr_safe(), mysql_real_data_home, "",
                  MY_RELATIVE_PATH | MY_UNPACK_FILENAME);

  /* Read only allowed from within dir specified by secure_file_priv */
  if (!is_secure_file_path(path)) {
    DBUG_ASSERT(maybe_null);
    return error_str();
  }

  if ((file = mysql_file_open(key_file_loadfile, file_name->ptr(), O_RDONLY,
                              MYF(0))) < 0) {
    DBUG_ASSERT(maybe_null);
    return error_str();
  }

  if (mysql_file_fstat(file, &stat_info) != 0) {
    mysql_file_close(file, MYF(0));
    DBUG_ASSERT(maybe_null);
    return error_str();
  }

  if (!MY_S_ISREG(stat_info.st_mode)) {
    my_error(ER_TEXTFILE_NOT_READABLE, MYF(0), file_name->c_ptr());
    mysql_file_close(file, MYF(0));
    DBUG_ASSERT(maybe_null);
    return error_str();
  }

  tmp_value.length(0);
  for (;;) {
    int ret = mysql_file_read(file, buf, sizeof(buf), MYF(0));
    if (ret == -1) {
      mysql_file_close(file, MYF(0));
      DBUG_ASSERT(maybe_null);
      return error_str();
    }
    if (ret == 0) {
      // EOF.
      break;
    }
    tmp_value.append(pointer_cast<char *>(buf), ret);
    if (tmp_value.length() > current_thd->variables.max_allowed_packet) {
      mysql_file_close(file, MYF(0));
      return push_packet_overflow_warning(current_thd, func_name());
    }
  }
  mysql_file_close(file, MYF(0));
  null_value = false;
  return &tmp_value;
}

String *Item_func_export_set::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String yes_buf, no_buf, sep_buf;
  const ulonglong the_set = (ulonglong)args[0]->val_int();
  const String *yes = args[1]->val_str(&yes_buf);
  const String *no = args[2]->val_str(&no_buf);
  const String *sep = nullptr;

  ulonglong num_set_values = 64;
  str->length(0);
  str->set_charset(collation.collation);

  /* Check if some argument is a NULL value */
  if (args[0]->null_value || args[1]->null_value || args[2]->null_value)
    return error_str();

  /*
    Arg count can only be 3, 4 or 5 here. This is guaranteed from the
    grammar for EXPORT_SET()
  */
  switch (arg_count) {
    case 5:
      num_set_values = static_cast<ulonglong>(args[4]->val_int());
      if (num_set_values > 64) num_set_values = 64;
      if (args[4]->null_value) return error_str();

      /* Fall through */
    case 4:
      if (!(sep = args[3]->val_str(&sep_buf)))  // Only true if NULL
        return error_str();

      break;
    case 3: {
      /* errors is not checked - assume "," can always be converted */
      uint errors;
      sep_buf.copy(STRING_WITH_LEN(","), &my_charset_bin, collation.collation,
                   &errors);
      sep = &sep_buf;
    } break;
    default:
      DBUG_ASSERT(0);  // cannot happen
  }
  null_value = false;

  const ulonglong max_allowed_packet =
      current_thd->variables.max_allowed_packet;
  const ulonglong num_separators = num_set_values > 0 ? num_set_values - 1 : 0;
  const ulonglong max_total_length =
      num_set_values * max(yes->length(), no->length()) +
      num_separators * sep->length();

  if (unlikely(max_total_length > max_allowed_packet)) {
    return push_packet_overflow_warning(current_thd, func_name());
  }

  uint ix;
  ulonglong mask;
  for (ix = 0, mask = 0x1; ix < num_set_values; ++ix, mask = (mask << 1)) {
    if (the_set & mask)
      str->append(*yes);
    else
      str->append(*no);
    if (ix != num_separators) str->append(*sep);
  }

  if (str->ptr() == nullptr) return make_empty_result();

  return str;
}

bool Item_func_export_set::resolve_type(THD *thd) {
  uint32 length = max(args[1]->max_char_length(), args[2]->max_char_length());
  uint32 sep_length = (arg_count > 3 ? args[3]->max_char_length() : 1);

  if (agg_arg_charsets_for_string_result(collation, args + 1,
                                         min(4U, arg_count) - 1))
    return true;
  set_data_type_string(length * 64U + sep_length * 63U);
  maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
  return false;
}

bool Item_func_quote::resolve_type(THD *thd) {
  /*
    Since QUOTE may add escapes to potentially all the characters in its
    argument, we need to compute the maximum by multiplying the argument's
    maximum character length with 2, and then add 2 for the surrounding
    single quotes added by QUOTE. NULLs print as NULL without single quotes
    so their maximum length is 4.
  */
  ulonglong max_result_length = max<ulonglong>(
      4, static_cast<ulonglong>(args[0]->max_char_length()) * 2U + 2U);
  collation.set(args[0]->collation);
  set_data_type_string(max_result_length);
  maybe_null = (maybe_null || max_length > thd->variables.max_allowed_packet);
  return false;
}

#define get_esc_bit(mask, num) (1 & (*((mask) + ((num) >> 3))) >> ((num)&7))

/**
  QUOTE() function returns argument string in single quotes suitable for
  using in a SQL statement.

  Adds a \\ before all characters that needs to be escaped in a SQL string.
  We also escape '^Z' (END-OF-FILE in windows) to avoid probelms when
  running commands from a file in windows.

  This function is very useful when you want to generate SQL statements.

  @note
    QUOTE(NULL) returns the string 'NULL' (4 letters, without quotes).

  @retval
    str	   Quoted string
  @retval
    NULL	   Out of memory.
*/

String *Item_func_quote::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  /*
    Bit mask that has 1 for set for the position of the following characters:
    0, \, ' and ^Z
  */

  static uchar escmask[32] = {0x01, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  char *to;
  const char *from, *end, *start;
  String *arg = args[0]->val_str(str);
  size_t arg_length, new_length;
  if (!arg)  // Null argument
  {
    /* Return the string 'NULL' */
    str->copy(STRING_WITH_LEN("NULL"), collation.collation);
    null_value = false;
    return str;
  }

  arg_length = arg->length();

  if (collation.collation->mbmaxlen == 1) {
    new_length = arg_length + 2; /* for beginning and ending ' signs */
    for (from = arg->ptr(), end = from + arg_length; from < end; from++)
      new_length += get_esc_bit(escmask, (uchar)*from);
  } else {
    new_length = (arg_length * 2) + /* For string characters */
                 (2 * collation.collation->mbmaxlen); /* For quotes */
  }

  if (tmp_value.alloc(new_length)) goto null;

  if (collation.collation->mbmaxlen > 1) {
    const CHARSET_INFO *cs = collation.collation;
    int mblen;
    uchar *to_end;
    to = tmp_value.ptr();
    to_end = (uchar *)to + new_length;

    /* Put leading quote */
    if ((mblen = cs->cset->wc_mb(cs, '\'', (uchar *)to, to_end)) <= 0)
      goto null;
    to += mblen;

    for (start = arg->ptr(), end = start + arg_length; start < end;) {
      my_wc_t wc;
      bool escape;
      if ((mblen = cs->cset->mb_wc(cs, &wc, pointer_cast<const uchar *>(start),
                                   pointer_cast<const uchar *>(end))) <= 0)
        goto null;
      start += mblen;
      switch (wc) {
        case 0:
          escape = true;
          wc = '0';
          break;
        case '\032':
          escape = true;
          wc = 'Z';
          break;
        case '\'':
          escape = true;
          break;
        case '\\':
          escape = true;
          break;
        default:
          escape = false;
          break;
      }
      if (escape) {
        if ((mblen = cs->cset->wc_mb(cs, '\\', (uchar *)to, to_end)) <= 0)
          goto null;
        to += mblen;
      }
      if ((mblen = cs->cset->wc_mb(cs, wc, (uchar *)to, to_end)) <= 0)
        goto null;
      to += mblen;
    }

    /* Put trailing quote */
    if ((mblen = cs->cset->wc_mb(cs, '\'', (uchar *)to, to_end)) <= 0)
      goto null;
    to += mblen;
    new_length = to - tmp_value.ptr();
    goto ret;
  }

  /*
    We replace characters from the end to the beginning
  */
  to = tmp_value.ptr() + new_length - 1;
  *to-- = '\'';
  for (start = arg->ptr(), end = start + arg_length; end-- != start; to--) {
    /*
      We can't use the bitmask here as we want to replace \O and ^Z with 0
      and Z
    */
    switch (*end) {
      case 0:
        *to-- = '0';
        *to = '\\';
        break;
      case '\032':
        *to-- = 'Z';
        *to = '\\';
        break;
      case '\'':
      case '\\':
        *to-- = *end;
        *to = '\\';
        break;
      default:
        *to = *end;
        break;
    }
  }
  *to = '\'';

ret:
  if (new_length > current_thd->variables.max_allowed_packet) {
    return push_packet_overflow_warning(current_thd, func_name());
  }

  tmp_value.length(new_length);
  tmp_value.set_charset(collation.collation);
  null_value = false;
  return &tmp_value;

null:
  null_value = true;
  return nullptr;
}

/**
  @returns The length that the compressed string args[0] had before
  being compressed.

  @note This function is supposed to handle this case:
  SELECT UNCOMPRESSED_LENGTH(COMPRESS(<some string>))
  However, in mysql tradition, the input argument can be *anything*.

  We return NULL if evaluation of the input argument returns NULL.
  If the input string does not look like something produced by
  Item_func_compress::val_str, we issue a warning and return 0.
 */
longlong Item_func_uncompressed_length::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);

  if ((null_value = args[0]->null_value)) return 0;

  if (!res || res->is_empty()) return 0;

  /*
    If length is <= 4 bytes, data is corrupt. This is the best we can do
    to detect garbage input without decompressing it.
  */
  if (res->length() <= 4) {
    push_warning(current_thd, Sql_condition::SL_WARNING, ER_ZLIB_Z_DATA_ERROR,
                 ER_THD(current_thd, ER_ZLIB_Z_DATA_ERROR));
    return 0;
  }

  /*
     res->ptr() using is safe because we have tested that string is at least
     5 bytes long.
     res->c_ptr() is not used because:
       - we do not need \0 terminated string to get first 4 bytes
       - c_ptr() tests simbol after string end (uninitialiozed memory) which
         confuse valgrind
   */
  return uint4korr(res->ptr()) & 0x3FFFFFFF;
}

longlong Item_func_crc32::val_int() {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(&value);
  if (!res) {
    null_value = true;
    return 0; /* purecov: inspected */
  }
  null_value = false;
  return (longlong)crc32(0L, (uchar *)res->ptr(), res->length());
}

String *Item_func_compress::val_str(String *str) {
  int err = Z_OK, code;
  ulong new_size;
  String *res;
  Byte *body;
  char *last_char;
  DBUG_ASSERT(fixed == 1);

  if (!(res = args[0]->val_str(str))) {
    null_value = true;
    return nullptr;
  }
  null_value = false;
  if (res->is_empty()) return res;

  /*
    Citation from zlib.h (comment for compress function):

    Compresses the source buffer into the destination buffer.  sourceLen is
    the byte length of the source buffer. Upon entry, destLen is the total
    size of the destination buffer, which must be at least 0.1% larger than
    sourceLen plus 12 bytes.
    We assume here that the buffer can't grow more than .25 %.
  */
  new_size = res->length() + res->length() / 5 + 12;

  // Check new_size overflow: new_size <= res->length()
  if (((new_size + 5) <= res->length()) ||
      buffer.mem_realloc(new_size + 4 + 1)) {
    null_value = true;
    return nullptr;
  }

  body = ((Byte *)buffer.ptr()) + 4;

  // As far as we have checked res->is_empty() we can use ptr()
  if ((err = compress(body, &new_size, (const Bytef *)res->ptr(),
                      res->length())) != Z_OK) {
    code = err == Z_MEM_ERROR ? ER_ZLIB_Z_MEM_ERROR : ER_ZLIB_Z_BUF_ERROR;
    push_warning(current_thd, Sql_condition::SL_WARNING, code,
                 err == Z_MEM_ERROR ? ER_THD(current_thd, ER_ZLIB_Z_MEM_ERROR)
                                    : ER_THD(current_thd, ER_ZLIB_Z_BUF_ERROR));
    null_value = true;
    return nullptr;
  }

  int4store(buffer.ptr(), res->length() & 0x3FFFFFFF);

  /* This is to ensure that things works for CHAR fields, which trim ' ': */
  last_char = ((char *)body) + new_size - 1;
  if (*last_char == ' ') {
    *++last_char = '.';
    new_size++;
  }

  buffer.length(new_size + 4);
  return &buffer;
}

String *Item_func_uncompress::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  String *res = args[0]->val_str(str);
  ulong new_size;
  int err;
  uint code;

  if (!res) goto err;
  null_value = false;
  if (res->is_empty()) return res;

  /* If length is less than 4 bytes, data is corrupt */
  if (res->length() <= 4) {
    push_warning(current_thd, Sql_condition::SL_WARNING, ER_ZLIB_Z_DATA_ERROR,
                 ER_THD(current_thd, ER_ZLIB_Z_DATA_ERROR));
    goto err;
  }

  /* Size of uncompressed data is stored as first 4 bytes of field */
  new_size = uint4korr(res->ptr()) & 0x3FFFFFFF;
  if (new_size > current_thd->variables.max_allowed_packet) {
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_TOO_BIG_FOR_UNCOMPRESS,
        ER_THD(current_thd, ER_TOO_BIG_FOR_UNCOMPRESS),
        static_cast<int>(current_thd->variables.max_allowed_packet));
    goto err;
  }
  if (buffer.mem_realloc((uint32)new_size)) goto err;

  if ((err = uncompress(pointer_cast<Byte *>(buffer.ptr()), &new_size,
                        pointer_cast<const Bytef *>(res->ptr()) + 4,
                        res->length() - 4)) == Z_OK) {
    buffer.length((uint32)new_size);
    return &buffer;
  }

  code = ((err == Z_BUF_ERROR) ? ER_ZLIB_Z_BUF_ERROR
                               : ((err == Z_MEM_ERROR) ? ER_ZLIB_Z_MEM_ERROR
                                                       : ER_ZLIB_Z_DATA_ERROR));
  push_warning(current_thd, Sql_condition::SL_WARNING, code,
               err == Z_BUF_ERROR
                   ? ER_THD(current_thd, ER_ZLIB_Z_BUF_ERROR)
                   : (err == Z_MEM_ERROR)
                         ? ER_THD(current_thd, ER_ZLIB_Z_MEM_ERROR)
                         : ER_THD(current_thd, ER_ZLIB_Z_DATA_ERROR));

err:
  null_value = true;
  return nullptr;
}

/*
  UUID, as in
    DCE 1.1: Remote Procedure Call,
    Open Group Technical Standard Document Number C706, October 1997,
    (supersedes C309 DCE: Remote Procedure Call 8/1994,
    which was basis for ISO/IEC 11578:1996 specification)
*/

static struct rand_struct uuid_rand;
static uint nanoseq;
static ulonglong uuid_time = 0;
static char clock_seq_and_node_str[] = "-0000-000000000000";

/**
  number of 100-nanosecond intervals between
  1582-10-15 00:00:00.00 and 1970-01-01 00:00:00.00.
*/
#define UUID_TIME_OFFSET ((ulonglong)141427 * 24 * 60 * 60 * 1000 * 1000 * 10)

#define UUID_VERSION 0x1000
#define UUID_VARIANT 0x8000

static void tohex(char *to, uint from, uint len) {
  to += len;
  while (len--) {
    *--to = _dig_vec_lower[from & 15];
    from >>= 4;
  }
}

static void set_clock_seq_str() {
  uint16 clock_seq = ((uint)(my_rnd(&uuid_rand) * 16383)) | UUID_VARIANT;
  tohex(clock_seq_and_node_str + 1, clock_seq, 4);
  nanoseq = 0;
}

bool Item_func_uuid::resolve_type(THD *) {
  collation.set(system_charset_info, DERIVATION_COERCIBLE, MY_REPERTOIRE_ASCII);
  set_data_type_string(uint32(UUID_LENGTH));
  return false;
}

bool Item_func_uuid::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_FUNCTION);
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

String *mysql_generate_uuid(String *str) {
  char *s;
  THD *thd = current_thd;

  mysql_mutex_lock(&LOCK_uuid_generator);
  if (!uuid_time) /* first UUID() call. initializing data */
  {
    ulong tmp = sql_rnd_with_mutex();
    uchar mac[6];
    int i;
    if (my_gethwaddr(mac)) {
      /* purecov: begin inspected */
      /*
        generating random "hardware addr"
        and because specs explicitly specify that it should NOT correlate
        with a clock_seq value (initialized random below), we use a separate
        randominit() here
      */
      randominit(&uuid_rand, tmp + (ulong)thd,
                 tmp + (ulong)atomic_global_query_id);
      for (i = 0; i < (int)sizeof(mac); i++)
        mac[i] = (uchar)(my_rnd(&uuid_rand) * 255);
      /* purecov: end */
    }
    s = clock_seq_and_node_str + sizeof(clock_seq_and_node_str) - 1;
    for (i = sizeof(mac) - 1; i >= 0; i--) {
      *--s = _dig_vec_lower[mac[i] & 15];
      *--s = _dig_vec_lower[mac[i] >> 4];
    }
    randominit(&uuid_rand, tmp + (ulong)server_start_time,
               tmp + (ulong)thd->status_var.bytes_sent);
    set_clock_seq_str();
  }

  ulonglong tv = my_getsystime() + UUID_TIME_OFFSET + nanoseq;

  if (likely(tv > uuid_time)) {
    /*
      Current time is ahead of last timestamp, as it should be.
      If we "borrowed time", give it back, just as long as we
      stay ahead of the previous timestamp.
    */
    if (nanoseq) {
      DBUG_ASSERT((tv > uuid_time) && (nanoseq > 0));
      /*
        -1 so we won't make tv= uuid_time for nanoseq >= (tv - uuid_time)
      */
      ulong delta = min<ulong>(nanoseq, (ulong)(tv - uuid_time - 1));
      tv -= delta;
      nanoseq -= delta;
    }
  } else {
    if (unlikely(tv == uuid_time)) {
      /*
        For low-res system clocks. If several requests for UUIDs
        end up on the same tick, we add a nano-second to make them
        different.
        ( current_timestamp + nanoseq * calls_in_this_period )
        may end up > next_timestamp; this is OK. Nonetheless, we'll
        try to unwind nanoseq when we get a chance to.
        If nanoseq overflows, we'll start over with a new numberspace
        (so the if() below is needed so we can avoid the ++tv and thus
        match the follow-up if() if nanoseq overflows!).
      */
      if (likely(++nanoseq)) ++tv;
    }

    if (unlikely(tv <= uuid_time)) {
      /*
        If the admin changes the system clock (or due to Daylight
        Saving Time), the system clock may be turned *back* so we
        go through a period once more for which we already gave out
        UUIDs.  To avoid duplicate UUIDs despite potentially identical
        times, we make a new random component.
        We also come here if the nanoseq "borrowing" overflows.
        In either case, we throw away any nanoseq borrowing since it's
        irrelevant in the new numberspace.
      */
      set_clock_seq_str();
      tv = my_getsystime() + UUID_TIME_OFFSET;
      nanoseq = 0;
      DBUG_PRINT("uuid", ("making new numberspace"));
    }
  }

  uuid_time = tv;
  mysql_mutex_unlock(&LOCK_uuid_generator);

  uint32 time_low = (uint32)(tv & 0xFFFFFFFF);
  uint16 time_mid = (uint16)((tv >> 32) & 0xFFFF);
  uint16 time_hi_and_version = (uint16)((tv >> 48) | UUID_VERSION);

  str->mem_realloc(UUID_LENGTH + 1);
  str->length(UUID_LENGTH);
  str->set_charset(system_charset_info);
  s = str->ptr();
  s[8] = s[13] = '-';
  tohex(s, time_low, 8);
  tohex(s + 9, time_mid, 4);
  tohex(s + 14, time_hi_and_version, 4);
  my_stpcpy(s + 18, clock_seq_and_node_str);
  DBUG_EXECUTE_IF("force_fake_uuid",
                  my_stpcpy(s, "a2d00942-b69c-11e4-a696-0020ff6fcbe6"););
  return str;
}

String *Item_func_uuid::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  return mysql_generate_uuid(str);
}

bool Item_func_gtid_subtract::resolve_type(THD *) {
  maybe_null = args[0]->maybe_null || args[1]->maybe_null;
  collation.set(default_charset(), DERIVATION_COERCIBLE, MY_REPERTOIRE_ASCII);
  /*
    In the worst case, the string grows after subtraction. This
    happens when a GTID in args[0] is split by a GTID in args[1],
    e.g., UUID:1-6 minus UUID:3-4 becomes UUID:1-2,5-6.  The worst
    case is UUID:1-100 minus UUID:9, where the two characters ":9" in
    args[1] yield the five characters "-8,10" in the result.
  */
  set_data_type_string(
      args[0]->max_length +
      max<ulonglong>(args[1]->max_length - binary_log::Uuid::TEXT_LENGTH, 0) *
          5 / 2);
  return false;
}

String *Item_func_gtid_subtract::val_str_ascii(String *str) {
  DBUG_TRACE;
  String *str1, *str2;
  const char *charp1, *charp2;
  enum_return_status status;
  /*
    We must execute args[*]->val_str_ascii() before checking
    args[*]->null_value to ensure that them are updated when
    this function is executed inside a stored procedure.
  */
  if ((str1 = args[0]->val_str_ascii(&buf1)) != nullptr &&
      (charp1 = str1->c_ptr_safe()) != nullptr &&
      (str2 = args[1]->val_str_ascii(&buf2)) != nullptr &&
      (charp2 = str2->c_ptr_safe()) != nullptr && !args[0]->null_value &&
      !args[1]->null_value) {
    Sid_map sid_map(nullptr /*no rwlock*/);
    // compute sets while holding locks
    Gtid_set set1(&sid_map, charp1, &status);
    if (status == RETURN_STATUS_OK) {
      Gtid_set set2(&sid_map, charp2, &status);
      size_t length;
      // subtract, save result, return result
      if (status == RETURN_STATUS_OK) {
        set1.remove_gtid_set(&set2);
        if (!str->mem_realloc((length = set1.get_string_length()) + 1)) {
          null_value = false;
          set1.to_string(str->ptr());
          str->length(length);
          return str;
        }
      }
    }
  }
  null_value = true;
  return nullptr;
}

/**
  @brief
    This function prepares string with list of column privileges.
    This is required for IS implementation which uses views on DD tables.
    In older non-DD model, we use to get this string using get_column_grant().
    With new IS implementation using DD, we can't call get_column_grant().
    The following UDF implementation solves the problem,

    Syntax:
    string get_dd_column_privileges(schema_name,
                                    table_name,
                                    field_name);

 */
String *Item_func_get_dd_column_privileges::val_str(String *str) {
  DBUG_TRACE;

  std::ostringstream oss("");

  //
  // Retrieve required values to form column type string.
  //

  // Read schema_name, table_name, field_name
  String schema_name;
  String *schema_name_ptr;
  String table_name;
  String *table_name_ptr = nullptr;
  String field_name;
  String *field_name_ptr = nullptr;
  if ((schema_name_ptr = args[0]->val_str(&schema_name)) != nullptr &&
      (table_name_ptr = args[1]->val_str(&table_name)) != nullptr &&
      (field_name_ptr = args[2]->val_str(&field_name)) != nullptr) {
    if (!is_infoschema_db(schema_name_ptr->c_ptr_safe())) {
      //
      // Get privileges
      //

      THD *thd = current_thd;
      GRANT_INFO grant_info;
      fill_effective_table_privileges(thd, &grant_info,
                                      schema_name_ptr->c_ptr_safe(),
                                      table_name_ptr->c_ptr_safe());

      // Get column grants
      uint col_access;
      col_access =
          get_column_grant(thd, &grant_info, schema_name_ptr->c_ptr_safe(),
                           table_name_ptr->c_ptr_safe(),
                           field_name_ptr->c_ptr_safe()) &
          COL_ACLS;

      // Prepare user readable output string with column access grants
      for (uint bitnr = 0; col_access; col_access >>= 1, bitnr++) {
        if (col_access & 1) {
          if (oss.str().length()) oss << ',';
          oss << grant_types.type_names[bitnr];
        }
      }
    }  // INFORMATION SCHEMA Tables have SELECT privileges.
    else
      oss << "select";
  }

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

/**
  @brief
    This function prepares string representing create_options for table.
    This is required for IS implementation which uses views on DD tables.
    In older non-DD model, FRM file had only user options specified in
    CREATE TABLE statement.
    With new IS implementation using DD, all internal option values are
    also stored in options field.
    So, this UDF filters internal options from user defined options

    Syntax:
      string get_dd_create_options(dd.table.options)

    The arguments accept values from options from 'tables' DD table,
    as shown above.

 */
String *Item_func_get_dd_create_options::val_str(String *str) {
  DBUG_TRACE;

  // Read tables.options
  String option;
  String *option_ptr;
  std::ostringstream oss("");

  if ((option_ptr = args[0]->val_str(&option)) == nullptr) {
    str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
    return str;
  }

  // Read required values from properties
  std::unique_ptr<dd::Properties> p(
      dd::Properties::parse_properties(option_ptr->c_ptr_safe()));

  // Warn if the property string is corrupt.
  if (!p.get()) {
    LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
           option_ptr->c_ptr_safe());
    if (DBUG_EVALUATE_IF("continue_on_property_string_parse_failure", 0, 1))
      DBUG_ASSERT(false);
    str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
    return str;
  }

  // Read used_flags
  char option_buff[350], *ptr;
  ptr = option_buff;

  if (p->exists("max_rows")) {
    uint opt_value = 0;
    p->get("max_rows", &opt_value);
    if (opt_value != 0) {
      ptr = my_stpcpy(ptr, " max_rows=");
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (p->exists("min_rows")) {
    uint opt_value = 0;
    p->get("min_rows", &opt_value);
    if (opt_value != 0) {
      ptr = my_stpcpy(ptr, " min_rows=");
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (p->exists("avg_row_length")) {
    uint opt_value = 0;
    p->get("avg_row_length", &opt_value);
    if (opt_value != 0) {
      ptr = my_stpcpy(ptr, " avg_row_length=");
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (p->exists("row_type")) {
    uint opt_value = 0;
    p->get("row_type", &opt_value);
    ptr = strxmov(ptr, " row_format=", ha_row_type[(uint)opt_value], NullS);
  }

  if (p->exists("stats_sample_pages")) {
    uint opt_value = 0;
    p->get("stats_sample_pages", &opt_value);
    if (opt_value != 0) {
      ptr = my_stpcpy(ptr, " stats_sample_pages=");
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (p->exists("stats_auto_recalc")) {
    uint opt_value = 0;
    p->get("stats_auto_recalc", &opt_value);
    enum_stats_auto_recalc sar = (enum_stats_auto_recalc)opt_value;

    if (sar == HA_STATS_AUTO_RECALC_ON)
      ptr = my_stpcpy(ptr, " stats_auto_recalc=1");
    else if (sar == HA_STATS_AUTO_RECALC_OFF)
      ptr = my_stpcpy(ptr, " stats_auto_recalc=0");
  }

  if (p->exists("key_block_size")) {
    uint opt_value = 0;
    p->get("key_block_size", &opt_value);
    if (opt_value != 0) {
      ptr = my_stpcpy(ptr, " KEY_BLOCK_SIZE=");
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (p->exists("compress")) {
    dd::String_type opt_value;
    p->get("compress", &opt_value);
    if (!opt_value.empty()) {
      if (opt_value.size() > 7) opt_value.erase(7, dd::String_type::npos);
      ptr = my_stpcpy(ptr, " COMPRESSION=\"");
      ptr = my_stpcpy(ptr, opt_value.c_str());
      ptr = my_stpcpy(ptr, "\"");
    }
  }

  // Print ENCRYPTION clause.
  dd::String_type encrypt_type;
  if (p->exists("encrypt_type")) {
    p->get("encrypt_type", &encrypt_type);
  } else {
    encrypt_type = "N";
  }

  // Show ENCRYPTION clause only if we have a encrypted table
  // OR if schema encryption default is different from table encryption.
  bool is_schema_encrypted = args[2]->val_int();
  bool encryption_request_type = is_encrypted(encrypt_type);
  if (encryption_request_type ||
      (is_schema_encrypted != encryption_request_type)) {
    ptr = my_stpcpy(ptr, " ENCRYPTION=\'");
    ptr = my_stpcpy(ptr, encrypt_type.c_str());
    ptr = my_stpcpy(ptr, "\'");
  }

  if (p->exists("stats_persistent")) {
    uint opt_value = 0;
    p->get("stats_persistent", &opt_value);
    if (opt_value)
      ptr = my_stpcpy(ptr, " stats_persistent=1");
    else
      ptr = my_stpcpy(ptr, " stats_persistent=0");
  }

  if (p->exists("pack_keys")) {
    uint opt_value = 0;
    p->get("pack_keys", &opt_value);
    if (opt_value)
      ptr = my_stpcpy(ptr, " pack_keys=1");
    else
      ptr = my_stpcpy(ptr, " pack_keys=0");
  }

  if (p->exists("checksum")) {
    uint opt_value = 0;
    p->get("checksum", &opt_value);
    if (opt_value) ptr = my_stpcpy(ptr, " checksum=1");
  }

  if (p->exists("delay_key_write")) {
    uint opt_value = 0;
    p->get("delay_key_write", &opt_value);
    if (opt_value) ptr = my_stpcpy(ptr, " delay_key_write=1");
  }

  bool is_partitioned = args[1]->val_int();
  if (is_partitioned) ptr = my_stpcpy(ptr, " partitioned");

  if (p->exists("secondary_engine")) {
    dd::String_type opt_value;
    p->get("secondary_engine", &opt_value);
    if (!opt_value.empty()) {
      ptr = my_stpcpy(ptr, " SECONDARY_ENGINE=\"");
      ptr = my_stpcpy(ptr, opt_value.c_str());
      ptr = my_stpcpy(ptr, "\"");
    }
  }

  if (ptr == option_buff)
    oss << "";
  else
    oss << option_buff + 1;

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

String *Item_func_internal_get_comment_or_error::val_str(String *str) {
  DBUG_TRACE;
  null_value = false;

  // Read arguements
  String schema;
  String view;
  String table_type;
  String options;
  String *schema_ptr = args[0]->val_str(&schema);
  String *view_ptr = args[1]->val_str(&view);
  String *table_type_ptr = args[2]->val_str(&table_type);
  String *options_ptr = args[3]->val_str(&options);
  String comment;
  String *comment_ptr = args[4]->val_str(&comment);

  if (table_type_ptr == nullptr || schema_ptr == nullptr ||
      view_ptr == nullptr || comment_ptr == nullptr) {
    null_value = true;
    return nullptr;
  }

  THD *thd = current_thd;
  std::ostringstream oss("");

  DBUG_EXECUTE_IF("fetch_system_view_definition", {
    dd::String_type definition;
    if (dd::info_schema::get_I_S_view_definition(
            dd::String_type(schema_ptr->c_ptr_safe()),
            dd::String_type(view_ptr->c_ptr_safe()), &definition) == false) {
      str->copy(definition.c_str(), definition.length(), system_charset_info);
      return str;
    }
  });

  if (options_ptr != nullptr &&
      strcmp(table_type_ptr->c_ptr_safe(), "VIEW") == 0) {
    bool is_view_valid = true;

    std::unique_ptr<dd::Properties> view_options(
        dd::Properties::parse_properties(options_ptr->c_ptr_safe()));

    // Warn if the property string is corrupt.
    if (!view_options.get()) {
      LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
             options_ptr->c_ptr_safe());
      DBUG_ASSERT(false);
      return nullptr;
    }

    if (view_options->get("view_valid", &is_view_valid)) return nullptr;

    if (is_view_valid == false && thd->lex->sql_command != SQLCOM_SHOW_TABLES) {
      oss << push_view_warning_or_error(current_thd, schema_ptr->c_ptr_safe(),
                                        view_ptr->c_ptr_safe());
    } else
      oss << "VIEW";
  } else if (!thd->lex->m_IS_table_stats.error().empty()) {
    /*
      There could be error generated due to INTERNAL_*() UDF calls
      in I_S query. If there was a error found, we show that as
      part of COMMENT field.
    */
    oss << thd->lex->m_IS_table_stats.error();
  } else {
    oss << comment_ptr->c_ptr_safe();
  }
  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

/*
  The function return 'default' in case the dd::Properties string passed as
  the argument 'str' does not contain 'nodegroup_id' key stored OR even
  when the option string is empty.
*/
String *Item_func_get_partition_nodegroup::val_str(String *str) {
  DBUG_TRACE;
  null_value = false;

  String options;
  String *options_ptr = args[0]->val_str(&options);
  std::ostringstream oss("");

  // If we have a option string.
  if (options_ptr != nullptr) {
    // Prepare dd::Properties
    std::unique_ptr<dd::Properties> view_options(
        dd::Properties::parse_properties(options_ptr->c_ptr_safe()));

    // Warn if the property string is corrupt.
    if (!view_options.get()) {
      LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
             options_ptr->c_ptr_safe());
      DBUG_ASSERT(false);
      str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
      return str;
    }

    if (view_options->exists("nodegroup_id")) {
      uint32 value;

      // Fetch nodegroup id.
      view_options->get("nodegroup_id", &value);
      oss << value;
    } else
      oss << "default";
  } else
    oss << "default";

  // Copy the value to output string.
  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

String *Item_func_internal_tablespace_type::val_str(String *str) {
  DBUG_TRACE;
  dd::String_type result;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_TYPE, &result);
    str->copy(result.c_str(), result.length(), system_charset_info);

    return str;
  }

  return nullptr;
}

String *Item_func_internal_tablespace_logfile_group_name::val_str(String *str) {
  DBUG_TRACE;
  dd::String_type result;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_LOGFILE_GROUP_NAME,
        &result);
    if (result.length())
      str->copy(result.c_str(), result.length(), system_charset_info);
    else {
      null_value = true;
      return nullptr;
    }

    return str;
  }

  return nullptr;
}

String *Item_func_internal_tablespace_status::val_str(String *str) {
  DBUG_TRACE;
  dd::String_type result;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_STATUS, &result);
    str->copy(result.c_str(), result.length(), system_charset_info);

    return str;
  }

  return nullptr;
}

String *Item_func_internal_tablespace_row_format::val_str(String *str) {
  DBUG_TRACE;
  dd::String_type result;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_ROW_FORMAT, &result);
    if (result.length())
      str->copy(result.c_str(), result.length(), system_charset_info);
    else {
      null_value = true;
      return nullptr;
    }

    return str;
  }

  return nullptr;
}

String *Item_func_internal_tablespace_extra::val_str(String *str) {
  DBUG_TRACE;
  dd::String_type result;

  THD *thd = current_thd;
  retrieve_tablespace_statistics(thd, args, &null_value);
  if (null_value == false) {
    thd->lex->m_IS_tablespace_stats.get_stat(
        dd::info_schema::enum_tablespace_stats_type::TS_EXTRA, &result);
    if (result.length())
      str->copy(result.c_str(), result.length(), system_charset_info);
    else {
      null_value = true;
      return nullptr;
    }

    return str;
  }

  return nullptr;
}

/**
  @brief
    This function prepares string representing se_private_data for tablespace.
    This is required for IS implementation which uses views on DD tablespace.

    Syntax:
      string get_dd_tablespace_private_data(dd.tablespace.se_private_data)

    The arguments accept values from se_private_data from 'tablespace'
    DD table.

 */
String *Item_func_get_dd_tablespace_private_data::val_str(String *str) {
  DBUG_TRACE;

  // Read tablespaces.se_private_data
  String option;
  String *option_ptr;
  std::ostringstream oss("");
  if ((option_ptr = args[0]->val_str(&option)) == nullptr) {
    str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
    return str;
  }

  // Read required values from properties
  std::unique_ptr<dd::Properties> p(
      dd::Properties::parse_properties(option_ptr->c_ptr_safe()));

  // Warn if the property string is corrupt.
  if (!p.get()) {
    LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
           option_ptr->c_ptr_safe());
    DBUG_ASSERT(false);
    str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
    return str;
  }

  // Read used_flags
  uint opt_value = 0;
  char option_buff[350], *ptr;
  ptr = option_buff;

  if (strcmp(args[1]->val_str(&option)->ptr(), "id") == 0) {
    if (p->exists("id")) {
      p->get("id", &opt_value);
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (strcmp(args[1]->val_str(&option)->ptr(), "flags") == 0) {
    if (p->exists("flags")) {
      p->get("flags", &opt_value);
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (ptr == option_buff)
    oss << "";
  else
    oss << option_buff;

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

/**
  @brief
    This function prepares string representing se_private_data for index.
    This is required for IS implementation which uses views on DD indexes.

    Syntax:
      string get_dd_index_private_data(dd.indexes.se_private_data)

    The arguments accept values from se_private_data from 'indexes'
    DD table.

 */
String *Item_func_get_dd_index_private_data::val_str(String *str) {
  DBUG_TRACE;

  // Read indexes.se_private_data
  String option;
  String *option_ptr;
  std::ostringstream oss("");
  if ((option_ptr = args[0]->val_str(&option)) == nullptr) {
    str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
    return str;
  }

  // Read required values from properties
  std::unique_ptr<dd::Properties> p(
      dd::Properties::parse_properties(option_ptr->c_ptr_safe()));

  // Warn if the property string is corrupt.
  if (!p.get()) {
    LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
           option_ptr->c_ptr_safe());
    DBUG_ASSERT(false);
    str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
    return str;
  }

  // Read used_flags
  uint opt_value = 0;
  char option_buff[350], *ptr;
  ptr = option_buff;

  if (strcmp(args[1]->val_str(&option)->ptr(), "id") == 0) {
    if (p->exists("id")) {
      p->get("id", &opt_value);
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (strcmp(args[1]->val_str(&option)->ptr(), "root") == 0) {
    if (p->exists("root")) {
      p->get("root", &opt_value);
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (strcmp(args[1]->val_str(&option)->ptr(), "trx_id") == 0) {
    if (p->exists("trx_id")) {
      p->get("trx_id", &opt_value);
      ptr = longlong10_to_str(opt_value, ptr, 10);
    }
  }

  if (ptr == option_buff)
    oss << "";
  else
    oss << option_buff;

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

/**
  Get collation by name, send error to client on failure.
  @param name     Collation name
  @param name_cs  Character set of the name string

  @retval         NULL on error
  @retval         Pointter to CHARSET_INFO with the given name on success
*/
CHARSET_INFO *mysqld_collation_get_by_name(const char *name,
                                           CHARSET_INFO *name_cs) {
  CHARSET_INFO *cs;
  MY_CHARSET_LOADER loader;
  char error[1024];
  my_charset_loader_init_mysys(&loader);
  if (!(cs = my_collation_get_by_name(&loader, name, MYF(0)))) {
    ErrConvString err(name, name_cs);
    my_error(ER_UNKNOWN_COLLATION, MYF(0), err.ptr());
    if (loader.errcode) {
      snprintf(error, sizeof(error) - 1, EE(loader.errcode), loader.errarg);
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_UNKNOWN_COLLATION, "%s", error);
    }
  }
  return cs;
}

String *Item_func_convert_cpu_id_mask::val_str(String *str) {
  DBUG_TRACE;
  null_value = false;

  String cpu_mask;
  String *cpu_mask_str = args[0]->val_str(&cpu_mask);

  if (cpu_mask_str == nullptr || cpu_mask_str->length() == 0) {
    null_value = true;
    return nullptr;
  }

  std::ostringstream oss("");
  cpu_mask_str->set_charset(&my_charset_bin);

  int bit_start = -1, bit_end = -1;
  int start_pos = cpu_mask_str->length() - 1;
  bool first = true;
  for (int i = start_pos; i >= 0; i--) {
    if (cpu_mask_str->ptr()[i] == '1')
      bit_start == -1 ? (bit_start = bit_end = start_pos - i) : bit_end++;
    else if (bit_start != -1) {
      if (first)
        first = false;
      else
        oss << ",";

      if (bit_start == bit_end)
        oss << bit_start;
      else
        oss << bit_start << "-" << bit_end;
      bit_start = bit_end = -1;
    }
  }
  if (oss.str().length() == 0)
    oss << "0-"
        << resourcegroups::Resource_group_mgr::instance()->num_vcpus() - 1;

  str->copy(oss.str().c_str(), oss.str().length(), &my_charset_bin);

  return str;
}

void Item_func_current_role::cleanup() {
  if (value_cache_set) {
    value_cache.set((char *)nullptr, 0, system_charset_info);
    value_cache_set = false;
  }
}

String *Item_func_current_role::val_str(String *) {
  set_current_role(current_thd);
  return &value_cache;
}

void Item_func_current_role::set_current_role(THD *thd) {
  if (!value_cache_set) {
    func_current_role(thd, &value_cache);
    value_cache_set = true;
  }
}

/**
 @brief Constructs and caches the graphml string

 Called once per query and the result cached inside value_cache

 @param         thd     The current session

 @retval false success
 @retval true failure
 */

bool Item_func_roles_graphml::calculate_graphml(THD *thd) {
  Security_context *sctx = thd->security_context();
  if (sctx &&
      (sctx->has_global_grant(STRING_WITH_LEN("ROLE_ADMIN")).first ||
       sctx->check_access(SUPER_ACL, "", false)) &&
      !skip_grant_tables())
    roles_graphml(thd, &value_cache);
  else
    value_cache.set_ascii(
        STRING_WITH_LEN("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                        "<graphml />"));
  value_cache_set = true;
  return false;
}

String *Item_func_roles_graphml::val_str(String *) {
  calculate_graphml(current_thd);
  return &value_cache;
}

void Item_func_roles_graphml::cleanup() {
  if (value_cache_set) {
    value_cache.set((char *)nullptr, 0, system_charset_info);
    value_cache_set = false;
  }
}

/**
  @brief
    This function prepares string representing value stored at key
    supplied.
    This is required for upgrade to parse encryption key value.

    Syntax:
      string get_dd_property_key_value(dd.table.options, key)
 */
String *Item_func_get_dd_property_key_value::val_str(String *str) {
  DBUG_TRACE;

  // Read tables.options
  String properties;
  String key;
  std::ostringstream oss("");
  null_value = false;

  String *properties_ptr = args[0]->val_str(&properties);
  String *key_ptr = args[1]->val_str(&key);

  if (key_ptr == nullptr || properties_ptr == nullptr) {
    null_value = true;
    return nullptr;
  }

  // Read required values from properties
  std::unique_ptr<dd::Properties> p(
      dd::Properties::parse_properties(properties_ptr->c_ptr_safe()));

  // Warn if the property string is corrupt.
  if (!p.get()) {
    LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
           properties_ptr->c_ptr_safe());
    str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
    null_value = true;
    return str;
  }

  // Read the value at key
  dd::String_type keyname(key_ptr->c_ptr_safe(), strlen(key_ptr->c_ptr_safe()));
  dd::String_type value;
  if (p->exists(keyname)) {
    p->get(keyname, &value);
    oss << value;
  } else {
    null_value = true;
    return nullptr;
  }

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

/**
  @brief
    This function removes a key value from given property string.
    This is required during upgrade to encryption key value.

    Syntax:
      string remove_dd_property_key(dd.table.options, key)

    The function either returns the string after removing the key OR
    returns the original property string if key is not found.
 */
String *Item_func_remove_dd_property_key::val_str(String *str) {
  DBUG_TRACE;

  // Read tables.options
  String properties;
  String key;
  std::ostringstream oss("");
  null_value = false;

  String *properties_ptr = args[0]->val_str(&properties);
  String *key_ptr = args[1]->val_str(&key);

  if (key_ptr == nullptr || properties_ptr == nullptr) {
    null_value = true;
    return nullptr;
  }

  // Read required values from properties
  std::unique_ptr<dd::Properties> p(
      dd::Properties::parse_properties(properties_ptr->c_ptr_safe()));

  // Warn if the property string is corrupt.
  if (!p.get()) {
    LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
           properties_ptr->c_ptr_safe());
    str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
    return str;
  }

  // Read the value at key
  dd::String_type keyname(key_ptr->c_ptr_safe(), strlen(key_ptr->c_ptr_safe()));
  (void)p->remove(keyname);
  oss << p->raw_string();

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

/*
  This function converts interval expression to the interval value specified
  by the user at time of creating events.
  @param str pointer to String whose output is filled with user supplied
             interval value at time of event creation.
  @return returns a pointer to the string with the converted interval value.
 */
String *Item_func_convert_interval_to_user_interval::val_str(String *str) {
  if (!args[0]->is_null() && !args[1]->is_null()) {
    longlong event_interval_val = args[0]->val_int();
    interval_type event_interval_unit = dd::get_old_interval_type(
        (dd::Event::enum_interval_field)args[1]->val_int());
    str->length(0);
    Events::reconstruct_interval_expression(str, event_interval_unit,
                                            event_interval_val);
    null_value = false;
    return str;
  }
  null_value = true;
  return nullptr;
}

/*
  This function retrives the user name in 'user@host' authentication
  identifier.

  @param str pointer to String whose output is filled with user name.

  @return returns a pointer to the string with the user name.
*/
String *Item_func_internal_get_username::val_str(String *str) {
  if (arg_count == 1 && args[0]->is_null()) {
    null_value = true;
    return nullptr;
  }

  String username;
  null_value = false;

  /*
    If the argument is not supplied, then return the current user name,
    otherwise retrieve the user name from the given argument.
  */
  if (arg_count == 0) {
    THD *thd = current_thd;
    str->copy(thd->m_main_security_ctx.priv_user().str,
              thd->m_main_security_ctx.priv_user().length, system_charset_info);
  } else {
    String *username_ptr = args[0]->val_str(&username);
    auto user_host_pair =
        get_authid_from_quoted_string(username_ptr->c_ptr_safe());

    str->copy(user_host_pair.first.c_str(), user_host_pair.first.length(),
              system_charset_info);
  }

  return str;
}

/*
  This function retrives the host name in 'user@host' authentication
  identifier.

  @param str pointer to String whose output is filled with user name.

  @return returns a pointer to the string with the user name.
*/
String *Item_func_internal_get_hostname::val_str(String *str) {
  if (arg_count == 1 && args[0]->is_null()) {
    null_value = true;
    return nullptr;
  }

  String hostname;
  null_value = false;

  /*
    If the argument is not supplied, then return the current user host,
    otherwise retrieve the host name from the given argument.
  */
  if (arg_count == 0) {
    THD *thd = current_thd;
    str->copy(thd->m_main_security_ctx.priv_host().str,
              thd->m_main_security_ctx.priv_host().length, system_charset_info);
  } else {
    String *hostname_ptr = args[0]->val_str(&hostname);
    auto user_host_pair =
        get_authid_from_quoted_string(hostname_ptr->c_ptr_safe());

    str->copy(user_host_pair.second.c_str(), user_host_pair.second.length(),
              system_charset_info);
  }

  return str;
}

/*
  Prepare JSON string with role name and host name pair, which are
  currently active.

  @return returns a pointer to the json string with the user name.
*/
String *Item_func_internal_get_enabled_role_json::val_str(String *str) {
  THD *thd = current_thd;
  std::ostringstream oss("");

  // Iterate through active roles.
  if (thd->security_context()->get_active_roles()->size()) {
    oss << "[";
    bool first = true;
    for (auto &ref : *thd->security_context()->get_active_roles()) {
      if (!first) {
        oss << ",";
      } else {
        first = false;
      }
      oss << R"({"ROLE_NAME":")" << ref.first.str << R"(",)";
      oss << R"("ROLE_HOST":")" << ref.second.str << R"("})";
    }
    oss << "]";
  } else
    oss << "[]";

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

/*
  Prepare JSON string with role name and host name pair, which are
  set as global mandatory roles for PUBLIC.

  @return returns a pointer to the json string with the user name.
*/
String *Item_func_internal_get_mandatory_roles_json::val_str(String *str) {
  std::ostringstream oss("");

  std::vector<Role_id> mandatory_roles;

  // We contact ACL system, only if it is initialized.
  if (is_acl_inited() && lock_and_get_mandatory_roles(&mandatory_roles)) {
    push_warning(current_thd, Sql_condition::SL_WARNING,
                 ER_FAILED_TO_FETCH_MANDATORY_ROLE_LIST,
                 ER_THD(current_thd, ER_FAILED_TO_FETCH_MANDATORY_ROLE_LIST));
    /*
      mandatory_roles list would be empty when we are here. And we return
      string '[]' from this function.
    */
  }

  // Iterate through mandatory roles.
  if (mandatory_roles.size()) {
    oss << "[";
    bool first = true;
    for (auto &role : mandatory_roles) {
      if (!first) {
        oss << ",";
      } else {
        first = false;
      }
      oss << R"({"ROLE_NAME":")" << role.user().c_str() << R"(",)";
      oss << R"("ROLE_HOST":")" << role.host().c_str() << R"("})";
    }
    oss << "]";
  } else
    oss << "[]";

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}

/**
  @brief
    This function prepares string representing EXTRA column for I_S.COLUMNS.

  @param str   A String object that we can write to.

    Syntax:
      string internal_get_dd_column_extra(dd.table.options)

  @return returns a pointer to the string containing column options.
 */
String *Item_func_internal_get_dd_column_extra::val_str(String *str) {
  DBUG_TRACE;

  std::ostringstream oss("");
  null_value = false;

  // Create UPDATE_OPTION. This can be null.
  String update_option;
  String *update_option_ptr = args[3]->val_str(&update_option);

  // Create COLUMNS.OPTIONS. This can not be null.
  String properties;
  String *properties_ptr = args[5]->val_str(&properties);

  // Stop if any of required argument is not supplied.
  if (args[0]->is_null() || args[1]->is_null() || args[2]->is_null() ||
      args[4]->is_null()) {
    null_value = true;
    return nullptr;
  }

  bool is_not_generated_column = args[0]->val_int();
  bool is_virtual = args[1]->val_int();
  bool is_auto_increment = args[2]->val_int();
  bool has_update_option = update_option_ptr != nullptr;
  bool is_default_option = args[4]->val_int();

  if (is_not_generated_column) {
    if (is_default_option) oss << "DEFAULT_GENERATED";
    if (has_update_option) {
      if (oss.str().length()) oss << " ";
      oss << "on update " << update_option_ptr->c_ptr_safe();
    }
    if (is_auto_increment) {
      if (oss.str().length()) oss << " ";
      oss << "auto_increment";
    }
  } else {
    oss << (is_virtual ? "VIRTUAL GENERATED" : "STORED GENERATED");
  }

  // Print the column property 'NOT SECONDARY'.
  if (properties_ptr != nullptr) {
    // Read required values from properties
    std::unique_ptr<dd::Properties> p(
        dd::Properties::parse_properties(properties_ptr->c_ptr_safe()));

    // Warn if the property string is corrupt.
    if (!p.get()) {
      LogErr(WARNING_LEVEL, ER_WARN_PROPERTY_STRING_PARSE_FAILED,
             properties_ptr->c_ptr_safe());
      str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);
      return str;
    }

    if (p->exists("not_secondary")) {
      if (oss.str().length()) oss << " ";
      oss << "NOT SECONDARY";
    }
  }

  str->copy(oss.str().c_str(), oss.str().length(), system_charset_info);

  return str;
}
