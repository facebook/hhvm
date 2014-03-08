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

#include "hphp/runtime/ext/icu/ext_icu_uconverter.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP { namespace Intl {
///////////////////////////////////////////////////////////////////////////////

#define FETCH_CNV(data, obj, def) \
  auto data = IntlUConverter::Get(obj); \
  if (!data) { \
    return def; \
  }

const StaticString
  s_UConverter("UConverter"),
  s_toUCallback("toUCallback"),
  s_fromUCallback("fromUCallback");

static Class* UConverterClass = nullptr;

static Class* getClass() {
  if (!UConverterClass) {
    UConverterClass = Unit::lookupClass(s_UConverter.get());
    assert(UConverterClass);
  }
  return UConverterClass;
}


template<class T>
bool checkLimits(IntlUConverter* data, T* args, int64_t needed) {
  if (needed > (args->targetLimit - args->target)) {
    data->failure(U_BUFFER_OVERFLOW_ERROR, "appendUTarget");
    return false;
  }
  return true;
}

static void appendToUTarget(IntlUConverter *data,
                            Variant val,
                            UConverterToUnicodeArgs *args) {
  if (val.isNull()) {
    // Ignore
    return;
  }
  if (val.isInteger()) {
    int64_t lval = val.toInt64();
    if (lval < 0 || lval > 0x10FFFF) {
      data->failure(U_ILLEGAL_ARGUMENT_ERROR, "appendToUTarger");
      return;
    }
    if (lval > 0xFFFF) {
      if (checkLimits(data, args, 2)) {
        *(args->target++) = (UChar)(((lval - 0x10000) >> 10)   | 0xD800);
        *(args->target++) = (UChar)(((lval - 0x10000) & 0x3FF) | 0xDC00);
      }
      return;
    }
    if (checkLimits(data, args, 1)) {
      *(args->target++) = (UChar)lval;
    }
    return;
  }
  if (val.isString()) {
    const char *strval = val.toString().data();
    int32_t i = 0, strlen = val.toString().size();
    while((i != strlen) && checkLimits(data, args, 1)) {
      UChar c;
      U8_NEXT(strval, i, strlen, c);
      *(args->target++) = c;
    }
    return;
  }
  if (val.isArray()) {
    for(ArrayIter it(val.toArray()); it; ++it) {
      appendToUTarget(data, it.second(), args);
    }
    return;
  }
  data->failure(U_ILLEGAL_ARGUMENT_ERROR, "appendToTarget");
}

static void ucnvToUCallback(ObjectData *objval,
                            UConverterToUnicodeArgs *args,
                            const char *codeUnits, int32_t length,
                            UConverterCallbackReason reason,
                            UErrorCode *pErrorCode) {
  if (MemoryManager::sweeping()) return;
  auto data = Native::data<IntlUConverter>(objval);
  String source(args->source, args->sourceLimit - args->source, CopyString);
  Variant errRef((int64_t)*pErrorCode);
  Variant ret = objval->o_invoke_few_args(
    s_toUCallback, 4,
    reason, source, String(codeUnits, length, CopyString), strongBind(errRef));
  if (errRef.is(KindOfInt64)) {
    *pErrorCode = (UErrorCode)errRef.toInt64();
  } else {
    data->failure(U_ILLEGAL_ARGUMENT_ERROR, "ucnvToUCallback()");
  }
  appendToUTarget(data, ret, args);
}

void appendFromUTarget(IntlUConverter *data,
                       Variant val,
                       UConverterFromUnicodeArgs *args) {
  if (val.isNull()) {
    // ignore
    return;
  }
  if (val.isInteger()) {
    int64_t lval = val.toInt64();
    if (lval < 0 || lval > 255) {
      data->failure(U_ILLEGAL_ARGUMENT_ERROR, "appendFromUTarget");
      return;
    }
    if (checkLimits(data, args, 1)) {
      *(args->target++) = (char)lval;
    }
    return;
  }
  if (val.isString()) {
    int32_t strlen = val.toString().size();
    if (checkLimits(data, args, strlen)) {
      memcpy(args->target, val.toString().data(), strlen);
      args->target += strlen;
    }
    return;
  }
  if (val.isArray()) {
    for(ArrayIter it(val.toArray()); it; ++it) {
      appendFromUTarget(data, it.second(), args);
    }
    return;
  }
  data->failure(U_ILLEGAL_ARGUMENT_ERROR, "appendFromUTarget");
}

static void ucnvFromUCallback(ObjectData *objval,
                              UConverterFromUnicodeArgs *args,
                              const UChar *codeUnits, int32_t length,
                              UChar32 codePoint,
                              UConverterCallbackReason reason,
                              UErrorCode *pErrorCode) {
  if (MemoryManager::sweeping()) return;
  auto data = Native::data<IntlUConverter>(objval);
  Array source = Array::Create();
  for(int i = 0; i < length; i++) {
    UChar32 c;
    U16_NEXT(codeUnits, i, length, c);
    source.append((int64_t)c);
  }
  Variant errRef((int64_t)*pErrorCode);
  Variant ret =
    objval->o_invoke_few_args(
      s_fromUCallback, 4,
      reason, source, (int64_t)codePoint, strongBind(errRef));
  if (errRef.is(KindOfInt64)) {
    *pErrorCode = (UErrorCode)errRef.toInt64();
  } else {
    data->failure(U_ILLEGAL_ARGUMENT_ERROR, "ucnvFromUCallback()");
  }
  appendFromUTarget(data, ret, args);
}

static bool setCallback(const Object& this_, UConverter *cnv) {
  UErrorCode error = U_ZERO_ERROR;
  ucnv_setToUCallBack(cnv, (UConverterToUCallback)ucnvToUCallback,
                           this_.get(), nullptr, nullptr, &error);
  if (U_FAILURE(error)) {
    Native::data<IntlUConverter>(this_.get())->
      failure(error, "ucnv_setToUCallback");
    return false;
  }
  error = U_ZERO_ERROR;
  ucnv_setFromUCallBack(cnv, (UConverterFromUCallback)ucnvFromUCallback,
                             this_.get(), nullptr, nullptr, &error);
  if (U_FAILURE(error)) {
    Native::data<IntlUConverter>(this_.get())->
      failure(error, "ucnv_setFromUCallback");
    return false;
  }

  return true;
}

/* get/set source/dest encodings */

static UConverter* doSetEncoding(IntlUConverter *data, const String& encoding) {
  UErrorCode error = U_ZERO_ERROR;
  UConverter *cnv = ucnv_open(encoding.c_str(), &error);

  if (error == U_AMBIGUOUS_ALIAS_WARNING) {
    UErrorCode getname_error = U_ZERO_ERROR;
    const char *actual_encoding = ucnv_getName(cnv, &getname_error);
    if (U_FAILURE(getname_error)) {
      actual_encoding = "(unknown)";
    }
    raise_warning("Ambiguous encoding specified, using %s", actual_encoding);
    return nullptr;
  } else if (U_FAILURE(error)) {
    data->failure(error, "ucnv_open");
    return nullptr;
  }
  return cnv;
}

static bool HHVM_METHOD(UConverter, setSourceEncoding,
                        const String& encoding) {
  FETCH_CNV(data, this_, false);
  return data->setSrc(doSetEncoding(data, encoding));
}

static bool HHVM_METHOD(UConverter, setDestinationEncoding,
                        const String& encoding) {
  FETCH_CNV(data, this_, false);
  return data->setDest(doSetEncoding(data, encoding));
}

static Variant doGetEncoding(IntlUConverter *data, UConverter *cnv) {
  if (!cnv) {
    return uninit_null();
  }

  UErrorCode error = U_ZERO_ERROR;
  const char *name = ucnv_getName(cnv, &error);
  if (U_FAILURE(error)) {
    data->failure(error, "ucnv_getName");
    return uninit_null();
  }

  return String(name);
}

static Variant HHVM_METHOD(UConverter, getDestinationEncoding) {
  FETCH_CNV(data, this_, uninit_null());
  return doGetEncoding(data, data->dest());
}

static Variant HHVM_METHOD(UConverter, getSourceEncoding) {
  FETCH_CNV(data, this_, uninit_null());
  return doGetEncoding(data, data->src());
}

/* ctor */

static void HHVM_METHOD(UConverter, __construct, const String& toEncoding,
                                                 const String& fromEncoding) {
  FETCH_CNV(data, this_,);
  data->setDest(doSetEncoding(data, toEncoding));
  data->setSrc(doSetEncoding(data, fromEncoding));
  if (this_->getVMClass() != getClass()) {
    setCallback(this_, data->dest());
    setCallback(this_, data->src());
  }
}

/* Get algorithmic types */

static int64_t HHVM_METHOD(UConverter, getSourceType) {
  FETCH_CNV(data, this_, UCNV_UNSUPPORTED_CONVERTER);
  if (!data->src()) {
    return UCNV_UNSUPPORTED_CONVERTER;
  }
  return ucnv_getType(data->src());
}

static int64_t HHVM_METHOD(UConverter, getDestinationType) {
  FETCH_CNV(data, this_, UCNV_UNSUPPORTED_CONVERTER);
  if (!data->dest()) {
    return UCNV_UNSUPPORTED_CONVERTER;
  }
  return ucnv_getType(data->dest());
}

/* Basic substitution */

static bool doSetSubstChars(IntlUConverter *data, UConverter *cnv,
                            const String& chars) {
  UErrorCode error = U_ZERO_ERROR;
  ucnv_setSubstChars(cnv, chars.c_str(), chars.size(), &error);
  if (U_FAILURE(error)) {
    data->setError(error, "ucnv_setSubstChars() returned error %d",
                          (int)error);
    return false;
  }
  return true;
}

static bool HHVM_METHOD(UConverter, setDestinationSubstChars,
                        const String& chars) {
  FETCH_CNV(data, this_, false);
  return doSetSubstChars(data, data->dest(), chars);
}

static bool HHVM_METHOD(UConverter, setSourceSubstChars,
                        const String& chars) {
  FETCH_CNV(data, this_, false);
  return doSetSubstChars(data, data->src(), chars);
}

static Variant doGetSubstChars(IntlUConverter *data, UConverter *cnv) {
  UErrorCode error = U_ZERO_ERROR;
  char chars[127];
  int8_t chars_len = sizeof(chars);

  ucnv_getSubstChars(cnv, chars, &chars_len, &error);
  if (U_FAILURE(error)) {
    data->failure(error, "ucnv_getSubstChars");
    return uninit_null();
  }

  return String(chars, chars_len, CopyString);
}

static Variant HHVM_METHOD(UConverter, getDestinationSubstChars) {
  FETCH_CNV(data, this_, "?");
  return doGetSubstChars(data, data->dest());
}

static Variant HHVM_METHOD(UConverter, getSourceSubstChars) {
  FETCH_CNV(data, this_, "?");
  return doGetSubstChars(data, data->src());
}

/* Main workhorse functions */

static Variant HHVM_METHOD(UConverter, convert,
                           const String& str, bool reverse /*= false */) {
  FETCH_CNV(data, this_, uninit_null());
  UConverter *toCnv   = reverse ? data->src()  : data->dest();
  UConverter *fromCnv = reverse ? data->dest() : data->src();

  /* Converters can re-enter the interpreter */
  SYNC_VM_REGS_SCOPED();

  /* Convert to UChar pivot encoding */
  UErrorCode error = U_ZERO_ERROR;
  int32_t temp_len = ucnv_toUChars(fromCnv, nullptr, 0,
                                   str.c_str(), str.size(), &error);
  if (U_FAILURE(error) && error != U_BUFFER_OVERFLOW_ERROR) {
    data->failure(error, "ucnv_toUChars");
    return uninit_null();
  }

  icu::UnicodeString tmp;
  error = U_ZERO_ERROR;
  temp_len = ucnv_toUChars(fromCnv,
                           tmp.getBuffer(temp_len + 1), temp_len + 1,
                           str.c_str(), str.size(), &error);
  tmp.releaseBuffer(temp_len);
  if (U_FAILURE(error)) {
    data->failure(error, "ucnv_toUChars");
    return uninit_null();
  }

  /* Convert to final encoding */
  error = U_ZERO_ERROR;
  int32_t dest_len = ucnv_fromUChars(toCnv, nullptr, 0,
                                     tmp.getBuffer(), tmp.length(), &error);
  if (U_FAILURE(error) && error != U_BUFFER_OVERFLOW_ERROR) {
    data->failure(error, "ucnv_fromUChars");
    return uninit_null();
  }

  String destStr(dest_len, ReserveString);
  char *dest = (char*) destStr.bufferSlice().ptr;
  error = U_ZERO_ERROR;
  dest_len = ucnv_fromUChars(toCnv, dest, dest_len,
                             tmp.getBuffer(), tmp.length(), &error);
  if (U_FAILURE(error)) {
    data->failure(error, "ucnv_fromUChars");
    return uninit_null();
  }
  return destStr.setSize(dest_len);
}

/* ext/intl error handling */

static int64_t HHVM_METHOD(UConverter, getErrorCode) {
  FETCH_CNV(data, this_, U_ILLEGAL_ARGUMENT_ERROR);
  return data->getErrorCode();
}

static String HHVM_METHOD(UConverter, getErrorMessage) {
  FETCH_CNV(data, this_, empty_string);
  return data->getErrorMessage();
}

/* Ennumerators and lookups */

#define UCNV_REASON_CASE(v) case UCNV_ ## v : return String("REASON_" #v );
static Variant HHVM_STATIC_METHOD(UConverter, reasonText, int64_t reason) {
  switch (reason) {
    UCNV_REASON_CASE(UNASSIGNED)
    UCNV_REASON_CASE(ILLEGAL)
    UCNV_REASON_CASE(IRREGULAR)
    UCNV_REASON_CASE(RESET)
    UCNV_REASON_CASE(CLOSE)
    UCNV_REASON_CASE(CLONE)
    default:
      raise_warning("Unknown UConverterCallbackReason: %ld", (long)reason);
      return uninit_null();
  }
}
#undef UCNV_REASON_CASE

static Array HHVM_STATIC_METHOD(UConverter, getAvailable) {
  int32_t i, count = ucnv_countAvailable();
  Array ret = Array::Create();

  for(i = 0; i < count; ++i) {
    ret.append(ucnv_getAvailableName(i));
  }

  return ret;
}

static Variant HHVM_STATIC_METHOD(UConverter, getAliases,
                                  const String& encoding) {
  UErrorCode error = U_ZERO_ERROR;
  int16_t i, count = ucnv_countAliases(encoding.c_str(), &error);

  if (U_FAILURE(error)) {
    IntlUConverter::Failure(error, "ucnv_getAliases");
    return uninit_null();
  }

  Array ret = Array::Create();
  for(i = 0; i < count; ++i) {
    error = U_ZERO_ERROR;
    const char *alias = ucnv_getAlias(encoding.c_str(), i, &error);
    if (U_FAILURE(error)) {
      IntlUConverter::Failure(error, "ucnv_getAliases");
      return uninit_null();
    }
    ret.append(alias);
  }
  return ret;
}

static Variant HHVM_STATIC_METHOD(UConverter, getStandards) {
  int16_t i, count = ucnv_countStandards();
  Array ret = Array::Create();

  for(i = 0; i < count; ++i) {
    UErrorCode error = U_ZERO_ERROR;
    const char *name = ucnv_getStandard(i, &error);
    if (U_FAILURE(error)) {
      IntlUConverter::Failure(error, "ucnv_getStandard");
      return uninit_null();
    }
    ret.append(name);
  }
  return ret;
}

static Variant HHVM_STATIC_METHOD(UConverter, getStandardName,
                                  const String& name,
                                  const String& standard) {
  UErrorCode error = U_ZERO_ERROR;
  const char *standard_name = ucnv_getStandardName(name.data(),
                                                   standard.data(),
                                                   &error);

  if (U_FAILURE(error)) {
    IntlUConverter::Failure(error, "ucnv_getStandardName");
    return uninit_null();
  }

  return String(standard_name, CopyString);
}

void IntlExtension::initUConverter() {
  HHVM_ME(UConverter, __construct);
  HHVM_ME(UConverter, convert);
  HHVM_ME(UConverter, getDestinationEncoding);
  HHVM_ME(UConverter, getSourceEncoding);
  HHVM_ME(UConverter, getDestinationType);
  HHVM_ME(UConverter, getSourceType);
  HHVM_ME(UConverter, getDestinationSubstChars);
  HHVM_ME(UConverter, getSourceSubstChars);
  HHVM_ME(UConverter, getErrorCode);
  HHVM_ME(UConverter, getErrorMessage);
  HHVM_ME(UConverter, setDestinationEncoding);
  HHVM_ME(UConverter, setSourceEncoding);
  HHVM_ME(UConverter, setDestinationSubstChars);
  HHVM_ME(UConverter, setSourceSubstChars);
  HHVM_STATIC_ME(UConverter, reasonText);
  HHVM_STATIC_ME(UConverter, getAvailable);
  HHVM_STATIC_ME(UConverter, getAliases);
  HHVM_STATIC_ME(UConverter, getStandards);
  HHVM_STATIC_ME(UConverter, getStandardName);

#define UCNV_REASON_CONST(v) \
        Native::registerClassConstant<KindOfInt64>( \
          s_UConverter.get(), makeStaticString("REASON_" #v), UCNV_ ## v);
#define UCNV_TYPE_CONST(v) \
        Native::registerClassConstant<KindOfInt64>( \
          s_UConverter.get(), makeStaticString(#v), UCNV_ ## v);

  UCNV_REASON_CONST(UNASSIGNED);
  UCNV_REASON_CONST(ILLEGAL);
  UCNV_REASON_CONST(IRREGULAR);
  UCNV_REASON_CONST(RESET);
  UCNV_REASON_CONST(CLOSE);
  UCNV_REASON_CONST(CLONE);

  UCNV_TYPE_CONST(UNSUPPORTED_CONVERTER);
  UCNV_TYPE_CONST(SBCS);
  UCNV_TYPE_CONST(DBCS);
  UCNV_TYPE_CONST(MBCS);
  UCNV_TYPE_CONST(LATIN_1);
  UCNV_TYPE_CONST(UTF8);
  UCNV_TYPE_CONST(UTF16_BigEndian);
  UCNV_TYPE_CONST(UTF16_LittleEndian);
  UCNV_TYPE_CONST(UTF32_BigEndian);
  UCNV_TYPE_CONST(UTF32_LittleEndian);
  UCNV_TYPE_CONST(EBCDIC_STATEFUL);
  UCNV_TYPE_CONST(ISO_2022);
  UCNV_TYPE_CONST(LMBCS_1);
  UCNV_TYPE_CONST(LMBCS_2);
  UCNV_TYPE_CONST(LMBCS_3);
  UCNV_TYPE_CONST(LMBCS_4);
  UCNV_TYPE_CONST(LMBCS_5);
  UCNV_TYPE_CONST(LMBCS_6);
  UCNV_TYPE_CONST(LMBCS_8);
  UCNV_TYPE_CONST(LMBCS_11);
  UCNV_TYPE_CONST(LMBCS_16);
  UCNV_TYPE_CONST(LMBCS_17);
  UCNV_TYPE_CONST(LMBCS_18);
  UCNV_TYPE_CONST(LMBCS_19);
  UCNV_TYPE_CONST(LMBCS_LAST);
  UCNV_TYPE_CONST(HZ);
  UCNV_TYPE_CONST(SCSU);
  UCNV_TYPE_CONST(ISCII);
  UCNV_TYPE_CONST(US_ASCII);
  UCNV_TYPE_CONST(UTF7);
  UCNV_TYPE_CONST(BOCU1);
  UCNV_TYPE_CONST(UTF16);
  UCNV_TYPE_CONST(UTF32);
  UCNV_TYPE_CONST(CESU8);
  UCNV_TYPE_CONST(IMAP_MAILBOX);

#undef UCNV_REASON_CONST
#undef UCNV_TYPE_CONST

  Native::registerNativeDataInfo<IntlUConverter>(s_UConverter.get());

  loadSystemlib("icu_uconverter");
}

///////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
