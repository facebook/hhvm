/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/php-globals.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

#define DEFINE_CONSTANT(name, value)                                           \
  const int64_t k_##name = value;                                              \
  const StaticString s_##name(#name)                                           \

DEFINE_CONSTANT(INPUT_POST, 0);
DEFINE_CONSTANT(INPUT_GET, 1);
DEFINE_CONSTANT(INPUT_COOKIE, 2);
DEFINE_CONSTANT(INPUT_ENV, 4);
DEFINE_CONSTANT(INPUT_SERVER, 5);
DEFINE_CONSTANT(INPUT_SESSION, 6);
DEFINE_CONSTANT(INPUT_REQUEST, 99);
DEFINE_CONSTANT(FILTER_FLAG_NONE, 0);
DEFINE_CONSTANT(FILTER_REQUIRE_SCALAR, 33554432);
DEFINE_CONSTANT(FILTER_REQUIRE_ARRAY, 16777216);
DEFINE_CONSTANT(FILTER_FORCE_ARRAY, 67108864);
DEFINE_CONSTANT(FILTER_NULL_ON_FAILURE, 134217728);
DEFINE_CONSTANT(FILTER_VALIDATE_INT, 257);
DEFINE_CONSTANT(FILTER_VALIDATE_BOOLEAN, 258);
DEFINE_CONSTANT(FILTER_VALIDATE_FLOAT, 259);
DEFINE_CONSTANT(FILTER_VALIDATE_REGEXP, 272);
DEFINE_CONSTANT(FILTER_VALIDATE_URL, 273);
DEFINE_CONSTANT(FILTER_VALIDATE_EMAIL, 274);
DEFINE_CONSTANT(FILTER_VALIDATE_IP, 275);
DEFINE_CONSTANT(FILTER_VALIDATE_MAC, 276);
DEFINE_CONSTANT(FILTER_DEFAULT, 516);
DEFINE_CONSTANT(FILTER_UNSAFE_RAW, 516);
DEFINE_CONSTANT(FILTER_SANITIZE_STRING, 513);
DEFINE_CONSTANT(FILTER_SANITIZE_STRIPPED, 513);
DEFINE_CONSTANT(FILTER_SANITIZE_ENCODED, 514);
DEFINE_CONSTANT(FILTER_SANITIZE_SPECIAL_CHARS, 515);
DEFINE_CONSTANT(FILTER_SANITIZE_FULL_SPECIAL_CHARS, 515);
DEFINE_CONSTANT(FILTER_SANITIZE_EMAIL, 517);
DEFINE_CONSTANT(FILTER_SANITIZE_URL, 518);
DEFINE_CONSTANT(FILTER_SANITIZE_NUMBER_INT, 519);
DEFINE_CONSTANT(FILTER_SANITIZE_NUMBER_FLOAT, 520);
DEFINE_CONSTANT(FILTER_SANITIZE_MAGIC_QUOTES, 521);
DEFINE_CONSTANT(FILTER_CALLBACK, 1024);
DEFINE_CONSTANT(FILTER_FLAG_ALLOW_OCTAL, 1);
DEFINE_CONSTANT(FILTER_FLAG_ALLOW_HEX, 2);
DEFINE_CONSTANT(FILTER_FLAG_STRIP_LOW, 4);
DEFINE_CONSTANT(FILTER_FLAG_STRIP_HIGH, 8);
DEFINE_CONSTANT(FILTER_FLAG_ENCODE_LOW, 16);
DEFINE_CONSTANT(FILTER_FLAG_ENCODE_HIGH, 32);
DEFINE_CONSTANT(FILTER_FLAG_ENCODE_AMP, 64);
DEFINE_CONSTANT(FILTER_FLAG_NO_ENCODE_QUOTES, 128);
DEFINE_CONSTANT(FILTER_FLAG_EMPTY_STRING_NULL, 256);
DEFINE_CONSTANT(FILTER_FLAG_STRIP_BACKTICK, 512);
DEFINE_CONSTANT(FILTER_FLAG_ALLOW_FRACTION, 4096);
DEFINE_CONSTANT(FILTER_FLAG_ALLOW_THOUSAND, 8192);
DEFINE_CONSTANT(FILTER_FLAG_ALLOW_SCIENTIFIC, 16384);
DEFINE_CONSTANT(FILTER_FLAG_SCHEME_REQUIRED, 65536);
DEFINE_CONSTANT(FILTER_FLAG_HOST_REQUIRED, 131072);
DEFINE_CONSTANT(FILTER_FLAG_PATH_REQUIRED, 262144);
DEFINE_CONSTANT(FILTER_FLAG_QUERY_REQUIRED, 524288);
DEFINE_CONSTANT(FILTER_FLAG_IPV4, 1048576);
DEFINE_CONSTANT(FILTER_FLAG_IPV6, 2097152);
DEFINE_CONSTANT(FILTER_FLAG_NO_RES_RANGE, 4194304);
DEFINE_CONSTANT(FILTER_FLAG_NO_PRIV_RANGE, 8388608);

#undef DEFINE_CONSTANT

const StaticString
  s_GET("_GET"),
  s_POST("_POST"),
  s_COOKIE("_COOKIE"),
  s_SERVER("_SERVER"),
  s_ENV("_ENV");

struct FilterRequestData final : RequestEventHandler {
  void requestInit() override {
    // This doesn't copy them yet, but will do COW if they are modified
    m_GET    = php_global(s_GET).toArray();
    m_POST   = php_global(s_POST).toArray();
    m_COOKIE = php_global(s_COOKIE).toArray();
    m_SERVER = php_global(s_SERVER).toArray();
    m_ENV    = php_global(s_ENV).toArray();
  }

  void requestShutdown() override {
    m_GET = nullptr;
    m_POST = nullptr;
    m_COOKIE = nullptr;
    m_SERVER = nullptr;
    m_ENV = nullptr;
  }

  Array getVar(int64_t type) {
    switch (type) {
      case k_INPUT_GET: return m_GET;
      case k_INPUT_POST: return m_POST;
      case k_INPUT_COOKIE: return m_COOKIE;
      case k_INPUT_SERVER: return m_SERVER;
      case k_INPUT_ENV: return m_ENV;
    }
    return empty_array();
  }

  void vscan(IMarker& mark) const override {
    mark(m_GET);
    mark(m_POST);
    mark(m_COOKIE);
    mark(m_SERVER);
    mark(m_ENV);
  }

private:
  Array m_GET;
  Array m_POST;
  Array m_COOKIE;
  Array m_SERVER;
  Array m_ENV;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(FilterRequestData, s_filter_request_data);

#define REGISTER_CONSTANT(name)                                                \
  Native::registerConstant<KindOfInt64>(s_##name.get(), k_##name)              \

static class FilterExtension final : public Extension {
public:
  FilterExtension() : Extension("filter", "0.11.0") {}

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
    HHVM_FE(__SystemLib_filter_input_get_var);
    HHVM_FE(_filter_snapshot_globals);
  }

  void moduleInit() override {
    REGISTER_CONSTANT(INPUT_POST);
    REGISTER_CONSTANT(INPUT_GET);
    REGISTER_CONSTANT(INPUT_COOKIE);
    REGISTER_CONSTANT(INPUT_ENV);
    REGISTER_CONSTANT(INPUT_SERVER);
    REGISTER_CONSTANT(INPUT_SESSION);
    REGISTER_CONSTANT(INPUT_REQUEST);
    REGISTER_CONSTANT(FILTER_FLAG_NONE);
    REGISTER_CONSTANT(FILTER_REQUIRE_SCALAR);
    REGISTER_CONSTANT(FILTER_REQUIRE_ARRAY);
    REGISTER_CONSTANT(FILTER_FORCE_ARRAY);
    REGISTER_CONSTANT(FILTER_NULL_ON_FAILURE);
    REGISTER_CONSTANT(FILTER_VALIDATE_INT);
    REGISTER_CONSTANT(FILTER_VALIDATE_BOOLEAN);
    REGISTER_CONSTANT(FILTER_VALIDATE_FLOAT);
    REGISTER_CONSTANT(FILTER_VALIDATE_REGEXP);
    REGISTER_CONSTANT(FILTER_VALIDATE_URL);
    REGISTER_CONSTANT(FILTER_VALIDATE_EMAIL);
    REGISTER_CONSTANT(FILTER_VALIDATE_IP);
    REGISTER_CONSTANT(FILTER_VALIDATE_MAC);
    REGISTER_CONSTANT(FILTER_DEFAULT);
    REGISTER_CONSTANT(FILTER_UNSAFE_RAW);
    REGISTER_CONSTANT(FILTER_SANITIZE_STRING);
    REGISTER_CONSTANT(FILTER_SANITIZE_STRIPPED);
    REGISTER_CONSTANT(FILTER_SANITIZE_ENCODED);
    REGISTER_CONSTANT(FILTER_SANITIZE_SPECIAL_CHARS);
    REGISTER_CONSTANT(FILTER_SANITIZE_FULL_SPECIAL_CHARS);
    REGISTER_CONSTANT(FILTER_SANITIZE_EMAIL);
    REGISTER_CONSTANT(FILTER_SANITIZE_URL);
    REGISTER_CONSTANT(FILTER_SANITIZE_NUMBER_INT);
    REGISTER_CONSTANT(FILTER_SANITIZE_NUMBER_FLOAT);
    REGISTER_CONSTANT(FILTER_SANITIZE_MAGIC_QUOTES);
    REGISTER_CONSTANT(FILTER_CALLBACK);
    REGISTER_CONSTANT(FILTER_FLAG_ALLOW_OCTAL);
    REGISTER_CONSTANT(FILTER_FLAG_ALLOW_HEX);
    REGISTER_CONSTANT(FILTER_FLAG_STRIP_LOW);
    REGISTER_CONSTANT(FILTER_FLAG_STRIP_HIGH);
    REGISTER_CONSTANT(FILTER_FLAG_ENCODE_LOW);
    REGISTER_CONSTANT(FILTER_FLAG_ENCODE_HIGH);
    REGISTER_CONSTANT(FILTER_FLAG_ENCODE_AMP);
    REGISTER_CONSTANT(FILTER_FLAG_NO_ENCODE_QUOTES);
    REGISTER_CONSTANT(FILTER_FLAG_EMPTY_STRING_NULL);
    REGISTER_CONSTANT(FILTER_FLAG_STRIP_BACKTICK);
    REGISTER_CONSTANT(FILTER_FLAG_ALLOW_FRACTION);
    REGISTER_CONSTANT(FILTER_FLAG_ALLOW_THOUSAND);
    REGISTER_CONSTANT(FILTER_FLAG_ALLOW_SCIENTIFIC);
    REGISTER_CONSTANT(FILTER_FLAG_SCHEME_REQUIRED);
    REGISTER_CONSTANT(FILTER_FLAG_HOST_REQUIRED);
    REGISTER_CONSTANT(FILTER_FLAG_PATH_REQUIRED);
    REGISTER_CONSTANT(FILTER_FLAG_QUERY_REQUIRED);
    REGISTER_CONSTANT(FILTER_FLAG_IPV4);
    REGISTER_CONSTANT(FILTER_FLAG_IPV6);
    REGISTER_CONSTANT(FILTER_FLAG_NO_RES_RANGE);
    REGISTER_CONSTANT(FILTER_FLAG_NO_PRIV_RANGE);

    HHVM_FE(filter_list);
    HHVM_FE(filter_id);
    HHVM_FE(filter_var);

    loadSystemlib();
  }

  void requestInit() override {
    // warm up the s_filter_request_data
    s_filter_request_data->requestInit();
  }

} s_filter_extension;

#undef REGISTER_CONSTANT

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

static bool filter_recursive(Variant& ret, const Variant& variable, int64_t filter,
                             const Variant& options) {
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

Variant HHVM_FUNCTION(filter_list) {
  size_t size = sizeof(filter_list) / sizeof(filter_list_entry);
  Array ret;
  for (size_t i = 0; i < size; ++i) {
    ret.append(filter_list[i].name);
  }
  return ret;
}

Variant HHVM_FUNCTION(filter_id,
                      const Variant& filtername) {
  if (filtername.isArray()) {
    raise_warning("Array to string conversion");
    return init_null();
  }

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
                      int64_t filter /* = 516 */,
                      const Variant& options /* = empty_array_ref */) {
  int64_t filter_flags;
  if (options.isArray()) {
    filter_flags = options.toCArrRef()[s_flags].toInt64();
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

Array HHVM_FUNCTION(__SystemLib_filter_input_get_var, int64_t variable_name) {
  return s_filter_request_data->getVar(variable_name);
}

void HHVM_FUNCTION(_filter_snapshot_globals) {
  s_filter_request_data->requestInit();
}

///////////////////////////////////////////////////////////////////////////////
}
