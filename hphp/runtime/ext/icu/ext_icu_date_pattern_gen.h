#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/dtptngen.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_IntlDatePatternGenerator;

struct IntlDatePatternGenerator : IntlError {
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
    Object ret{ SystemLib::classLoad(s_IntlDatePatternGenerator.get(),
                                     c_IntlDatePatternGenerator) };
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
} // namespace HPHP::Intl
