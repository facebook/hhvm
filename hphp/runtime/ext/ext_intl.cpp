/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/ext_intl.h"
#include "hphp/runtime/base/builtin-functions.h" // throw_expected_*
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/intl-convert.h"
#include "hphp/runtime/base/zend-collator.h"
#include "hphp/runtime/base/zend-qsort.h"
#include <unicode/ustring.h>
#include <unicode/ucol.h> // icu
#include <unicode/uclean.h> // icu
#include <unicode/putil.h> // icu
#include <unicode/utypes.h>
#include <unicode/unorm.h>

#include "hphp/system/systemlib.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

const int64_t q_Collator$$SORT_REGULAR = 0;
const int64_t q_Collator$$SORT_STRING = 1;
const int64_t q_Collator$$SORT_NUMERIC = 2;
const int64_t q_Collator$$FRENCH_COLLATION = UCOL_FRENCH_COLLATION;
const int64_t q_Collator$$ALTERNATE_HANDLING = UCOL_ALTERNATE_HANDLING;
const int64_t q_Collator$$CASE_FIRST = UCOL_CASE_FIRST;
const int64_t q_Collator$$CASE_LEVEL = UCOL_CASE_LEVEL;
const int64_t q_Collator$$NORMALIZATION_MODE = UCOL_NORMALIZATION_MODE;
const int64_t q_Collator$$STRENGTH = UCOL_STRENGTH;
const int64_t q_Collator$$HIRAGANA_QUATERNARY_MODE = UCOL_HIRAGANA_QUATERNARY_MODE;
const int64_t q_Collator$$NUMERIC_COLLATION = UCOL_NUMERIC_COLLATION;
const int64_t q_Collator$$DEFAULT_VALUE = UCOL_DEFAULT;
const int64_t q_Collator$$PRIMARY = UCOL_PRIMARY;
const int64_t q_Collator$$SECONDARY = UCOL_SECONDARY;
const int64_t q_Collator$$TERTIARY = UCOL_TERTIARY;
const int64_t q_Collator$$DEFAULT_STRENGTH = UCOL_DEFAULT_STRENGTH;
const int64_t q_Collator$$QUATERNARY = UCOL_QUATERNARY;
const int64_t q_Collator$$IDENTICAL = UCOL_IDENTICAL;
const int64_t q_Collator$$OFF = UCOL_OFF;
const int64_t q_Collator$$ON = UCOL_ON;
const int64_t q_Collator$$SHIFTED = UCOL_SHIFTED;
const int64_t q_Collator$$NON_IGNORABLE = UCOL_NON_IGNORABLE;
const int64_t q_Collator$$LOWER_FIRST = UCOL_LOWER_FIRST;
const int64_t q_Collator$$UPPER_FIRST = UCOL_UPPER_FIRST;

///////////////////////////////////////////////////////////////////////////////

c_Collator::c_Collator(Class* cb) :
    ExtObjectData(cb), m_locale(), m_ucoll(NULL), m_errcode() {
}

c_Collator::~c_Collator() {
  if (m_ucoll) {
    ucol_close(m_ucoll);
    m_ucoll = NULL;
  }
}

void c_Collator::t___construct(const String& locale) {
  if (m_ucoll) {
    ucol_close(m_ucoll);
    m_ucoll = NULL;
  }
  m_errcode.clearError();
  if (!locale.empty()) {
    UErrorCode error = U_ZERO_ERROR;
    m_locale = locale;
    m_ucoll = ucol_open(locale.data(), &error);
    if (!U_FAILURE(error)) {
      // If the specified locale opened successfully, return
      return;
    }
    m_errcode.setError(error);
  }
  // If the empty string was given or if the specified locale did
  // not open successfully, so fall back to using the default locale
  m_errcode.setError(U_USING_FALLBACK_WARNING);
  if (m_ucoll) {
    ucol_close(m_ucoll);
    m_ucoll = NULL;
  }
  UErrorCode errcode = U_ZERO_ERROR;
  m_locale = String(uloc_getDefault(), CopyString);
  m_ucoll = ucol_open(m_locale.data(), &errcode);
  if (U_FAILURE(errcode)) {
    m_errcode.setError(errcode,
                       "collator_create: unable to open ICU collator");
    if (m_ucoll) {
      ucol_close(m_ucoll);
      m_ucoll = NULL;
    }
  }
}

bool c_Collator::t_asort(VRefParam arr,
                         int64_t sort_flag /* = q_Collator$$SORT_REGULAR */) {
  if (!arr.isArray()) {
    throw_expected_array_exception();
    return false;
  }
  if (!m_ucoll) {
    raise_warning("asort called on uninitialized Collator object");
    return false;
  }
  m_errcode.clearError();
  bool ret = collator_asort(arr, sort_flag, true, m_ucoll, &m_errcode);
  if (U_FAILURE(m_errcode.getErrorCode())) {
    return false;
  }
  return ret;
}

Variant c_Collator::t_compare(const String& str1, const String& str2) {
  if (!m_ucoll) {
    raise_warning("compare called on uninitialized Collator object");
    return 0;
  }
  UChar* ustr1 = NULL;
  UChar* ustr2 = NULL;
  int ustr1_len = 0;
  int ustr2_len = 0;
  m_errcode.clearError();
  UErrorCode error = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&ustr1, &ustr1_len,
                             str1.data(), str1.length(),
                             &error);
  if (U_FAILURE(error)) {
    m_errcode.setError(error);
    free(ustr1);
    return false;
  }
  error = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&ustr2, &ustr2_len,
                             str2.data(), str2.length(),
                             &error);
  if (U_FAILURE(error)) {
    m_errcode.setError(error);
    free(ustr1);
    free(ustr2);
    return false;
  }
  int64_t ret = ucol_strcoll(m_ucoll, ustr1, ustr1_len, ustr2, ustr2_len);
  free(ustr1);
  free(ustr2);
  return ret;
}

Variant c_Collator::ti_create(const String& locale) {
  p_Collator c(NEWOBJ(c_Collator)());
  c.get()->t___construct(locale);
  return c;
}

int64_t c_Collator::t_getattribute(int64_t attr) {
  if (!m_ucoll) {
    raise_warning("getattribute called on uninitialized Collator object");
    return 0;
  }
  m_errcode.clearError();
  UErrorCode error = U_ZERO_ERROR;
  int64_t ret = (int64_t)ucol_getAttribute(m_ucoll, (UColAttribute)attr,
                                       &error);
  if (U_FAILURE(error)) {
    m_errcode.setError(error, "Error getting attribute value");
    return 0;
  }
  return ret;
}

int64_t c_Collator::t_geterrorcode() {
  return m_errcode.getErrorCode();
}

String c_Collator::t_geterrormessage() {
  return String(u_errorName(m_errcode.getErrorCode()), CopyString);
}

String c_Collator::t_getlocale(int64_t type /* = 0 */) {
  if (!m_ucoll) {
    raise_error("getlocale called on uninitialized Collator object");
    return "";
  }
  m_errcode.clearError();
  UErrorCode error = U_ZERO_ERROR;
  String ret(
    (char*)ucol_getLocaleByType(m_ucoll, (ULocDataLocaleType)type,
                                &error),
    CopyString);
  if (U_FAILURE(error)) {
    m_errcode.setError(error, "Error getting locale by type");
    return "";
  }
  return ret;
}

int64_t c_Collator::t_getstrength() {
  if (!m_ucoll) {
    raise_warning("getstrength called on uninitialized Collator object");
    return 0;
  }
  return ucol_getStrength(m_ucoll);
}

bool c_Collator::t_setattribute(int64_t attr, int64_t val) {
  if (!m_ucoll) {
    raise_warning("setattribute called on uninitialized Collator object");
    return false;
  }
  m_errcode.clearError();
  UErrorCode error = U_ZERO_ERROR;
  ucol_setAttribute(m_ucoll, (UColAttribute)attr,
                    (UColAttributeValue)val, &error);
  if (U_FAILURE(error)) {
    m_errcode.setError(error, "Error setting attribute value");
    return false;
  }
  return true;
}

bool c_Collator::t_setstrength(int64_t strength) {
  if (!m_ucoll) {
    raise_warning("setstrength called on uninitialized Collator object");
    return false;
  }
  ucol_setStrength(m_ucoll, (UCollationStrength)strength);
  return true;
}

typedef struct _collator_sort_key_index {
  char* key;       /* pointer to sort key */
  ssize_t valPos;  /* position of the original array element */
} collator_sort_key_index_t;

static const int32_t DEF_SORT_KEYS_BUF_SIZE = 1048576;
static const int32_t DEF_SORT_KEYS_BUF_INCREMENT = 1048576;

static const int32_t DEF_SORT_KEYS_INDX_BUF_SIZE = 1048576;
static const int32_t DEF_SORT_KEYS_INDX_BUF_INCREMENT = 1048576;

static const int32_t DEF_UTF16_BUF_SIZE = 1024;

/* {{{ collator_cmp_sort_keys
 * Compare sort keys
 */
static int collator_cmp_sort_keys(const void* p1, const void* p2, const void*) {
  char* key1 = ((collator_sort_key_index_t*)p1)->key;
  char* key2 = ((collator_sort_key_index_t*)p2)->key;
  return strcmp( key1, key2 );
}

bool c_Collator::t_sortwithsortkeys(VRefParam arr) {
  char*       sortKeyBuf = NULL; /* buffer to store sort keys */
  int32_t     sortKeyBufSize = DEF_SORT_KEYS_BUF_SIZE; /* buffer size */
  ptrdiff_t   sortKeyBufOffset = 0; /* pos in buffer to store sort key */
  int32_t     sortKeyLen = 0; /* the length of currently processing key */
  int32_t     bufLeft = 0;
  int32_t     bufIncrement = 0;

  /* buffer to store 'indexes' which will be passed to 'qsort' */
  collator_sort_key_index_t* sortKeyIndxBuf = NULL;
  int32_t     sortKeyIndxBufSize   = DEF_SORT_KEYS_INDX_BUF_SIZE;
  int32_t     sortKeyIndxSize      = sizeof( collator_sort_key_index_t );

  int32_t     sortKeyCount         = 0;
  int32_t     j                    = 0;

  /* tmp buffer to hold current processing string in utf-16 */
  UChar*      utf16_buf            = NULL;
  /* the length of utf16_buf */
  int         utf16_buf_size       = DEF_UTF16_BUF_SIZE;
  /* length of converted string */
  int         utf16_len            = 0;

  m_errcode.clearError();

  /*
   * Sort specified array.
   */
  if (!arr.isArray()) {
    return true;
  }
  Array hash = arr.toArray();
  if (hash.size() == 0) {
    return true;
  }

  /* Create bufers */
  sortKeyBuf     = (char*)calloc(sortKeyBufSize, sizeof(char));
  sortKeyIndxBuf = (collator_sort_key_index_t*)malloc(sortKeyIndxBufSize);
  utf16_buf      = (UChar*)malloc(utf16_buf_size);

  /* Iterate through input hash and create a sort key for each value. */
  for (ssize_t pos = hash->iter_begin(); pos != ArrayData::invalid_index;
       pos = hash->iter_advance(pos)) {
    /* Convert current hash item from UTF-8 to UTF-16LE and save the result
     * to utf16_buf. */
    utf16_len = utf16_buf_size;
    /* Process string values only. */
    Variant val(hash->getValue(pos));
    if (val.isString()) {
      String str = val.toString();
      m_errcode.clearError();
      UErrorCode error = U_ZERO_ERROR;
      intl_convert_utf8_to_utf16(&utf16_buf, &utf16_len, str.data(),
                                 str.size(), &error);
      if (U_FAILURE(error)) {
        m_errcode.setError(error, "Sort with sort keys failed");
        if (utf16_buf) {
          free(utf16_buf);
        }
        free(sortKeyIndxBuf);
        free(sortKeyBuf);
        return false;
      }
    } else {
      /* Set empty string */
      utf16_len = 0;
      utf16_buf[utf16_len] = 0;
    }

    if ((utf16_len + 1) > utf16_buf_size) {
      utf16_buf_size = utf16_len + 1;
    }

    /* Get sort key, reallocating the buffer if needed. */
    bufLeft = sortKeyBufSize - sortKeyBufOffset;

    sortKeyLen = ucol_getSortKey(m_ucoll,
                    utf16_buf,
                    utf16_len,
                    (uint8_t*)sortKeyBuf + sortKeyBufOffset,
                    bufLeft);

    /* check for sortKeyBuf overflow, increasing its size of the buffer if
       needed */
    if (sortKeyLen > bufLeft) {
      bufIncrement = ( sortKeyLen > DEF_SORT_KEYS_BUF_INCREMENT ) ?
        sortKeyLen : DEF_SORT_KEYS_BUF_INCREMENT;
      sortKeyBufSize += bufIncrement;
      bufLeft += bufIncrement;
      sortKeyBuf = (char*)realloc(sortKeyBuf, sortKeyBufSize);
      sortKeyLen = ucol_getSortKey(m_ucoll, utf16_buf, utf16_len,
                                   (uint8_t*)sortKeyBuf + sortKeyBufOffset,
                                   bufLeft);
    }

    /* check sortKeyIndxBuf overflow, increasing its size of the buffer if
       needed */
    if ((sortKeyCount + 1) * sortKeyIndxSize > sortKeyIndxBufSize) {
      bufIncrement = (sortKeyIndxSize > DEF_SORT_KEYS_INDX_BUF_INCREMENT) ?
        sortKeyIndxSize : DEF_SORT_KEYS_INDX_BUF_INCREMENT;
      sortKeyIndxBufSize += bufIncrement;
      sortKeyIndxBuf = (collator_sort_key_index_t*)realloc(sortKeyIndxBuf,
                                                           sortKeyIndxBufSize);
    }
    sortKeyIndxBuf[sortKeyCount].key = (char*)sortKeyBufOffset;
    sortKeyIndxBuf[sortKeyCount].valPos = pos;
    sortKeyBufOffset += sortKeyLen;
    ++sortKeyCount;
  }

  /* update ptrs to point to valid keys. */
  for( j = 0; j < sortKeyCount; j++ )
    sortKeyIndxBuf[j].key = sortKeyBuf + (ptrdiff_t)sortKeyIndxBuf[j].key;

  /* sort it */
  zend_qsort(sortKeyIndxBuf, sortKeyCount, sortKeyIndxSize,
             collator_cmp_sort_keys, NULL);

  /* for resulting hash we'll assign new hash keys rather then reordering */
  Array sortedHash = Array::Create();

  for (j = 0; j < sortKeyCount; j++) {
    sortedHash.append(hash->getValue(sortKeyIndxBuf[j].valPos));
  }

  /* Save sorted hash into return variable. */
  arr = sortedHash;

  if (utf16_buf)
    free(utf16_buf);

  free(sortKeyIndxBuf);
  free(sortKeyBuf);

  return true;
}

bool c_Collator::t_sort(VRefParam arr,
                        int64_t sort_flag /* = q_Collator$$SORT_REGULAR */) {
  if (!arr.isArray()) {
    throw_expected_array_exception();
    return false;
  }
  if (!m_ucoll) {
    raise_warning("sort called on uninitialized Collator object");
    return false;
  }
  m_errcode.clearError();
  bool ret = collator_sort(arr, sort_flag, true, m_ucoll, &m_errcode);
  if (U_FAILURE(m_errcode.getErrorCode())) {
    return false;
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_COLL(obj)                                    \
  c_Collator *coll = NULL;                                 \
  if (obj.isObject()) {                                    \
    coll = obj.toObject().getTyped<c_Collator>();          \
  }                                                        \
  if (!coll) {                                             \
    raise_warning("Expecting collator object");            \
    return false;                                          \
  }                                                        \

Variant f_collator_asort(CVarRef obj, VRefParam arr,
                         int64_t sort_flag /* = q_Collator$$SORT_REGULAR */) {
  CHECK_COLL(obj);
  return coll->t_asort(ref(arr), sort_flag);
}

Variant f_collator_compare(CVarRef obj, const String& str1,
                           const String& str2) {
  CHECK_COLL(obj);
  return coll->t_compare(str1, str2);
}

Variant f_collator_create(const String& locale) {
  return c_Collator::ti_create(locale);
}

Variant f_collator_get_attribute(CVarRef obj, int64_t attr) {
  CHECK_COLL(obj);
  return coll->t_getattribute(attr);
}

Variant f_collator_get_error_code(CVarRef obj) {
  CHECK_COLL(obj);
  return coll->t_geterrorcode();
}

Variant f_collator_get_error_message(CVarRef obj) {
  CHECK_COLL(obj);
  return coll->t_geterrormessage();
}

Variant f_collator_get_locale(CVarRef obj, int64_t type /* = 0 */) {
  CHECK_COLL(obj);
  return coll->t_getlocale(type);
}

Variant f_collator_get_strength(CVarRef obj) {
  CHECK_COLL(obj);
  return coll->t_getstrength();
}

Variant f_collator_set_attribute(CVarRef obj, int64_t attr, int64_t val) {
  CHECK_COLL(obj);
  return coll->t_setattribute(attr, val);
}

Variant f_collator_set_strength(CVarRef obj, int64_t strength) {
  CHECK_COLL(obj);
  return coll->t_setstrength(strength);
}

Variant f_collator_sort_with_sort_keys(CVarRef obj, VRefParam arr) {
  CHECK_COLL(obj);
  return coll->t_sortwithsortkeys(ref(arr));
}

Variant f_collator_sort(CVarRef obj, VRefParam arr,
                        int64_t sort_flag /* = q_Collator$$SORT_REGULAR */) {
  CHECK_COLL(obj);
  return coll->t_sort(ref(arr), sort_flag);
}

///////////////////////////////////////////////////////////////////////////////

const int64_t q_Normalizer$$NONE     = UNORM_NONE;
const int64_t q_Normalizer$$FORM_D   = UNORM_NFD;
const int64_t q_Normalizer$$NFD      = UNORM_NFD;
const int64_t q_Normalizer$$FORM_KD  = UNORM_NFKD;
const int64_t q_Normalizer$$NFKD     = UNORM_NFKD;
const int64_t q_Normalizer$$FORM_C   = UNORM_NFC;
const int64_t q_Normalizer$$NFC      = UNORM_NFC;
const int64_t q_Normalizer$$FORM_KC  = UNORM_NFKC;
const int64_t q_Normalizer$$NFKC     = UNORM_NFKC;

///////////////////////////////////////////////////////////////////////////////

c_Normalizer::c_Normalizer(Class* cb) : ExtObjectData(cb) {
}

c_Normalizer::~c_Normalizer() {
}

void c_Normalizer::t___construct() {
}

///////////////////////////////////////////////////////////////////////////////

Variant c_Normalizer::ti_isnormalized(const String& input,
                                      int64_t form /* = q_Normalizer$$FORM_C */) {
  s_intl_error->clearError();

  switch (form) {
  case UNORM_NFD:
  case UNORM_NFKD:
  case UNORM_NFC:
  case UNORM_NFKC:
    break;
  default:
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "normalizer_isnormalized: "
                           "illegal normalization form");
    return uninit_null();
  }

  /* First convert the string to UTF-16. */
  UChar* uinput = NULL; int uinput_len = 0;
  UErrorCode status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&uinput, &uinput_len, input.data(), input.size(),
                             &status);

  if (U_FAILURE(status)) {
    s_intl_error->setError(status, "Error converting string to UTF-16.");
    free(uinput);
    return false;
  }

  /* test string */
  UBool uret = unorm_isNormalizedWithOptions(uinput, uinput_len,
                                             (UNormalizationMode)form,
                                             (int32_t)0, &status);
  free(uinput);

  /* Bail out if an unexpected error occured. */
  if (U_FAILURE(status)) {
    s_intl_error->setError(status, "Error testing if string is the given "
                                   "normalization form.");
    return false;
  }

  return uret;
}

Variant c_Normalizer::ti_normalize(const String& input,
                                   int64_t form /* = q_Normalizer$$FORM_C */) {
  s_intl_error->clearError();

  int expansion_factor = 1;
  switch(form) {
  case UNORM_NONE:
  case UNORM_NFC:
  case UNORM_NFKC:
    break;
  case UNORM_NFD:
  case UNORM_NFKD:
    expansion_factor = 3;
    break;
  default:
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "normalizer_normalize: "
                           "illegal normalization form");
    return uninit_null();
  }

  /* First convert the string to UTF-16. */
  UChar* uinput = NULL; int uinput_len = 0;
  UErrorCode status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&uinput, &uinput_len, input.data(), input.size(),
                             &status);

  if (U_FAILURE(status)) {
    s_intl_error->setError(status, "Error converting string to UTF-16.");
    free(uinput);
    return uninit_null();
  }

  /* Allocate memory for the destination buffer for normalization */
  int uret_len = uinput_len * expansion_factor;
  UChar *uret_buf = (UChar*)malloc((uret_len + 1) * sizeof(UChar));

  /* normalize */
  int size_needed = unorm_normalize(uinput, uinput_len,
                                    (UNormalizationMode)form, (int32_t) 0,
                                    uret_buf, uret_len, &status);

  /* Bail out if an unexpected error occured.
   * (U_BUFFER_OVERFLOW_ERROR means that *target buffer is not large enough).
   * (U_STRING_NOT_TERMINATED_WARNING usually means that the input string
   * is empty).
   */
  if (U_FAILURE(status) &&
      status != U_BUFFER_OVERFLOW_ERROR &&
      status != U_STRING_NOT_TERMINATED_WARNING) {
    free(uret_buf);
    free(uinput);
    return uninit_null();
  }

  if (size_needed > uret_len) {
    /* realloc does not seem to work properly - memory is corrupted
     * uret_buf =  eurealloc(uret_buf, size_needed + 1); */
    free(uret_buf);
    uret_buf = (UChar*)malloc((size_needed + 1) * sizeof(UChar));
    uret_len = size_needed;

    status = U_ZERO_ERROR;

    /* try normalize again */
    size_needed = unorm_normalize( uinput, uinput_len,
                                   (UNormalizationMode)form, (int32_t) 0,
                                   uret_buf, uret_len, &status);

    /* Bail out if an unexpected error occured. */
    if (U_FAILURE(status)) {
      /* Set error messages. */
      s_intl_error->setError(status, "Error normalizing string");
      free(uret_buf);
      free(uinput);
      return uninit_null();
    }
  }

  free(uinput);

  /* the buffer we actually used */
  uret_len = size_needed;

  /* Convert normalized string from UTF-16 to UTF-8. */
  char* ret_buf = NULL; int ret_len = 0;
  intl_convert_utf16_to_utf8(&ret_buf, &ret_len, uret_buf, uret_len, &status);
  free(uret_buf);
  if (U_FAILURE(status)) {
    s_intl_error->setError(status, "normalizer_normalize: "
                                   "error converting normalized text UTF-8");
    return uninit_null();
  }

  return String(ret_buf, ret_len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
