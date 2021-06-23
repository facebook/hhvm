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

#include <stdexcept>
#include <exception>

#include <folly/ScopeGuard.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-refcount.h"

#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * When constant-evaluating certain operations, it's possible they
 * will return non-static objects, or throw exceptions (e.g. tvAdd()
 * with an array and an int).
 *
 * This routine converts these things back to types.  In the case of
 * an exception it returns TInitCell.
 */
template<class Pred>
Optional<Type> eval_cell(Pred p) {
  try {
    assertx(!RuntimeOption::EvalJit);
    ThrowAllErrorsSetter taes;

    TypedValue c = p();
    if (isRefcountedType(c.m_type)) tvAsVariant(&c).setEvalScalar();

    /*
     * We need to get rid of statics if we're not actually going to do
     * constant propagation.  When ConstantProp is on, the types we
     * create here can reflect that we'll be changing bytecode later
     * to actually make these into non-reference-counted SStr or
     * SArrs.  If we leave the bytecode alone, though, it generally
     * won't actually be static at runtime.
     */
    auto const t = from_cell(c);
    return options.ConstantProp ? t : loosen_staticness(t);
  } catch (const Object&) {
    return std::nullopt;
  } catch (const std::exception&) {
    return std::nullopt;
  } catch (...) {
    always_assert_flog(0, "a non-std::exception was thrown in eval_cell");
  }
}

template<typename Pred>
Optional<typename std::result_of<Pred()>::type>
eval_cell_value(Pred p) {
  try {
    ThrowAllErrorsSetter taes;
    return p();
  } catch (const Object&) {
    return std::nullopt;
  } catch (const std::exception&) {
    return std::nullopt;
  } catch (...) {
    always_assert_flog(0, "a non-std::exception was thrown in eval_cell_value");
  }
}

//////////////////////////////////////////////////////////////////////

}}
