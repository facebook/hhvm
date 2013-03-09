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
#ifndef incl_RUNTIME_VM_NAME_VALUE_TABLE_WRAPPER_H
#define incl_RUNTIME_VM_NAME_VALUE_TABLE_WRAPPER_H

#include "runtime/vm/name_value_table.h"
#include "runtime/base/array/array_data.h"

namespace HPHP { namespace VM {

//////////////////////////////////////////////////////////////////////

/*
 * Wrapper to provide a KindOfArray interface to a NameValueTable.
 * This is used to expose a NameValueTable to php code, particularly
 * for $GLOBALS, but also for some internal generated-bytecode
 * initialization routines (86pinit, 86sinit).
 *
 * Some differences compared to normal php arrays:
 *
 *   - Does not behave as if it has value semantics.  (I.e., no COW.)
 *
 *   - Iteration order is not specified.
 *
 *   - Non-string keys are not really supported.  (Integers are
 *     converted to strings.)
 *
 *   - size() is an O(N) operation.  (This is because of KindOfIndirect
 *     support in the underlying NameValueTable.)
 *
 *   - Append/prepend operations are not supported.
 *
 *   - Strong iterators "past the end" are not updated when new
 *     elements are added.  (Since iteration order is unspecified,
 *     this semantic would seem weird anyway.)
 *
 * This holds a pointer to a NameValueTable whose lifetime must be
 * guaranteed to outlast the lifetime of the NameValueTableWrapper.
 * (The wrapper is refcounted, as required by ArrayData, but the table
 * pointed to is not.)
 */
struct NameValueTableWrapper : public ArrayData {
  explicit NameValueTableWrapper(NameValueTable* tab)
    : m_tab(tab)
  {}

public: // ArrayData implementation

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

  virtual void release() {}
  virtual ssize_t vsize() const;
  virtual Variant getKey(ssize_t pos) const;
  virtual Variant getValue(ssize_t pos) const;
  virtual CVarRef getValueRef(ssize_t pos) const;
  virtual bool noCopyOnWrite() const;

  virtual bool exists(int64_t k) const;
  virtual bool exists(const StringData* k) const;
  virtual bool idxExists(ssize_t idx) const;

  virtual CVarRef get(int64_t k, bool error = false) const;
  virtual CVarRef get(const StringData* k, bool error = false) const;

  virtual TypedValue* nvGet(int64_t k) const;
  virtual TypedValue* nvGet(const StringData* k) const;

  virtual ssize_t getIndex(int64_t k) const;
  virtual ssize_t getIndex(const StringData* k) const;

  virtual ArrayData* lval(int64_t k, Variant*& ret, bool copy,
                          bool checkExist = false);
  virtual ArrayData* lval(StringData* k, Variant*& ret,
                          bool copy, bool checkExist = false);
  virtual ArrayData* lvalNew(Variant*& ret, bool copy);

  virtual ArrayData* set(int64_t k, CVarRef v, bool copy);
  virtual ArrayData* set(StringData* k, CVarRef v, bool copy);
  virtual ArrayData* setRef(int64_t k, CVarRef v, bool copy);
  virtual ArrayData* setRef(StringData* k, CVarRef v, bool copy);
  virtual ArrayData* remove(int64_t k, bool copy);
  virtual ArrayData* remove(const StringData* k, bool copy);

  virtual ArrayData* copy() const { return 0; }

  virtual ArrayData* append(CVarRef v, bool copy);
  virtual ArrayData* appendRef(CVarRef v, bool copy);
  virtual ArrayData* appendWithRef(CVarRef v, bool copy);

  virtual ArrayData* append(const ArrayData* elems, ArrayOp op, bool copy);

  virtual ArrayData* prepend(CVarRef v, bool copy);

  virtual ssize_t iter_begin() const;
  virtual ssize_t iter_end() const;
  virtual ssize_t iter_advance(ssize_t prev) const;
  virtual ssize_t iter_rewind(ssize_t prev) const;

  virtual Variant reset();
  virtual Variant prev();
  virtual Variant current() const;
  virtual CVarRef currentRef();
  virtual Variant next();
  virtual Variant end();
  virtual CVarRef endRef();
  virtual Variant key() const;
  virtual Variant value(ssize_t &pos) const;
  virtual Variant each();
  virtual bool validFullPos(const FullPos & fp) const;
  virtual void getFullPos(FullPos&);
  virtual bool setFullPos(const FullPos&);

  virtual ArrayData* escalateForSort();
  virtual void ksort(int sort_flags, bool ascending);
  virtual void sort(int sort_flags, bool ascending);
  virtual void asort(int sort_flags, bool ascending);
  virtual void uksort(CVarRef cmp_function);
  virtual void usort(CVarRef cmp_function);
  virtual void uasort(CVarRef cmp_function);

private:
  NameValueTable* const m_tab;
};

class GlobalNameValueTableWrapper : public NameValueTableWrapper {
 public:
  GlobalNameValueTableWrapper(NameValueTable* tab);
  Variant gvm_argc;
  Variant gvm_argv;
  Variant gvm__SERVER;
  Variant gvm__GET;
  Variant gvm__POST;
  Variant gvm__COOKIE;
  Variant gvm__FILES;
  Variant gvm__ENV;
  Variant gvm__REQUEST;
  Variant gvm__SESSION;
  Variant gvm_HTTP_RAW_POST_DATA;
  Variant gvm_http_response_header;

  Variant __realPropProxy;

  Variant stgv_Variant[1];
  #define k_SID stgv_Variant[0]
};

//////////////////////////////////////////////////////////////////////

}}

#endif
