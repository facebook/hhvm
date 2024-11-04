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
#include "hphp/hhbbc/type-builtins.h"

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

Type native_function_return_type(const php::Func* f) {
  assertx(f->isNative);

  // Infer the type from the HNI declaration
  auto t = [&]{
    auto const hni = f->retTypeConstraints.main().asSystemlibType();
    return hni ? from_DataType(*hni) : TInitCell;
  }();

  // Non-simple types (ones that are represented by pointers) can always
  // possibly be null.
  if (t.subtypeOf(BStr | BArrLike | BObj | BRes)) {
    t = opt(std::move(t));
  } else {
    // Otherwise it should be a simple type or possibly everything.
    assertx(t == TInitCell || t.subtypeOf(BBool | BInt | BDbl | BNull));
  }

  t = remove_uninit(std::move(t));
  if (!f->hasInOutArgs) return t;

  std::vector<Type> types;
  types.emplace_back(std::move(t));

  for (auto const& p : f->params) {
    if (!p.inout) continue;
    auto const dt =
      Native::builtinOutType(p.typeConstraints.main(), p.userAttributes);
    if (!dt) {
      types.emplace_back(TInitCell);
      continue;
    }
    auto t = from_DataType(*dt);
    if (p.typeConstraints.main().isNullable()) t = opt(std::move(t));
    types.emplace_back(remove_uninit(std::move(t)));
  }
  std::reverse(types.begin()+1, types.end());

  return vec(std::move(types));
}

}
