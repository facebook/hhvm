#include "hphp/runtime/ext/icu/ext_icu_iterator.h"

namespace HPHP { namespace Intl {
//////////////////////////////////////////////////////////////////////////////

const StaticString s_IntlIterator("IntlIterator");

IntlIterator *IntlIterator::Get(Object obj) {
  return GetResData<IntlIterator>(obj, s_IntlIterator.get());
}

Object IntlIterator::wrap() {
  return WrapResData(s_IntlIterator.get());
}

#define II_GET(dest, src, def) \
  auto dest = IntlIterator::Get(src); \
  if (!dest) { \
    return def; \
  }

//////////////////////////////////////////////////////////////////////////////
// class IntlIterator

static Variant HHVM_METHOD(IntlIterator, current) {
  II_GET(data, this_, false);
  return data->current();
}

static Variant HHVM_METHOD(IntlIterator, key) {
  // Current uses of IntlIterator don't use key,
  // but reserve the calling semantics for now.
  return uninit_null();
}

static Variant HHVM_METHOD(IntlIterator, next) {
  II_GET(data, this_, false);
  return data->next();
}

static Variant HHVM_METHOD(IntlIterator, rewind) {
  II_GET(data, this_, false);
  data->rewind();
  return data->current();
}

static bool HHVM_METHOD(IntlIterator, valid) {
  II_GET(data, this_, false);
  return data->valid();
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::initIterator() {
  HHVM_ME(IntlIterator, current);
  HHVM_ME(IntlIterator, key);
  HHVM_ME(IntlIterator, next);
  HHVM_ME(IntlIterator, rewind);
  HHVM_ME(IntlIterator, valid);
  loadSystemlib("icu_iterator");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
