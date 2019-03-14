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
#ifndef incl_HHBBC_CONTEXT_H_
#define incl_HHBBC_CONTEXT_H_

#include <folly/Hash.h>
#include <folly/Optional.h>

#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-system.h"

#include "hphp/util/compact-vector.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct ContextHash;

//////////////////////////////////////////////////////////////////////

/*
 * A Context is a (unit, func, class) triple, where cls and func
 * fields may be null in some situations.  Most queries to the Index
 * need a "context", to allow recording dependencies.
 */
struct Context {
  const php::Unit* unit;
  php::Func* func;
  const php::Class* cls;

  using Hash = ContextHash;
};
struct ContextHash {
  size_t operator()(const Context& c) const {
    return pointer_hash<void>{}(c.func ? (void*)c.func :
                                c.cls ? (void*)c.cls : (void*)c.unit);
  }
};

inline bool operator==(Context a, Context b) {
  return a.unit == b.unit && a.func == b.func && a.cls == b.cls;
}

inline bool operator<(Context a, Context b) {
  return std::make_tuple(a.unit, a.func, a.cls) <
         std::make_tuple(b.unit, b.func, b.cls);
}

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

//////////////////////////////////////////////////////////////////////

}}

#endif
