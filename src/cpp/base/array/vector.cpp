/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <cpp/base/array/vector.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant Vector::getKey(ssize_t pos) const {
  ASSERT(pos != ArrayData::invalid_index && pos < size());
  return (int64)pos;
}


Variant Vector::get(int64 k, int64 prehash /* = -1 */) const {
  return getImpl(getIndex(k));
}

Variant Vector::get(litstr k, int64 prehash /* = -1 */) const {
  return getImpl(getIndex(k));
}

Variant Vector::get(CStrRef k, int64 prehash /* = -1 */) const {
  return getImpl(getIndex(k));
}

Variant Vector::get(CVarRef k, int64 prehash /* = -1 */) const {
  return getImpl(getIndex(k));
}


bool Vector::exists(int64 k, int64 prehash /* = -1 */) const {
  return getIndex(k) >= 0;
}

bool Vector::exists(litstr k, int64 prehash /* = -1 */) const {
  return getIndex(k) >= 0;
}

bool Vector::exists(CStrRef k, int64 prehash /* = -1 */) const {
  return getIndex(k) >= 0;
}

bool Vector::exists(CVarRef k, int64 prehash /* = -1 */) const {
  return getIndex(k) >= 0;
}

bool Vector::idxExists(ssize_t idx) const {
  return idx != ArrayData::invalid_index;
}

ssize_t Vector::getIndex(int64 k, bool expanding) const {
  if (k >= 0 && (ssize_t)k <= size() - (expanding ? 0 : 1)) {
    return k;
  }
  return ArrayData::invalid_index;
}

ssize_t Vector::getIndex(litstr k, bool expanding) const {
  return ArrayData::invalid_index;
}

ssize_t Vector::getIndex(CStrRef k, bool expanding) const {
  return ArrayData::invalid_index;
}

ssize_t Vector::getIndex(CVarRef k, bool expanding) const {
  if (k.isNumeric()) {
    int64 index = k.toInt64();
    if (index >= 0 && (ssize_t)index <= size() - (expanding ? 0 : 1)) {
      return index;
    }
  }
  return ArrayData::invalid_index;
}

ssize_t Vector::getIndex(int64 k, int64 prehash /* = -1 */) const {
  return getIndex(k, false);
}

ssize_t Vector::getIndex(litstr k, int64 prehash /* = -1 */) const {
  return getIndex(k, false);
}

ssize_t Vector::getIndex(CStrRef k, int64 prehash /* = -1 */) const {
  return getIndex(k, false);
}

ssize_t Vector::getIndex(CVarRef k, int64 prehash /* = -1 */) const {
  return getIndex(k, false);
}

///////////////////////////////////////////////////////////////////////////////
}
