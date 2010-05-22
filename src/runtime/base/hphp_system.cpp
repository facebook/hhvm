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
#include <runtime/base/hphp_system.h>

namespace HPHP {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

bool Globals::class_exists(const char *name) {
  return false;
}

Variant Globals::getByIdx(ssize_t pos, Variant& k) {
  return getRefByIdx(pos, k);
}

CVarRef Globals::getRefByIdx(ssize_t pos, Variant& k) {
  if (pos < -1) {
    ArrayData *arr = Array::get();
    pos = wrapIter(pos);
    k = arr->getKey(pos);
    return arr->getValueRef(pos);
  }
  ASSERT(false);
  throw FatalErrorException("bad code generation");
}
ssize_t Globals::getIndex(const char* s, int64 prehash) const {
  return Array::get()->getIndex(s, prehash);
}
ssize_t Globals::size() const {
  return staticSize() + Array::size();
}
bool Globals::empty() const {
  return staticSize() == 0 && Array::size() == 0;
}
ssize_t Globals::staticSize() const { return 0; }

ssize_t Globals::iter_begin() const {
  if (empty()) return ArrayData::invalid_index;
  if (staticSize() > 0) {
    return 0;
  }
  return wrapIter(Array::get()->iter_begin());
}
ssize_t Globals::iter_end() const {
  if (empty()) return ArrayData::invalid_index;
  if (Array::size() > 0) {
    return wrapIter(Array::get()->iter_end());
  }
  return staticSize() - 1;
}
ssize_t Globals::iter_advance(ssize_t prev) const {
  ArrayData *arr = Array::get();
  if (prev < -1) {
    return wrapIter(arr->iter_advance(wrapIter(prev)));
  }
  ssize_t next = prev + 1;
  if (next == staticSize()) {
    if (arr) return wrapIter(arr->iter_begin());
    return ArrayData::invalid_index;
  }
  return next;
}
ssize_t Globals::iter_rewind(ssize_t prev) const {
  if (prev < -1) {
    ArrayData *arr = Array::get();
    ASSERT(arr);
    ssize_t next = arr->iter_rewind(wrapIter(prev));
    if (next == ArrayData::invalid_index) {
      if (staticSize() > 0) {
        return staticSize() - 1;
      }
      return ArrayData::invalid_index;
    } else {
      return wrapIter(next);
    }
  }
  ssize_t next = prev - 1;
  if (next < 0) return ArrayData::invalid_index;
  return next;
}

void Globals::getFullPos(FullPos &pos) {
  ArrayData *arr = Array::get();
  arr->getFullPos(pos);
}

bool Globals::setFullPos(const FullPos &pos) {
  ArrayData *arr = Array::get();
  return arr->setFullPos(pos);
}

Array Globals::getDefinedVars() {
  Array ret = Array::Create();
  for (ssize_t iter = iter_begin(); iter != ArrayData::invalid_index;
       iter = iter_advance(iter)) {
    Variant k;
    Variant v = getByIdx(iter, k);
    ret.set(k, v);
  }
  return ret;
}

ssize_t Globals::wrapIter(ssize_t it) const {
  if (it != ArrayData::invalid_index) {
    return -(it+2);
  }
  return ArrayData::invalid_index;
}

///////////////////////////////////////////////////////////////////////////////
}
