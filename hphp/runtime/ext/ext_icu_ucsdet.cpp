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

#include "hphp/runtime/ext/ext_icu_ucsdet.h"
#include <unicode/unistr.h>

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DEFAULT_EXTENSION(icu_ucsdet);

c_EncodingDetector::c_EncodingDetector(Class* cb) :
    ExtObjectData(cb) {
  UErrorCode status = U_ZERO_ERROR;
  m_encoding_detector = ucsdet_open(&status);

  if (U_FAILURE(status)) {
    throw Exception("Could not open spoof checker, error %d (%s)",
                    status, u_errorName(status));
  }
}

void c_EncodingDetector::t___construct() {
}

void c_EncodingDetector::t_settext(const String& text) {
  UErrorCode status = U_ZERO_ERROR;
  m_text = text;
  ucsdet_setText(
    m_encoding_detector,
    m_text.data(),
    m_text.length(),
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not set encoding detector text to [%s], error %d (%s)",
      text.c_str(), status, u_errorName(status));
  }
}

void c_EncodingDetector::t_setdeclaredencoding(const String& text) {
  UErrorCode status = U_ZERO_ERROR;
  m_declaredencoding = text;
  ucsdet_setDeclaredEncoding(
    m_encoding_detector,
    m_declaredencoding.data(),
    m_declaredencoding.length(),
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not set encoding detector declared encoding to [%s], error %d (%s)",
      text.c_str(), status, u_errorName(status));
  }
}

Object c_EncodingDetector::t_detect() {
  UErrorCode status = U_ZERO_ERROR;
  const UCharsetMatch* match = ucsdet_detect(
    m_encoding_detector,
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not detect encoding, error %d (%s)", status, u_errorName(status));
  }

  p_EncodingMatch matchobj = NEWOBJ(c_EncodingMatch)();
  matchobj->m_encoding_match = match;
  return matchobj;
}

Array c_EncodingDetector::t_detectall() {
  int32_t matchesFound;
  UErrorCode status = U_ZERO_ERROR;
  const UCharsetMatch** matches = ucsdet_detectAll(
    m_encoding_detector,
    &matchesFound,
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not detect all encodings, error %d (%s)", status, u_errorName(status));
  }

  Array ret = Array::Create();
  int32_t i;
  for (i = 0; i < matchesFound; i++) {
    p_EncodingMatch matchobj = NEWOBJ(c_EncodingMatch)();
    matchobj->m_encoding_match = matches[i];
    ret.append(matchobj);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void c_EncodingMatch::t___construct() {
}

void c_EncodingMatch::validate() {
  if (m_encoding_match == 0) {
    throw Exception("EncodingMatch object is not valid! Call isValid() before using.");
  }
}

bool c_EncodingMatch::t_isvalid() {
  return m_encoding_match != 0;
}

String c_EncodingMatch::t_getencoding() {
  validate();

  UErrorCode status = U_ZERO_ERROR;
  const char* encoding = ucsdet_getName(
    m_encoding_match,
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not get encoding for match, error %d (%s)",
      status, u_errorName(status));
  }
  return String(encoding);
}

int64_t c_EncodingMatch::t_getconfidence() {
  validate();

  UErrorCode status = U_ZERO_ERROR;
  int32_t confidence = ucsdet_getConfidence(
    m_encoding_match,
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not get confidence for match, error %d (%s)",
      status, u_errorName(status));
  }
  return confidence;
}

String c_EncodingMatch::t_getlanguage() {
  validate();

  UErrorCode status = U_ZERO_ERROR;
  const char* language = ucsdet_getLanguage(
    m_encoding_match,
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not get language for match, error %d (%s)",
      status, u_errorName(status));
  }
  return String(language);
}

String c_EncodingMatch::t_getutf8() {
  validate();

  UErrorCode status;
  icu::UnicodeString ustr;
  int32_t ustrSize = ustr.getCapacity();

  do {
    status = U_ZERO_ERROR;
    UChar* buf = ustr.getBuffer(ustrSize);
    ustrSize = ucsdet_getUChars(
      m_encoding_match,
      buf,
      ustrSize,
      &status);
    ustr.releaseBuffer();
    ustr.truncate(ustrSize);
  } while (status == U_BUFFER_OVERFLOW_ERROR);

  if (U_FAILURE(status)) {
    throw Exception(
      "Could not get UTF-8 for match, error %d (%s)",
      status, u_errorName(status));
  }
  std::string utf8str;
  ustr.toUTF8String(utf8str);
  return String(utf8str);
}

///////////////////////////////////////////////////////////////////////////////
}
