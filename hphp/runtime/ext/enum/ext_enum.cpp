/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/enum-cache.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////
// class BuiltinEnum
static Array HHVM_STATIC_METHOD(BuiltinEnum, getValues) {
  const EnumCache::EnumValues* values = EnumCache::getValuesBuiltin(self_);
  return values->values;
}

const StaticString s_overlappingErrorMessage("Enum has overlapping values");

static Array HHVM_STATIC_METHOD(BuiltinEnum, getNames) {
  const EnumCache::EnumValues* values = EnumCache::getValuesBuiltin(self_);
  if (values->names.size() != values->values.size()) {
    invoke("\\HH\\invariant_violation",
           make_packed_array(s_overlappingErrorMessage));
  }

  return values->names;
}

static bool HHVM_STATIC_METHOD(BuiltinEnum, isValid, const Variant &value) {
  if (UNLIKELY(!value.isInteger() && !value.isString())) return false;

  const EnumCache::EnumValues* values = EnumCache::getValuesBuiltin(self_);
  return values->names.exists(value);
}

static Variant HHVM_STATIC_METHOD(BuiltinEnum, coerce, const Variant &value) {
  if (UNLIKELY(!value.isInteger() && !value.isString())) {
    return Variant(Variant::NullInit{});
  }

  auto base = self_->enumBaseTy();
  Variant res = value;

  // First, if the base type is an int and we have a string containing
  // an int, do the coercion first. This saves having to also do it in
  // the array lookup (since it will be stored as an int there).
  int64_t num;
  if (base == KindOfInt64 && value.isString() &&
      value.getStringData()->isStrictlyInteger(num)) {
    res = Variant(num);
  }

  // Make sure that the value is in the map. Then, if we have an int
  // and the underlying type is a string, convert it to a string so
  // the output type is right.
  const EnumCache::EnumValues* values = EnumCache::getValuesBuiltin(self_);
  if (!values->names.exists(value)) {
    res = Variant(Variant::NullInit{});
  } else if (base && IS_STRING_TYPE(*base) && value.isInteger()) {
    res = Variant(value.toString());
  }

  return res;
}

//////////////////////////////////////////////////////////////////////////////

class enumExtension final : public Extension {
 public:
  enumExtension() : Extension("enum", "1.0.0-dev") {}
  void moduleInit() override {
    HHVM_STATIC_MALIAS(HH\\BuiltinEnum, getValues, BuiltinEnum, getValues);
    HHVM_STATIC_MALIAS(HH\\BuiltinEnum, getNames, BuiltinEnum, getNames);
    HHVM_STATIC_MALIAS(HH\\BuiltinEnum, isValid, BuiltinEnum, isValid);
    HHVM_STATIC_MALIAS(HH\\BuiltinEnum, coerce, BuiltinEnum, coerce);
    loadSystemlib();
  }
} s_enum_extension;

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
