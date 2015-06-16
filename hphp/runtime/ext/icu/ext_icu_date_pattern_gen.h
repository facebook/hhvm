#ifndef incl_HPHP_ICU_DATE_PATTERN_GEN_H
#define incl_HPHP_ICU_DATE_PATTERN_GEN_H

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/dtptngen.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_IntlDatePatternGenerator;

class IntlDatePatternGenerator : public IntlError {
 public:
  IntlDatePatternGenerator() {}
  IntlDatePatternGenerator(const IntlDatePatternGenerator&) = delete;
  IntlDatePatternGenerator& operator=(const IntlDatePatternGenerator& src)
    = delete;

  bool isValid() const {
    return m_generator.get();
  }

  static Object newInstance(
    std::unique_ptr<icu::DateTimePatternGenerator> generator
  ) {
    if (!c_IntlDatePatternGenerator) {
      c_IntlDatePatternGenerator =
        Unit::lookupClass(s_IntlDatePatternGenerator.get());
      assert(c_IntlDatePatternGenerator);
    }
    Object ret{c_IntlDatePatternGenerator};
    if (generator) {
      Native::data<IntlDatePatternGenerator>(ret)
        ->setGenerator(std::move(generator));
    }
    return ret;
  }

  static IntlDatePatternGenerator* Get(ObjectData* obj) {
    return GetData<IntlDatePatternGenerator>(obj, s_IntlDatePatternGenerator);
  }

  icu::DateTimePatternGenerator &generator() const {
    return *m_generator;
  }

  void setGenerator(std::unique_ptr<icu::DateTimePatternGenerator> generator) {
    m_generator = std::move(generator);
  }

 private:
  std::unique_ptr<icu::DateTimePatternGenerator> m_generator;

  static Class* c_IntlDatePatternGenerator;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
#endif // incl_HPHP_ICU_DATE_PATTERN_GEN_H
