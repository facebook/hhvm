#include "hphp/runtime/ext/icu/ext_icu_ucsdet.h"

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
// class EncodingDetector

const StaticString s_EncodingDetector("EncodingDetector");

#define FETCH_DET(dest, src) \
  auto dest = EncodingDetector::Get(src); \
  if (!dest) { \
    throw s_intl_error->getException( \
                              "Call to invalid EncodingDetector Object"); \
  }

static void HHVM_METHOD(EncodingDetector, setText, const String& text) {
  FETCH_DET(data, this_);
  data->setText(text.toCppString());
}

static void HHVM_METHOD(EncodingDetector, setDeclaredEncoding,
                        const String& declaredEncoding) {
  FETCH_DET(data, this_);
  data->setDeclaredEncoding(declaredEncoding.toCppString());
}

static Object HHVM_METHOD(EncodingDetector, detect) {
  FETCH_DET(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  auto match = ucsdet_detect(data->detector(), &error);
  if (U_FAILURE(error)) {
    throw data->getException("Could not detect encoding, error %d (%s)",
                             error, u_errorName(error));
  }
  return EncodingMatch::newInstance(match);
}

static Array HHVM_METHOD(EncodingDetector, detectAll) {
  FETCH_DET(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  int32_t count = 0;
  auto matches = ucsdet_detectAll(data->detector(), &count, &error);
  if (U_FAILURE(error)) {
    throw data->getException("Could not detect all encodings, error %d (%s)",
                             error, u_errorName(error));
  }
  Array ret = Array::Create();
  for (int i = 0; i < count; ++i) {
    ret.append(EncodingMatch::newInstance(matches[i]));
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////
// class EncodingMatch

const StaticString s_EncodingMatch("EncodingMatch");
Class* EncodingMatch::c_EncodingMatch = nullptr;

#define FETCH_MATCH(dest, src) \
  auto dest = EncodingMatch::Get(src); \
  if (!dest) { \
      SystemLib::throwExceptionObject( \
        "Call to invalid EncodingMatch Object");        \
  }

static bool HHVM_METHOD(EncodingMatch, isValid) {
  auto data = Native::data<EncodingMatch>(this_);
  return data && data->isValid();
}

static String HHVM_METHOD(EncodingMatch, getEncoding) {
  FETCH_MATCH(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  auto encoding = ucsdet_getName(data->match(), &error);
  if (U_FAILURE(error)) {
    throw data->getException("Could not get encoding for match, error %d (%s)",
                             error, u_errorName(error));
  }
  return String(encoding, CopyString);
}

static int64_t HHVM_METHOD(EncodingMatch, getConfidence) {
  FETCH_MATCH(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  auto confidence = ucsdet_getConfidence(data->match(), &error);
  if (U_FAILURE(error)) {
    throw data->getException("Could not get confidence for match, error "
                             "%d (%s)", error, u_errorName(error));
  }
  return confidence;
}

static String HHVM_METHOD(EncodingMatch, getLanguage) {
  FETCH_MATCH(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  auto language = ucsdet_getLanguage(data->match(), &error);
  if (U_FAILURE(error)) {
    throw data->getException("Could not get language for match, error %d (%s)",
                             error, u_errorName(error));
  }
  return String(language, CopyString);
}

static String HHVM_METHOD(EncodingMatch, getUTF8) {
  FETCH_MATCH(data, this_);
  UErrorCode error;
  icu::UnicodeString ustr;
  int32_t ustrSize = ustr.getCapacity();

  do {
    if (UNLIKELY(ustrSize < ustr.getCapacity())) {
      // Should never happen
      error = U_INTERNAL_PROGRAM_ERROR;
      break;
    }
    error = U_ZERO_ERROR;
    UChar* buf = ustr.getBuffer(ustrSize);
    ustrSize = ucsdet_getUChars(data->match(), buf, ustrSize, &error);
    ustr.releaseBuffer(ustrSize);
  } while (error == U_BUFFER_OVERFLOW_ERROR);

  if (U_FAILURE(error)) {
    throw data->getException("Could not get UTF-8 for match, error %d (%s)",
                             error, u_errorName(error));
  }

  error = U_ZERO_ERROR;
  String ret(u8(ustr, error));
  if (U_FAILURE(error)) {
    throw data->getException("Error converting buffer to UTF8 %d (%d)",
                         error, u_errorName(error));
  }
  return ret;
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::initUcsDet() {

  HHVM_ME(EncodingDetector, setText);
  HHVM_ME(EncodingDetector, setDeclaredEncoding);
  HHVM_ME(EncodingDetector, detect);
  HHVM_ME(EncodingDetector, detectAll);

  HHVM_ME(EncodingMatch, isValid);
  HHVM_ME(EncodingMatch, getEncoding);
  HHVM_ME(EncodingMatch, getConfidence);
  HHVM_ME(EncodingMatch, getLanguage);
  HHVM_ME(EncodingMatch, getUTF8);

  Native::registerNativeDataInfo<EncodingDetector>(s_EncodingDetector.get());
  Native::registerNativeDataInfo<EncodingMatch>(s_EncodingMatch.get());

  loadSystemlib("icu_ucsdet");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
