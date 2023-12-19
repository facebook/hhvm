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
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
namespace {
  Array HHVM_FUNCTION(HH_regex_match, const String& haystack,
                      const String& pattern, int64_t& offset) {
    // TODO(alexeyt): make a version of preg_match that behaves how we want
    // instead of working around it here
    Variant matches, error;
    auto const& status = [&]{
      auto oldERL = RID().getErrorReportingLevel();
      SCOPE_EXIT { RID().setErrorReportingLevel(oldERL); };
      RID().setErrorReportingLevel(0);
      PregWithErrorGuard guard(error);
      return preg_match(pattern.get(), haystack.get(), &matches,
                        PREG_FB__PRIVATE__HSL_IMPL | PREG_OFFSET_CAPTURE,
                        offset);
    }();

    // preg_match will return 0, 1, false, or null
    if (status.isBoolean()) {
      assertx(status.asBooleanVal() == false);
      assertx(error.isInteger());
      return make_vec_array(init_null(), std::move(error));
    }
    if (status.isNull()) {
      assertx(error.isInteger());
      return make_vec_array(init_null(), std::move(error));
    }

    assertx(status.isInteger());
    assertx(error.isNull());

    if (status.asInt64Val() == 0) {
      return make_vec_array(init_null(), init_null()); // no match
    }

    // match
    assertx(status.asInt64Val() == 1);
    assertx(matches.isArray());
    auto matches_out = DictInit(matches.asCArrRef().size());
    IterateKV(
      matches.asCArrRef().get(),
      [&](TypedValue k, TypedValue v) {
        assertx(isArrayLikeType(type(v)));
        assertx(val(v).parr->exists(int64_t(0)));
        matches_out.setValidKey(k, val(v).parr->at(int64_t(0)));
      }
    );
    assertx(matches.asCArrRef().exists(0));
    assertx(matches.asCArrRef()[0].isArray());
    assertx(matches.asCArrRef()[0].asCArrRef().exists(1));
    assertx(matches.asCArrRef()[0].asCArrRef()[1].isInteger());
    offset = matches.asCArrRef()[0].asCArrRef()[1].asInt64Val();
    return make_vec_array(matches_out.toArray(), init_null());
  }

  Array HHVM_FUNCTION(HH_regex_replace, const String& haystack,
                      const String& pattern, const String& replacement) {
    // TODO(alexeyt): make a version of preg_replace that behaves how we want
    // instead of working around it here
    Variant error;
    auto const& replaced = [&]{
      auto oldERL = RID().getErrorReportingLevel();
      SCOPE_EXIT { RID().setErrorReportingLevel(oldERL); };
      RID().setErrorReportingLevel(0);
      PregWithErrorGuard guard(error);
      return preg_replace_impl(pattern, replacement, haystack, -1,
                               nullptr, false, false);
    }();

    // haystack is a string so preg_replace_impl returns string, false, or null
    if (replaced.isString()) {
      assertx(error.isNull());
      return make_vec_array(std::move(replaced), init_null());
    }

    assertx(error.isInteger());
    if (replaced.isBoolean()) {
      assertx(replaced.asBooleanVal() == false);
    } else {
      assertx(replaced.isNull());
    }
    return make_vec_array(init_null(), std::move(error));
  }

  struct RegexExtension final : Extension {
    RegexExtension() : Extension("hsl_regex", "0.1", NO_ONCALL_YET) {}

    void moduleInit() override {
      HHVM_FALIAS(
        HH\\Lib\\_Private\\_Regex\\match,
        HH_regex_match
      );
      HHVM_FALIAS(
        HH\\Lib\\_Private\\_Regex\\replace,
        HH_regex_replace
      );
    }
  } s_regex_extension;

} // anonymous namespace
} // namespace HPHP
