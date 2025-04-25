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

#pragma once

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/thrift/transport.h"

namespace HPHP::thrift {

enum TError {
  ERR_UNKNOWN = 0,
  ERR_INVALID_DATA = 1,
  ERR_BAD_VERSION = 4
};

[[noreturn]] inline void thrift_error(const String& what, TError why) {
  throw_object(s_TProtocolException, make_vec_array(what, why));
}

inline void set_with_intish_key_cast(
    DictInit& arr,
    const Variant& key,
    const Variant& value) {
  if (key.isString()) {
    int64_t intish_key;
    if (key.getStringData()->isStrictlyInteger(intish_key)) {
      arr.set(intish_key, value);
    } else {
      arr.set(key.toString(), value);
    }
  } else if (key.isInteger()) {
    arr.set(key.toInt64(), value);
  } else {
    thrift_error(
        "Unable to deserialize non int/string array keys",
        ERR_INVALID_DATA);
  }
}


inline bool is_value_type_default(int8_t thrift_typeID, const Variant& value) {
  switch(thrift_typeID){
    case T_BOOL:
      return value.toBoolean() == false;
    case T_BYTE:
    case T_I16:
    case T_I32:
    case T_U64:
    case T_I64:
      return value.toInt64() == 0;
    case T_DOUBLE:
    case T_FLOAT:
      return value.toDouble() == 0.0;
    case T_UTF8:
    case T_UTF16:
    case T_STRING:
      return value.toString().empty();
    case T_MAP:
    case T_LIST:
    case T_SET:
      return value.toArray<IntishCast::Cast>().empty();
    default:
      return false;
    }
}

inline Array initialize_array(const uint32_t size) {
  // Reserve up to 16k entries for perf - but after that use "normal"
  // array expansion so that we ensure the data is actually there before
  // allocating massive arrays.
  return Array::attach(VanillaVec::MakeReserveVec(std::min(16384u, size)));
}

inline void check_container_size(const uint32_t size) {
  if (UNLIKELY(size > std::numeric_limits<int32_t>::max())) {
    raise_warning(
        "%u exceeds array size limit %u",
        size,
        std::numeric_limits<int32_t>::max());
  }
}

class StrictUnionChecker {
public:
  explicit StrictUnionChecker(bool enabled) : enabled_{enabled}, unionFieldFound_{false} {}

  void markFieldFound() {
    if (!enabled_) {
      return;
    }
    
    if (unionFieldFound_) {
      thrift_error("Union field already set", ERR_INVALID_DATA);
    }
    unionFieldFound_ = true;
  }
private:
  bool enabled_{false};
  bool unionFieldFound_{false};
};

}
