#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

namespace HPHP {

const StaticString
  s_IntlIterator("IntlIterator"),
  s_resdata("__resdata");

//////////////////////////////////////////////////////////////////////////////
// Internal resource data

class IntlIterator : public SweepableResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(IntlIterator);
  CLASSNAME_IS("IntlIterator");
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit IntlIterator(icu::StringEnumeration *se) : m_enum(se) {}

  void sweep() override {
    if (m_enum) {
      delete m_enum;
      m_enum = nullptr;
    }
  }

  bool isInvalid() const override {
    return m_enum == nullptr;
  }

  static IntlIterator *Get(Object obj) {
    if (obj.isNull()) {
      raise_error("NULL object passed");
      return nullptr;
    }
    auto res = obj->o_get(s_resdata, false, s_IntlIterator.get());
    if (!res.isResource()) {
      return nullptr;
    }
    auto ret = res.toResource().getTyped<IntlIterator>(false, false);
    if (!ret) {
      return nullptr;
    }
    if (ret->isInvalid()) {
      raise_error("Found unconstructed IntlIterator");
      return nullptr;
    }
    return ret;
  }

  Object wrap() {
    auto cls = Unit::lookupClass(s_IntlIterator.get());
    auto obj = ObjectData::newInstance(cls);
    obj->o_set(s_resdata, Resource(this), s_IntlIterator.get());
    return Object(obj);
  }

  Variant current() const { return m_current; }
  bool valid() const { return m_current.isString(); }

  Variant next() {
    UErrorCode error = U_ZERO_ERROR;
    int32_t len;
    const char *e = m_enum->next(&len, error);
    if (U_FAILURE(error)) {
      s_intl_error->set(error, "Error fetching next iteration element");
      m_current = uninit_null();
    } else {
      m_current = String(e, len, CopyString);
    }
    return m_current;
  }

  bool rewind() {
    UErrorCode error = U_ZERO_ERROR;
    m_enum->reset(error);
    if (U_FAILURE(error)) {
      s_intl_error->set(error, "Error resetting enumeration");
      m_current = uninit_null();
      return false;
    }
    next();
    return true;
  }
 private:
  icu::StringEnumeration *m_enum = nullptr;
  Variant m_current = null_string;
};

namespace Intl {
  Object iteratorFromEnumeration(icu::StringEnumeration *se) {
    return (NEWOBJ(IntlIterator)(se))->wrap();
  }
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

void Intl::IntlExtension::initIterator() {
  HHVM_ME(IntlIterator, current);
  HHVM_ME(IntlIterator, key);
  HHVM_ME(IntlIterator, next);
  HHVM_ME(IntlIterator, rewind);
  HHVM_ME(IntlIterator, valid);
  loadSystemlib("icu_iterator");
}

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
