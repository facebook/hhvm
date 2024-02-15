#include "hphp/runtime/ext/collections/ext_collections-pair.h"

#include "hphp/runtime/ext/collections/ext_collections.h"
#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-set.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/native.h"

namespace HPHP { namespace collections {

/////////////////////////////////////////////////////////////////////////////
// PairIterator

static Variant HHVM_METHOD(PairIterator, current) {
  return Native::data<PairIterator>(this_)->current();
}

static int64_t HHVM_METHOD(PairIterator, key) {
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

c_Pair::~c_Pair() {
  tvDecRefGen(&elm0);
  tvDecRefGen(&elm1);
}

int64_t c_Pair::linearSearch(const Variant& value) const {
  for (uint64_t i = 0; i < 2; ++i) {
    if (same(value, tvAsCVarRef(&getElms()[i]))) {
      return i;
    }
  }
  return -1;
}

Array c_Pair::toPHPArrayImpl() const {
  return make_dict_array(0, tvAsCVarRef(&elm0),
                         1, tvAsCVarRef(&elm1));
}

c_Pair* c_Pair::Clone(ObjectData* obj) {
  auto thiz = static_cast<c_Pair*>(obj);
  auto pair = req::make<c_Pair>(thiz->elm0, thiz->elm1);
  return pair.detach();
}

void c_Pair::throwBadKeyType() {
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only integer keys may be used with Pairs");
}

bool c_Pair::OffsetIsset(ObjectData* obj, const TypedValue* key) {
  auto pair = static_cast<c_Pair*>(obj);
  TypedValue* result;
  if (key->m_type == KindOfInt64) {
    result = pair->get(key->m_data.num);
  } else {
    throwBadKeyType();
    result = nullptr;
  }
  return result ? !tvIsNull(result) : false;
}

bool c_Pair::OffsetContains(ObjectData* obj, const TypedValue* key) {
  auto pair = static_cast<c_Pair*>(obj);
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
  return HPHP::equal(tvAsCVarRef(&pair1->elm0), tvAsCVarRef(&pair2->elm0)) &&
         HPHP::equal(tvAsCVarRef(&pair1->elm1), tvAsCVarRef(&pair2->elm1));
}

Object c_Pair::getIterator() {
  auto iter = collections::PairIterator::newInstance();
  Native::data<collections::PairIterator>(iter)->setPair(this);
  return iter;
}

namespace collections {
/////////////////////////////////////////////////////////////////////////////

void CollectionsExtension::registerNativePair() {
  HHVM_ME(PairIterator, current);
  HHVM_ME(PairIterator, key);
  HHVM_ME(PairIterator, valid);
  HHVM_ME(PairIterator, next);
  HHVM_ME(PairIterator, rewind);

  Native::registerNativeDataInfo<PairIterator>(Native::NDIFlags::NO_SWEEP);

  HHVM_NAMED_ME(HH\\Pair, values,         materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\Pair, at,             &c_Pair::php_at);
  HHVM_NAMED_ME(HH\\Pair, get,            &c_Pair::php_get);
  HHVM_NAMED_ME(HH\\Pair, linearSearch,   &c_Pair::linearSearch);
  HHVM_NAMED_ME(HH\\Pair, getIterator,    &c_Pair::getIterator);

  HHVM_NAMED_ME(HH\\Pair, toVector,       materialize<c_Vector>);
  HHVM_NAMED_ME(HH\\Pair, toImmVector,    materialize<c_ImmVector>);
  HHVM_NAMED_ME(HH\\Pair, toMap,          materialize<c_Map>);
  HHVM_NAMED_ME(HH\\Pair, toImmMap,       materialize<c_ImmMap>);
  HHVM_NAMED_ME(HH\\Pair, toSet,          materialize<c_Set>);
  HHVM_NAMED_ME(HH\\Pair, toImmSet,       materialize<c_ImmSet>);

  Native::registerNativePropHandler<CollectionPropHandler>(c_Pair::className());

  Native::registerClassExtraDataHandler(c_Pair::className(), finish_class<c_Pair>);
}

/////////////////////////////////////////////////////////////////////////////
}}
