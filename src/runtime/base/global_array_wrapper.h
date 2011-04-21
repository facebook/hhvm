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

  virtual void release();
  virtual ssize_t size() const;
  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool isGlobalArrayWrapper() const;

  virtual bool exists(int64   k) const;
  virtual bool exists(litstr  k) const;
  virtual bool exists(CStrRef k) const;
  virtual bool exists(CVarRef k) const;
  virtual bool idxExists(ssize_t idx) const;

  virtual CVarRef get(int64   k, bool error = false) const;
  virtual CVarRef get(litstr  k, bool error = false) const;
  virtual CVarRef get(CStrRef k, bool error = false) const;
  virtual CVarRef get(CVarRef k, bool error = false) const;

  virtual void load(CVarRef k, Variant &v) const;

  virtual ssize_t getIndex(int64 k) const;
  virtual ssize_t getIndex(litstr k) const;
  virtual ssize_t getIndex(CStrRef k) const;
  virtual ssize_t getIndex(CVarRef k) const;

  virtual ArrayData *lval(int64   k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(litstr  k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(CVarRef k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lval(CStrRef k, Variant *&ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData *lvalNew(Variant *&ret, bool copy);

  virtual ArrayData *set(int64   k, CVarRef v, bool copy);
  virtual ArrayData *set(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *set(CVarRef k, CVarRef v, bool copy);
  virtual ArrayData *setRef(int64   k, CVarRef v, bool copy);
  virtual ArrayData *setRef(CStrRef k, CVarRef v, bool copy);
  virtual ArrayData *setRef(CVarRef k, CVarRef v, bool copy);
  virtual ArrayData *remove(int64   k, bool copy);
  virtual ArrayData *remove(CStrRef k, bool copy);
  virtual ArrayData *remove(CVarRef k, bool copy);

  virtual ArrayData *copy() const;

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

  virtual bool isHead() const;
  virtual bool isTail() const;
  virtual bool isInvalid() const;

  virtual void getFullPos(FullPos &fp);
  virtual bool setFullPos(const FullPos &fp);

  virtual CVarRef currentRef();
  virtual CVarRef endRef();

private:
  Globals* m_globals;
};

///////////////////////////////////////////////////////////////////////////////

}
#endif
