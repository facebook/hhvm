#ifndef incl_HPHP_ICU_MSGFMT_H
#define incl_HPHP_ICU_MSGFMT_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/umsg.h>
#include <unicode/msgfmt.h>
#include <unicode/fmtable.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_MessageFormatter;

class MessageFormatter : public IntlError {
public:
  MessageFormatter() {}
  MessageFormatter(const MessageFormatter&) = delete;
  MessageFormatter& operator=(const MessageFormatter& src) {
    if (src.m_formatter) {
      auto fmt = (icu::MessageFormat*)src.m_formatter;
      m_formatter = (UMessageFormat*)fmt->clone();
    }
    return *this;
  }
  ~MessageFormatter() {
    if (m_formatter) {
      umsg_close(m_formatter);
    }
  }

  bool openFormatter(const String& pattern, const String& locale);
  UNUSED
  bool processNamedTypes();
  bool processNumericTypes();
  bool mapArgs(std::vector<icu::Formattable>& types,
               std::vector<icu::UnicodeString>& names,
               const Array& args);
  bool isValid() const {
    return m_formatter;
  }

  bool cachedTypes() const {
    return !m_namedParts.empty() || !m_numericParts.empty();
  }

  static MessageFormatter* Get(Object obj) {
    return GetData<MessageFormatter>(obj, s_MessageFormatter);
  }

  UMessageFormat* formatter() const { return m_formatter; }
  icu::MessageFormat* formatterObj() const {
    return (icu::MessageFormat*)m_formatter;
  }

  bool setPattern(const String& pattern);

  bool isTzSet() const { return m_tzSet; }
  bool usesDate() const { return m_usesDate; }
  void tzSet(bool tzset) { m_tzSet = tzset; }

private:
  /** Helper to make old versions of icu act like recent ones */
  UNUSED static bool FixQuotes(icu::UnicodeString& pat) {
    if (pat.length() && (pat.indexOf('\'') >= 0)) {
      UErrorCode error = U_ZERO_ERROR;
      icu::UnicodeString newpat;
      int32_t maxlen = pat.length() * 2;
      int32_t newpat_len =
        umsg_autoQuoteApostrophe(pat.getBuffer(), pat.length(),
                                 newpat.getBuffer(maxlen), maxlen,
                                 &error);
      newpat.releaseBuffer(newpat_len);
      if (U_FAILURE(error)) {
        return false;
      }
      pat = newpat;
    }
    return true;
  }

  typedef std::map<icu::UnicodeString, icu::Formattable::Type> NamedPartsMap;
  typedef std::map<int64_t, icu::Formattable::Type> NumericPartsMap;

  UMessageFormat* m_formatter{nullptr};
  NamedPartsMap m_namedParts{};
  NumericPartsMap m_numericParts{};
  bool m_tzSet{false};
  bool m_usesDate{false};
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_MSGFMT_H
