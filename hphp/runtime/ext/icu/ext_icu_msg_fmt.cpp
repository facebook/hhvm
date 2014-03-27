#include "hphp/runtime/ext/icu/ext_icu_msg_fmt.h"
#include "hphp/runtime/ext/icu/ext_icu_timezone.h"

#include <unicode/format.h>
#include <unicode/datefmt.h>

#if U_ICU_VERSION_MAJOR_NUM * 100 + U_ICU_VERSION_MINOR_NUM < 408
# define MSG_FORMAT_QUOTE_APOS 1
#else
# define HAS_MESSAGE_PATTERN 1
#endif

U_NAMESPACE_BEGIN
/**
 * This class isolates our access to private internal methods of
 * MessageFormat.  It is never instantiated; it exists only for C++
 * access management.
 */
class MessageFormatAdapter {
public:
    MessageFormatAdapter() = delete;

    static const Formattable::Type* getArgTypeList(const MessageFormat& m,
                                                   int32_t& count) {
      return m.getArgTypeList(count);
    }

#ifdef HAS_MESSAGE_PATTERN
    static const MessagePattern getMessagePattern(MessageFormat* m) {
      return m->msgPattern;
    }
#endif
};
U_NAMESPACE_END

namespace HPHP { namespace Intl {
//////////////////////////////////////////////////////////////////////////////
// Internal resource data

#define FETCH_MFMT(data, obj) \
  auto data = MessageFormatter::Get(obj); \
  if (!data) { \
    throw s_intl_error->getException("Uninitialized Message Formatter"); \
  }

const StaticString s_MessageFormatter("MessageFormatter");

bool MessageFormatter::openFormatter(const String& pattern,
                                     const String& locale) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString pat(u16(pattern, error));
  if (U_FAILURE(error)) {
    setError(error, "Error converting pattern to UTF-8");
    return false;
  }

#ifdef MSG_FORMAT_QUOTE_APOS
  if (!FixQuotes(pat)) {
    setError(U_INTERNAL_PROGRAM_ERROR,
             "msgfmt_create: error converting pattern to "
             "quote-friendly format");
    return false;
  }
#endif

  error = U_ZERO_ERROR;
  m_formatter = umsg_open(pat.getBuffer(), pat.length(),
                          locale.c_str(), nullptr, &error);
  if (U_FAILURE(error)) {
    setError(error, "msgfmt_create: message formatter creation failed");
    return false;
  }
  return true;
}

static void HHVM_METHOD(MessageFormatter, __construct,
                        const String& locale,
                        const String& pattern) {
  auto data = Native::data<MessageFormatter>(this_.get());
  if (!data->openFormatter(pattern, localeOrDefault(locale))) {
    throw data->getException("%s", data->getErrorMessage().c_str());
  }
}

bool MessageFormatter::processNamedTypes() {
#ifdef HAS_MESSAGE_PATTERN
  auto formatter = formatterObj();
  auto pat = MessageFormatAdapter::getMessagePattern(formatter);

  auto parts_count = pat.countParts();

  // See MessageFormat::cacheExplicitFormats()
  /*
   * Looking through the pattern, go to each arg_start part type.
   * The arg-typeof that tells us the argument type (simple, complicated)
   * then the next part is either the arg_name or arg number
   * and then if it's simple after that there could be a part-type=arg-type
   * while substring will tell us number, spellout, etc.
   * If the next thing isn't an arg-type then assume string.
   *
   * The last two "parts" can at most be ARG_LIMIT and MSG_LIMIT
   * which we need not examine.
   */
  clearError();
  MessageFormatter::NamedPartsMap namedParts;
  MessageFormatter::NumericPartsMap numericParts;

  for (int32_t i = 0; i < parts_count - 2; i++) {
    auto p = pat.getPart(i);
    if (p.getType() != UMSGPAT_PART_TYPE_ARG_START) {
      continue;
    }
    auto name_part = pat.getPart(++i);

    icu::Formattable::Type type = icu::Formattable::kObject;
    switch (p.getArgType()) {
      case UMSGPAT_ARG_TYPE_SIMPLE: {
        auto type_part = pat.getPart(++i);
        if (type_part.getType() != UMSGPAT_PART_TYPE_ARG_TYPE) {
          setError(U_PARSE_ERROR,
            "Expected UMSGPAT_PART_TYPE_ARG_TYPE part following "
            "UMSGPAT_ARG_TYPE_SIMPLE part");
          return false;
        }
        auto type_string = pat.getSubstring(type_part);
        if (type_string == "number") {
          type = icu::Formattable::kDouble;
          auto style_part = pat.getPart(i + 1);
          if (style_part.getType() == UMSGPAT_PART_TYPE_ARG_STYLE) {
            auto style_string = pat.getSubstring(style_part);
            if (style_string == "integer") {
              type = icu::Formattable::kInt64;
            }
          }
        } else if ((type_string == "date") || (type_string == "time")) {
          m_usesDate = true;
          type = icu::Formattable::kDate;
        } else if ((type_string == "spellout") || (type_string == "ordinal") ||
                   (type_string == "duration")) {
          type = icu::Formattable::kDouble;
        }
        break;
      }
      case UMSGPAT_ARG_TYPE_PLURAL:
      case UMSGPAT_ARG_TYPE_CHOICE:
        type = icu::Formattable::kDouble;
        break;
      case UMSGPAT_ARG_TYPE_NONE:
      case UMSGPAT_ARG_TYPE_SELECT:
      default:
        type = icu::Formattable::kString;
        break;
    }

    if (name_part.getType() == UMSGPAT_PART_TYPE_ARG_NAME) {
      auto argName = pat.getSubstring(name_part);
      auto it = namedParts.find(argName);
      if (it != namedParts.end()) {
        if (it->second != type) {
          setError(U_ARGUMENT_TYPE_MISMATCH,
                   "Inconsistent types declared for an argument");
          return false;
        }
      } else {
        namedParts[argName] = type;
      }
    } else if (name_part.getType() == UMSGPAT_PART_TYPE_ARG_NUMBER) {
      auto argNumber = name_part.getValue();
      if (argNumber < 0) {
        setError(U_INVALID_FORMAT_ERROR, "Found part with negative number");
        return false;
      }
      auto it = numericParts.find(argNumber);
      if (it != numericParts.end()) {
        if (it->second != type) {
          setError(U_ARGUMENT_TYPE_MISMATCH,
                   "Inconsistent types declared for an argument");
          return false;
        }
      } else {
        numericParts[argNumber] = type;
      }
    } else {
      setError(U_INVALID_FORMAT_ERROR, "Invalid part type encountered");
    }
  }

  m_namedParts = namedParts;
  m_numericParts = numericParts;
  return true;
#else
  return false;
#endif
}

bool MessageFormatter::processNumericTypes() {
  auto formatter = formatterObj();
  int32_t count = 0;
  auto types = MessageFormatAdapter::getArgTypeList(*formatter, count);

  clearError();
  m_namedParts.clear();
  m_numericParts.clear();

  for (auto i = 0; i < count; ++i) {
    m_numericParts[i] = types[i];
  }
  return true;
}

inline bool processTypes(MessageFormatter* data) {
  if (data->cachedTypes()) {
    return true;
  }

  if (data->formatterObj()->usesNamedArguments()) {
#ifdef HAS_MESSAGE_PATTERN
    return data->processNamedTypes();
#else
    data->setError(U_UNSUPPORTED_ERROR,
                   "This extension supports named arguments only on ICU 4.8+");
    return false;
#endif
  }
  return data->processNumericTypes();
}

inline bool setTimeZones(MessageFormatter* data) {
  if (!data->usesDate() || data->isTzSet()) {
    return true;
  }
  int32_t count;
  auto obj = data->formatterObj();
  auto formats = obj->getFormats(count);
  if (!formats) {
    data->setError(U_MEMORY_ALLOCATION_ERROR,
                   "Out of memory retrieving subformats");
    return false;
  }
  icu::TimeZone *tz = nullptr;
  SCOPE_EXIT{ if (tz) { delete tz; } };
  for (int i = 0; i < count; ++i) {
    auto df = dynamic_cast<icu::DateFormat*>(
                const_cast<icu::Format *>(formats[i]));
    if (!df) { continue; }
    if (!tz) {
      tz = IntlTimeZone::ParseArg(uninit_null(), "msgfmt_format", data);
      if (U_FAILURE(data->getErrorCode())) {
        return false;
      }
    }
    if (tz) {
      df->setTimeZone(*tz);
    }
  }
  data->tzSet(true);
  return true;
}

bool MessageFormatter::mapArgs(std::vector<icu::Formattable>& types,
                               std::vector<icu::UnicodeString>& names,
                               const Array& args) {
  int32_t count = args.size(), arg_num = 0;
  types.resize(count);
  names.resize(count);
  for (auto idx = args->iter_begin();
       idx != ArrayData::invalid_index;
       idx = args->iter_advance(idx), ++arg_num) {
    auto key = args->getKey(idx);
    icu::Formattable::Type type = icu::Formattable::kObject; // unknown
    if (key.isString()) {
      UErrorCode error = U_ZERO_ERROR;
      icu::UnicodeString key16(u16(key.toString(), error));
      if (U_FAILURE(error)) {
        setError(U_ILLEGAL_ARGUMENT_ERROR,
                 "Invalid UTF-8 data in argument key: '%s'",
                 key.toString().c_str());
        return false;
      }
      if (m_namedParts.find(key16) != m_namedParts.end()) {
        type = m_namedParts[key16];
      }
      names[arg_num] = key16;
    } else {
      auto num = key.toInt64();
      if (m_numericParts.find(num) != m_numericParts.end()) {
        type = m_numericParts[num];
      }
      char buffer[12];
      int32_t len = snprintf(buffer, sizeof(buffer), "%d", (int)num);
      UErrorCode error = U_ZERO_ERROR;
      icu::UnicodeString numName(u16(String(buffer, len, CopyString), error));
      names[arg_num] = numName;
    }

    auto val = args->getValue(idx);
    icu::Formattable formattable;
    switch(type) {
      case icu::Formattable::kString: {
string_val:
        UErrorCode error = U_ZERO_ERROR;
        icu::UnicodeString ustr(u16(val.toString(), error));
        if (U_FAILURE(error)) {
          setError(error, "Invalid UTF-8 data in string argument: '%s'",
                          val.toString().c_str());
          return false;
        }
        formattable.adoptString(new icu::UnicodeString(ustr));
        break;
      }
      case icu::Formattable::kDouble:
        formattable.setDouble(val.toDouble());
        break;
      case icu::Formattable::kLong:
        formattable.setLong(val.toInt64());
        break;
      case icu::Formattable::kInt64:
        formattable.setInt64(val.toInt64());
        break;
      case icu::Formattable::kDate:
        formattable.setDate(VariantToMilliseconds(val));
        break;
      default:
        // No context for arg, so make assupmtion based on value
        if (val.isDouble()) {
          formattable.setDouble(val.toDouble());
        } else if (val.isNull() || val.isBoolean() || val.isInteger()) {
          formattable.setInt64(val.toInt64());
        } else if (val.isString() || val.isObject()) {
          goto string_val;
        } else {
          setError(U_ILLEGAL_ARGUMENT_ERROR,
                   "No strategy to convert the "
                   "value given for the argument with key '%s' "
                   "is available", val.toString().c_str());
          return false;
        }
    }
    types[arg_num] = formattable;
  }
  return true;
}

static Variant HHVM_METHOD(MessageFormatter, format, const Array& args) {
  FETCH_MFMT(data, this_);
  std::vector<icu::Formattable> arg_types;
  std::vector<icu::UnicodeString> arg_names;

  if (!processTypes(data) ||
      !setTimeZones(data) ||
      !data->mapArgs(arg_types, arg_names, args)) {
    return false;
  }
  assert(arg_types.size() == arg_names.size());

  icu::UnicodeString result;
  UErrorCode error = U_ZERO_ERROR;
  data->formatterObj()->format(arg_names.empty() ? nullptr : &arg_names[0],
                               arg_types.empty() ? nullptr : &arg_types[0],
                               arg_names.size(), result, error);
  if (U_FAILURE(error)) {
    data->setError(error, "Call to ICU MessageFormat::format() has failed");
    return false;
  }

  String ret(u8(result, error));
  if (U_FAILURE(error)) {
    data->setError(error, "Unable to format result as UTF-8");
    return false;
  }
  return ret;
}

static int64_t HHVM_METHOD(MessageFormatter, getErrorCode) {
  FETCH_MFMT(data, this_);
  return data->getErrorCode();
}

static String HHVM_METHOD(MessageFormatter, getErrorMessage) {
  FETCH_MFMT(data, this_);
  return data->getErrorMessage();
}

static String HHVM_METHOD(MessageFormatter, getLocale) {
  FETCH_MFMT(data, this_);
  return String(umsg_getLocale(data->formatter()), CopyString);
}

static String HHVM_METHOD(MessageFormatter, getPattern) {
  FETCH_MFMT(data, this_);
  icu::UnicodeString pat16;
  data->formatterObj()->toPattern(pat16);
  UErrorCode error = U_ZERO_ERROR;
  String pat(u8(pat16, error));
  if (U_FAILURE(error)) {
    throw data->getException("Unable to return pattern to utf8");
    not_reached();
  }
  return pat;
}

static Variant HHVM_METHOD(MessageFormatter, parse, const String& value) {
  FETCH_MFMT(data, this_);
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString ustr(u16(value, error));
  if (U_FAILURE(error)) {
    data->setError(error, "Error converting string to UTF-16");
    return false;
  }

  int32_t count;
  error = U_ZERO_ERROR;
  auto fargs = data->formatterObj()->parse(ustr, count, error);
  SCOPE_EXIT{ if (fargs) delete[] fargs; };
  if (U_FAILURE(error)) {
    data->setError(error, "Failed parsing value");
    return false;
  }

  Array ret = Array::Create();
  for (int i = 0; i < count; ++i) {
    switch (fargs[i].getType()) {
      case icu::Formattable::kDate:
        ret.append((double)fargs[i].getDate() / U_MILLIS_PER_SECOND);
        break;
      case icu::Formattable::kDouble:
        ret.append((double)fargs[i].getDouble());
        break;
      case icu::Formattable::kLong:
        ret.append((int64_t)fargs[i].getLong());
        break;
      case icu::Formattable::kInt64:
        ret.append((int64_t)fargs[i].getInt64());
        break;
      case icu::Formattable::kString: {
        icu::UnicodeString tmp;
        fargs[i].getString(tmp);
        error = U_ZERO_ERROR;
        String str(u8(tmp, error));
        if (U_FAILURE(error)) {
          data->setError(error, "Unable to convert to utf-8");
          return false;
        }
        ret.append(str);
        break;
      }
      case icu::Formattable::kObject:
      case icu::Formattable::kArray:
        data->setError(U_ILLEGAL_ARGUMENT_ERROR);
        break;
    }
  }
  return ret;
}

bool MessageFormatter::setPattern(const String& pattern) {
  UErrorCode error = U_ZERO_ERROR;
  icu::UnicodeString pat(u16(pattern, error));
  if (U_FAILURE(error)) {
    setError(error, "Error converting pattern to UTF-16");
    return false;
  }

#ifdef MSG_FORMAT_QUOTE_APOS
  if (!FixQuotes(pat)) {
    setError(U_INTERNAL_PROGRAM_ERROR,
             "msgfmt_set_pattern: error converting pattern to "
             "quote-friendly format");
    return false;
  }
#endif

  m_namedParts.clear();
  m_numericParts.clear();
  m_tzSet = false;
  m_usesDate = false;

  error = U_ZERO_ERROR;
  umsg_applyPattern(formatter(),
                    pat.getBuffer(), pat.length(),
                    nullptr, &error);
  if (U_FAILURE(error)) {
    setError(error, "Error setting symbol value");
    return false;
  }

  return true;
}

static bool HHVM_METHOD(MessageFormatter, setPattern, const String& value) {
  FETCH_MFMT(data, this_);
  return data->setPattern(value);
}

//////////////////////////////////////////////////////////////////////////////

void IntlExtension::initMessageFormatter() {
  HHVM_ME(MessageFormatter, __construct);
  HHVM_ME(MessageFormatter, format);
  HHVM_ME(MessageFormatter, getErrorCode);
  HHVM_ME(MessageFormatter, getErrorMessage);
  HHVM_ME(MessageFormatter, getLocale);
  HHVM_ME(MessageFormatter, getPattern);
  HHVM_ME(MessageFormatter, parse);
  HHVM_ME(MessageFormatter, setPattern);

  Native::registerNativeDataInfo<MessageFormatter>(s_MessageFormatter.get());

  loadSystemlib("icu_msg_fmt");
}

//////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
