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
#pragma once

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"

#include "hphp/util/trace.h"

namespace HPHP::HHBBC {

struct Context;

namespace php {
struct Class;
struct Func;
struct Unit;
}

//////////////////////////////////////////////////////////////////////

bool is_trace_function(const php::Class*,
                       const php::Func*,
                       const php::Unit* = nullptr);
bool is_trace_function(SString cls, SString func, SString unit);
bool is_trace_function(const Context&);

int trace_bump_for(const php::Class*,
                   const php::Func*,
                   const php::Unit* = nullptr);
int trace_bump_for(SString cls, SString func, SString unit);
int trace_bump_for(const Context&);

//////////////////////////////////////////////////////////////////////

template <typename... T>
std::array<Trace::Bump, sizeof...(T)> trace_bump(const Context& ctx,
                                                 T... mods) {
  auto const b = trace_bump_for(ctx);
  return { Trace::Bump{mods, b}... };
}

//////////////////////////////////////////////////////////////////////

}
