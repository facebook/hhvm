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

#ifndef incl_HPHP_EXT_THRIFT_UTIL_H
#define incl_HPHP_EXT_THRIFT_UTIL_H

#include "hphp/runtime/base/array-init.h"

namespace HPHP { namespace thrift {

enum TError {
  ERR_UNKNOWN = 0,
  ERR_INVALID_DATA = 1,
  ERR_BAD_VERSION = 4
};

[[noreturn]] inline void thrift_error(const String& what, TError why) {
  throw_object(s_TProtocolException, make_vec_array(what, why));
}

inline void set_with_intish_key_cast(
  DArrayInit& arr,
  const Variant& key,
  const Variant& value
) {
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

}}

#endif // incl_HPHP_EXT_THRIFT_UTIL_H
