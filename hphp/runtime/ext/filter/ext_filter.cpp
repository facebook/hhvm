/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/filter/ext_filter.h"

#include "hphp/runtime/ext/filter/logical_filters.h"
#include "hphp/runtime/ext/filter/sanitizing_filters.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/comparisons.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

const int64_t k_FILTER_FLAG_NONE = 0;
const int64_t k_FILTER_REQUIRE_SCALAR = 33554432;
const int64_t k_FILTER_REQUIRE_ARRAY = 16777216;
const int64_t k_FILTER_FORCE_ARRAY = 67108864;
const int64_t k_FILTER_NULL_ON_FAILURE = 134217728;
const int64_t k_FILTER_VALIDATE_INT = 257;
const int64_t k_FILTER_VALIDATE_BOOLEAN = 258;
const int64_t k_FILTER_VALIDATE_FLOAT = 259;
const int64_t k_FILTER_VALIDATE_REGEXP = 272;
const int64_t k_FILTER_VALIDATE_URL = 273;
const int64_t k_FILTER_VALIDATE_EMAIL = 274;
const int64_t k_FILTER_VALIDATE_IP = 275;
const int64_t k_FILTER_VALIDATE_MAC = 276;
const int64_t k_FILTER_DEFAULT = 516;
const int64_t k_FILTER_UNSAFE_RAW = 516;
const int64_t k_FILTER_SANITIZE_STRING = 513;
const int64_t k_FILTER_SANITIZE_STRIPPED = 513;
const int64_t k_FILTER_SANITIZE_ENCODED = 514;
const int64_t k_FILTER_SANITIZE_SPECIAL_CHARS = 515;
const int64_t k_FILTER_SANITIZE_FULL_SPECIAL_CHARS = 515;
const int64_t k_FILTER_SANITIZE_EMAIL = 517;
const int64_t k_FILTER_SANITIZE_URL = 518;
const int64_t k_FILTER_SANITIZE_NUMBER_INT = 519;
const int64_t k_FILTER_SANITIZE_NUMBER_FLOAT = 520;
const int64_t k_FILTER_SANITIZE_MAGIC_QUOTES = 521;
const int64_t k_FILTER_FLAG_ALLOW_OCTAL = 1;
const int64_t k_FILTER_FLAG_ALLOW_HEX = 2;
const int64_t k_FILTER_FLAG_STRIP_LOW = 4;
const int64_t k_FILTER_FLAG_STRIP_HIGH = 8;
const int64_t k_FILTER_FLAG_ENCODE_LOW = 16;
const int64_t k_FILTER_FLAG_ENCODE_HIGH = 32;
const int64_t k_FILTER_FLAG_ENCODE_AMP = 64;
const int64_t k_FILTER_FLAG_NO_ENCODE_QUOTES = 128;
const int64_t k_FILTER_FLAG_EMPTY_STRING_NULL = 256;
const int64_t k_FILTER_FLAG_STRIP_BACKTICK = 512;
const int64_t k_FILTER_FLAG_ALLOW_FRACTION = 4096;
const int64_t k_FILTER_FLAG_ALLOW_THOUSAND = 8192;
const int64_t k_FILTER_FLAG_ALLOW_SCIENTIFIC = 16384;
const int64_t k_FILTER_FLAG_SCHEME_REQUIRED = 65536;
const int64_t k_FILTER_FLAG_HOST_REQUIRED = 131072;
const int64_t k_FILTER_FLAG_PATH_REQUIRED = 262144;
const int64_t k_FILTER_FLAG_QUERY_REQUIRED = 524288;
const int64_t k_FILTER_FLAG_IPV4 = 1048576;
const int64_t k_FILTER_FLAG_IPV6 = 2097152;
const int64_t k_FILTER_FLAG_NO_RES_RANGE = 4194304;
const int64_t k_FILTER_FLAG_NO_PRIV_RANGE = 8388608;


static struct FilterExtension final : Extension {
  FilterExtension() : Extension("filter", "0.12.1", NO_ONCALL_YET) {}

  void moduleRegisterNative() override;

} s_filter_extension;

///////////////////////////////////////////////////////////////////////////////

typedef struct filter_list_entry {
  StaticString name;
  int64_t id;
  Variant (*function)(PHP_INPUT_FILTER_PARAM_DECL);
} filter_list_entry;

static const filter_list_entry filter_list[] = {
  {
    StaticString("int"),
    k_FILTER_VALIDATE_INT,
    php_filter_int
  }, {
    StaticString("boolean"),
    k_FILTER_VALIDATE_BOOLEAN,
    php_filter_boolean
  }, {
    StaticString("float"),
    k_FILTER_VALIDATE_FLOAT,
    php_filter_float
  }, {
    StaticString("validate_regexp"),
    k_FILTER_VALIDATE_REGEXP,
    php_filter_validate_regexp
  }, {
    StaticString("validate_url"),
    k_FILTER_VALIDATE_URL,
    php_filter_validate_url
  }, {
    StaticString("validate_email"),
    k_FILTER_VALIDATE_EMAIL,
    php_filter_validate_email
  }, {
    StaticString("validate_ip"),
    k_FILTER_VALIDATE_IP,
    php_filter_validate_ip
  }, {
    StaticString("validate_mac"),
    k_FILTER_VALIDATE_MAC,
    php_filter_validate_mac
  }, {
    StaticString("string"),
    k_FILTER_SANITIZE_STRING,
    php_filter_string
  }, {
    StaticString("stripped"),
    k_FILTER_SANITIZE_STRING,
    php_filter_string
  }, {
    StaticString("encoded"),
    k_FILTER_SANITIZE_ENCODED,
    php_filter_encoded
  }, {
    StaticString("special_chars"),
    k_FILTER_SANITIZE_SPECIAL_CHARS,
    php_filter_special_chars
  }, {
    StaticString("full_special_chars"),
    k_FILTER_SANITIZE_FULL_SPECIAL_CHARS,
    php_filter_full_special_chars
  }, {
    StaticString("unsafe_raw"),
    k_FILTER_UNSAFE_RAW,
    php_filter_unsafe_raw
  }, {
    StaticString("email"),
    k_FILTER_SANITIZE_EMAIL,
    php_filter_email
  }, {
    StaticString("url"),
    k_FILTER_SANITIZE_URL,
    php_filter_url
  }, {
    StaticString("number_int"),
    k_FILTER_SANITIZE_NUMBER_INT,
    php_filter_number_int
  }, {
    StaticString("number_float"),
    k_FILTER_SANITIZE_NUMBER_FLOAT,
    php_filter_number_float
  }, {
    StaticString("magic_quotes"),
    k_FILTER_SANITIZE_MAGIC_QUOTES,
    php_filter_magic_quotes
  },
};

const StaticString
  s_flags("flags"),
  s_default("default"),
  s_options("options");

static Variant fail(bool return_null, const Variant& options) {
  if (options.isArray()) {
    const Array& arr(options.toArray());
    if (arr.exists(s_default)) {
      return arr[s_default];
    }
  }
  if (return_null) {
    return init_null();
  }
  return false;
}

static const filter_list_entry* php_find_filter(uint64_t id) {
  int i, size = sizeof(filter_list) / sizeof(filter_list_entry);

  for (i = 0; i < size; ++i) {
    if (filter_list[i].id == id) {
      return &filter_list[i];
    }
  }

  return nullptr;
}

#define FAIL_IF(x) do { if (x) return false; } while (0)

static bool filter_var(Variant& ret, const Variant& variable, int64_t filter,
                       const Variant& options) {
  const filter_list_entry* filter_func = php_find_filter(filter);
  if (!filter_func) {
    return false;
  }

  int64_t flags;
  Variant option_array;
  if (options.isArray()) {
    auto arr = options.toArray();
    flags = arr[s_flags].toInt64();
    option_array = arr[s_options];
  } else {
    flags = options.toInt64();
  }

  FAIL_IF(variable.isObject() && !variable.getObjectData()->hasToString());

  ret = filter_func->function(variable.toString(), flags, option_array);
  if (option_array.isArray() && option_array.toArray().exists(s_default) &&
      ((flags & k_FILTER_NULL_ON_FAILURE && ret.isNull()) ||
       (!(flags & k_FILTER_NULL_ON_FAILURE) && ret.isBoolean() &&
        ret.asBooleanVal() == 0))) {
    ret = option_array.toArray()[s_default];
  }
  return true;
}

static bool filter_recursive(Variant& ret, const Variant& variable,
                             int64_t filter, const Variant& options) {
  Array arr = Array::CreateDict();
  for (ArrayIter iter(variable.toArray()); iter; ++iter) {
    Variant v;
    if (iter.second().isArray()) {
      FAIL_IF(!filter_recursive(v, iter.second().toArray(), filter, options));
    } else {
      FAIL_IF(!filter_var(v, iter.second(), filter, options));
    }
    arr.set(iter.first(), v);
  }
  ret = arr;
  return true;
}

#undef FAIL_IF

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(filter_list) {
  size_t size = sizeof(filter_list) / sizeof(filter_list_entry);
  Array ret;
  for (size_t i = 0; i < size; ++i) {
    ret.append(filter_list[i].name);
  }
  return ret;
}

Variant HHVM_FUNCTION(filter_id,
                      const String& filtername) {
  size_t size = sizeof(filter_list) / sizeof(filter_list_entry);
  for (size_t i = 0; i < size; ++i) {
    if (filter_list[i].name == filtername) {
      return filter_list[i].id;
    }
  }

  return false;
}

#define FAIL_IF(x) \
  do { \
    if (x) return fail(filter_flags & k_FILTER_NULL_ON_FAILURE, options); \
  } while(0)

Variant HHVM_FUNCTION(filter_var,
                      const Variant& variable,
                      int64_t filter /* = FILTER_DEFAULT */,
                      const Variant& options /* = shape() */) {
  int64_t filter_flags;
  if (options.isArray()) {
    filter_flags = options.asCArrRef()[s_flags].toInt64();
  } else {
    filter_flags = options.toInt64();
  }

  if (!(filter_flags & k_FILTER_REQUIRE_ARRAY ||
        filter_flags & k_FILTER_FORCE_ARRAY)) {
    filter_flags |= k_FILTER_REQUIRE_SCALAR;
  }

  if (variable.isArray()) {
    FAIL_IF(filter_flags & k_FILTER_REQUIRE_SCALAR);
    Variant ret;
    FAIL_IF(!filter_recursive(ret, variable, filter, options));
    return ret;
  }
  FAIL_IF(filter_flags & k_FILTER_REQUIRE_ARRAY);

  Variant ret;
  FAIL_IF(!filter_var(ret, variable, filter, options));
  if (filter_flags & k_FILTER_FORCE_ARRAY && !ret.isArray()) {
    ret = make_vec_array(ret);
  }
  return ret;
}

#undef FAIL_IF

void FilterExtension::moduleRegisterNative() {
  HHVM_RC_INT(FILTER_FLAG_NONE, k_FILTER_FLAG_NONE);
  HHVM_RC_INT(FILTER_REQUIRE_SCALAR, k_FILTER_REQUIRE_SCALAR);
  HHVM_RC_INT(FILTER_REQUIRE_ARRAY, k_FILTER_REQUIRE_ARRAY);
  HHVM_RC_INT(FILTER_FORCE_ARRAY, k_FILTER_FORCE_ARRAY);
  HHVM_RC_INT(FILTER_NULL_ON_FAILURE, k_FILTER_NULL_ON_FAILURE);
  HHVM_RC_INT(FILTER_VALIDATE_INT, k_FILTER_VALIDATE_INT);
  HHVM_RC_INT(FILTER_VALIDATE_BOOLEAN, k_FILTER_VALIDATE_BOOLEAN);
  HHVM_RC_INT(FILTER_VALIDATE_FLOAT, k_FILTER_VALIDATE_FLOAT);
  HHVM_RC_INT(FILTER_VALIDATE_REGEXP, k_FILTER_VALIDATE_REGEXP);
  HHVM_RC_INT(FILTER_VALIDATE_URL, k_FILTER_VALIDATE_URL);
  HHVM_RC_INT(FILTER_VALIDATE_EMAIL, k_FILTER_VALIDATE_EMAIL);
  HHVM_RC_INT(FILTER_VALIDATE_IP, k_FILTER_VALIDATE_IP);
  HHVM_RC_INT(FILTER_VALIDATE_MAC, k_FILTER_VALIDATE_MAC);
  HHVM_RC_INT(FILTER_DEFAULT, k_FILTER_DEFAULT);
  HHVM_RC_INT(FILTER_UNSAFE_RAW, k_FILTER_UNSAFE_RAW);
  HHVM_RC_INT(FILTER_SANITIZE_STRING, k_FILTER_SANITIZE_STRING);
  HHVM_RC_INT(FILTER_SANITIZE_STRIPPED, k_FILTER_SANITIZE_STRIPPED);
  HHVM_RC_INT(FILTER_SANITIZE_ENCODED, k_FILTER_SANITIZE_ENCODED);
  HHVM_RC_INT(FILTER_SANITIZE_SPECIAL_CHARS, k_FILTER_SANITIZE_SPECIAL_CHARS);
  HHVM_RC_INT(FILTER_SANITIZE_FULL_SPECIAL_CHARS,
              k_FILTER_SANITIZE_FULL_SPECIAL_CHARS);
  HHVM_RC_INT(FILTER_SANITIZE_EMAIL, k_FILTER_SANITIZE_EMAIL);
  HHVM_RC_INT(FILTER_SANITIZE_URL, k_FILTER_SANITIZE_URL);
  HHVM_RC_INT(FILTER_SANITIZE_NUMBER_INT, k_FILTER_SANITIZE_NUMBER_INT);
  HHVM_RC_INT(FILTER_SANITIZE_NUMBER_FLOAT, k_FILTER_SANITIZE_NUMBER_FLOAT);
  HHVM_RC_INT(FILTER_SANITIZE_MAGIC_QUOTES, k_FILTER_SANITIZE_MAGIC_QUOTES);
  HHVM_RC_INT(FILTER_FLAG_ALLOW_OCTAL, k_FILTER_FLAG_ALLOW_OCTAL);
  HHVM_RC_INT(FILTER_FLAG_ALLOW_HEX, k_FILTER_FLAG_ALLOW_HEX);
  HHVM_RC_INT(FILTER_FLAG_STRIP_LOW, k_FILTER_FLAG_STRIP_LOW);
  HHVM_RC_INT(FILTER_FLAG_STRIP_HIGH, k_FILTER_FLAG_STRIP_HIGH);
  HHVM_RC_INT(FILTER_FLAG_ENCODE_LOW, k_FILTER_FLAG_ENCODE_LOW);
  HHVM_RC_INT(FILTER_FLAG_ENCODE_HIGH, k_FILTER_FLAG_ENCODE_HIGH);
  HHVM_RC_INT(FILTER_FLAG_ENCODE_AMP, k_FILTER_FLAG_ENCODE_AMP);
  HHVM_RC_INT(FILTER_FLAG_NO_ENCODE_QUOTES, k_FILTER_FLAG_NO_ENCODE_QUOTES);
  HHVM_RC_INT(FILTER_FLAG_EMPTY_STRING_NULL, k_FILTER_FLAG_EMPTY_STRING_NULL);
  HHVM_RC_INT(FILTER_FLAG_STRIP_BACKTICK, k_FILTER_FLAG_STRIP_BACKTICK);
  HHVM_RC_INT(FILTER_FLAG_ALLOW_FRACTION, k_FILTER_FLAG_ALLOW_FRACTION);
  HHVM_RC_INT(FILTER_FLAG_ALLOW_THOUSAND, k_FILTER_FLAG_ALLOW_THOUSAND);
  HHVM_RC_INT(FILTER_FLAG_ALLOW_SCIENTIFIC, k_FILTER_FLAG_ALLOW_SCIENTIFIC);
  HHVM_RC_INT(FILTER_FLAG_SCHEME_REQUIRED, k_FILTER_FLAG_SCHEME_REQUIRED);
  HHVM_RC_INT(FILTER_FLAG_HOST_REQUIRED, k_FILTER_FLAG_HOST_REQUIRED);
  HHVM_RC_INT(FILTER_FLAG_PATH_REQUIRED, k_FILTER_FLAG_PATH_REQUIRED);
  HHVM_RC_INT(FILTER_FLAG_QUERY_REQUIRED, k_FILTER_FLAG_QUERY_REQUIRED);
  HHVM_RC_INT(FILTER_FLAG_IPV4, k_FILTER_FLAG_IPV4);
  HHVM_RC_INT(FILTER_FLAG_IPV6, k_FILTER_FLAG_IPV6);
  HHVM_RC_INT(FILTER_FLAG_NO_RES_RANGE, k_FILTER_FLAG_NO_RES_RANGE);
  HHVM_RC_INT(FILTER_FLAG_NO_PRIV_RANGE, k_FILTER_FLAG_NO_PRIV_RANGE);

  HHVM_FE(filter_list);
  HHVM_FE(filter_id);
  HHVM_FE(filter_var);
}

///////////////////////////////////////////////////////////////////////////////
}
