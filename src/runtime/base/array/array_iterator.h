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

#ifndef __HPHP_ARRAY_ITERATOR_H__
#define __HPHP_ARRAY_ITERATOR_H__

#include <runtime/base/types.h>
#include <runtime/base/util/smart_ptr.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * An iteration normally looks like this:
 *
 *   for (ArrayIter iter(data); iter; ++iter) {
 *     ...
 *   }
 */
class IArrayIterator : public Countable {
public:
  virtual ~IArrayIterator() {}
  void release() { delete this;}

  operator bool() { return !end();}
  void operator++() { next();}

  virtual bool end() = 0;
  virtual void next() = 0;

  /**
   * Getting key, value or l-value at current position.
   */
  virtual Variant first() = 0;
  virtual Variant second() = 0;
  virtual void second(Variant & v) = 0;
  virtual CVarRef secondRef() = 0;
};
typedef SmartPtr<IArrayIterator> ArrayIterPtr;

/**
 * Iterator for an immutable array.
 */
class ArrayIter : public IArrayIterator {
public:
  /**
   * Constructors.
   */
  ArrayIter(const ArrayData *data);
  ArrayIter(const ArrayIter &iter);
  ArrayIter(CArrRef array);
  ~ArrayIter();

  bool end();
  void next();
  Variant first();
  Variant second();
  void second(Variant & v);
  CVarRef secondRef();

private:
  const ArrayData *m_data;
  ssize_t m_pos;

  void create();
};

///////////////////////////////////////////////////////////////////////////////

struct FullPos {
  ssize_t primary;
  ssize_t secondary;
  FullPos() : primary(0), secondary(0) {}
};

/**
 * Iterator for "foreach ($arr => &$v)" or "foreach ($array as $n => &$v)".
 * In this case, any changes to $arr inside iteration needs to be visible to
 * the iteration. Therefore, we need to store Variant* with the iterator to
 * see those changes. This class should only be used for generated code.
 */
class MutableArrayIter : public Countable {
public:
  MutableArrayIter(const Variant *var, Variant *key, Variant &val);
  MutableArrayIter(ArrayData *data, Variant *key, Variant &val);
  ~MutableArrayIter();
  void release() { delete this;}
  bool advance();

private:
  const Variant *m_var;
  ArrayData *m_data;
  Variant *m_key;
  Variant &m_val;
  FullPos m_pos;
  int size();
  ArrayData *getData();
};
typedef SmartPtr<MutableArrayIter> MutableArrayIterPtr;

///////////////////////////////////////////////////////////////////////////////

/**
 * Iterator for "iterator" class of objects.
 */
class ObjectArrayIter : public IArrayIterator {
public:
  ObjectArrayIter(ObjectData *obj, Variant *iterator = NULL);
  ~ObjectArrayIter();

  // implementing IArrayIterator
  bool end();
  void next();
  Variant first();
  Variant second();
  void second(Variant & v);
  CVarRef secondRef();

private:
  ObjectData *m_obj;
  Variant *m_iterator;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARRAY_ITERATOR_H__
