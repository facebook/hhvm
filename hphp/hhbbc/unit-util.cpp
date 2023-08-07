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
#include "hphp/hhbbc/unit-util.h"

#include "hphp/hhbbc/representation.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/vm/native.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

namespace {
const StaticString s_nativeUnitName("/!native");
}

//////////////////////////////////////////////////////////////////////

bool is_systemlib_part(SString filename) {
  return FileUtil::isSystemName(filename->slice());
}

bool is_systemlib_part(const php::Unit& unit) {
  return is_systemlib_part(unit.filename);
}

//////////////////////////////////////////////////////////////////////

bool is_native_unit(SString unit) {
  return unit == s_nativeUnitName.get();
}

bool is_native_unit(const php::Unit& unit) {
  return is_native_unit(unit.filename);
}

std::unique_ptr<php::Unit> make_native_unit() {
  auto unit = std::make_unique<php::Unit>();
  unit->filename = s_nativeUnitName.get();

  for (auto const& [name, val] : Native::getConstants()) {
    assertx(type(val) != KindOfUninit);
    auto cns = std::make_unique<php::Constant>();
    cns->name = name;
    cns->val = val;
    cns->attrs = AttrUnique | AttrPersistent;
    unit->constants.emplace_back(std::move(cns));
  }

  // Keep a consistent ordering of the constants.
  std::sort(
    begin(unit->constants),
    end(unit->constants),
    [] (const std::unique_ptr<php::Constant>& a,
        const std::unique_ptr<php::Constant>& b) {
      return string_data_lt{}(a->name, b->name);
    }
  );

  return unit;
}

//////////////////////////////////////////////////////////////////////

}
