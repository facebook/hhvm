/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_intl.h>
#include <runtime/ext/ext_array.h> // for throw_bad_array_exception
#include <runtime/base/util/request_local.h>
#include <runtime/base/zend/intl_convert.h>
#include <runtime/base/zend/zend_collator.h>
#include <runtime/base/zend/zend_qsort.h>
#include <unicode/uidna.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h> // icu
#include <unicode/uclean.h> // icu
#include <unicode/putil.h> // icu
#include <unicode/utypes.h>
#include <unicode/unorm.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(idn);
///////////////////////////////////////////////////////////////////////////////

int64 f_intl_get_error_code() {
  return s_intl_error->m_error.code;
}

String f_intl_get_error_message() {
  if (!s_intl_error->m_error.custom_error_message.empty()) {
    return s_intl_error->m_error.custom_error_message;
  }
  return String(u_errorName(s_intl_error->m_error.code), AttachLiteral);
}

String f_intl_error_name(int64 error_code) {
  return String(u_errorName((UErrorCode)error_code), AttachLiteral);
}

bool f_intl_is_failure(int64 error_code) {
  if (U_FAILURE((UErrorCode)error_code)) return true;
  return false;
}

///////////////////////////////////////////////////////////////////////////////

const int64 q_Collator_SORT_REGULAR = 0;
const int64 q_Collator_SORT_STRING = 1;
const int64 q_Collator_SORT_NUMERIC = 2;
const int64 q_Collator_FRENCH_COLLATION = UCOL_FRENCH_COLLATION;
const int64 q_Collator_ALTERNATE_HANDLING = UCOL_ALTERNATE_HANDLING;
const int64 q_Collator_CASE_FIRST = UCOL_CASE_FIRST;
const int64 q_Collator_CASE_LEVEL = UCOL_CASE_LEVEL;
const int64 q_Collator_NORMALIZATION_MODE = UCOL_NORMALIZATION_MODE;
const int64 q_Collator_STRENGTH = UCOL_STRENGTH;
const int64 q_Collator_HIRAGANA_QUATERNARY_MODE = UCOL_HIRAGANA_QUATERNARY_MODE;
const int64 q_Collator_NUMERIC_COLLATION = UCOL_NUMERIC_COLLATION;
const int64 q_Collator_DEFAULT_VALUE = UCOL_DEFAULT;
const int64 q_Collator_PRIMARY = UCOL_PRIMARY;
const int64 q_Collator_SECONDARY = UCOL_SECONDARY;
const int64 q_Collator_TERTIARY = UCOL_TERTIARY;
const int64 q_Collator_DEFAULT_STRENGTH = UCOL_DEFAULT_STRENGTH;
const int64 q_Collator_QUATERNARY = UCOL_QUATERNARY;
const int64 q_Collator_IDENTICAL = UCOL_IDENTICAL;
const int64 q_Collator_OFF = UCOL_OFF;
const int64 q_Collator_ON = UCOL_ON;
const int64 q_Collator_SHIFTED = UCOL_SHIFTED;
const int64 q_Collator_NON_IGNORABLE = UCOL_NON_IGNORABLE;
const int64 q_Collator_LOWER_FIRST = UCOL_LOWER_FIRST;
const int64 q_Collator_UPPER_FIRST = UCOL_UPPER_FIRST;

///////////////////////////////////////////////////////////////////////////////

c_Collator::c_Collator() : m_locale(), m_ucoll(NULL), m_errcode() {
}

c_Collator::~c_Collator() {
  if (m_ucoll) {
    ucol_close(m_ucoll);
    m_ucoll = NULL;
  }
}

void c_Collator::t___construct(CStrRef locale) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::__construct);
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

bool c_Collator::t_asort(Variant arr,
                         int64 sort_flag /* = q_Collator_SORT_REGULAR */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::asort);
  if (!arr.isArray()) {
    throw_bad_array_exception();
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

Variant c_Collator::t_compare(CStrRef str1, CStrRef str2) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::compare);
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
  int64 ret = ucol_strcoll(m_ucoll, ustr1, ustr1_len, ustr2, ustr2_len);
  free(ustr1);
  free(ustr2);
  return ret;
}

Variant c_Collator::ti_create(const char* cls, CStrRef locale) {
  STATIC_METHOD_INJECTION_BUILTIN(Collator, Collator::create);
  return (NEWOBJ(c_Collator)())->create(locale);
}

int64 c_Collator::t_getattribute(int64 attr) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::getattribute);
  if (!m_ucoll) {
    raise_warning("getattribute called on uninitialized Collator object");
    return 0;
  }
  m_errcode.clear();
  int64 ret = (int64)ucol_getAttribute(m_ucoll, (UColAttribute)attr,
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

int64 c_Collator::t_geterrorcode() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::geterrorcode);
  return m_errcode.code;
}

String c_Collator::t_geterrormessage() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::geterrormessage);
  return String(u_errorName(m_errcode.code), AttachLiteral);
}

String c_Collator::t_getlocale(int64 type /* = 0 */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::getlocale);
  if (!m_ucoll) {
    raise_warning("getlocale called on uninitialized Collator object");
    return "";
  }
  m_errcode.clear();
  String ret(
    (char*)ucol_getLocaleByType(m_ucoll, (ULocDataLocaleType)type,
                                &(m_errcode.code)),
    AttachLiteral);
  if (U_FAILURE(m_errcode.code)) {
    m_errcode.custom_error_message = "Error getting locale by type";
    s_intl_error->m_error.code = m_errcode.code;
    s_intl_error->m_error.custom_error_message =
      m_errcode.custom_error_message;
    return "";
  }
  return ret;
}

int64 c_Collator::t_getstrength() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::getstrength);
  if (!m_ucoll) {
    raise_warning("getstrength called on uninitialized Collator object");
    return 0;
  }
  return ucol_getStrength(m_ucoll);
}

bool c_Collator::t_setattribute(int64 attr, int64 val) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::setattribute);
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

bool c_Collator::t_setstrength(int64 strength) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::setstrength);
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

bool c_Collator::t_sortwithsortkeys(Variant arr) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::sortwithsortkeys);
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

bool c_Collator::t_sort(Variant arr,
                        int64 sort_flag /* = q_Collator_SORT_REGULAR */) {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::sort);
  if (!arr.isArray()) {
    throw_bad_array_exception();
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

Variant c_Collator::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Collator, Collator::__destruct);
  return null;
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

Variant f_collator_asort(CVarRef obj, Variant arr, int64 sort_flag /* = 0 */) {
  CHECK_COLL(obj);
  return coll->t_asort(ref(arr), sort_flag);
}

Variant f_collator_compare(CVarRef obj, CStrRef str1, CStrRef str2) {
  CHECK_COLL(obj);
  return coll->t_compare(str1, str2);
}

Variant f_collator_create(CStrRef locale) {
  return (NEWOBJ(c_Collator)())->create(locale);
}

Variant f_collator_get_attribute(CVarRef obj, int64 attr) {
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

Variant f_collator_get_locale(CVarRef obj, int64 type /* = 0 */) {
  CHECK_COLL(obj);
  return coll->t_getlocale(type);
}

Variant f_collator_get_strength(CVarRef obj) {
  CHECK_COLL(obj);
  return coll->t_getstrength();
}

Variant f_collator_set_attribute(CVarRef obj, int64 attr, int64 val) {
  CHECK_COLL(obj);
  return coll->t_setattribute(attr, val);
}

Variant f_collator_set_strength(CVarRef obj, int64 strength) {
  CHECK_COLL(obj);
  return coll->t_setstrength(strength);
}

Variant f_collator_sort_with_sort_keys(CVarRef obj, Variant arr) {
  CHECK_COLL(obj);
  return coll->t_sortwithsortkeys(ref(arr));
}

Variant f_collator_sort(CVarRef obj, Variant arr, int64 sort_flag /* = 0 */) {
  CHECK_COLL(obj);
  return coll->t_sort(ref(arr), sort_flag);
}

///////////////////////////////////////////////////////////////////////////////

const int64 q_Locale_ACTUAL_LOCALE = 0;
const int64 q_Locale_VALID_LOCALE = 1;

///////////////////////////////////////////////////////////////////////////////

c_Locale::c_Locale() {
}

c_Locale::~c_Locale() {
}

void c_Locale::t___construct() {
}

Variant c_Locale::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Locale, Locale::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////

const int64 q_Normalizer_NONE     = UNORM_NONE;
const int64 q_Normalizer_FORM_D   = UNORM_NFD;
const int64 q_Normalizer_NFD      = UNORM_NFD;
const int64 q_Normalizer_FORM_KD  = UNORM_NFKD;
const int64 q_Normalizer_NFKD     = UNORM_NFKD;
const int64 q_Normalizer_FORM_C   = UNORM_NFC;
const int64 q_Normalizer_NFC      = UNORM_NFC;
const int64 q_Normalizer_FORM_KC  = UNORM_NFKC;
const int64 q_Normalizer_NFKC     = UNORM_NFKC;

///////////////////////////////////////////////////////////////////////////////

c_Normalizer::c_Normalizer() {
}

c_Normalizer::~c_Normalizer() {
}

void c_Normalizer::t___construct() {
}

Variant c_Normalizer::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(Normalizer, Normalizer::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////

Variant c_Normalizer::ti_isnormalized(const char* cls , CStrRef input,
                                      int64 form /* = q_Normalizer_FORM_C */) {
  STATIC_METHOD_INJECTION_BUILTIN(Normalizer, Normalizer::isnormalized);
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
    return null;
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

Variant c_Normalizer::ti_normalize(const char* cls , CStrRef input,
                                   int64 form /* = q_Normalizer_FORM_C */) {
  STATIC_METHOD_INJECTION_BUILTIN(Normalizer, Normalizer::normalize);
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
    return null;
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
    return null;
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
    return null;
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
      return null;
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
    return null;
  }

  return String(ret_buf, ret_len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////

enum {
  INTL_IDN_TO_ASCII = 0,
  INTL_IDN_TO_UTF8
};

Variant php_intl_idn_to(CStrRef domain, Variant errorcode, int mode) {
  long option = 0;
  UChar* ustring = NULL;
  int ustring_len = 0;
  UErrorCode status;
  char     *converted_utf8 = NULL;
  int32_t   converted_utf8_len;
  UChar*    converted = NULL;
  int32_t   converted_ret_len;

  // Convert the string to UTF-16
  status = U_ZERO_ERROR;
  intl_convert_utf8_to_utf16(&ustring, &ustring_len,
      (char*)domain.data(), domain.size(), &status);
  if (U_FAILURE(status)) {
    free(ustring);
    errorcode = status;
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
      errorcode = U_MEMORY_ALLOCATION_ERROR;
      return false;
    }
    if (mode == INTL_IDN_TO_ASCII) {
      converted_ret_len = uidna_IDNToASCII(ustring,
          ustring_len, converted, converted_len,
          (int32_t)option, &parse_error, &status);
    } else {
      converted_ret_len = uidna_IDNToUnicode(ustring,
          ustring_len, converted, converted_len,
          (int32_t)option, &parse_error, &status);
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
    errorcode = status;
    return false;
  }

  // Convert the string back to UTF-8
  status = U_ZERO_ERROR;
  intl_convert_utf16_to_utf8(&converted_utf8, &converted_utf8_len,
      converted, converted_ret_len, &status);
  free(converted);
  if (U_FAILURE(status)) {
    free(converted_utf8);
    errorcode = status;
    return false;
  }

  // Return the string
  return String(converted_utf8, converted_utf8_len, AttachString);
}

Variant f_idn_to_ascii(CStrRef domain, Variant errorcode /* = null */) {
  return php_intl_idn_to(domain, ref(errorcode), INTL_IDN_TO_ASCII);
}

Variant f_idn_to_unicode(CStrRef domain, Variant errorcode /* = null */) {
  return php_intl_idn_to(domain, ref(errorcode), INTL_IDN_TO_UTF8);
}

Variant f_idn_to_utf8(CStrRef domain, Variant errorcode /* = null */) {
  return php_intl_idn_to(domain, ref(errorcode), INTL_IDN_TO_UTF8);
}

///////////////////////////////////////////////////////////////////////////////
}

