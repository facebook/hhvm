#ifndef incl_HPHP_EXT_ICU_CODEPOINTBREAKITERATOR_H
#define incl_HPHP_EXT_ICU_CODEPOINTBREAKITERATOR_H

#include <unicode/brkiter.h>
#include <unicode/uchriter.h>
#include <typeinfo>

#ifndef UTEXT_CURRENT32
#define UTEXT_CURRENT32 utext_current32
#endif

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

class CodePointBreakIterator : public icu::BreakIterator {
 public:
  static UClassID getStaticClassID();
  UClassID getDynamicClassID(void) const override;

  CodePointBreakIterator() : BreakIterator() {
    UErrorCode error = U_ZERO_ERROR;
    m_text = utext_openUChars(nullptr, nullptr, 0, &error);
  }
  CodePointBreakIterator(const CodePointBreakIterator &other) {
    *this = other;
  }
  CodePointBreakIterator& operator=(const CodePointBreakIterator& that) {
    UErrorCode error = U_ZERO_ERROR;
    m_text = utext_clone(nullptr, that.m_text, false, true, &error);
    clearCurrentCharIter();
    m_lastCodePoint = that.m_lastCodePoint;
    return *this;
  }

  ~CodePointBreakIterator() override {
    if (m_text) {
      utext_close(m_text);
    }
    clearCurrentCharIter();
  }

  UBool operator==(const BreakIterator& that) const override {
    if (typeid(*this) != typeid(that)) {
      return false;
    }
    const CodePointBreakIterator *that2 =
                dynamic_cast<const CodePointBreakIterator*>(&that);
    return utext_equals(m_text, that2->m_text);
  }

  CodePointBreakIterator* clone(void) const override {
    return new CodePointBreakIterator(*this);
  }

  CharacterIterator& getText(void) const override {
    if (!m_charIter) {
      // this method is deprecated anyway; setup bogus iterator
      static const UChar c = 0;
      m_charIter = new UCharCharacterIterator(&c, 0);
    }
    return *m_charIter;
  }

  UText *getUText(UText *fillIn, UErrorCode &status) const override {
    return utext_clone(fillIn, m_text, false, true, &status);
  }

  void setText(const UnicodeString &text) override {
    UErrorCode error = U_ZERO_ERROR;
    m_text = utext_openConstUnicodeString(m_text, &text, &error);
    clearCurrentCharIter();
  }

  void setText(UText *text, UErrorCode &status) override {
    if (U_FAILURE(status)) {
      return;
    }
    m_text = utext_clone(m_text, text, false, true, &status);
    clearCurrentCharIter();
  }

  void adoptText(CharacterIterator* it) override {
    clearCurrentCharIter();
    UErrorCode error = U_ZERO_ERROR;
    m_charIter = it;
    m_text = utext_openCharacterIterator(m_text, it, &error);
  }

  int32_t first(void) override {
    UTEXT_SETNATIVEINDEX(m_text, 0);
    m_lastCodePoint = UTEXT_CURRENT32(m_text);
    return (int32_t)UTEXT_GETNATIVEINDEX(m_text);
  }

  int32_t last(void) override {
    int32_t pos = (int32_t)utext_nativeLength(m_text);
    UTEXT_SETNATIVEINDEX(m_text, pos);
    m_lastCodePoint = U_SENTINEL;
    return (int32_t)UTEXT_GETNATIVEINDEX(m_text);
  }

  int32_t previous(void) override {
    m_lastCodePoint = UTEXT_PREVIOUS32(m_text);
    return current();
  }

  int32_t next(void) override {
    m_lastCodePoint = UTEXT_NEXT32(m_text);
    return current();
  }

  int32_t current(void) const override {
    if (m_lastCodePoint == U_SENTINEL) {
      return BreakIterator::DONE;
    }
    return (int32_t)UTEXT_GETNATIVEINDEX(m_text);
  }

  int32_t following(int32_t offset) override {
    m_lastCodePoint = utext_next32From(m_text, offset);
    return current();
  }

  int32_t preceding(int32_t offset) override {
    m_lastCodePoint = utext_previous32From(m_text, offset);
    return current();
  }

  UBool isBoundary(int32_t offset) override {
    //this function has side effects, and it's supposed to
    utext_setNativeIndex(m_text, offset);
    return (offset == utext_getNativeIndex(m_text));
  }

  int32_t next(int32_t n) override {
    UBool res = utext_moveIndex32(m_text, n);

    if (res) {
      m_lastCodePoint = UTEXT_CURRENT32(m_text);
      return (int32_t)UTEXT_GETNATIVEINDEX(m_text);
    } else {
      m_lastCodePoint = U_SENTINEL;
      return BreakIterator::DONE;
    }
  }

  CodePointBreakIterator *createBufferClone(void *stackBuffer,
                                            int32_t &BufferSize,
                                            UErrorCode &status) override;

#if U_ICU_VERSION_MAJOR_NUM * 100 + U_ICU_VERSION_MINOR_NUM >= 409 
  CodePointBreakIterator &refreshInputText(UText *input,
                                           UErrorCode &status) override;
#endif

  inline UChar32 getLastCodePoint() { return m_lastCodePoint; }

 private:
  UText  *m_text{nullptr};
  UChar32 m_lastCodePoint{U_SENTINEL};
  mutable CharacterIterator *m_charIter{nullptr};

  inline void clearCurrentCharIter() {
    if (m_charIter) {
      delete m_charIter;
      m_charIter = nullptr;
    }
    m_lastCodePoint = U_SENTINEL;
  }
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
#endif // incl_HPHP_EXT_ICU_CODEPOINTBREAKITERATOR_H
