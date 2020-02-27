/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/enum-cache.h"
#include "hphp/runtime/base/enum-util.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

namespace {

// Large enums get the dummy LargeEnum tag (so that we can cache a single
// static value for these enums). Small enums get a tag based on the caller.
Array tagEnumWithProvenance(Array input) {
  assertx(arrprov::getTag(input.get()));
  if (input.size() > RO::EvalArrayProvenanceLargeEnumLimit) return input;
  auto const ad = input->copy();
  arrprov::setTag<arrprov::Mode::Emplace>(ad, arrprov::tagFromPC());
  return Array::attach(ad);
}

}

//////////////////////////////////////////////////////////////////////////////
// class BuiltinEnum
static Array HHVM_STATIC_METHOD(BuiltinEnum, getValues) {
  const EnumValues* values = EnumCache::getValuesBuiltin(self_);
  assertx(values->values.isDictOrDArray());
  if (!RO::EvalLogArrayProvenance) return values->values;
  return tagEnumWithProvenance(values->values);
}

const StaticString
  s_invariant_violation("HH\\invariant_violation"),
  s_overlappingErrorMessage("Enum has overlapping values");

static Array HHVM_STATIC_METHOD(BuiltinEnum, getNames) {
  const EnumValues* values = EnumCache::getValuesBuiltin(self_);
  if (values->names.size() != values->values.size()) {
    vm_call_user_func(
      Unit::lookupFunc(s_invariant_violation.get()),
      make_vec_array(s_overlappingErrorMessage)
    );
  }

  assertx(values->names.isDictOrDArray());
  if (!RO::EvalLogArrayProvenance) return values->names;
  return tagEnumWithProvenance(values->names);
}

static bool HHVM_STATIC_METHOD(BuiltinEnum, isValid, const Variant &value) {
  return enumHasValue(self_, value.asTypedValue());
}

static Variant HHVM_STATIC_METHOD(BuiltinEnum, coerce, const Variant &value) {
  if (UNLIKELY(!value.isInteger() && !value.isString())) {
    return init_null();
  }

  auto res = value;

  // Manually do int-like string conversion. This is to ensure the lookup
  // succeeds below (since the values array does int-like string key conversion
  // when created, even if its a dict).
  int64_t num;
  if (value.isString() &&
      value.getStringData()->isStrictlyInteger(num)) {
    res = Variant(num);
  }

  auto values = EnumCache::getValuesBuiltin(self_);
  if (!values->names.exists(res)) {
    res = init_null();
  } else if (auto base = self_->enumBaseTy()) {
    if (isStringType(*base) && res.isInteger()) {
      res = Variant(res.toString());
    }
  } else {
    // If the value is present, but the enum has no base type, return the value
    // as specified, undoing any int-like string conversion we did on it.
    return value;
  }

  return res;
}

//////////////////////////////////////////////////////////////////////////////

struct enumExtension final : Extension {
  enumExtension() : Extension("enum", "1.0.0-dev") {}
  void moduleInit() override {
    HHVM_STATIC_MALIAS(HH\\BuiltinEnum, getValues, BuiltinEnum, getValues);
    HHVM_STATIC_MALIAS(HH\\BuiltinEnum, getNames, BuiltinEnum, getNames);
    HHVM_STATIC_MALIAS(HH\\BuiltinEnum, isValid, BuiltinEnum, isValid);
    HHVM_STATIC_MALIAS(HH\\BuiltinEnum, coerce, BuiltinEnum, coerce);
    HHVM_RC_STR(HH\\BUILTIN_ENUM, "HH\\BuiltinEnum");
    loadSystemlib();
  }
} s_enum_extension;

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
