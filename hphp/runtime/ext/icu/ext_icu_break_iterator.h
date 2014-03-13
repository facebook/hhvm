#ifndef incl_HPHP_EXT_ICU_BREAK_ITERATOR_H
#define incl_HPHP_EXT_ICU_BREAK_ITERATOR_H

#include "hphp/runtime/ext/icu/icu.h"
#include "hphp/runtime/ext/icu/CodePointBreakIterator.h"

#include <unicode/brkiter.h>
#include <unicode/rbbi.h>
#include <unicode/utext.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_IntlBreakIterator, s_IntlCodePointBreakIterator;

class IntlBreakIterator : public IntlError {
 public:
  IntlBreakIterator() {}
  IntlBreakIterator(const IntlBreakIterator&) = delete;
  IntlBreakIterator& operator=(const IntlBreakIterator& src) {
    setBreakIterator(src.breakIterator()->clone());
    return *this;
  }
  ~IntlBreakIterator() { setBreakIterator(nullptr); }

  void setBreakIterator(icu::BreakIterator *bi) {
    if (m_breakIterator) {
      delete m_breakIterator;
    }
    m_breakIterator = bi;
    if (m_uText) {
      utext_close(m_uText);
      m_uText = nullptr;
    }
  }

  static IntlBreakIterator* Get(Object obj) {
    return GetData<IntlBreakIterator>(obj, s_IntlBreakIterator);
  }

  static Object newInstance(icu::BreakIterator* bi = nullptr) {
    if (!c_IntlBreakIterator) {
      c_IntlBreakIterator = Unit::lookupClass(s_IntlBreakIterator.get());
      assert(c_IntlBreakIterator);
    }
    auto obj = ObjectData::newInstance(c_IntlBreakIterator);
    if (bi) {
      Native::data<IntlBreakIterator>(obj)->setBreakIterator(bi);
    }
    return obj;
  }

  static Object newCodePointInstance(CodePointBreakIterator* bi = nullptr) {
    if (!c_IntlCodePointBreakIterator) {
      c_IntlCodePointBreakIterator =
        Unit::lookupClass(s_IntlCodePointBreakIterator.get());
      assert(c_IntlCodePointBreakIterator);
    }
    auto obj = ObjectData::newInstance(c_IntlCodePointBreakIterator);
    if (bi) {
      Native::data<IntlBreakIterator>(obj)->setBreakIterator(bi);
    }
    return obj;
  }

  icu::BreakIterator* breakIterator() const { return m_breakIterator; }
  UText* uText() const { return m_uText; }
  const std::string& text() const { return m_text; }
  bool isValid() { return m_breakIterator; }
  void setStarted(bool started) { m_started = started; }
  bool started() const { return m_started; }

  icu::RuleBasedBreakIterator* ruleBasedBreakIterator() const {
    return dynamic_cast<icu::RuleBasedBreakIterator*>(m_breakIterator);
  }

  CodePointBreakIterator* codePointBreakIterator() const {
    return dynamic_cast<CodePointBreakIterator*>(m_breakIterator);
  }

  bool setText(const String& str) {
    assert(isValid());
    m_text = str.toCppString();
    UErrorCode error = U_ZERO_ERROR;
    m_uText = utext_openUTF8(m_uText, m_text.c_str(), m_text.size(), &error);
    if (U_FAILURE(error)) {
      setError(error, "Failed setting UTF-8 text");
      return false;
    }
    error = U_ZERO_ERROR;
    breakIterator()->setText(m_uText, error);
    if (U_FAILURE(error)) {
      setError(error);
      return false;
    }
    m_started = false;
    m_key = 0;
    return true;
  }

  // BreakIterator doesn't keep track of this for us, so we have to
  // A value of -1 means unknown and unknowable
  int32_t m_key{0};
 private:
  icu::BreakIterator *m_breakIterator{nullptr};
  std::string m_text;
  UText *m_uText{nullptr};
  bool m_started{false};

  static Class* c_IntlBreakIterator;
  static Class* c_IntlCodePointBreakIterator;
};


/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
#endif // icl_HPHP_EXT_ICU_BREAK_ITERATOR_H
