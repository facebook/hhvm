#include "hphp/runtime/ext/collections/ext_collections-pair.h"

namespace HPHP { namespace collections {
/////////////////////////////////////////////////////////////////////////////

const StaticString
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

  loadSystemlib("collections-pair");
}

/////////////////////////////////////////////////////////////////////////////
}}
