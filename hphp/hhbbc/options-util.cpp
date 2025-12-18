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
#include "hphp/hhbbc/options-util.h"

#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/unit-util.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

#ifdef HPHP_TRACE

namespace {

bool method_map_contains(const MethodMap& mmap,
                         SString cls,
                         SString func,
                         SString unit) {
  if (unit) {
    if (auto const m = folly::get_ptr(mmap, unit->toCppString())) {
      return m->empty();
    }
  }
  if (cls) {
    if (auto const m = folly::get_ptr(mmap, cls->toCppString())) {
      if (m->empty()) return true;
      return func && m->count(func->toCppString());
    }
    return false;
  }
  if (func) {
    if (auto const m = folly::get_ptr(mmap, func->toCppString())) {
      return m->empty();
    }
  }
  return false;
}

}

//////////////////////////////////////////////////////////////////////

bool is_trace_function(const php::Class* inCls,
                       const php::Func* func,
                       const php::Unit* inUnit) {
  auto const unit = [&] () -> SString {
    if (inUnit) return inUnit->filename;
    if (inCls) return inCls->unit;
    if (func) return func->unit;
    return nullptr;
  }();
  auto const cls = [&] () -> SString {
    if (inCls) return inCls->name;
    if (func && func->cls) return func->cls->name;
    return nullptr;
  }();
  return is_trace_function(cls,
                           func ? func->name : nullptr,
                           unit);
}

bool is_trace_function(const Context& ctx) {
  auto const unit = [&] () -> SString {
    if (ctx.unit) return ctx.unit;
    if (ctx.cls) return ctx.cls->unit;
    if (ctx.func) return ctx.func->unit;
    return nullptr;
  }();
  auto const cls = [&] () -> SString {
    if (ctx.cls) return ctx.cls->name;
    if (ctx.func && ctx.func->cls) return ctx.func->cls->name;
    return nullptr;
  }();
  return is_trace_function(cls,
                           ctx.func ? ctx.func->name : nullptr,
                           unit);
}

bool is_trace_function(SString cls,
                       SString func,
                       SString unit) {
  return method_map_contains(options.TraceFunctions, cls, func, unit);
}

int trace_bump_for(const php::Class* inCls,
                   const php::Func* func,
                   const php::Unit* inUnit) {
  auto const unit = [&] () -> SString {
    if (inUnit) return inUnit->filename;
    if (inCls) return inCls->unit;
    if (func) return func->unit;
    return nullptr;
  }();
  auto const cls = [&] () -> SString {
    if (inCls) return inCls->name;
    if (func && func->cls) return func->cls->name;
    return nullptr;
  }();
  return trace_bump_for(cls,
                        func ? func->name : nullptr,
                        unit);
}

int trace_bump_for(const Context& ctx) {
  auto const unit = [&] () -> SString {
    if (ctx.unit) return ctx.unit;
    if (ctx.cls) return ctx.cls->unit;
    if (ctx.func) return ctx.func->unit;
    return nullptr;
  }();
  auto const cls = [&] () -> SString {
    if (ctx.cls) return ctx.cls->name;
    if (ctx.func && ctx.func->cls) return ctx.func->cls->name;
    return nullptr;
  }();
  return trace_bump_for(cls,
                        ctx.func ? ctx.func->name : nullptr,
                        unit);
}

int trace_bump_for(SString cls,
                   SString func,
                   SString unit) {
  return is_trace_function(cls, func, unit) ? kTraceFuncBump :
    ((unit && is_systemlib_part(unit)) ? kSystemLibBump : 0);
}

#endif

//////////////////////////////////////////////////////////////////////

}
