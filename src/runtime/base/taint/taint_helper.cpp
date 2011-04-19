/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifdef TAINTED

#include <runtime/base/types.h>
#include <runtime/base/array/array_iterator.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/taint/taint_data.h>
#include <runtime/base/taint/taint_helper.h>

namespace HPHP {

void taint_warn_if_tainted(CStrRef s, const bitstring bit) {
  if (s.get()->getTaintDataRef().getTaint() & bit) {
    std::string buf = "using a tainted string !!!\n";
    if (s.get()->getTaintDataRef().getOriginalStr()) {
      buf += "original string: ";
      buf += s.get()->getTaintDataRef().getOriginalStr();
    } else {
      buf += "first 100 bytes: ";
      buf += s.substr(0, 100).data();
    }
    buf += "\n\n";
    raise_warning(buf);
  }
}

void taint_array_variant(Variant &v, const std::string original_str) {
  if (v.isString()) {
    std::string s = "";
    s += v.toString().c_str();
    s += " from ";
    s += original_str;

    v.asStrRef().get()->getTaintData()->setTaint(TAINT_BIT_ALL, s.c_str());
  }

  if (v.isArray()) {
    Array a = v.toArray();
    for(ArrayIter iter(a); iter; ++iter) {
      Variant key = iter.first();

      if (!key.isString()) {
        // if the URI is /foo.php?123=hello, 123 will be an int, not a string
        // so just skip it.
        continue;
      }

      ASSERT(key.isString());
      std::string s = original_str + "[" + key.toString().c_str() + "]";
      taint_array_variant(key, s);

      Variant value = iter.second();
      taint_array_variant(value, s);
    }
  }
}

}

#endif // TAINTED
