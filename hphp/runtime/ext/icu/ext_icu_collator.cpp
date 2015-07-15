#include "hphp/runtime/ext/icu/ext_icu_collator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/zend-collator.h"
#include "hphp/runtime/base/zend-qsort.h"

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
// class Collator

enum CollatorSort {
  SORT_REGULAR = 0,
  SORT_STRING = 1,
  SORT_NUMERIC = 2,
};

const StaticString s_Collator("Collator");

#define FETCH_COL(dest, src, ret) \
  auto dest = Collator::Get(src); \
  if (!dest) { \
    raise_recoverable_error("Collator not initialized"); \
    return ret; \
  }

static void HHVM_METHOD(Collator, __construct, const String& locale) {
  auto data = Native::data<Collator>(this_);
  data->clearError();
  if (!locale.empty()) {
    UErrorCode error = U_ZERO_ERROR;
    data->setCollator(ucol_open(locale.c_str(), &error));
    if (U_SUCCESS(error)) {
      return;
    }
    /* Fallthrough and use default collator */
  }
  data->setError(U_USING_FALLBACK_WARNING);
  UErrorCode error = U_ZERO_ERROR;
  data->setCollator(ucol_open(uloc_getDefault(), &error));
  if (U_FAILURE(error)) {
    data->setError(error, "collator_create: unable to open ICU collator");
    data->setCollator(nullptr);
    return;
  }
}

static bool HHVM_METHOD(Collator, asort, VRefParam arr, int64_t flag) {
  FETCH_COL(data, this_, false);
  if (!arr.isArray()) {
    throw_expected_array_exception("Collator::asort");
    return false;
  }
  data->clearError();
  bool ret = collator_asort(arr, flag, true, data->collator(), data);
  if (U_FAILURE(data->getErrorCode())) {
    return false;
  }
  return ret;
}

static Variant HHVM_METHOD(Collator, compare, const Variant& str1, const Variant& str2) {
  FETCH_COL(data, this_, false);
  data->clearError();
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString ustr1(u16(str1.toString(), error));
  if (U_FAILURE(error)) {
    data->setError(error);
    return false;
  }
  error = U_ZERO_ERROR;
  icu::UnicodeString ustr2(u16(str2.toString(), error));
  if (U_FAILURE(error)) {
    data->setError(error);
    return false;
  }
  return (int64_t)ucol_strcoll(data->collator(),
                               ustr1.getBuffer(), ustr1.length(),
                               ustr2.getBuffer(), ustr2.length());
}

static int64_t HHVM_METHOD(Collator, getAttribute, int64_t attr) {
  FETCH_COL(data, this_, 0);
  data->clearError();
  UErrorCode error = U_ZERO_ERROR;
  int64_t ret = (int64_t)ucol_getAttribute(data->collator(),
                                           (UColAttribute)attr,
                                           &error);
  if (U_FAILURE(error)) {
    data->setError(error, "Error getting attribute value");
    return 0;
  }
  return ret;
}

static int64_t HHVM_METHOD(Collator, getErrorCode) {
  FETCH_COL(data, this_, 0);
  return data->getErrorCode();
}

static String HHVM_METHOD(Collator, getErrorMessage) {
  FETCH_COL(data, this_, "");
  return data->getErrorMessage();
}

static String HHVM_METHOD(Collator, getLocale, int64_t type) {
  FETCH_COL(data, this_, "");
  data->clearError();
  UErrorCode error = U_ZERO_ERROR;
  auto loc = ucol_getLocaleByType(data->collator(), (ULocDataLocaleType)type,
                                  &error);
  if (U_FAILURE(error)) {
    data->setError(error, "Error getting locale by type");
  }
  return String(loc, CopyString);
}

static int64_t HHVM_METHOD(Collator, getStrength) {
  FETCH_COL(data, this_, false);
  return ucol_getStrength(data->collator());
}

static bool HHVM_METHOD(Collator, setAttribute, int64_t attr, int64_t val) {
  FETCH_COL(data, this_, false);
  data->clearError();
  UErrorCode error = U_ZERO_ERROR;
  ucol_setAttribute(data->collator(), (UColAttribute)attr,
                                      (UColAttributeValue)val, &error);
  if (U_FAILURE(error)) {
    data->setError(error, "Error setting attribute value");
    return false;
  }
  return true;
}

static Variant HHVM_METHOD(Collator, getSortKey, const String& val) {
  FETCH_COL(data, this_, false);
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString strval(u16(val, error));
  if (U_FAILURE(error)) {
    return false;
  }

  int sortkey_len = ucol_getSortKey(data->collator(),
                                    strval.getBuffer(), strval.length(),
                                    nullptr,
                                    0);
  if (sortkey_len <= 0) {
    return false;
  }

  String ret(sortkey_len + 1, ReserveString);
  sortkey_len = ucol_getSortKey(data->collator(),
                                strval.getBuffer(), strval.length(),
                                (uint8_t*) ret.get()->mutableData(),
                                ret.capacity() + 1);
  if (sortkey_len <= 0) {
    return false;
  }

  ret.setSize(sortkey_len);
  return ret;
}

static bool HHVM_METHOD(Collator, setStrength, int64_t strength) {
  FETCH_COL(data, this_, false);
  ucol_setStrength(data->collator(), (UCollationStrength)strength);
  return true;
}

typedef struct _collator_sort_key_index {
  char* key;       /* pointer to sort key */
  ssize_t valPos;  /* position of the original array element */
} collator_sort_key_index_t;

/* Bytes to reserve for sort keys */
static const int32_t DEF_SORT_KEYS_BUF_SIZE = 1048576;
static const int32_t DEF_SORT_KEYS_BUF_INCREMENT = 1048576;

/* Number of keys position to allocate */
static const int32_t DEF_SORT_KEYS_INDX_BUF_SIZE = 512;
static const int32_t DEF_SORT_KEYS_INDX_BUF_INCREMENT = 64;

static int collator_cmp_sort_keys(const void* p1, const void* p2, const void*) {
  char* key1 = ((collator_sort_key_index_t*)p1)->key;
  char* key2 = ((collator_sort_key_index_t*)p2)->key;
  return strcmp( key1, key2 );
}

static bool HHVM_METHOD(Collator, sortWithSortKeys, VRefParam arr) {
  FETCH_COL(data, this_, false);
  data->clearError();

  if (!arr.isArray()) {
    return true;
  }

  Array hash = arr.toArray();
  if (hash.size() == 0) {
    return true;
  }

  // Preallocate sort keys buffer
  size_t sortKeysOffset = 0;
  size_t sortKeysLength = DEF_SORT_KEYS_BUF_SIZE;
  char*  sortKeys = (char*)req::malloc(sortKeysLength);
  if (!sortKeys) {
    throw Exception("Out of memory");
  }
  SCOPE_EXIT{ req::free(sortKeys); };

  // Preallocate index buffer
  size_t sortIndexPos = 0;
  size_t sortIndexLength = DEF_SORT_KEYS_INDX_BUF_SIZE;
  auto   sortIndex = (collator_sort_key_index_t*)req::malloc(
                  sortIndexLength * sizeof(collator_sort_key_index_t));
  if (!sortIndex) {
    throw Exception("Out of memory");
  }
  SCOPE_EXIT{ req::free(sortIndex); };

  // Translate input hash to sortable index
  auto pos_limit = hash->iter_end();
  for (ssize_t pos = hash->iter_begin(); pos != pos_limit;
       pos = hash->iter_advance(pos)) {
    Variant val(hash->getValue(pos));

    // Convert to UTF16
    icu::UnicodeString strval;
    if (val.isString()) {
      UErrorCode error = U_ZERO_ERROR;
      strval = u16(val.toString(), error);
      if (U_FAILURE(error)) {
        return false;
      }
     }

    // Generate sort key
    int sortkey_len =
      ucol_getSortKey(data->collator(),
                      strval.getBuffer(), strval.length(),
                      (uint8_t*)(sortKeys + sortKeysOffset),
                      sortKeysLength - sortKeysOffset);

    // Check for key buffer overflow
    if (sortkey_len > (sortKeysLength - sortKeysOffset)) {
      int32_t inc = (sortkey_len > DEF_SORT_KEYS_BUF_INCREMENT)
                  ?  sortkey_len : DEF_SORT_KEYS_BUF_INCREMENT;
      sortKeysLength += inc;
      sortKeys = (char*)req::realloc(sortKeys, sortKeysLength);
      if (!sortKeys) {
        throw Exception("Out of memory");
      }
      sortkey_len =
        ucol_getSortKey(data->collator(),
                        strval.getBuffer(), strval.length(),
                        (uint8_t*)(sortKeys + sortKeysOffset),
                        sortKeysLength - sortKeysOffset);
      assert(sortkey_len <= (sortKeysLength - sortKeysOffset));
    }

    // Check for index buffer overflow
    if ((sortIndexPos + 1) > sortIndexLength) {
      sortIndexLength += DEF_SORT_KEYS_INDX_BUF_INCREMENT;
      sortIndex = (collator_sort_key_index_t*)req::realloc(sortIndex,
                      sortIndexLength * sizeof(collator_sort_key_index_t));
      if (!sortIndex) {
        throw Exception("Out of memory");
      }
    }

    // Initially store offset into buffer, update later to deal with reallocs
    sortIndex[sortIndexPos].key = (char*)sortKeysOffset;
    sortKeysOffset += sortkey_len;

    sortIndex[sortIndexPos].valPos = pos;
    ++sortIndexPos;
  }

  // Update keys to location in realloc'd buffer
  for (int i = 0; i < sortIndexPos; ++i) {
    sortIndex[i].key = sortKeys + (ptrdiff_t)sortIndex[i].key;
  }

  zend_qsort(sortIndex, sortIndexPos,
             sizeof(collator_sort_key_index_t),
             collator_cmp_sort_keys, nullptr);

  Array ret = Array::Create();
  for (int i = 0; i < sortIndexPos; ++i) {
    ret.append(hash->getValue(sortIndex[i].valPos));
  }
  arr = ret;
  return true;
}

static bool HHVM_METHOD(Collator, sort, VRefParam arr,
                        int64_t sort_flag /* = Collator::SORT_REGULAR */) {
  FETCH_COL(data, this_, false);
  if (!arr.isArray()) {
    throw_expected_array_exception("Collator::sort");
    return false;
  }
  data->clearError();
  bool ret = collator_sort(arr, sort_flag, true, data->collator(), data);
  if (U_FAILURE(data->getErrorCode())) {
    return false;
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////

#define CONST_SORT(v)  Native::registerClassConstant<KindOfInt64> \
          (s_Collator.get(), makeStaticString("SORT_" #v), SORT_##v);

#define CONST_UCOL(v)  Native::registerClassConstant<KindOfInt64> \
          (s_Collator.get(), makeStaticString(#v), UCOL_##v);

const StaticString s_DEFAULT_VALUE("DEFAULT_VALUE");

void IntlExtension::initCollator() {
  HHVM_ME(Collator, __construct);
  HHVM_ME(Collator, asort);
  HHVM_ME(Collator, compare);
  HHVM_ME(Collator, getAttribute);
  HHVM_ME(Collator, getErrorCode);
  HHVM_ME(Collator, getErrorMessage);
  HHVM_ME(Collator, getLocale);
  HHVM_ME(Collator, getSortKey);
  HHVM_ME(Collator, getStrength);
  HHVM_ME(Collator, setAttribute);
  HHVM_ME(Collator, setStrength);
  HHVM_ME(Collator, sortWithSortKeys);
  HHVM_ME(Collator, sort);

  CONST_SORT(REGULAR);
  CONST_SORT(STRING);
  CONST_SORT(NUMERIC);

  CONST_UCOL(FRENCH_COLLATION);
  CONST_UCOL(ALTERNATE_HANDLING);
  CONST_UCOL(CASE_FIRST);
  CONST_UCOL(CASE_LEVEL);
  CONST_UCOL(NORMALIZATION_MODE);
  CONST_UCOL(STRENGTH);
  CONST_UCOL(HIRAGANA_QUATERNARY_MODE);
  CONST_UCOL(NUMERIC_COLLATION);
  CONST_UCOL(PRIMARY);
  CONST_UCOL(SECONDARY);
  CONST_UCOL(TERTIARY);
  CONST_UCOL(DEFAULT_STRENGTH);
  CONST_UCOL(QUATERNARY);
  CONST_UCOL(IDENTICAL);
  CONST_UCOL(OFF);
  CONST_UCOL(ON);
  CONST_UCOL(SHIFTED);
  CONST_UCOL(NON_IGNORABLE);
  CONST_UCOL(LOWER_FIRST);
  CONST_UCOL(UPPER_FIRST);

  Native::registerClassConstant<KindOfInt64>
    (s_Collator.get(), s_DEFAULT_VALUE.get(), UCOL_DEFAULT);

  Native::registerNativeDataInfo<Collator>(s_Collator.get());

  loadSystemlib("icu_collator");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
