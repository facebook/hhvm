#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/dtptngen.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////

struct IntlDatePatternGenerator
    : IntlError, SystemLib::ClassLoader<"IntlDatePatternGenerator"> {
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
    Object ret{ classof() };
    if (generator) {
      Native::data<IntlDatePatternGenerator>(ret)
        ->setGenerator(std::move(generator));
    }
    return ret;
  }

  static IntlDatePatternGenerator* Get(ObjectData* obj) {
    return GetData<IntlDatePatternGenerator>(obj, className());
  }

  icu::DateTimePatternGenerator &generator() const {
    return *m_generator;
  }

  void setGenerator(std::unique_ptr<icu::DateTimePatternGenerator> generator) {
    m_generator = std::move(generator);
  }

 private:
  std::unique_ptr<icu::DateTimePatternGenerator> m_generator;
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
