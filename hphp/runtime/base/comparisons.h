/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/opaque-resource.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// Variant

#define DELETE_WITH_LHS_VARIANT(name, rhs) \
bool name(const Variant&, rhs) = delete;

DELETE_WITH_LHS_VARIANT(same, int);
DELETE_WITH_LHS_VARIANT(same, const char*)
DELETE_WITH_LHS_VARIANT(same, const String&)
DELETE_WITH_LHS_VARIANT(same, const Array&)
DELETE_WITH_LHS_VARIANT(same, const Object&)
DELETE_WITH_LHS_VARIANT(same, const ObjectData*)
DELETE_WITH_LHS_VARIANT(same, const Resource&)

DELETE_WITH_LHS_VARIANT(equal, int)
DELETE_WITH_LHS_VARIANT(equal, const String&)
DELETE_WITH_LHS_VARIANT(equal, const char*)
DELETE_WITH_LHS_VARIANT(equal, const Array&)
DELETE_WITH_LHS_VARIANT(equal, const Object&)
DELETE_WITH_LHS_VARIANT(equal, const Resource&)

DELETE_WITH_LHS_VARIANT(less, bool)
DELETE_WITH_LHS_VARIANT(less, int)
DELETE_WITH_LHS_VARIANT(less, const StringData*)
DELETE_WITH_LHS_VARIANT(less, const ECLString&)
DELETE_WITH_LHS_VARIANT(less, const String&)
DELETE_WITH_LHS_VARIANT(less, const char*)
DELETE_WITH_LHS_VARIANT(less, const Array&)
DELETE_WITH_LHS_VARIANT(less, const Object&)
DELETE_WITH_LHS_VARIANT(less, const Resource&)

DELETE_WITH_LHS_VARIANT(more, bool)
DELETE_WITH_LHS_VARIANT(more, int)
DELETE_WITH_LHS_VARIANT(more, const StringData*)
DELETE_WITH_LHS_VARIANT(more, const ECLString&)
DELETE_WITH_LHS_VARIANT(more, const String&)
DELETE_WITH_LHS_VARIANT(more, const char*)
DELETE_WITH_LHS_VARIANT(more, const Array&)
DELETE_WITH_LHS_VARIANT(more, const Object&)
DELETE_WITH_LHS_VARIANT(more, const Resource&)

#undef DELETE_WITH_LHS_VARIANT

inline bool same(const Variant& v1, bool v2) {
  return v1.isBoolean() && v2 == v1.getBoolean();
}
inline bool same(const Variant& v1, int64_t v2) {
  return v1.isInteger() && v2 == v1.asInt64Val();
}
inline bool same(const Variant& v1, double d) {
  return v1.isDouble() && v1.getDouble() == d;
}
bool same(const Variant& v1, const StringData* v2);
inline bool same(const Variant& v1, const Variant& v2) {
  return tvSame(*v1.asTypedValue(), *v2.asTypedValue());
}

inline bool equal(const Variant& v1, bool v2) {
  return tvEqual(*v1.asTypedValue(), v2);
}
inline bool equal(const Variant& v1, int64_t v2) {
  return tvEqual(*v1.asTypedValue(), v2);
}
inline bool equal(const Variant& v1, double  v2) {
  return tvEqual(*v1.asTypedValue(), v2);
}
inline bool equal(const Variant& v1, const StringData* v2) {
  return tvEqual(*v1.asTypedValue(), v2);
}
inline bool equal(const Variant& v1, const Variant& v2) {
  return tvEqual(*v1.asTypedValue(), *v2.asTypedValue());
}

inline bool less(const Variant& v1, int64_t v2) {
  return tvLess(*v1.asTypedValue(), v2);
}
inline bool less(const Variant& v1, double v2) {
  return tvLess(*v1.asTypedValue(), v2);
}
inline bool less(const Variant& v1, const Variant& v2) {
  return tvLess(*v1.asTypedValue(), *v2.asTypedValue());
}

inline bool more(const Variant& v1, int64_t v2) {
  return tvGreater(*v1.asTypedValue(), v2);
}
inline bool more(const Variant& v1, double v2) {
  return tvGreater(*v1.asTypedValue(), v2);
}
inline bool more(const Variant& v1, const Variant& v2) {
  return tvGreater(*v1.asTypedValue(), *v2.asTypedValue());
}

inline int64_t compare(const Variant& v1, const Variant& v2) {
  return tvCompare(*v1.asTypedValue(), *v2.asTypedValue());
}

///////////////////////////////////////////////////////////////////////////////

// magic delete all versions that both don't have Variant on the LHS
// and aren't comparing the same effective type (minus const etc)
#define DELETE_IF_NOT_LHS_VARIANT(name) \
template<class T, class U, class Ret> \
std::enable_if_t< \
  !std::is_same_v<std::remove_const_t<std::remove_reference_t<T>>, Variant> && \
  !std::is_same_v< \
    std::remove_const_t<std::remove_reference_t<std::remove_pointer_t<T>>>, \
    std::remove_const_t<std::remove_reference_t<std::remove_pointer_t<U>>>  \
  >, \
  Ret \
> name(T, U) = delete;

DELETE_IF_NOT_LHS_VARIANT(same);
DELETE_IF_NOT_LHS_VARIANT(equal);
DELETE_IF_NOT_LHS_VARIANT(less);
DELETE_IF_NOT_LHS_VARIANT(more);
DELETE_IF_NOT_LHS_VARIANT(compare);

#undef DELETE_IF_NOT_LHS_VARIANT


///////////////////////////////////////////////////////////////////////////////
// bool

inline bool same(bool v1, bool    v2) = delete;
inline bool equal(bool v1, bool    v2) { return v1 == v2; }
inline bool less(bool v1, bool    v2) { return (v1 ? 1 : 0) < (v2 ? 1 : 0); }
inline bool more(bool v1, bool    v2) { return (v1 ? 1 : 0) > (v2 ? 1 : 0); }
inline int64_t compare(bool v1, bool v2) { return v1 - v2; }

///////////////////////////////////////////////////////////////////////////////
// int64

inline bool same(int64_t v1, int64_t v2) = delete;
inline bool equal(int64_t v1, int64_t v2) { return v1 == v2; }
inline bool less(int64_t v1, int64_t v2) { return v1 < v2; }
inline bool less(int64_t v1, double  v2) { return v1 < v2; }
inline bool more(int64_t v1, int64_t v2) { return v1 > v2; }
inline bool more(int64_t v1, double  v2) { return v1 > v2; }
inline int64_t compare(int64_t v1, int64_t v2) {
  return (v1 < v2) ? -1 : ((v1 > v2) ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////
// double

inline bool same(double v1, double v2) = delete;
inline bool equal(double v1, double  v2) { return v1 == v2; }
inline bool less(double v1, int64_t v2) { return v1 < v2; }
inline bool less(double v1, double  v2) { return v1 < v2; }
inline bool more(double v1, int64_t v2) { return v1 > v2; }
inline bool more(double v1, double  v2) { return v1 > v2; }

inline int64_t compare(double v1, double v2) {
  // This ordering is required so that -1 is returned for NaNs (to match PHP7
  // behavior).
  return (v1 == v2) ? 0 : ((v1 > v2) ? 1 : -1);
}

///////////////////////////////////////////////////////////////////////////////
// StringData *

inline bool same(const StringData *v1, const StringData *v2) {
  if (v1 == v2) return true;
  if (v1 && v2) return v1->same(v2);
  return false;
}

inline bool nsame(const StringData* v1, const StringData* v2) {
  return !same(v1, v2);
}

inline bool equal(const StringData *v1, const StringData *v2) {
  if (v1 == v2) return true;
  if (v1 == nullptr) return v2->empty();
  if (v2 == nullptr) return v1->empty();
  return v1->equal(v2);
}

inline bool nequal(const StringData* v1, const StringData* v2) {
  return !equal(v1, v2);
}

inline bool less(const StringData *v1, const StringData *v2) {
  if (v1 == v2 || v2 == nullptr) return false;
  if (v1 == nullptr) return !v2->empty();
  return v1->compare(v2) < 0;
}

inline bool lessEqual(const StringData* v1, const StringData* v2) {
  if (v1 == v2 || v1 == nullptr) return true;
  if (v2 == nullptr) return v1->empty();
  return v1->compare(v2) <= 0;
}

inline bool more(const StringData *v1, const StringData *v2) {
  if (v1 == nullptr) return false;
  if (v2 == nullptr) return !v1->empty();
  return v1->compare(v2) > 0;
}

inline bool moreEqual(const StringData* v1, const StringData* v2) {
  return lessEqual(v2, v1);
}

inline int64_t compare(const StringData* v1, const StringData* v2) {
  assertx(v1);
  assertx(v2);
  // Clamp return values to just -1, 0, 1.
  auto cmp = v1->compare(v2);
  return (cmp < 0) ? -1 : ((cmp > 0) ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////
// ObjectData*

bool same(const ObjectData* v1, const ObjectData* v2) = delete;

inline bool equal(const ObjectData* v1, const ObjectData* v2) {
  assertx(v1);
  assertx(v2);
  return v1->equal(*v2);
}

inline bool nequal(const ObjectData* v1, const ObjectData* v2) {
  return !equal(v1, v2);
}

inline bool less(const ObjectData* v1, const ObjectData* v2) {
  assertx(v1);
  assertx(v2);
  return v1->less(*v2);
}

inline bool lessEqual(const ObjectData* v1, const ObjectData* v2) {
  assertx(v1);
  assertx(v2);
  return v1->lessEqual(*v2);
}

inline bool more(const ObjectData* v1, const ObjectData* v2) {
  assertx(v1);
  assertx(v2);
  return v1->more(*v2);
}

inline bool moreEqual(const ObjectData* v1, const ObjectData* v2) {
  assertx(v1);
  assertx(v2);
  return v1->moreEqual(*v2);
}

inline int64_t compare(const ObjectData* v1, const ObjectData* v2) {
  assertx(v1);
  assertx(v2);
  return v1->compare(*v2);
}

///////////////////////////////////////////////////////////////////////////////
// ResourceHdr*

bool same(const ResourceHdr* v1, const ResourceHdr* v2) = delete;

inline bool equal(const ResourceHdr* v1, const ResourceHdr* v2) {
  return v1 == v2;
}

inline bool nequal(const ResourceHdr* v1, const ResourceHdr* v2) {
  return v1 != v2;
}

inline bool less(const ResourceHdr* v1, const ResourceHdr* v2) {
  assertx(v1);
  assertx(v2);
  if (v1->data()->template instanceof<OpaqueResource>() ||
      v2->data()->template instanceof<OpaqueResource>()) {
    throw_opaque_resource_compare_exception();
  }
  return v1->data()->o_toInt64() < v2->data()->o_toInt64();
}

inline bool lessEqual(const ResourceHdr* v1, const ResourceHdr* v2) {
  return less(v1, v2) || equal(v1, v2);
}

inline bool more(const ResourceHdr* v1, const ResourceHdr* v2) {
  assertx(v1);
  assertx(v2);
  if (v1->data()->template instanceof<OpaqueResource>() ||
      v2->data()->template instanceof<OpaqueResource>()) {
    throw_opaque_resource_compare_exception();
  }
  return v1->data()->o_toInt64() > v2->data()->o_toInt64();
}

inline bool moreEqual(const ResourceHdr* v1, const ResourceHdr* v2) {
  return more(v1, v2) || equal(v1, v2);
}

inline int64_t compare(const ResourceHdr* v1, const ResourceHdr* v2) {
  assertx(v1);
  assertx(v1);
  if (v1->data()->template instanceof<OpaqueResource>() ||
      v2->data()->template instanceof<OpaqueResource>()) {
    throw_opaque_resource_compare_exception();
  }
  auto id1 = v1->data()->o_toInt64();
  auto id2 = v2->data()->o_toInt64();
  return (id1 < id2) ? -1 : ((id1 > id2) ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////
// ECLString

bool same(const ECLString& v1, const ECLString& v2) = delete;

inline bool equal(const ECLString& v1, const ECLString& v2) {
  return v1.val == v2.val;
}

inline bool nequal(const ECLString& v1, const ECLString& v2) {
  return v1.val != v2.val;
}

inline bool less(const ECLString&, const ECLString&) {
  throw_ecl_compare_exception();
}

inline bool lessEqual(const ECLString&, const ECLString&) {
  throw_ecl_compare_exception();
}

inline bool more(const ECLString&, const ECLString&) {
  throw_ecl_compare_exception();
}

inline bool moreEqual(const ECLString&, const ECLString&) {
  throw_ecl_compare_exception();
}

inline int64_t compare(const ECLString&, const ECLString&) {
  throw_ecl_compare_exception();
}

}
