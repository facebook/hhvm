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

#ifndef __HPHP_GLOBAL_ARRAY_WRAPPER_H__
#define __HPHP_GLOBAL_ARRAY_WRAPPER_H__

#include <runtime/base/array/array_data.h>
#include <runtime/base/hphp_system.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class GlobalArrayWrapper : public ArrayData {
public:
  GlobalArrayWrapper() {
    m_globals = (Globals*)get_global_variables();
  }

  // these using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions
  // from a CVarRef key to int64.
  using ArrayData::exists;
  using ArrayData::get;
  using ArrayData::getIndex;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::lvalPtr;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::addLval;
  using ArrayData::remove;

  virtual void release();
  virtual ssize_t vsize() const;
  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool noCopyOnWrite() const;

  virtual bool exists(int64 k) const;
  virtual bool exists(const StringData* k) const;

  virtual CVarRef get(int64 k, bool error = false) const;
  virtual CVarRef get(const StringData* k, bool error = false) const;

  virtual ssize_t getIndex(int64 k) const;
  virtual ssize_t getIndex(const StringData* k) const;

  virtual ArrayData *lval(int64 k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(StringData* k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lvalNew(Variant *&ret, bool copy);

  virtual ArrayData *set(int64 k, CVarRef v, bool copy);
  virtual ArrayData *set(StringData* k, CVarRef v, bool copy);
  virtual ArrayData *setRef(int64 k, CVarRef v, bool copy);
  virtual ArrayData *setRef(StringData* k, CVarRef v, bool copy);
  virtual ArrayData *remove(int64 k, bool copy);
  virtual ArrayData *remove(const StringData* k, bool copy);

  virtual ArrayData *copy() const;
  virtual ArrayData *nonSmartCopy() const { not_reached(); }

  virtual ArrayData *append(CVarRef v, bool copy);
  virtual ArrayData *appendRef(CVarRef v, bool copy);
  virtual ArrayData *appendWithRef(CVarRef v, bool copy);

  virtual ArrayData *append(const ArrayData *elems, ArrayOp op, bool copy);

  virtual ArrayData *pop(Variant &value);

  virtual ArrayData *dequeue(Variant &value);

  virtual ArrayData *prepend(CVarRef v, bool copy);

  ssize_t iter_begin() const;
  ssize_t iter_end() const;
  ssize_t iter_advance(ssize_t prev) const;
  ssize_t iter_rewind(ssize_t prev) const;

  virtual Variant reset();
  virtual Variant prev();
  virtual Variant current() const;
  virtual Variant next();
  virtual Variant end();
  virtual Variant key() const;
  virtual Variant value(ssize_t &pos) const;
  virtual Variant each();

  virtual void getFullPos(FullPos &fp);
  virtual bool setFullPos(const FullPos &fp);

  virtual CVarRef currentRef();
  virtual CVarRef endRef();

  virtual ArrayData* escalateForSort();
  virtual void ksort(int sort_flags, bool ascending);
  virtual void sort(int sort_flags, bool ascending);
  virtual void asort(int sort_flags, bool ascending);
  virtual void uksort(CVarRef cmp_function);
  virtual void usort(CVarRef cmp_function);
  virtual void uasort(CVarRef cmp_function);

private:
  Globals* m_globals;
};

///////////////////////////////////////////////////////////////////////////////

}
#endif
