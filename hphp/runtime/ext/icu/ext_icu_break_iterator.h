#pragma once

#include "hphp/runtime/ext/icu/icu.h"
#include "hphp/runtime/ext/icu/CodePointBreakIterator.h"

#include <unicode/brkiter.h>
#include <unicode/rbbi.h>
#include <unicode/utext.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////

struct IntlBreakIterator : IntlError,
                           SystemLib::ClassLoader<"IntlBreakIterator"> {
  IntlBreakIterator() {}
  IntlBreakIterator(const IntlBreakIterator&) = delete;
  IntlBreakIterator& operator=(const IntlBreakIterator& src) {
    setBreakIterator(src.breakIterator()->clone());
    return *this;
  }
  ~IntlBreakIterator() { setBreakIterator(nullptr); }

  void sweep() {
    setBreakIterator(nullptr);
    std::destroy_at(&m_text);
  }

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

  static IntlBreakIterator* Get(ObjectData* obj) {
    return GetData<IntlBreakIterator>(obj, className());
  }

  static Object newInstance(icu::BreakIterator* bi = nullptr) {
    Object obj{ classof() };
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
    assertx(isValid());
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
  String m_compiledRules;
 private:
  icu::BreakIterator *m_breakIterator{nullptr};
  std::string m_text;
  UText *m_uText{nullptr};
  bool m_started{false};
};

struct IntlCodePointBreakIterator :
  SystemLib::ClassLoader<"IntlCodePointBreakIterator"> {

  static Object newInstance(CodePointBreakIterator* bi = nullptr);
};


/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
