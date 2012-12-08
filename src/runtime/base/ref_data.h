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

#ifndef __INSIDE_HPHP_COMPLEX_TYPES_H__
#error Directly including 'ref_data.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#ifndef __HPHP_REF_DATA_H
#define __HPHP_REF_DATA_H

namespace HPHP {

/**
 * We heap allocate a RefData when we make a reference to something.
 * A Variant or TypedValue can be KindOfRef and point to a RefData,
 * but the value held here must not be KindOfRef.
 */
class RefData {
public:
  RefData() {}
  RefData(DataType t, int64_t datum) { init(t, datum); }
  ~RefData();

  // Don't extend Countable but use these methods to directly
  // update _count, declared below.
  IMPLEMENT_COUNTABLE_METHODS_NO_STATIC

  // Memory allocator methods
  DECLARE_SMART_ALLOCATION(RefData);
  void dump() const;

  const TypedValue* tv() const { return &m_tv; }
  TypedValue* tv() { return &m_tv; }
  const Variant* var() const { return (const Variant*)&m_tv; }
  Variant* var() { return (Variant*)&m_tv; }


private:
  // initialize this value by laundering uninitNull -> Null
  void init(DataType t, int64_t datum) {
    _count = 1;
    if (!IS_NULL_TYPE(t)) {
      m_tv.m_type = t;
      m_tv.m_data.num = datum;
    } else {
      m_tv.m_type = KindOfNull;
    }
  }
private:
  union {
    // overlap TypedValue with an explicit struct to expose the _count
    // field to the macro-expanded refcount methods above.
    struct { void* ptr; mutable int32_t _count; int32_t type; };
    TypedValue m_tv;
  };
};

ALWAYS_INLINE inline void decRefRef(RefData* ref) {
  if (ref->decRefCount() == 0) ref->release();
}

} // namespace HPHP

#endif //__HPHP_REF_DATA_H
