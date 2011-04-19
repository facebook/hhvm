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

#ifndef __HPHP_ARRAY_ITERATOR_H__
#define __HPHP_ARRAY_ITERATOR_H__

#include <runtime/base/types.h>
#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * An iteration normally looks like this:
 *
 *   for (ArrayIter iter(data); iter; ++iter) {
 *     ...
 *   }
 */

/**
 * Iterator for an immutable array.
 */
class ArrayIter {
public:
  /**
   * Constructors.
   */
  ArrayIter();
  ArrayIter(const ArrayData *data);
  ArrayIter(CArrRef array);
  ArrayIter(ObjectData *obj, bool rewind = true);
  ~ArrayIter();

  operator bool() { return !end(); }
  void operator++() { next(); }

  bool end() {
    if (!m_obj) {
      return m_pos == ArrayData::invalid_index;
    }
    return endHelper();
  }
  void next() {
    if (!m_obj) {
      ASSERT(m_data);
      ASSERT(m_pos != ArrayData::invalid_index);
      m_pos = m_data->iter_advance(m_pos);
      return;
    }
    return nextHelper();
  }
  Variant first() {
    if (!m_obj) {
      ASSERT(m_data);
      ASSERT(m_pos != ArrayData::invalid_index);
      return m_data->getKey(m_pos);
    }
    return firstHelper();
  }
  Variant second();
  void second(Variant &v) {
    if (!m_obj) {
      ASSERT(m_data);
      ASSERT(m_pos != ArrayData::invalid_index);
      v = m_data->getValueRef(m_pos);
      return;
    }
    secondHelper(v);
  }
  CVarRef secondRef();

private:
  const ArrayData *m_data;
  ObjectData *m_obj;
  ssize_t m_pos;

  bool endHelper();
  void nextHelper();
  Variant firstHelper();
  void secondHelper(Variant &v);
};

///////////////////////////////////////////////////////////////////////////////

struct FullPos {
  ssize_t pos;
  ArrayData * container;
  FullPos() : pos(0), container(NULL) {}
};

/**
 * Iterator for "foreach ($arr as &$v)" or "foreach ($array as $n => &$v)".
 * In this case, any changes to $arr inside iteration needs to be visible to
 * the iteration. Therefore, we need to store Variant* with the iterator to
 * see those changes. This class should only be used for generated code.
 */
class MutableArrayIter {
public:
  MutableArrayIter(const Variant* var, Variant* key, Variant& val);
  MutableArrayIter(ArrayData* data, Variant* key, Variant& val);
  ~MutableArrayIter();
  void release() { delete this;}
  bool advance();

private:
  const Variant* m_var;
  ArrayData* m_data;
  Variant* m_key;
  Variant& m_val;
  FullPos m_fp;
  int size();
  ArrayData* getData();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARRAY_ITERATOR_H__
