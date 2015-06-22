#include "hphp/runtime/ext/icu/ext_icu_rsrc_bundle.h"


namespace HPHP { namespace Intl {
//////////////////////////////////////////////////////////////////////////////

#define FETCH_RSRC(data, obj) \
  auto data = ResourceBundle::Get(obj); \
  if (!data) { \
    throw s_intl_error->getException("Uninitialized Message Formatter"); \
  }


const StaticString s_ResourceBundle("ResourceBundle");
Class* ResourceBundle::c_ResourceBundle = nullptr;

static Variant extractValue(ResourceBundle* data,
                            const icu::ResourceBundle& bundle) {
#define EXTRACT_ERR(type) \
  if (U_FAILURE(error)) { \
    data->setError(error, "Failed to retreive " #type " value"); \
    return init_null(); \
  }

  UErrorCode error = U_ZERO_ERROR;
  switch (bundle.getType()) {
    case URES_STRING: {
      auto ret = bundle.getString(error);
      EXTRACT_ERR(string);
      error = U_ZERO_ERROR;
      String ret8(u8(ret, error));
      if (U_FAILURE(error)) {
        data->setError(error, "Failed converting value to utf-8");
        return init_null();
      }
      return ret8;
    }
    case URES_BINARY: {
      int32_t len;
      auto ret = bundle.getBinary(len, error);
      EXTRACT_ERR(binary);
      return String((char*)ret, len, CopyString);
    }
    case URES_INT: {
      auto ret = bundle.getInt(error);
      EXTRACT_ERR(int);
      return ret;
    }
    case URES_INT_VECTOR: {
      int32_t len;
      auto vec = bundle.getIntVector(len, error);
      EXTRACT_ERR(vector);
      Array ret = Array::Create();
      for (int i = 0; i < len; ++i) {
        ret.append((int64_t)vec[i]);
      }
      return ret;
    }
    case URES_ARRAY:
    case URES_TABLE:
      return ResourceBundle::newInstance(new icu::ResourceBundle(bundle));
    default:
      data->setError(U_ILLEGAL_ARGUMENT_ERROR, "Unknown resource type");
      return init_null();
  }
#undef EXTRACT_ERR
}

static void HHVM_METHOD(ResourceBundle, __construct, const Variant& locale,
                                                     const Variant& bundleName,
                                                     bool fallback) {
  const char *bundle = bundleName.isNull() ? nullptr :
                       bundleName.toString().c_str();
  auto loc = Locale::createFromName(localeOrDefault(locale.toString()).c_str());
  auto data = Native::data<ResourceBundle>(this_);
  UErrorCode error = U_ZERO_ERROR;
  auto rsrc = new icu::ResourceBundle(bundle, loc, error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "resourcebundle_ctor: "
                                  "Cannot load libICU resource bundle");
    throw data->getException("%s", s_intl_error->getErrorMessage().c_str());
  }
  if (!fallback &&
      ((error == U_USING_FALLBACK_WARNING) ||
       (error == U_USING_DEFAULT_WARNING))) {
    UErrorCode dummy = U_ZERO_ERROR;
    s_intl_error->setError(error,
      "resourcebundle_ctor: Cannot load libICU resource "
      "'%s' without fallback from %s to %s",
      bundle ? bundle : "(default data)", loc.getName(),
      rsrc->getLocale(ULOC_ACTUAL_LOCALE, dummy).getName());
    delete rsrc;
    throw data->getException("%s", s_intl_error->getErrorMessage().c_str());
  }
  data->setResource(rsrc);
}

static int64_t HHVM_METHOD(ResourceBundle, count) {
  FETCH_RSRC(data, this_);
  return data->count();
}

static int64_t HHVM_METHOD(ResourceBundle, getErrorCode) {
  FETCH_RSRC(data, this_);
  return data->getErrorCode();
}

static String HHVM_METHOD(ResourceBundle, getErrorMessage) {
  FETCH_RSRC(data, this_);
  return data->getErrorMessage();
}

static Variant HHVM_METHOD(ResourceBundle, get,
                           const Variant& index,
                           bool fallback) {
  FETCH_RSRC(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  icu::ResourceBundle child(error);
  if (U_FAILURE(error)) {
    data->setError(error, "Unable to generate default resource bundle");
    return false;
  }
  if (index.isInteger()) {
    child = data->resource()->get((int32_t)index.toInt64(), error);
    if (U_FAILURE(error)) {
      data->setError(error, "Cannot load resource element %d",
                            (int)index.toInt64());
      return init_null();
    }
  } else if (index.isString()) {
    child = data->resource()->get(index.toString().c_str(), error);
    if (U_FAILURE(error)) {
      data->setError(error, "Cannot load resource element '%s'",
                            index.toString().c_str());
      return init_null();
    }
  } else {
    data->setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "resourcebundle_get: index should be integer or string");
    return init_null();
  }

  if (!fallback &&
      ((error == U_USING_FALLBACK_WARNING) ||
       (error == U_USING_DEFAULT_WARNING))) {
    UErrorCode dummy = U_ZERO_ERROR;
    auto locale = data->resource()->getLocale(ULOC_ACTUAL_LOCALE, dummy);
    if (index.isInteger()) {
      data->setError(error,
                     "Cannot load element %d without fallback from to %s",
                     (int)index.toInt64(), locale.getName());
    } else {
      data->setError(error,
                     "Cannot load element %s without fallback from to %s",
                     index.toString().c_str(), locale.getName());
    }
    return init_null();
  }

  return extractValue(data, child);
}

static Variant HHVM_STATIC_METHOD(ResourceBundle, getLocales,
                                                  const String& bundleName) {
  UErrorCode error = U_ZERO_ERROR;
  const char *bundle = bundleName.length() ? bundleName.c_str() : nullptr;
  auto le = ures_openAvailableLocales(bundle, &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Cannot fetch locales list");
    return false;
  }
  error = U_ZERO_ERROR;
  uenum_reset(le, &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Cannot iterate locales list");
    return false;
  }

  Array ret = Array::Create();
  const char *entry;
  int32_t entry_len;
  while ((entry = uenum_next(le, &entry_len, &error))) {
    ret.append(String(entry, entry_len, CopyString));
  }
  return ret;
}


static Variant HHVM_METHOD(ResourceBundle, current) {
  FETCH_RSRC(data, this_);
  if (!data->iterValid()) {
    return init_null();
  }
  UErrorCode error = U_ZERO_ERROR;
  return extractValue(data, data->iterCurrent(error));
}

static Variant HHVM_METHOD(ResourceBundle, key) {
  FETCH_RSRC(data, this_);
  if (!data->iterValid()) {
    return init_null();
  }
  return data->iterKey();
}

static Variant HHVM_METHOD(ResourceBundle, next) {
  FETCH_RSRC(data, this_);
  if (!data->iterNext()) {
    return init_null();
  }
  UErrorCode error = U_ZERO_ERROR;
  return extractValue(data, data->iterCurrent(error));
}

static Variant HHVM_METHOD(ResourceBundle, rewind) {
  FETCH_RSRC(data, this_);
  data->iterRewind();
  if (!data->iterValid()) {
    return init_null();
  }
  UErrorCode error = U_ZERO_ERROR;
  return extractValue(data, data->iterCurrent(error));
}

static bool HHVM_METHOD(ResourceBundle, valid) {
  FETCH_RSRC(data, this_);
  return data->iterValid();
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::initResourceBundle() {
  HHVM_ME(ResourceBundle, __construct);
  HHVM_ME(ResourceBundle, count);
  HHVM_ME(ResourceBundle, getErrorCode);
  HHVM_ME(ResourceBundle, getErrorMessage);
  HHVM_ME(ResourceBundle, get);
  HHVM_STATIC_ME(ResourceBundle, getLocales);

  HHVM_ME(ResourceBundle, current);
  HHVM_ME(ResourceBundle, key);
  HHVM_ME(ResourceBundle, next);
  HHVM_ME(ResourceBundle, rewind);
  HHVM_ME(ResourceBundle, valid);

  Native::registerNativeDataInfo<ResourceBundle>(s_ResourceBundle.get());

  loadSystemlib("icu_rsrc_bundle");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
