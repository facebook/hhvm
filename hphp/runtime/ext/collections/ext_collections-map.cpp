#include "hphp/runtime/ext/collections/ext_collections-map.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_MapIterator("MapIterator");

/////////////////////////////////////////////////////////////////////////////
// MapIterator

static Variant HHVM_METHOD(MapIterator, current) {
  return Native::data<MapIterator>(this_)->current();
}

static Variant HHVM_METHOD(MapIterator, key) {
  return Native::data<MapIterator>(this_)->key();
}

static bool HHVM_METHOD(MapIterator, valid) {
  return Native::data<MapIterator>(this_)->valid();
}

static void HHVM_METHOD(MapIterator, next) {
  Native::data<MapIterator>(this_)->next();
}

static void HHVM_METHOD(MapIterator, rewind) {
  Native::data<MapIterator>(this_)->rewind();
}

/////////////////////////////////////////////////////////////////////////////

void CollectionsExtension::initMap() {
  HHVM_ME(MapIterator, current);
  HHVM_ME(MapIterator, key);
  HHVM_ME(MapIterator, valid);
  HHVM_ME(MapIterator, next);
  HHVM_ME(MapIterator, rewind);

  Native::registerNativeDataInfo<MapIterator>(
    s_MapIterator.get(),
    Native::NDIFlags::NO_SWEEP
  );

  loadSystemlib("collections-map");
}

/////////////////////////////////////////////////////////////////////////////
}}
