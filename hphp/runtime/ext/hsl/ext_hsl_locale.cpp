/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/locale.h"
#include "hphp/runtime/base/thread-safe-setlocale.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/hsl/ext_hsl_locale.h"
#include "hphp/runtime/ext/hsl/hsl_locale_byte_ops.h"
#include "hphp/runtime/ext/hsl/hsl_locale_icu_ops.h"
#include "hphp/runtime/ext/hsl/hsl_locale_libc_ops.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
namespace {

const StaticString
  s_InvalidLocaleException("HH\\Lib\\Locale\\InvalidLocaleException");

} // namespace

HSLLocale::HSLLocale(std::shared_ptr<Locale> loc): m_locale(loc) {
  if (loc == Locale::getCLocale()) {
    m_ops = new HSLLocaleByteOps();
  } else if (loc->getCodesetKind() == Locale::CodesetKind::SINGLE_BYTE) {
    m_ops = new HSLLocaleLibcOps(*loc.get());
  } else {
    m_ops = new HSLLocaleICUOps(*loc.get());
  }
}

HSLLocale::~HSLLocale() {
  sweep();
}

void HSLLocale::sweep() {
  delete m_ops;
  m_locale.reset();
}

Object HSLLocale::newInstance(std::shared_ptr<Locale> loc) {
  Object obj { HSLLocale::classof() };
  auto* data = Native::data<HSLLocale>(obj);
  new (data) HSLLocale(loc);
  return obj;
}

Array HSLLocale::__debugInfo() const {
  if(!m_locale) {
    raise_fatal_error("Locale is null");
  }
  Array ret(Array::CreateDict());
  for (const auto& [category, locale] : m_locale->getAllCategoryLocaleNames()) {
    if (category == "LC_ALL") {
      // Misleading; only set as an implementation detail. If it's truly set and
      // overriding, all the other categories will be set to match anyway.
      continue;
    }
    ret.set(
      String(category.data(), category.size(), CopyString),
      String(locale.data(), locale.size(), CopyString)
    );
  }
  return ret;
}

HSLLocale* HSLLocale::fromObject(const Object& obj) {
  if (obj.isNull()) {
    raise_typehint_error_without_first_frame(
      "Expected an HSL Locale, got null");
  }
  if (!obj->instanceof(HSLLocale::classof())) {
    raise_typehint_error_without_first_frame(
      folly::sformat(
        "Expected an HSL Locale, got instance of class '{}'",
        obj->getClassName().c_str()
      )
    );
  }
  return Native::data<HSLLocale>(obj);
}

namespace {

Array HHVM_METHOD(HSLLocale, __debugInfo) {
  return Native::data<HSLLocale>(this_)->__debugInfo();
}

Object HHVM_FUNCTION(get_c_locale) {
  return HSLLocale::newInstance(Locale::getCLocale());
}

Object HHVM_FUNCTION(get_environment_locale) {
  return HSLLocale::newInstance(Locale::getEnvLocale());
}

Object HHVM_FUNCTION(get_request_locale) {
  return HSLLocale::newInstance(ThreadSafeLocaleHandler::getRequestLocale());
}

void HHVM_FUNCTION(set_request_locale, const Object& locale) {
  ThreadSafeLocaleHandler::setRequestLocale(HSLLocale::fromObject(locale)->get());
}

Object HHVM_FUNCTION(newlocale_mask,
                     int64_t mask,
                     const String& locale,
                     const Object& base) {
  auto loc = HSLLocale::fromObject(base)->get()->newlocale(LocaleCategoryMask, mask, locale.c_str());
  if (!loc) {
    throw_object(
      s_InvalidLocaleException,
      make_vec_array(folly::sformat("Invalid locale: '{}'", locale.slice()))
    );
  }
  return HSLLocale::newInstance(loc);
}

Object HHVM_FUNCTION(newlocale_category,
                     int64_t category,
                     const String& locale,
                     const Object& base) {
  auto loc = HSLLocale::fromObject(base)->get()->newlocale(LocaleCategory, category, locale.c_str());
  if (!loc) {
    throw_object(
      s_InvalidLocaleException,
      make_vec_array(folly::sformat("Invalid locale: '{}'", locale.slice()))
    );
  }
  return HSLLocale::newInstance(loc);
}

Object HHVM_FUNCTION(newlocale_all,
                     const String& locale) {
  // As this function is pure:
  // - we need to ban all the magic behavior
  // - implemented in C++ instead of Hack so that we can enforce purity in
  //   Hack code in the future.
  if (locale.isNull()) {
    throw_object(
      s_InvalidLocaleException,
      make_vec_array("Locale must not be null")
    );
  }
  if (locale.empty() || (locale.length() == 1 && locale[0] == '0')) {
    throw_object(
      s_InvalidLocaleException,
      make_vec_array("Magic locales are not supported.")
    );
  }
  return HHVM_FN(newlocale_mask)(
    LC_ALL_MASK,
    locale,
    HHVM_FN(get_c_locale)()
  );
}

struct LocaleExtension final : Extension {

  LocaleExtension() : Extension("hsl_locale", "0.1", NO_ONCALL_YET) {}

  void moduleInit() override {
    // Remember to update the HHI :)

    Native::registerNativeDataInfo<HSLLocale>();
    HHVM_NAMED_ME(HH\\Lib\\_Private\\_Locale\\Locale, __debugInfo, HHVM_MN(HSLLocale, __debugInfo));

    HHVM_FALIAS(HH\\Lib\\_Private\\_Locale\\get_c_locale, get_c_locale);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Locale\\get_environment_locale, get_environment_locale);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Locale\\get_request_locale, get_request_locale);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Locale\\set_request_locale, set_request_locale);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Locale\\newlocale_mask, newlocale_mask);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Locale\\newlocale_category, newlocale_category);
    HHVM_FALIAS(HH\\Lib\\_Private\\_Locale\\newlocale_all, newlocale_all);

#define LC_(x) \
    HHVM_RC_INT(HH\\Lib\\_Private\\_Locale\\LC_##x, LC_##x); \
    HHVM_RC_INT(HH\\Lib\\_Private\\_Locale\\LC_##x##_MASK, LC_##x##_MASK);
    LC_(ALL);
    LC_(COLLATE);
    LC_(CTYPE);
    LC_(MONETARY);
    LC_(NUMERIC);
    LC_(TIME);
#ifdef __GLIBC__
    LC_(PAPER);
    LC_(NAME);
    LC_(ADDRESS);
    LC_(TELEPHONE);
    LC_(MEASUREMENT);
    LC_(IDENTIFICATION);
#endif
#undef LC_
  }
} s_locale_extension;

} // namespace
} // namespace HPHP
