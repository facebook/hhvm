#include "hphp/runtime/ext/collections/ext_collections-pair.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_HH_Pair("HH\\Pair"),
  s_PairIterator("PairIterator");

/////////////////////////////////////////////////////////////////////////////
// PairIterator

static Variant HHVM_METHOD(PairIterator, current) {
  return Native::data<PairIterator>(this_)->current();
}

static Variant HHVM_METHOD(PairIterator, key) {
  return Native::data<PairIterator>(this_)->key();
}

static bool HHVM_METHOD(PairIterator, valid) {
  return Native::data<PairIterator>(this_)->valid();
}

static void HHVM_METHOD(PairIterator, next) {
  Native::data<PairIterator>(this_)->next();
}

static void HHVM_METHOD(PairIterator, rewind) {
  Native::data<PairIterator>(this_)->rewind();
}

/////////////////////////////////////////////////////////////////////////////
// Pair
}

Class* c_Pair::s_cls;

c_Pair::~c_Pair() {
  if (LIKELY(m_size == 2)) {
    tvRefcountedDecRef(&elm0);
    tvRefcountedDecRef(&elm1);
    return;
  }
  if (m_size == 1) {
    tvRefcountedDecRef(&elm0);
  }
}

int64_t c_Pair::linearSearch(const Variant& value) const {
  assertx(isFullyConstructed());
  for (uint64_t i = 0; i < 2; ++i) {
    if (same(value, tvAsCVarRef(&getElms()[i]))) {
      return i;
    }
  }
  return -1;
}

Array c_Pair::toArrayImpl() const {
  // Parsing/scanning the heap (e.g., objprof) can cause us to get here before
  // we've initialized the elms.
  if (!isFullyConstructed()) return empty_array();
  return make_packed_array(tvAsCVarRef(&elm0), tvAsCVarRef(&elm1));
}

c_Pair* c_Pair::Clone(ObjectData* obj) {
  auto thiz = static_cast<c_Pair*>(obj);
  auto pair = static_cast<c_Pair*>(obj->cloneImpl());
  assertx(thiz->isFullyConstructed());
  pair->incRefCount();
  pair->m_size = 2;
  cellDup(thiz->elm0, pair->elm0);
  cellDup(thiz->elm1, pair->elm1);
  return pair;
}

void c_Pair::throwBadKeyType() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only integer keys may be used with Pairs");
}

Array c_Pair::ToArray(const ObjectData* obj) {
  auto pair = static_cast<const c_Pair*>(obj);
  check_collection_cast_to_array();
  return pair->toArrayImpl();
}

bool c_Pair::OffsetIsset(ObjectData* obj, const TypedValue* key) {
  assertx(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  assertx(pair->isFullyConstructed());
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = pair->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellIsNull(result) : false;
}

bool c_Pair::OffsetEmpty(ObjectData* obj, const TypedValue* key) {
  assertx(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  assertx(pair->isFullyConstructed());
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = pair->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !cellToBool(*result) : true;
}

bool c_Pair::OffsetContains(ObjectData* obj, const TypedValue* key) {
  assertx(key->m_type != KindOfRef);
  auto pair = static_cast<c_Pair*>(obj);
  assertx(pair->isFullyConstructed());
  if (key->m_type == KindOfInt64) {
    return pair->contains(key->m_data.num);
  } else {
    throwBadKeyType();
    return false;
  }
}

bool c_Pair::Equals(const ObjectData* obj1, const ObjectData* obj2) {
  auto pair1 = static_cast<const c_Pair*>(obj1);
  auto pair2 = static_cast<const c_Pair*>(obj2);
  assertx(pair1->isFullyConstructed());
  assertx(pair2->isFullyConstructed());
  return HPHP::equal(tvAsCVarRef(&pair1->elm0), tvAsCVarRef(&pair2->elm0)) &&
         HPHP::equal(tvAsCVarRef(&pair1->elm1), tvAsCVarRef(&pair2->elm1));
}

Object c_Pair::getIterator() {
  assertx(isFullyConstructed());
  auto iter = collections::PairIterator::newInstance();
  Native::data<collections::PairIterator>(iter)->setPair(this);
  return iter;
}

namespace collections {
/////////////////////////////////////////////////////////////////////////////

void CollectionsExtension::initPair() {
  HHVM_ME(PairIterator, current);
  HHVM_ME(PairIterator, key);
  HHVM_ME(PairIterator, valid);
  HHVM_ME(PairIterator, next);
  HHVM_ME(PairIterator, rewind);

  Native::registerNativeDataInfo<PairIterator>(
    s_PairIterator.get(),
    Native::NDIFlags::NO_SWEEP
  );

  HHVM_NAMED_ME(HH\\Pair, values,         materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\Pair, at,             &c_Pair::php_at);
  HHVM_NAMED_ME(HH\\Pair, get,            &c_Pair::php_get);
  HHVM_NAMED_ME(HH\\Pair, linearSearch,   &c_Pair::linearSearch);
  HHVM_NAMED_ME(HH\\Pair, toArray,        &c_Pair::toArrayImpl);
  HHVM_NAMED_ME(HH\\Pair, toValuesArray,  &c_Pair::toArrayImpl);
  HHVM_NAMED_ME(HH\\Pair, getIterator,    &c_Pair::getIterator);

  HHVM_NAMED_ME(HH\\Pair, toVector,       materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\Pair, toImmVector,    materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\Pair, toMap,          materialize<c_Map>);
  HHVM_NAMED_ME(HH\\Pair, toImmMap,       materialize<c_ImmMap>);
  HHVM_NAMED_ME(HH\\Pair, toSet,          materialize<c_Set>);
  HHVM_NAMED_ME(HH\\Pair, toImmSet,       materialize<c_ImmSet>);

  loadSystemlib("collections-pair");

  c_Pair::s_cls = Unit::lookupClass(s_HH_Pair.get());
  assertx(c_Pair::s_cls);

  finishClass<c_Pair>();
}

/////////////////////////////////////////////////////////////////////////////
}}
