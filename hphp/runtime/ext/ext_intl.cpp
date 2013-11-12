/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include <unicode/uidna.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h> // icu
#include <unicode/uclean.h> // icu
#include <unicode/putil.h> // icu
#include <unicode/utypes.h>
#include <unicode/unorm.h>

#include "hphp/system/systemlib.h"

#ifdef UIDNA_INFO_INITIALIZER
#define HAVE_46_API 1 /* has UTS#46 API (introduced in ICU 4.6) */
#endif

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(idn);
///////////////////////////////////////////////////////////////////////////////

int64_t f_intl_get_error_code() {
  return s_intl_error->m_error.code;
}

String f_intl_get_error_message() {
  if (!s_intl_error->m_error.custom_error_message.empty()) {
    return s_intl_error->m_error.custom_error_message;
  }
  return String(u_errorName(s_intl_error->m_error.code), CopyString);
}

String f_intl_error_name(int64_t error_code) {
  return String(u_errorName((UErrorCode)error_code), CopyString);
}

bool f_intl_is_failure(int64_t error_code) {
  if (U_FAILURE((UErrorCode)error_code)) return true;
  return false;
}

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
  m_errcode.clear();
  if (!locale.empty()) {
    m_locale = locale;
    m_ucoll = ucol_open(locale.data(), &(m_errcode.code));
    if (!U_FAILURE(m_errcode.code)) {
      // If the specified locale opened successfully, return
      s_intl_error->m_error.clear();
      s_intl_error->m_error.code = m_errcode.code;
      return;
    }
  }
  // If the empty string was given or if the specified locale did
  // not open successfully, so fall back to using the default locale
  m_errcode.code = U_USING_FALLBACK_WARNING;
  s_intl_error->m_error.clear();
  s_intl_error->m_error.code = m_errcode.code;
  if (m_ucoll) {
    ucol_close(m_ucoll);
    m_ucoll = NULL;
  }
  UErrorCode errcode = U_ZERO_ERROR;
  m_locale = String(uloc_getDefault(), CopyString);
  m_ucoll = ucol_open(m_locale.data(), &errcode);
  if (U_FAILURE(errcode)) {
    m_errcode.code = errcode;
    m_errcode.custom_error_message =
      "collator_create: unable to open ICU collator";
    s_intl_error->m_error.clear();
    s_intl_error->m_error.code = m_errcode.code;
    s_intl_error->m_error.custom_error_message = m_errcode.custom_error_message;
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
  m_errcode.clear();
  bool ret = collator_asort(arr, sort_flag, true, m_ucoll, &m_errcode);
  s_intl_error->m_error.clear();
  s_intl_error->m_error.code = m_errcode.code;
  s_intl_error->m_error.custom_error_message = m_errcode.custom_error_message;
  if (U_FAILURE(m_errcode.code)) {
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
  m_errcode.clear();
  intl_convert_utf8_to_utf16(&ustr1, &ustr1_len,
                             str1.data(), str1.length(),
                             &(m_errcode.code));
  if (U_FAILURE(m_errcode.code)) {
    free(ustr1);
    return false;
  }
  intl_convert_utf8_to_utf16(&ustr2, &ustr2_len,
                             str2.data(), str2.length(),
                             &(m_errcode.code));
  if (U_FAILURE(m_errcode.code)) {
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
  m_errcode.clear();
  int64_t ret = (int64_t)ucol_getAttribute(m_ucoll, (UColAttribute)attr,
                                       &(m_errcode.code));
  s_intl_error->m_error.clear();
  s_intl_error->m_error.code = m_errcode.code;
  if (U_FAILURE(m_errcode.code)) {
    m_errcode.custom_error_message = "Error getting attribute value";
    s_intl_error->m_error.custom_error_message = m_errcode.custom_error_message;
    return 0;
  }
  return ret;
}

int64_t c_Collator::t_geterrorcode() {
  return m_errcode.code;
}

String c_Collator::t_geterrormessage() {
  return String(u_errorName(m_errcode.code), CopyString);
}

String c_Collator::t_getlocale(int64_t type /* = 0 */) {
  if (!m_ucoll) {
    raise_warning("getlocale called on uninitialized Collator object");
    return "";
  }
  m_errcode.clear();
  String ret(
    (char*)ucol_getLocaleByType(m_ucoll, (ULocDataLocaleType)type,
                                &(m_errcode.code)),
    CopyString);
  if (U_FAILURE(m_errcode.code)) {
    m_errcode.custom_error_message = "Error getting locale by type";
    s_intl_error->m_error.code = m_errcode.code;
    s_intl_error->m_error.custom_error_message =
      m_errcode.custom_error_message;
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
  m_errcode.clear();
  ucol_setAttribute(m_ucoll, (UColAttribute)attr,
                    (UColAttributeValue)val, &(m_errcode.code));
  s_intl_error->m_error.clear();
  s_intl_error->m_error.code = m_errcode.code;
  if (U_FAILURE(m_errcode.code)) {
    m_errcode.custom_error_message = "Error setting attribute value";
    s_intl_error->m_error.custom_error_message = m_errcode.custom_error_message;
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

  m_errcode.clear();
  s_intl_error->m_error.clear();

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
      intl_convert_utf8_to_utf16(&utf16_buf, &utf16_len, str.data(),
                                 str.size(), &(m_errcode.code));
      if (U_FAILURE(m_errcode.code)) {
        m_errcode.custom_error_message = "Sort with sort keys failed";
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
  m_errcode.clear();
  bool ret = collator_sort(arr, sort_flag, true, m_ucoll, &(m_errcode));
  s_intl_error->m_error.clear();
  s_intl_error->m_error.code = m_errcode.code;
  s_intl_error->m_error.custom_error_message = m_errcode.custom_error_message;
  if (U_FAILURE(m_errcode.code)) {
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
  s_intl_error->m_error.clear();

  switch (form) {
  case UNORM_NFD:
  case UNORM_NFKD:
  case UNORM_NFC:
  case UNORM_NFKC:
    break;
  default:
    s_intl_error->m_error.code = U_ILLEGAL_ARGUMENT_ERROR;
    s_intl_error->m_error.custom_error_message =
      "normalizer_isnormalized: illegal normalization form";
    return uninit_null();
  }

  /* First convert the string to UTF-16. */
  UChar* uinput = NULL; int uinput_len = 0;
  UErrorCode status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&uinput, &uinput_len, input.data(), input.size(),
                             &status);

  if (U_FAILURE(status)) {
    s_intl_error->m_error.code = status;
    s_intl_error->m_error.custom_error_message = "Error converting string to UTF-16.";
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
    s_intl_error->m_error.code = status;
    s_intl_error->m_error.custom_error_message =
      "Error testing if string is the given normalization form.";
    return false;
  }

  return uret;
}

Variant c_Normalizer::ti_normalize(const String& input,
                                   int64_t form /* = q_Normalizer$$FORM_C */) {
  s_intl_error->m_error.clear();

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
    s_intl_error->m_error.code = U_ILLEGAL_ARGUMENT_ERROR;
    s_intl_error->m_error.custom_error_message =
      "normalizer_normalize: illegal normalization form";
    return uninit_null();
  }

  /* First convert the string to UTF-16. */
  UChar* uinput = NULL; int uinput_len = 0;
  UErrorCode status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&uinput, &uinput_len, input.data(), input.size(),
                             &status);

  if (U_FAILURE(status)) {
    s_intl_error->m_error.code = status;
    s_intl_error->m_error.custom_error_message =
        "Error converting string to UTF-16.";
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
      s_intl_error->m_error.code = status;
      s_intl_error->m_error.custom_error_message = "Error normalizing string";
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
    s_intl_error->m_error.code = status;
    s_intl_error->m_error.custom_error_message =
      "normalizer_normalize: error converting normalized text UTF-8";
    return uninit_null();
  }

  return String(ret_buf, ret_len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////

enum IdnVariant {
  INTL_IDN_VARIANT_2003 = 0,
  INTL_IDN_VARIANT_UTS46
};

enum {
  INTL_IDN_TO_ASCII = 0,
  INTL_IDN_TO_UTF8
};

#ifdef HAVE_46_API

const StaticString
  s_result("result"),
  s_isTransitionalDifferent("isTransitionalDifferent"),
  s_errors("errors");

static Variant php_intl_idn_to_46(const String& domain, int64_t options,
                                  IdnVariant idn_variant, VRefParam idna_info,
                                  int mode) {
  int32_t     converted_capacity;
  char        *converted = NULL;
  int32_t     converted_len;
  UIDNA       *uts46;
  UIDNAInfo   info = UIDNA_INFO_INITIALIZER;
  UErrorCode  status = U_ZERO_ERROR;

  // Get UIDNA instance which implements UTS #46.
  uts46 = uidna_openUTS46(options, &status);
  SCOPE_EXIT { uidna_close(uts46); };
  if (U_FAILURE(status)) return false;

  // Call the appropriate IDN function
  status = U_ZERO_ERROR;
  converted_capacity = 255; // no domain name may exceed this
  String result(converted_capacity, ReserveString); // reserves converted_capacity+1 characters.
  converted = result.bufferSlice().ptr;
  if (mode == INTL_IDN_TO_ASCII) {
    converted_len = uidna_nameToASCII_UTF8(uts46, (char*)domain.data(), domain.size(),
      converted, converted_capacity, &info, &status);
  } else {
    converted_len = uidna_nameToUnicodeUTF8(uts46, (char*)domain.data(), domain.size(),
      converted, converted_capacity, &info, &status);
  }
  if (U_FAILURE(status) || converted_len > converted_capacity) return false;
  if (info.errors == 0) {
    result.setSize(converted_len);
  } else {
    result.setSize(0);
  }

  // Set up the array returned in idna_info.
  ArrayInit arr(3);
  arr.set(s_result, result);
  arr.set(s_isTransitionalDifferent, info.isTransitionalDifferent);
  arr.set(s_errors, (long)info.errors);
  // As in Zend, the previous value of idn_variant is overwritten, not modified.
  idna_info = arr.create();
  if (info.errors == 0) {
    return result;
  }
  return false;
}

#endif

static Variant php_intl_idn_to(const String& domain, int64_t options,
                               IdnVariant idn_variant, VRefParam idna_info,
                               int mode) {
  UChar* ustring = NULL;
  int ustring_len = 0;
  UErrorCode status;
  char     *converted_utf8 = NULL;
  int32_t   converted_utf8_len;
  UChar*    converted = NULL;
  int32_t   converted_ret_len;

  if (idn_variant != INTL_IDN_VARIANT_2003) {
#ifdef HAVE_46_API
    if (idn_variant == INTL_IDN_VARIANT_UTS46) {
      return php_intl_idn_to_46(domain, options, idn_variant, ref(idna_info), mode);
    }
#endif
    return false;
  }

  // Convert the string to UTF-16
  status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&ustring, &ustring_len,
      (char*)domain.data(), domain.size(), &status);
  if (U_FAILURE(status)) {
    free(ustring);
    return false;
  }

  // Call the appropriate IDN function
  int converted_len = (ustring_len > 1) ? ustring_len : 1;
  for (;;) {
    UParseError parse_error;
    status = U_ZERO_ERROR;
    converted = (UChar*)malloc(sizeof(UChar)*converted_len);
    // If the malloc failed, bail out
    if (!converted) {
      free(ustring);
      return false;
    }
    if (mode == INTL_IDN_TO_ASCII) {
      converted_ret_len = uidna_IDNToASCII(ustring,
          ustring_len, converted, converted_len,
          (int32_t)options, &parse_error, &status);
    } else {
      converted_ret_len = uidna_IDNToUnicode(ustring,
          ustring_len, converted, converted_len,
          (int32_t)options, &parse_error, &status);
    }
    if (status != U_BUFFER_OVERFLOW_ERROR)
      break;
    // If we have a buffer overflow error, try again with a larger buffer
    free(converted);
    converted = NULL;
    converted_len = converted_len * 2;
  }
  free(ustring);
  if (U_FAILURE(status)) {
    free(converted);
    return false;
  }

  // Convert the string back to UTF-8
  status = U_ZERO_ERROR;
  intl_convert_utf16_to_utf8(&converted_utf8, &converted_utf8_len,
      converted, converted_ret_len, &status);
  free(converted);
  if (U_FAILURE(status)) {
    free(converted_utf8);
    return false;
  }

  // Return the string
  return String(converted_utf8, converted_utf8_len, AttachString);
}

Variant f_idn_to_ascii(const String& domain, int64_t options /* = 0 */,
                       int64_t variant /* = 0 */,
                       VRefParam idna_info /* = null */) {
  return php_intl_idn_to(domain, options, (IdnVariant)variant, idna_info,
                         INTL_IDN_TO_ASCII);
}

Variant f_idn_to_unicode(const String& domain, int64_t options /* = 0 */,
                         int64_t variant /* = 0 */,
                         VRefParam idna_info /* = null */) {
  return php_intl_idn_to(domain, options, (IdnVariant)variant, idna_info,
                         INTL_IDN_TO_UTF8);
}

Variant f_idn_to_utf8(const String& domain, int64_t options /* = 0 */,
                      int64_t variant /* = 0 */,
                      VRefParam idna_info /* = null */) {
  return php_intl_idn_to(domain, options, (IdnVariant)variant, idna_info,
                         INTL_IDN_TO_UTF8);
}

///////////////////////////////////////////////////////////////////////////////
}
