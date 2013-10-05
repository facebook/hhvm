/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/hhbbc/hhbbc.h"

#include "hphp/runtime/vm/unit.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/parse.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/emit.h"
#include "hphp/hhbbc/abstract-interp.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

std::unique_ptr<UnitEmitter> single_unit(std::unique_ptr<UnitEmitter> input) {
  trace_time timer("single unit");

  php::Program program;
  program.units.push_back(parse_unit(*input));
  auto const u = borrow(program.units[0]);

  // Single-unit index.
  Index index{u};

  // Visit each method in the unit, except the pseudomain, which isn't
  // supported for anything yet.
  for (auto& c : u->classes) {
    for (auto& m : c->methods) {
      analyze_and_optimize_func(index, Context { u, borrow(m), borrow(c) });
    }
    assert(check(*c));
  }
  for (auto& f : u->funcs) {
    analyze_and_optimize_func(index, Context { u, borrow(f) });
    assert(check(*f));
  }

  assert(check(*u));
  return emit_unit(*u);
}

//////////////////////////////////////////////////////////////////////

}}

