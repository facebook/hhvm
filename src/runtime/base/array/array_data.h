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

#ifndef __HPHP_ARRAY_DATA_H__
#define __HPHP_ARRAY_DATA_H__

#include <runtime/base/util/countable.h>
#include <runtime/base/types.h>
#include <runtime/base/macros.h>
#include <climits>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Base class/interface for all types of specialized array data.
 */
class ArrayData : public Countable {
 public:
  enum ArrayOp {
    Plus,
    Merge,
  };

  static const ssize_t invalid_index = -1;

  ArrayData() : m_pos(0) {}
  ArrayData(const ArrayData *src) : m_pos(src->m_pos) {}
  virtual ~ArrayData();

  /**
   * Create a new ArrayData with specified array element(s). If "replace" is
   * true, only unique elements will be inserted (with the last one replacing
   * previous ones with the same name).
   */
  static ArrayData *Create();
  static ArrayData *Create(CVarRef value);
  static ArrayData *Create(CVarRef name, CVarRef value);

  /**
   * Type conversion functions. All other types are handled inside Array class.
   */
  Object toObject() const;

  /**
   * Array interface functions.
   *
   * 1. For functions that return ArrayData pointers, these are the ones that
   *    can potentially escalate into a different ArrayData type. Return NULL
   *    if no escalation is needed.
   *
   * 2. All functions with a "key" parameter are type-specialized.
   */

  /**
   * For SmartAllocator.
   */
  virtual void release() = 0;

  /**
   * Whether this array has any element.
   */
  virtual bool empty() const {
    return size() == 0;
  }

  /**
   * Number of elements this array has.
   */
  virtual ssize_t size() const = 0;

  /**
   * For ArrayIter to work. Get key or value at position "pos". getValueRef()
   * gets a reference to value at that position, and it's only available to
   * VectorVariant and MapVariant types. Generated code made sure array data is
   * in Variant type by calling escalate() beforehand.
   */
  virtual Variant getKey(ssize_t pos) const = 0;
  virtual Variant getValue(ssize_t pos) const = 0;
  virtual void fetchValue(ssize_t pos, Variant & v) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool isVectorData() const;
  virtual bool supportValueRef() const { return false;}

  /**
   * Position-based iterations.
   */
  virtual Variant reset();
  virtual Variant prev();
  virtual Variant current() const;
  virtual Variant next();
  virtual Variant end();
  virtual Variant key() const;
  virtual Variant value(ssize_t &pos) const;
  virtual Variant each();

  /**
   * Testing whether a key exists.
   */
  virtual bool exists(int64   k, int64 prehash = -1) const = 0;
  virtual bool exists(litstr  k, int64 prehash = -1) const = 0;
  virtual bool exists(CStrRef k, int64 prehash = -1) const = 0;
  virtual bool exists(CVarRef k, int64 prehash = -1) const = 0;

  virtual bool idxExists(ssize_t idx) const = 0;

  /**
   * Getting value at specified key.
   */
  virtual Variant get(int64   k, int64 prehash = -1,
                      bool error = false) const = 0;
  virtual Variant get(litstr  k, int64 prehash = -1,
                      bool error = false) const = 0;
  virtual Variant get(CStrRef k, int64 prehash = -1,
                      bool error = false) const = 0;
  virtual Variant get(CVarRef k, int64 prehash = -1,
                      bool error = false) const = 0;

  /**
   * Get the numeric index for a key. Only these need to be
   * in ArrayData.
   */
  virtual ssize_t getIndex(int64 k, int64 prehash = -1) const = 0;
  virtual ssize_t getIndex(litstr k, int64 prehash = -1) const = 0;
  virtual ssize_t getIndex(CStrRef k, int64 prehash = -1) const = 0;
  virtual ssize_t getIndex(CVarRef k, int64 prehash = -1) const = 0;

  /**
   * Getting l-value (that Variant pointer) at specified key. Return NULL if
   * escalation is not needed, or an escalated array data.
   */
  virtual ArrayData *lval(Variant *&ret, bool copy) = 0;
  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false) = 0;
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false) = 0;
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false) = 0;
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          int64 prehash = -1, bool checkExist = false) = 0;

  /**
   * Setting a value at specified key. If "copy" is true, make a copy first
   * then set the value. Return NULL if escalation is not needed, or an
   * escalated array data.
   */
  virtual ArrayData *set(int64   k, CVarRef v,
                         bool copy, int64 prehash = -1) = 0;
  virtual ArrayData *set(litstr  k, CVarRef v,
                         bool copy, int64 prehash = -1) = 0;
  virtual ArrayData *set(CStrRef k, CVarRef v,
                         bool copy, int64 prehash = -1) = 0;
  virtual ArrayData *set(CVarRef k, CVarRef v,
                         bool copy, int64 prehash = -1) = 0;

  /**
   * Remove a value at specified key. If "copy" is true, make a copy first
   * then remove the value. Return NULL if escalation is not needed, or an
   * escalated array data.
   */
  virtual ArrayData *remove(int64   k, bool copy, int64 prehash = -1) = 0;
  virtual ArrayData *remove(litstr  k, bool copy, int64 prehash = -1) = 0;
  virtual ArrayData *remove(CStrRef k, bool copy, int64 prehash = -1) = 0;
  virtual ArrayData *remove(CVarRef k, bool copy, int64 prehash = -1) = 0;

  virtual ssize_t iter_begin() const;
  virtual ssize_t iter_end() const;
  virtual ssize_t iter_advance(ssize_t prev) const;
  virtual ssize_t iter_rewind(ssize_t prev) const;

  virtual void getFullPos(FullPos &pos);
  virtual bool setFullPos(const FullPos &pos);
  virtual CVarRef currentRef();
  virtual CVarRef endRef();

  /**
   * Make a copy of myself.
   */
  virtual ArrayData *copy() const = 0;

  /**
   * Append a value to the array. If "copy" is true, make a copy first
   * then append the value. Return NULL if escalation is not needed, or an
   * escalated array data.
   */
  virtual ArrayData *append(CVarRef v, bool copy) = 0;

  /**
   * Implementing array appending and merging. If "copy" is true, make a copy
   * first then append/merge arrays. Return NULL if escalation is not needed,
   * or an escalated array data.
   */
  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy) = 0;

  /**
   * Stack function: pop the last item and return it.
   */
  virtual ArrayData *pop(Variant &value);

  /**
   * Queue function: remove the 1st item and return it.
   */
  virtual ArrayData *dequeue(Variant &value);

  /**
   * Array function: prepend a new item.
   */
  virtual ArrayData *prepend(CVarRef v, bool copy) = 0;

  /**
   * Only map classes need this. Re-index all numeric keys to start from 0.
   */
  virtual void renumber() {}

  /**
   * When an array data is set static, some calculated data members need to
   * be initialized, for example, Map::getKeyVector(). More importantly, all
   * sub elements will have to setStatic().
   */
  virtual void onSetStatic() { ASSERT(false);}

  /**
   * Serialize this array. We could have made this virtual function to ask
   * sub-classes to implement it specifically, but since this is not a critical
   * function to optimize, we implement it in a generic way in this base class.
   * Then all the sudden we find out all Zend HashTable functions are similar
   * to implementing array functions in this base class than utilizing a type
   * specialized implementation, which is normally more optimized.
   */
  void serialize(VariableSerializer *serializer) const;

  virtual void dump();
  virtual void dump(std::string &out);

  /**
   * Comparisons. Similar to serialize(), we implemented it here generically.
   */
  int compare(const ArrayData *v2, bool strict) const;

  virtual void setPosition(ssize_t p) { m_pos = p; }

  virtual ArrayData *escalate(bool mutableIteration = false) const {
    return const_cast<ArrayData *>(this);
  }

 protected:
  ssize_t m_pos;

  /**
   * Helpers.
   */
  static void dumpKey(std::ostream &out, int indent, unsigned int index);
  static void dumpKey(std::ostream &out, int indent, CStrRef key);

#ifdef FAST_REFCOUNT_FOR_VARIANT
 private:
  static void compileTimeAssertions() {
    CT_ASSERT(offsetof(ArrayData, _count) == FAST_REFCOUNT_OFFSET);
  }
#endif
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ARRAY_DATA_H__
