/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HHBBC_EVAL_CELL_H_
#define incl_HHBBC_EVAL_CELL_H_

#include <stdexcept>
#include <exception>

#include <folly/ScopeGuard.h>
#include <folly/Optional.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/repo.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * When constant-evaluating certain operations, it's possible they
 * will return non-static objects, or throw exceptions (e.g. cellAdd()
 * with an array and an int).
 *
 * This routine converts these things back to types.  In the case of
 * an exception it returns TInitCell.
 */
template<class Pred>
folly::Optional<Type> eval_cell(Pred p) {
  try {
    g_context->setThrowAllErrors(true);
    SCOPE_EXIT { g_context->setThrowAllErrors(false); };

    Cell c = p();
    if (IS_REFCOUNTED_TYPE(c.m_type)) {
      switch (c.m_type) {
      case KindOfString:
        {
          if (c.m_data.pstr->size() > Repo::get().stringLengthLimit()) {
            tvDecRef(&c);
            return TStr;
          }
          auto const sstr = makeStaticString(c.m_data.pstr);
          tvDecRef(&c);
          c = make_tv<KindOfStaticString>(sstr);
        }
        break;
      case KindOfArray:
        {
          auto const sarr = ArrayData::GetScalarArray(c.m_data.parr);
          tvDecRef(&c);
          c = make_tv<KindOfArray>(sarr);
        }
        break;
      default:
        always_assert(0 && "Impossible constant evaluation occurred");
      }
    }

    /*
     * We need to get rid of statics if we're not actually going to do
     * constant propagation.  When ConstantProp is on, the types we
     * create here can reflect that we'll be changing bytecode later
     * to actually make these into non-reference-counted SStr or
     * SArrs.  If we leave the bytecode alone, though, it generally
     * won't actually be static at runtime.
     *
     * TODO(#3696042): loosen_statics here should ideally not give up
     * on the array or string value, just its staticness.
     */
    auto const t = from_cell(c);
    return options.ConstantProp ? t : loosen_statics(t);
  } catch (const std::exception&) {
    return folly::none;
  } catch (...) {
    always_assert_flog(0, "a non-std::exception was thrown in eval_cell");
  }
}

//////////////////////////////////////////////////////////////////////

}}

#endif
