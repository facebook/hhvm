#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/icu/icu.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <unicode/uchar.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////

static bool parseUChar32(const Variant& vcp, UChar32& cp) {
  if (vcp.isInteger()) {
    cp = vcp.toInt64();
  } else if (vcp.isString()) {
    String strval = vcp.toString();
    auto cstr = strval.c_str();
    auto cstr_len = strval.size();

    int i = 0;
    U8_NEXT(cstr, i, cstr_len, cp);
    if (i != cstr_len) {
      s_intl_error->setError(
        U_ILLEGAL_ARGUMENT_ERROR,
        "Passing a UTF-8 character for codepoint requires a string which is "
        "exactly one UTF-8 codepoint long."
      );
      return false;
    }
  } else {
    s_intl_error->setError(
      U_ILLEGAL_ARGUMENT_ERROR,
      "Invalid parameter for unicode point.  "
      "Must be either integer or UTF-8 sequence."
    );
    return false;
  }
  if ((cp < UCHAR_MIN_VALUE) || (cp > UCHAR_MAX_VALUE)) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR, "Codepoint out of range");
    return false;
  }
  return true;
}

#define GETCP(arg, cp) \
  UChar32 cp; if (!parseUChar32(arg, cp)) { return init_null(); }

#define GETCP_VOID(arg, cp) \
  UChar32 cp; if (!parseUChar32(arg, cp)) { return; }

Variant HHVM_STATIC_METHOD(IntlChar, chr, const Variant& arg) {
  GETCP(arg, cp);

  /* We can use unsafe because we know the codepoint is in valid range
   * and that 4 bytes is enough for any unicode point
   */
  char buffer[5];
  int buffer_len = 0;
  U8_APPEND_UNSAFE(buffer, buffer_len, cp);
  buffer[buffer_len] = 0;
  return String(buffer, buffer_len, CopyString);
}

Variant HHVM_STATIC_METHOD(IntlChar, ord, const Variant& arg) {
  GETCP(arg, cp);
  return cp;
}

Variant HHVM_STATIC_METHOD(IntlChar, hasBinaryProperty,
                           const Variant& arg, int64_t prop) {
  GETCP(arg, cp);
  return (bool)u_hasBinaryProperty(cp, (UProperty)prop);
}

Variant HHVM_STATIC_METHOD(IntlChar, getIntPropertyValue,
                           const Variant& arg, int64_t prop) {
  GETCP(arg, cp);
  return u_getIntPropertyValue(cp, (UProperty)prop);
}

int64_t HHVM_STATIC_METHOD(IntlChar, getIntPropertyMinValue, int64_t prop) {
  return u_getIntPropertyMinValue((UProperty)prop);
}

int64_t HHVM_STATIC_METHOD(IntlChar, getIntPropertyMaxValue, int64_t prop) {
  return u_getIntPropertyMaxValue((UProperty)prop);
}

Variant HHVM_STATIC_METHOD(IntlChar, getNumericValue, const Variant& arg) {
  GETCP(arg, cp);
  return u_getNumericValue(cp);
}

static UBool enumCharType_callback(CallCtx* ctx,
                                   UChar32 start, UChar32 limit,
                                   UCharCategory type) {
  TypedValue args[3];
  args[0].m_type = args[1].m_type = args[2].m_type = KindOfInt64;
  args[0].m_data.num = start;
  args[1].m_data.num = limit;
  args[2].m_data.num = type;
  tvDecRefGen(
    g_context->invokeFuncFew(*ctx, 3, args, RuntimeCoeffects::fixme())
  );
  return true;
}

void HHVM_STATIC_METHOD(IntlChar, enumCharTypes, const Variant& callback) {
  CallCtx ctx;
  ctx.func = nullptr;
  if (!callback.isNull()) {
    vm_decode_function(callback, ctx);
  }
  if (!ctx.func) {
    s_intl_error->setError(U_INTERNAL_PROGRAM_ERROR,
                           "enumCharTypes callback failed");
    return;
  }
  u_enumCharTypes((UCharEnumTypeRange*)enumCharType_callback, &ctx);
}

Variant HHVM_STATIC_METHOD(IntlChar, getBlockCode, const Variant& arg) {
  GETCP(arg, cp);
  return ublock_getCode(cp);
}

Variant HHVM_STATIC_METHOD(IntlChar, charName,
                           const Variant& arg, int64_t choice) {
  GETCP(arg, cp);

  UErrorCode error = U_ZERO_ERROR;
  int32_t buffer_len = u_charName(cp, (UCharNameChoice)choice,
                                  nullptr, 0, &error);

  String buffer(buffer_len, ReserveString);
  error = U_ZERO_ERROR;
  buffer_len = u_charName(cp, (UCharNameChoice)choice,
                          buffer.bufferSlice().data(), buffer_len + 1, &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Failure getting character name");
    return init_null();
  }
  buffer.setSize(buffer_len);
  return buffer;
}

Variant HHVM_STATIC_METHOD(IntlChar, charFromName,
                           const String& name, int64_t choice) {
  UErrorCode error = U_ZERO_ERROR;
  auto ret = u_charFromName((UCharNameChoice)choice, name.c_str(), &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error);
    return init_null();
  }
  return ret;
}

static UBool enumCharNames_callback(CallCtx *ctx,
                                    UChar32 code, UCharNameChoice nameChoice,
                                    const char *name, int32_t length) {
  TypedValue args[3];
  args[0].m_type = args[1].m_type = KindOfInt64;
  args[0].m_data.num = code;
  args[1].m_data.num = nameChoice;

  Variant charName = String(name, length, CopyString);
  tvCopy(*charName.asTypedValue(), args[2]);

  tvDecRefGen(
    g_context->invokeFuncFew(*ctx, 3, args, RuntimeCoeffects::fixme())
  );
  return true;
}

void HHVM_STATIC_METHOD(IntlChar, enumCharNames,
                        const Variant& vStart, const Variant& vLimit,
                        const Variant& callback, int64_t choice) {
  GETCP_VOID(vStart, start);
  GETCP_VOID(vLimit, limit);
  CallCtx ctx;
  ctx.func = nullptr;
  if (!callback.isNull()) {
    vm_decode_function(callback, ctx);
  }
  if (!ctx.func) {
    s_intl_error->setError(U_INTERNAL_PROGRAM_ERROR,
                           "enumCharNames callback failed");
    return;
  }

  UErrorCode error = U_ZERO_ERROR;
  u_enumCharNames(start, limit, (UEnumCharNamesFn*)enumCharNames_callback, &ctx,
                  (UCharNameChoice)choice, &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error);
  }
}

Variant HHVM_STATIC_METHOD(IntlChar, getPropertyName,
                           int64_t prop, int64_t choice) {
  auto ret = u_getPropertyName((UProperty)prop, (UPropertyNameChoice)choice);
  if (ret) {
    return String(ret, CopyString);
  } else {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "Failed to get property name");
    return false;
  }
}

int64_t HHVM_STATIC_METHOD(IntlChar, getPropertyEnum, const String& alias) {
  return u_getPropertyEnum(alias.c_str());
}

Variant HHVM_STATIC_METHOD(IntlChar, getPropertyValueName,
                           int64_t prop, int64_t value, int64_t choice) {
  auto ret = u_getPropertyValueName((UProperty)prop, value,
                                    (UPropertyNameChoice)choice);
  if (ret) {
    return String(ret, CopyString);
  } else {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR,
                           "Failed to get property value name");
    return false;
  }
}

int64_t HHVM_STATIC_METHOD(IntlChar, getPropertyValueEnum,
                           int64_t prop, const String& name) {
  return u_getPropertyValueEnum((UProperty)prop, name.c_str());
}

Variant HHVM_STATIC_METHOD(IntlChar, foldCase,
                           const Variant& arg, int64_t options) {
  GETCP(arg, cp);
  auto ret = u_foldCase(cp, options);
  if (arg.isString()) {
    char buffer[5];
    int buffer_len = 0;
    U8_APPEND_UNSAFE(buffer, buffer_len, ret);
    return String(buffer, buffer_len, CopyString);
  } else {
    return ret;
  }
}

Variant HHVM_STATIC_METHOD(IntlChar, digit, const Variant& arg, int64_t radix) {
  GETCP(arg, cp);
  auto ret = u_digit(cp, radix);
  if (ret < 0) {
    s_intl_error->setError(U_ILLEGAL_ARGUMENT_ERROR, "Invalid digit");
    return false;
  }
  return ret;
}

int64_t HHVM_STATIC_METHOD(IntlChar, forDigit, int64_t digit, int64_t radix) {
  return u_forDigit(digit, radix);
}

Variant HHVM_STATIC_METHOD(IntlChar, charAge, const Variant& arg) {
  GETCP(arg, cp);

  UVersionInfo version;
  u_charAge(cp, version);
  Array ret = Array::CreateVec();
  for(int i = 0; i < U_MAX_VERSION_LENGTH; ++i) {
    ret.append(version[i]);
  }
  return ret;
}

Array HHVM_STATIC_METHOD(IntlChar, getUnicodeVersion) {
  UVersionInfo version;
  u_getUnicodeVersion(version);
  VecInit ret(U_MAX_VERSION_LENGTH);
  for(int i = 0; i < U_MAX_VERSION_LENGTH; ++i) {
    ret.append(version[i]);
  }
  return ret.toArray();
}

Variant HHVM_STATIC_METHOD(IntlChar, getFC_NFKC_Closure, const Variant& arg) {
  GETCP(arg, cp);
  UErrorCode error = U_ZERO_ERROR;
  auto closure_len = u_getFC_NFKC_Closure(cp, nullptr, 0, &error);
  if (closure_len == 0) {
    return empty_string();
  }
  icu::UnicodeString closure;
  auto out = closure.getBuffer(closure_len + 1);
  error = U_ZERO_ERROR;
  closure_len = u_getFC_NFKC_Closure(cp, out, closure_len + 1, &error);
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Failed getting closure");
    return false;
  }
  closure.releaseBuffer(closure_len);
  error = U_ZERO_ERROR;
  String ret(u8(closure, error));
  if (U_FAILURE(error)) {
    s_intl_error->setError(error, "Failed converting output to UTF8");
    return false;
  }
  return ret;
}

/* These four methods all have the same(ish) signature,
 * but don't fit nearly into a template, so I went hacky...
 */
#define IC_INT_METHOD_CHAR(name) \
Variant HHVM_STATIC_METHOD(IntlChar, name, const Variant& arg) { \
  GETCP(arg, cp); \
  return (int64_t)u_##name(cp); \
}
IC_INT_METHOD_CHAR(charDirection)
IC_INT_METHOD_CHAR(charType)
IC_INT_METHOD_CHAR(getCombiningClass)
IC_INT_METHOD_CHAR(charDigitValue)
#undef IC_INT_METHOD_CHAR

/* All methods which take one UChar32 argument and return bool */
template <UBool (*T)(UChar32)>
Variant uchar_method(const Class* /*self_*/, const Variant& arg) {
  GETCP(arg, cp);
  return (bool)T(cp);
}

/* All methods which take one UChar32 argument and return same */
template <UChar32 (*T)(UChar32)>
Variant uchar_method(const Class* /*self_*/, const Variant& arg) {
  GETCP(arg, cp);
  auto ret = T(cp);
  if (arg.isString()) {
    String buf(5, ReserveString);
    auto s = buf.bufferSlice().data();
    int s_len = 0;
    U8_APPEND_UNSAFE(s, s_len, ret);
    s[s_len] = 0;
    buf.setSize(s_len);
    return buf;
  } else {
    return ret;
  }
}

const StaticString s_IntlChar("IntlChar");

void IntlExtension::registerNativeUChar() {
  HHVM_RCC_STR(IntlChar, UNICODE_VERSION, U_UNICODE_VERSION);

  HHVM_RCC_INT(IntlChar, CODEPOINT_MIN, UCHAR_MIN_VALUE);
  HHVM_RCC_INT(IntlChar, CODEPOINT_MAX, UCHAR_MAX_VALUE);
  HHVM_RCC_INT(IntlChar, FOLD_CASE_DEFAULT, U_FOLD_CASE_DEFAULT);
  HHVM_RCC_INT(IntlChar, FOLD_CASE_EXCLUDE_SPECIAL_I,
               U_FOLD_CASE_EXCLUDE_SPECIAL_I);
  HHVM_RCC_DBL(IntlChar, NO_NUMERIC_VALUE, U_NO_NUMERIC_VALUE);

/* All enums used by the uchar APIs.  There are a LOT of them,
  * so they're separated out into include files,
  * leaving this source file for actual implementation.
  *
  * Note that these includes are shared between PHP and HHVM
  */
#define IC_CONSTL(name, val) HHVM_RCC_INT(IntlChar, name, val);
#define UPROPERTY(name) IC_CONSTL(PROPERTY_##name, UCHAR_##name)
#define UCHARCATEGORY(name) IC_CONSTL(CHAR_CATEGORY_##name, U_##name)
#define UCHARDIRECTION(name) IC_CONSTL(CHAR_DIRECTION_##name, U_##name)
#define UBLOCKCODE(name) IC_CONSTL(BLOCK_CODE_##name, UBLOCK_##name)
#define UOTHER(name) IC_CONSTL(name, U_##name)

#include "hphp/runtime/ext/icu/uproperty-enum.h"
#include "hphp/runtime/ext/icu/ucharcategory-enum.h"
#include "hphp/runtime/ext/icu/uchardirection-enum.h"
#include "hphp/runtime/ext/icu/ublockcode-enum.h"
  /* Smaller, self-destribing enums */
#include "hphp/runtime/ext/icu/uother-enum.h"

#undef UPROPERTY
#undef UCHARCATEGORY
#undef UCHARDIRECTION
#undef UBLOCKCODE
#undef UOTHER
#undef IC_CONSTL

// Methods returning bool/UChar32 and taking a single UChar32 argument
#define UCHAR_ME(name) \
  HHVM_NAMED_STATIC_ME(IntlChar, name, uchar_method<u_##name>)
  UCHAR_ME(isUAlphabetic);
  UCHAR_ME(isULowercase);
  UCHAR_ME(isUUppercase);
  UCHAR_ME(isUWhiteSpace);
  UCHAR_ME(islower);
  UCHAR_ME(isupper);
  UCHAR_ME(istitle);
  UCHAR_ME(isdigit);
  UCHAR_ME(isalpha);
  UCHAR_ME(isalnum);
  UCHAR_ME(isxdigit);
  UCHAR_ME(ispunct);
  UCHAR_ME(isgraph);
  UCHAR_ME(isblank);
  UCHAR_ME(isdefined);
  UCHAR_ME(isspace);
  UCHAR_ME(isJavaSpaceChar);
  UCHAR_ME(isWhitespace);
  UCHAR_ME(iscntrl);
  UCHAR_ME(isISOControl);
  UCHAR_ME(isprint);
  UCHAR_ME(isbase);
  UCHAR_ME(isMirrored);
  UCHAR_ME(isIDStart);
  UCHAR_ME(isIDPart);
  UCHAR_ME(isIDIgnorable);
  UCHAR_ME(isJavaIDStart);
  UCHAR_ME(isJavaIDPart);
  UCHAR_ME(charMirror);
  UCHAR_ME(tolower);
  UCHAR_ME(toupper);
  UCHAR_ME(totitle);
#if U_ICU_VERSION_MAJOR_NUM >= 52
  UCHAR_ME(getBidiPairedBracket);
#endif /* ICU >= 52 */
#undef UCHAR_ME

// Methods with unique signatures
#define ICS_ME(name) HHVM_STATIC_ME(IntlChar, name)
  ICS_ME(chr);
  ICS_ME(ord);
  ICS_ME(hasBinaryProperty);
  ICS_ME(getIntPropertyValue);
  ICS_ME(getIntPropertyMinValue);
  ICS_ME(getIntPropertyMaxValue);
  ICS_ME(getNumericValue);
  ICS_ME(enumCharTypes);
  ICS_ME(getBlockCode);
  ICS_ME(getBlockCode);
  ICS_ME(charName);
  ICS_ME(charFromName);
  ICS_ME(enumCharNames);
  ICS_ME(getPropertyName);
  ICS_ME(getPropertyEnum);
  ICS_ME(getPropertyValueName);
  ICS_ME(getPropertyValueEnum);
  ICS_ME(foldCase);
  ICS_ME(digit);
  ICS_ME(forDigit);
  ICS_ME(charAge);
  ICS_ME(getUnicodeVersion);
  ICS_ME(getFC_NFKC_Closure);

  ICS_ME(charDirection);
  ICS_ME(charType);
  ICS_ME(getCombiningClass);
  ICS_ME(charDigitValue);
#undef ICS_ME
}

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
