/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/php7/unit.h"

namespace HPHP { namespace php7 {

std::unique_ptr<Unit> makeFatalUnit(const std::string& filename,
                                    const std::string& msg) {
  auto unit = std::make_unique<Unit>();
  unit->name = filename;

  unit->pseudomain->cfg = CFG()
    .then(bc::String{msg})
    .then(bc::Fatal{FatalOp::Parse})
    .makeExitsReal()
    .inRegion(std::make_unique<Region>(Region::Kind::Entry));

  return unit;
}

Function* Class::getConstructor() {
  for (const auto& func : methods) {
    if (func->name == "__construct" || func->name == "86ctor") {
      return func.get();
    }
  }

  // we must not have a constructor yet so make a default one
  using namespace bc;
  auto func = makeMethod();
  func->name = "86ctor";
  func->attr |= Attr::AttrPublic;
  func->cfg = CFG({
    Null{},
    RetC{}
  }).makeExitsReal()
    .inRegion(std::make_unique<Region>(Region::Kind::Entry));

  return func;
}

}} // HPHP::php7
