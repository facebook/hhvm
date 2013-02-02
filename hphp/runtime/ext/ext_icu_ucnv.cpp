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

#include <runtime/ext/ext_icu_ucnv.h>
#include <runtime/vm/translator/translator-inline.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define UCNV_REASON_CONST(v) \
        const int64 q_UConverter$$REASON_ ## v = UCNV_ ## v ;
#define UCNV_TYPE_CONST(v) \
        const int64 q_UConverter$$ ## v        = UCNV_ ## v ;

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

static StaticString s_toUCallback("toUCallback");
static StaticString s_fromUCallback("fromUCallback");

#define THROW_UFAILURE(fname, uerr, ierr) throwFailure(uerr, #fname, ierr);

c_UConverter::c_UConverter(const ObjectStaticCallbacks *cb)
    : ExtObjectData(cb), m_src(NULL), m_dest(NULL) {
  m_error.code = U_ZERO_ERROR;
  m_error.custom_error_message = "";
}

c_UConverter::~c_UConverter() { }

void c_UConverter::throwFailure(UErrorCode error, const char *fname,
                                intl_error &merror) {
  char message[1024];
  snprintf(message, sizeof(message), "%s() returned error %ld: %s",
           fname, (long)error, u_errorName(error));
  merror.code = error;
  merror.custom_error_message = String((const char*)message, CopyString);
}

void c_UConverter::t___construct(CStrRef toEncoding, CStrRef fromEncoding) {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::__construct);
  setEncoding(toEncoding,   &m_dest, m_error);
  setEncoding(fromEncoding, &m_src,  m_error);
  setCallback(m_dest);
  setCallback(m_src);
}

Variant c_UConverter::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::__destruct);
  if (m_src) {
    ucnv_close(m_src);
  }
  if (m_dest) {
    ucnv_close(m_dest);
  }

  return null;
}

/* get/set source/dest encodings */

#define TARGET_CHECK(args, len) \
        checkLimits(args->targetLimit - args->target, len)
bool c_UConverter::checkLimits(int64_t available, int64_t needed) {
  if (needed > available) {
    THROW_UFAILURE(appendUTarget, U_BUFFER_OVERFLOW_ERROR, m_error);
    return false;
  }
  return true;
}

void c_UConverter::appendToUTarget(Variant val,
                                   UConverterToUnicodeArgs *args) {
  if (val.isNull()) {
    // Ignore
    return;
  }
  if (val.isInteger()) {
    int64_t lval = val.toInt64();
    if (lval < 0 || lval > 0x10FFFF) {
      THROW_UFAILURE(appendToUTarget, U_ILLEGAL_ARGUMENT_ERROR, m_error);
      return;
    }
    if (lval > 0xFFFF) {
      if (TARGET_CHECK(args, 2)) {
        *(args->target++) = (UChar)(((lval - 0x10000) >> 10)   | 0xD800);
        *(args->target++) = (UChar)(((lval - 0x10000) & 0x3FF) | 0xDC00);
      }
      return;
    }
    if (TARGET_CHECK(args, 1)) {
      *(args->target++) = (UChar)lval;
    }
    return;
  }
  if (val.isString()) {
    const char *strval = val.toString().data();
    int32_t i = 0, strlen = val.toString().size();
    while((i != strlen) && TARGET_CHECK(args, 1)) {
      UChar c;
      U8_NEXT(strval, i, strlen, c);
      *(args->target++) = c;
    }
    return;
  }
  if (val.isArray()) {
    for(ArrayIter it(val.toArray()); it; ++it) {
      appendToUTarget(it.second(), args);
    }
    return;
  }
  THROW_UFAILURE(appendToTarget, U_ILLEGAL_ARGUMENT_ERROR, m_error);
}

void c_UConverter::ucnvToUCallback(c_UConverter *objval,
                                   UConverterToUnicodeArgs *args,
                                   const char *codeUnits, int32_t length,
                                   UConverterCallbackReason reason,
                                   UErrorCode *pErrorCode) {
  String source(args->source, args->sourceLimit - args->source, CopyString);
  VRefParam errRef((long)*pErrorCode);
  Variant ret = objval->o_invoke(s_toUCallback, CREATE_VECTOR4(
    reason, source, String(codeUnits, length, CopyString), ref(errRef)
  ), -1);
  if (errRef.is(KindOfInt64)) {
    *pErrorCode = (UErrorCode)errRef.toInt64();
  } else {
    throwFailure(U_ILLEGAL_ARGUMENT_ERROR, "ucnvToUCallback()",
                 objval->m_error);
  }
  objval->appendToUTarget(ret, args);
}

void c_UConverter::appendFromUTarget(Variant val,
                                     UConverterFromUnicodeArgs *args) {
  if (val.isNull()) {
    // ignore
    return;
  }
  if (val.isInteger()) {
    int64_t lval = val.toInt64();
    if (lval < 0 || lval > 255) {
      THROW_UFAILURE(appendFromUTarget, U_ILLEGAL_ARGUMENT_ERROR, m_error);
      return;
    }
    if (TARGET_CHECK(args, 1)) {
      *(args->target++) = (char)lval;
    }
    return;
  }
  if (val.isString()) {
    int32_t strlen = val.toString().size();
    if (TARGET_CHECK(args, strlen)) {
      memcpy(args->target, val.toString().data(), strlen);
      args->target += strlen;
    }
    return;
  }
  if (val.isArray()) {
    for(ArrayIter it(val.toArray()); it; ++it) {
      appendFromUTarget(it.second(), args);
    }
    return;
  }
  THROW_UFAILURE(appendFromUTarget, U_ILLEGAL_ARGUMENT_ERROR, m_error);
}

void c_UConverter::ucnvFromUCallback(c_UConverter *objval,
                                     UConverterFromUnicodeArgs *args,
                                     const UChar *codeUnits, int32_t length,
                                     UChar32 codePoint,
                                     UConverterCallbackReason reason,
                                     UErrorCode *pErrorCode) {
  Array source = Array::Create();
  for(int i = 0; i < length; i++) {
    UChar32 c;
    U16_NEXT(codeUnits, i, length, c);
    source.append((int64_t)c);
  }
  VRefParam errRef((int64_t)*pErrorCode);
  Variant ret = objval->o_invoke(s_fromUCallback, CREATE_VECTOR4(
    reason, source, (int64_t)codePoint, ref(errRef)
  ), -1);
  if (errRef.is(KindOfInt64)) {
    *pErrorCode = (UErrorCode)errRef.toInt64();
  } else {
    throwFailure(U_ILLEGAL_ARGUMENT_ERROR, "ucnvFromUCallback()",
                 objval->m_error);
  }
  objval->appendFromUTarget(ret, args);
}

bool c_UConverter::setCallback(UConverter *cnv) {
  if (o_getClassName().get()->isame(String("UConverter").get())) {
    return true;
  }

  UErrorCode error = U_ZERO_ERROR;
  ucnv_setToUCallBack(cnv, (UConverterToUCallback)ucnvToUCallback,
                           (const void*)this, NULL, NULL, &error);
  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_setToUCallback, error, m_error);
    ucnv_close(cnv);
    return false;
  }
  error = U_ZERO_ERROR;
  ucnv_setFromUCallBack(cnv, (UConverterFromUCallback)ucnvFromUCallback,
                             (const void*)this, NULL, NULL, &error);
  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_setFromUCallback, error, m_error);
    ucnv_close(cnv);
    return false;
  }

  return true;
}

bool c_UConverter::setEncoding(CStrRef encoding, UConverter **pcnv,
                               intl_error &err) {
  UErrorCode error = U_ZERO_ERROR;
  UConverter *cnv = ucnv_open(encoding.data(), &error);

  if (error == U_AMBIGUOUS_ALIAS_WARNING) {
    UErrorCode getname_error = U_ZERO_ERROR;
    const char *actual_encoding = ucnv_getName(cnv, &getname_error);
    if (U_FAILURE(getname_error)) {
      actual_encoding = "(unknown)";
    }
    raise_warning("Ambiguous encoding specified, using %s", actual_encoding);
  } else if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_open, error, err);
    return false;
  }

  if (*pcnv) {
    ucnv_close(*pcnv);
  }
  *pcnv = cnv;

  return true;
}

void c_UConverter::t_setsourceencoding(CStrRef encoding) {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::setsourceencoding);
  setEncoding(encoding, &m_src, m_error);
}

void c_UConverter::t_setdestinationencoding(CStrRef encoding) {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::setdestinationencoding);
  setEncoding(encoding, &m_dest, m_error);
}

String c_UConverter::t_getsourceencoding() {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::getsourceencoding);
  if (!m_src) {
    return null;
  }

  UErrorCode error = U_ZERO_ERROR;
  const char *name = ucnv_getName(m_src, &error);
  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_getName, error, m_error);
    return null;
  }

  return String(name);
}

String c_UConverter::t_getdestinationencoding() {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::getdestinationencoding);
  if (!m_dest) {
    return null;
  }

  UErrorCode error = U_ZERO_ERROR;
  const char *name = ucnv_getName(m_dest, &error);
  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_getName, error, m_error);
    return null;
  }

  return String(name);
}

/* Get algorithmic types */

int64 c_UConverter::t_getsourcetype() {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::getsourcetype);
  if (!m_src) {
    return UCNV_UNSUPPORTED_CONVERTER;
  }

  return ucnv_getType(m_src);
}

int64 c_UConverter::t_getdestinationtype() {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::getdestinationtype);
  if (!m_dest) {
    return UCNV_UNSUPPORTED_CONVERTER;
  }

  return ucnv_getType(m_dest);
}

/* Basic substitution */

bool c_UConverter::setSubstChars(String chars, UConverter *cnv,
                                 intl_error &err) {
  UErrorCode error = U_ZERO_ERROR;
  ucnv_setSubstChars(cnv, chars.data(), chars.size(), &error);
  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_setSubstChars, error, err);
    return false;
  }
  return true;
}

bool c_UConverter::t_setsubstchars(CStrRef chars) {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::setsubstchars);
  return setSubstChars(chars, m_dest, m_error) &&
         setSubstChars(chars, m_src,  m_error);
}

String c_UConverter::t_getsubstchars() {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::getsubstchars);
  UErrorCode error = U_ZERO_ERROR;
  char chars[127];
  int8_t chars_len;

  ucnv_getSubstChars(m_src, chars, &chars_len, &error);
  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_getSubstChars, error, m_error);
    return null;
  }

  return String(chars, chars_len, CopyString);
}

/* Callbacks */

Variant c_UConverter::defaultCallback(int64 reason, VRefParam error) {
  switch(reason) {
    case UCNV_UNASSIGNED:
    case UCNV_ILLEGAL:
    case UCNV_IRREGULAR:
      error = U_ZERO_ERROR;
      return t_getsubstchars();
  }

  return null;
}

Variant c_UConverter::t_fromucallback(int64 reason,
                                      CArrRef source, int64 codepoint,
                                      VRefParam error) {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::fromucallback);
  return defaultCallback(reason, error);
}

Variant c_UConverter::t_toucallback(int64 reason,
                                    CStrRef source, CStrRef codeunits,
                                    VRefParam error) {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::toucallback);
  return defaultCallback(reason, error);
}

/* Main workhorse functions */

Variant c_UConverter::t_convert(CStrRef str, bool reverse) {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::convert);
  SYNC_VM_REGS_SCOPED();
  return doConvert(str, reverse ? m_src : m_dest,
                        reverse ? m_dest : m_src, m_error);
}

String c_UConverter::doConvert(CStrRef str,
                               UConverter *toCnv, UConverter *fromCnv,
                               intl_error &err) {
  UErrorCode error = U_ZERO_ERROR;

  if (!fromCnv || !toCnv) {
    err.code = U_INVALID_STATE_ERROR;
    err.custom_error_message = "Internal converters not initialized";
    return null;
  }

  /* Convert to UChar pivot encoding */
  int32_t temp_len = ucnv_toUChars(fromCnv, NULL, 0,
                                   str.c_str(), str.size(), &error);
  if (U_FAILURE(error) && error != U_BUFFER_OVERFLOW_ERROR) {
    THROW_UFAILURE(ucnv_toUChars, error, err);
    return null;
  }
  // Explicitly include the space for a \u0000 UChar since String
  // only allocates one extra byte (not the 2 needed)
  String tempStr(sizeof(UChar) * (temp_len + 1), ReserveString);
  UChar *temp = (UChar*) tempStr.mutableSlice().ptr;

  error = U_ZERO_ERROR;
  temp_len = ucnv_toUChars(fromCnv, temp, temp_len,
                           str.c_str(), str.size(), &error);
  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_toUChars, error, err);
    return null;
  }
  temp[temp_len] = 0;

  /* Convert to final encoding */
  error = U_ZERO_ERROR;
  int32_t dest_len = ucnv_fromUChars(toCnv, NULL, 0,
                                     temp, temp_len, &error);
  if (U_FAILURE(error) && error != U_BUFFER_OVERFLOW_ERROR) {
    THROW_UFAILURE(ucnv_fromUChars, error, err);
    return null;
  }
  String destStr(dest_len, ReserveString);
  char *dest = (char*) destStr.mutableSlice().ptr;

  error = U_ZERO_ERROR;
  dest_len = ucnv_fromUChars(toCnv, dest, dest_len,
                             temp, temp_len, &error);
  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_fromUChars, error, err);
    return null;
  }
  return destStr.setSize(dest_len);
}

Variant c_UConverter::ti_transcode(const char* cls , CStrRef str,
                                   CStrRef toEncoding, CStrRef fromEncoding,
                                   CArrRef options) {
  STATIC_METHOD_INJECTION_BUILTIN(UConverter, UConverter::transcode);
  UConverter *fromCnv = NULL, *toCnv = NULL;
  if (!setEncoding(fromEncoding, &fromCnv, s_intl_error->m_error)) {
    return null;
  }
  if (!setEncoding(toEncoding, &toCnv, s_intl_error->m_error)) {
    return null;
  }
  if (options.exists("from_subst") &&
     !setSubstChars(options["from_subst"].toString(), fromCnv,
                                          s_intl_error->m_error)) {
    return null;
  }
  if (options.exists("to_subst") &&
     !setSubstChars(options["to_subst"].toString(), toCnv,
                                        s_intl_error->m_error)) {
    return null;
  }
  Variant ret = doConvert(str, toCnv, fromCnv, s_intl_error->m_error);
  ucnv_close(toCnv);
  ucnv_close(fromCnv);
  return ret;
}

/* ext/intl error handling */

int64 c_UConverter::t_geterrorcode() {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::geterrorcode);
  return m_error.code;
}

String c_UConverter::t_geterrormessage() {
  INSTANCE_METHOD_INJECTION_BUILTIN(UConverter, UConverter::geterrormessage);
  return m_error.custom_error_message;
}

/* Ennumerators and lookups */

#define UCNV_REASON_CASE(v) case UCNV_ ## v : return String("REASON_" #v );
String c_UConverter::ti_reasontext(const char* cls , int64 reason) {
  STATIC_METHOD_INJECTION_BUILTIN(UConverter, UConverter::reasontext);
  switch (reason) {
    UCNV_REASON_CASE(UNASSIGNED)
    UCNV_REASON_CASE(ILLEGAL)
    UCNV_REASON_CASE(IRREGULAR)
    UCNV_REASON_CASE(RESET)
    UCNV_REASON_CASE(CLOSE)
    UCNV_REASON_CASE(CLONE)
    default:
      raise_warning("Unknown UConverterCallbackReason: %ld", (long)reason);
      return null;
  }
}

Array c_UConverter::ti_getavailable(const char* cls ) {
  STATIC_METHOD_INJECTION_BUILTIN(UConverter, UConverter::getavailable);
  int32_t i, count = ucnv_countAvailable();
  Array ret = Array::Create();

  for(i = 0; i < count; ++i) {
    ret.append(ucnv_getAvailableName(i));
  }

  return ret;
}

Array c_UConverter::ti_getaliases(const char* cls , CStrRef encoding) {
  STATIC_METHOD_INJECTION_BUILTIN(UConverter, UConverter::getaliases);
  UErrorCode error = U_ZERO_ERROR;
  int16_t i, count = ucnv_countAliases(encoding.data(), &error);

  if (U_FAILURE(error)) {
    THROW_UFAILURE(ucnv_getAliases, error, s_intl_error->m_error);
    return null;
  }

  Array ret = Array::Create();
  for(i = 0; i < count; ++i) {
    error = U_ZERO_ERROR;
    const char *alias = ucnv_getAlias(encoding, i, &error);
    if (U_FAILURE(error)) {
      THROW_UFAILURE(ucnv_getAlias, error, s_intl_error->m_error);
      return null;
    }
    ret.append(alias);
  }
  return ret;
}

Array c_UConverter::ti_getstandards(const char* cls ) {
  STATIC_METHOD_INJECTION_BUILTIN(UConverter, UConverter::getstandards);
  int16_t i, count = ucnv_countStandards();
  Array ret = Array::Create();

  for(i = 0; i < count; ++i) {
    UErrorCode error = U_ZERO_ERROR;
    const char *name = ucnv_getStandard(i, &error);
    if (U_FAILURE(error)) {
      THROW_UFAILURE(ucnv_getStandard, error, s_intl_error->m_error);
      return null;
    }
    ret.append(name);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
