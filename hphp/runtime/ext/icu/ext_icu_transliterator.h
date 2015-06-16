#ifndef incl_HPHP_ICU_TRANSLITERATOR_H
#define incl_HPHP_ICU_TRANSLITERATOR_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/translit.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_Transliterator;

class Transliterator : public IntlError {
public:
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
    if (!c_Transliterator) {
      c_Transliterator = Unit::lookupClass(s_Transliterator.get());
      assert(c_Transliterator);
    }
    Object obj{c_Transliterator};
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
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_TRANSLITERATOR_H
