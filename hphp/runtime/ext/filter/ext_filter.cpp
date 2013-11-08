/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_filter.h"
#include "hphp/runtime/ext/filter/logical_filters.h"
#include "hphp/runtime/ext/filter/sanitizing_filters.h"

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(filter);
///////////////////////////////////////////////////////////////////////////////

const int64_t k_INPUT_POST = 0;
const int64_t k_INPUT_GET = 1;
const int64_t k_INPUT_COOKIE = 2;
const int64_t k_INPUT_ENV = 4;
const int64_t k_INPUT_SERVER = 5;
const int64_t k_INPUT_SESSION = 6;
const int64_t k_INPUT_REQUEST = 99;
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
const int64_t k_FILTER_CALLBACK = 1024;
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
  }, {
    StaticString("callback"),
    k_FILTER_CALLBACK,
    php_filter_callback
  },
};

const StaticString
  s_flags("flags"),
  s_default("default"),
  s_options("options");

static Variant fail(bool return_null, CVarRef options) {
  if (options.isArray()) {
    CArrRef arr(options.toArray());
    if (arr.exists(s_default)) {
      return options[s_default];
    }
  }
  if (return_null) {
    return uninit_null();
  }
  return false;
}

static filter_list_entry php_find_filter(uint64_t id) {
  int i, size = sizeof(filter_list) / sizeof(filter_list_entry);

  for (i = 0; i < size; ++i) {
    if (filter_list[i].id == id) {
      return filter_list[i];
    }
  }
  /* Fallback to "string" filter */
  for (i = 0; i < size; ++i) {
    if (filter_list[i].id == k_FILTER_DEFAULT) {
      return filter_list[i];
    }
  }
  // never hit
  return filter_list[0];
}

#define FAIL_IF(x) do { if (x) return false; } while (0)

static bool filter_var(Variant& ret, CVarRef variable, int64_t filter,
                       CVarRef options) {
  filter_list_entry filter_func = php_find_filter(filter);

  int64_t flags;
  Variant option_array;
  if (options.isArray()) {
    flags = options[s_flags].toInt64();
    option_array = options[s_options];
  } else {
    flags = options.toInt64();
  }

  FAIL_IF(variable.isObject() && !variable.getObjectData()->hasToString());

  ret = filter_func.function(variable.toString(), flags, option_array);
  if (option_array.isArray() && option_array.toArray().exists(s_default) &&
      ((flags & k_FILTER_NULL_ON_FAILURE && ret.isNull()) ||
       (!(flags & k_FILTER_NULL_ON_FAILURE) && ret.isBoolean() &&
        ret.asBooleanVal() == 0))) {
    ret = option_array[s_default];
  }
  return true;
}

static bool filter_recursive(Variant& ret, CVarRef variable, int64_t filter,
                             CVarRef options) {
  Array arr;
  for (ArrayIter iter(variable.toArray()); iter; ++iter) {
    Variant v;
    if (iter.second().isArray()) {
      FAIL_IF(!filter_recursive(v, iter.second().toArray(), filter, options));
    } else {
      FAIL_IF(!filter_var(v, iter.second(), filter, options));
    }
    arr.add(iter.first(), v);
  }
  ret = arr;
  return true;
}

#undef FAIL_IF

///////////////////////////////////////////////////////////////////////////////

Variant f_filter_list() {
  size_t size = sizeof(filter_list) / sizeof(filter_list_entry);
  Array ret;
  for (size_t i = 0; i < size; ++i) {
    ret.append(filter_list[i].name);
  }
  return ret;
}

Variant f_filter_id(const String& filtername) {
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

Variant f_filter_var(CVarRef variable, int64_t filter /* = 516 */,
                     CVarRef options /* = empty_array */) {
  int64_t filter_flags;
  if (options.isArray()) {
    filter_flags = options[s_flags].toInt64();
  } else {
    filter_flags = options.toInt64();
  }

  if (!(filter_flags & k_FILTER_REQUIRE_ARRAY ||
        filter_flags & k_FILTER_FORCE_ARRAY)) {
    filter_flags |= k_FILTER_REQUIRE_SCALAR;
  }

  // No idea why, but zend does this..
  if (filter == k_FILTER_CALLBACK) {
    filter_flags = 0;
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
    ret = make_packed_array(ret);
  }
  return ret;
}

#undef FAIL_IF

///////////////////////////////////////////////////////////////////////////////
}
