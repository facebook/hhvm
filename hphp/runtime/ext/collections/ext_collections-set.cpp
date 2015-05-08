#include "hphp/runtime/ext/collections/ext_collections-set.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_SetIterator("SetIterator");

/////////////////////////////////////////////////////////////////////////////
// SetIterator

static Variant HHVM_METHOD(SetIterator, current) {
  return Native::data<SetIterator>(this_)->current();
}

static Variant HHVM_METHOD(SetIterator, key) {
  return Native::data<SetIterator>(this_)->key();
}

static bool HHVM_METHOD(SetIterator, valid) {
  return Native::data<SetIterator>(this_)->valid();
}

static void HHVM_METHOD(SetIterator, next) {
  Native::data<SetIterator>(this_)->next();
}

static void HHVM_METHOD(SetIterator, rewind) {
  Native::data<SetIterator>(this_)->rewind();
}

/////////////////////////////////////////////////////////////////////////////

void CollectionsExtension::initSet() {
  HHVM_ME(SetIterator, current);
  HHVM_ME(SetIterator, key);
  HHVM_ME(SetIterator, valid);
  HHVM_ME(SetIterator, next);
  HHVM_ME(SetIterator, rewind);

  Native::registerNativeDataInfo<SetIterator>(
    s_SetIterator.get(),
    Native::NDIFlags::NO_SWEEP
  );

  loadSystemlib("collections-set");
}

/////////////////////////////////////////////////////////////////////////////
}}
