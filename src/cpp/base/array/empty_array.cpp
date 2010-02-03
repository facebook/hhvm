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

#include <cpp/base/array/empty_array.h>
#include <cpp/base/array/map_long.h>
#include <cpp/base/array/map_string.h>
#include <cpp/base/array/map_variant.h>
#include <cpp/base/array/array_element.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/types.h>

namespace HPHP {

IMPLEMENT_SMART_ALLOCATION_NOCALLBACKS(EmptyArray);
///////////////////////////////////////////////////////////////////////////////

StaticEmptyArray StaticEmptyArray::s_theEmptyArray;

bool EmptyArray::empty() const {
  return true;
}

ssize_t EmptyArray::size() const {
  return 0;
}

Variant EmptyArray::getKey(ssize_t pos) const {
  ASSERT(false);
  return null;
}

Variant EmptyArray::getValue(ssize_t pos) const {
  ASSERT(false);
  return null;
}

bool EmptyArray::exists(int64 k, int64 prehash /* = -1 */) const {
  return false;
}

bool EmptyArray::exists(litstr k, int64 prehash /* = -1 */) const {
  return false;
}

bool EmptyArray::exists(CStrRef k, int64 prehash /* = -1 */) const {
  return false;
}

bool EmptyArray::exists(CVarRef k, int64 prehash /* = -1 */) const {
  return false;
}

bool EmptyArray::idxExists(ssize_t idx) const {
  return false;
}

Variant EmptyArray::get(int64 k, int64 prehash /* = -1 */) const {
  return null;
}

Variant EmptyArray::get(litstr k, int64 prehash /* = -1 */) const {
  return null;
}

Variant EmptyArray::get(CStrRef k, int64 prehash /* = -1 */) const {
  return null;
}

Variant EmptyArray::get(CVarRef k, int64 prehash /* = -1 */) const {
  return null;
}

ssize_t EmptyArray::getIndex(int64 k, int64 prehash /* = -1 */) const {
  return -1;
}

ssize_t EmptyArray::getIndex(litstr k, int64 prehash /* = -1 */) const {
  return -1;
}

ssize_t EmptyArray::getIndex(CStrRef k, int64 prehash /* = -1 */) const {
  return -1;
}

ssize_t EmptyArray::getIndex(CVarRef k, int64 prehash /* = -1 */) const {
  return -1;
}

ArrayData *EmptyArray::lval(Variant *&ret, bool copy) {
  ASSERT(false);
  ret = NULL;
  return NULL;
}

ArrayData *EmptyArray::lval(int64 k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  return lvalImpl(k, ret, copy, prehash);
}

ArrayData *EmptyArray::lval(litstr k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  return lvalImpl(k, ret, copy, prehash);
}

ArrayData *EmptyArray::lval(CStrRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  return lvalImpl(k, ret, copy, prehash);
}

ArrayData *EmptyArray::lval(CVarRef k, Variant *&ret, bool copy,
                            int64 prehash /* = -1 */) {
  return lvalImpl(k, ret, copy, prehash);
}

ArrayData *EmptyArray::set(int64 k, CVarRef v,
                           bool copy, int64 prehash /* = -1 */) {
  return ArrayData::Create(k, v);
}

ArrayData *EmptyArray::set(litstr k, CVarRef v,
                           bool copy, int64 prehash /* = -1 */) {
  return ArrayData::Create(k, v);
}

ArrayData *EmptyArray::set(CStrRef k, CVarRef v,
                           bool copy, int64 prehash /* = -1 */) {
  return ArrayData::Create(k, v);
}

ArrayData *EmptyArray::set(CVarRef k, CVarRef v,
                           bool copy, int64 prehash /* = -1 */) {
  if (k.isInteger()) {
    return ArrayData::Create(k.toInt64(), v);
  }
  return ArrayData::Create(k, v);
}

ArrayData *EmptyArray::remove(int64 k, bool copy, int64 prehash /* = -1 */) {
  return NULL;
}

ArrayData *EmptyArray::remove(litstr k, bool copy, int64 prehash /* = -1 */) {
  return NULL;
}

ArrayData *EmptyArray::remove(CStrRef k, bool copy, int64 prehash /* = -1 */) {
  return NULL;
}

ArrayData *EmptyArray::remove(CVarRef k, bool copy, int64 prehash /* = -1 */) {
  return NULL;
}

ArrayData *EmptyArray::copy() const {
  return NEW(EmptyArray)();
}

ArrayData *EmptyArray::append(CVarRef v, bool copy) {
  return ArrayData::Create(v);
}

ArrayData *EmptyArray::append(const ArrayData *elems, ArrayOp op, bool copy) {
  switch (op) {
  case Plus:
    break;
  case Merge:
    {
      const Map *mapElems = dynamic_cast<const Map *>(elems);
      if (mapElems && mapElems->hasNumericKeys()) {
        return elems->copy();
      }
    }
  default:
    ASSERT(false);
    break;
  }
  return const_cast<ArrayData *>(elems);
}

ArrayData *EmptyArray::insert(ssize_t pos, CVarRef v, bool copy) {
  return append(v, false);
}

///////////////////////////////////////////////////////////////////////////////
}
