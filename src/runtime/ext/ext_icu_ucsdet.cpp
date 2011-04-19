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

#include <runtime/ext/ext_icu_ucsdet.h>
#include <unicode/unistr.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DEFAULT_EXTENSION(icu_ucsdet);

c_EncodingDetector::c_EncodingDetector() {
  UErrorCode status = U_ZERO_ERROR;
  m_encoding_detector = ucsdet_open(&status);

  if (U_FAILURE(status)) {
    throw Exception("Could not open spoof checker, error %d (%s)",
                    status, u_errorName(status));
  }
}

c_EncodingDetector::~c_EncodingDetector() {
  ucsdet_close(m_encoding_detector);
}

void c_EncodingDetector::t___construct() {
}

void c_EncodingDetector::t_settext(CStrRef text) {
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingDetector, EncodingDetector::settext);
  UErrorCode status = U_ZERO_ERROR;
  ucsdet_setText(
    m_encoding_detector,
    text.data(),
    text.length(),
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not set encoding detector text to [%s], error %d (%s)",
      text.c_str(), status, u_errorName(status));
  }
}

void c_EncodingDetector::t_setdeclaredencoding(CStrRef text) {
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingDetector, EncodingDetector::setdeclaredencoding);
  UErrorCode status = U_ZERO_ERROR;
  ucsdet_setDeclaredEncoding(
    m_encoding_detector,
    text.data(),
    text.length(),
    &status);
  if (U_FAILURE(status)) {
    throw Exception(
      "Could not set encoding detector declared encoding to [%s], error %d (%s)",
      text.c_str(), status, u_errorName(status));
  }
}

Object c_EncodingDetector::t_detect() {
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingDetector, EncodingDetector::detect);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingDetector, EncodingDetector::detectall);
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

Variant c_EncodingDetector::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingDetector, EncodingDetector::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////
c_EncodingMatch::c_EncodingMatch() :
m_encoding_match(0) {
}

c_EncodingMatch::~c_EncodingMatch() {
}

void c_EncodingMatch::t___construct() {
}

void c_EncodingMatch::validate() {
  if (m_encoding_match == 0) {
    throw Exception("EncodingMatch object is not valid! Call isValid() before using.");
  }
}

bool c_EncodingMatch::t_isvalid() {
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingMatch, EncodingMatch::isvalid);
  return m_encoding_match != 0;
}

String c_EncodingMatch::t_getencoding() {
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingMatch, EncodingMatch::getencoding);
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

int c_EncodingMatch::t_getconfidence() {
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingMatch, EncodingMatch::getconfidence);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingMatch, EncodingMatch::getlanguage);
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
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingMatch, EncodingMatch::getutf8);
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

Variant c_EncodingMatch::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(EncodingMatch, EncodingMatch::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
