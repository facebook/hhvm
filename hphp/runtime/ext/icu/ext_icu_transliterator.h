#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/translit.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_Transliterator;

struct Transliterator : IntlError {
  Transliterator() {}
  Transliterator(const Transliterator&) = delete;
  Transliterator& operator=(const Transliterator& src) {
    if (src.m_trans) {
      m_trans = src.m_trans->clone();
    }
    return *this;
  }
  ~Transliterator() { setTransliterator(nullptr); }

  bool isValid() const {
    return m_trans;
  }

  static Transliterator* Get(ObjectData* obj) {
    return GetData<Transliterator>(obj, s_Transliterator);
  }

  static Object newInstance(icu::Transliterator* trans) {
    Object obj{SystemLib::classLoad(s_Transliterator.get(), c_Transliterator)};
    auto data = Native::data<Transliterator>(obj);
    data->setTransliterator(trans);
    return obj;
  }

  icu::Transliterator* trans() const { return m_trans; }
  void setTransliterator(icu::Transliterator* trans) {
    if (m_trans) {
      delete m_trans;
    }
    m_trans = trans;
  }

private:
  icu::Transliterator* m_trans{nullptr};

  static Class* c_Transliterator;
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl

