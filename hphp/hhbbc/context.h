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

#include <folly/Hash.h>

#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"
#include "hphp/hhbbc/wide-func.h"

#include "hphp/util/compact-vector.h"

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

struct CollectedInfo;
struct ContextHash;
struct FuncAnalysis;
struct Index;

//////////////////////////////////////////////////////////////////////

/*
 * A Context is a (unit, func, class) triple, where cls and func
 * fields may be null in some situations.  Most queries to the Index
 * need a "context", to allow recording dependencies.
 */
struct Context {
  SString unit;
  const php::Func* func;
  const php::Class* cls;
  const Context* dep{nullptr};

  const Context& forDep() const { return dep ? *dep : *this; }
};

/*
 * Context for a call to a function.  This is the function itself,
 * plus the types and number of arguments, and the type of
 * $this/get_called_context().
 */
struct CallContext {
  const php::Func* callee;
  CompactVector<Type> args;
  Type context;
};

inline bool operator==(const CallContext& a, const CallContext& b) {
  return a.callee == b.callee &&
         equivalently_refined(a.args, b.args) &&
         equivalently_refined(a.context, b.context);
}

struct CallContextHasher {
  size_t operator()(const CallContext& c) const {
    auto ret = folly::hash::hash_combine(
      c.callee,
      c.args.size(),
      c.context.hash()
    );
    for (auto& t : c.args) {
      ret = folly::hash::hash_combine(ret, t.hash());
    }
    return ret;
  }
};

struct CallContextHashCompare {
  bool equal(const CallContext& a, const CallContext& b) const {
    return a == b;
  }
  size_t hash(const CallContext& c) const {
    return CallContextHasher{}(c);
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * Context used to analyze a function. Anyone constructing this context
 * must ensure that the provided WideFunc lives longer than the context.
 */
struct AnalysisContext {
  SString unit;
  const php::WideFunc& func;
  const php::Class* cls;
  const Context* dep{nullptr};

  operator Context() const { return { unit, func, cls, dep }; }
};

/*
 * Context used for per-block optimizations. As with AnalysisContext,
 * callers must ensure that all the references here outlive this context.
 * The WideFunc in this struct will always match the func in ainfo.ctx.
 */
struct VisitContext {
  const Index& index;
  const FuncAnalysis& ainfo;
  CollectedInfo& collect;
  php::WideFunc& func;

  VisitContext(const Index& index, const FuncAnalysis& ainfo,
               CollectedInfo& collect, php::WideFunc& func);
};

//////////////////////////////////////////////////////////////////////

}
