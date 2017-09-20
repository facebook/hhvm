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
#ifndef incl_HPHP_FUNC_UTIL_H_
#define incl_HPHP_FUNC_UTIL_H_

#include <cstdint>

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {
namespace php { struct Func; struct Local; }

//////////////////////////////////////////////////////////////////////

/*
 * Return the number of use-vars for a closure body.
 *
 * Pre: f->isClosureBody
 */
uint32_t closure_num_use_vars(borrowed_ptr<const php::Func>);

/*
 * Returns whether a given php::Func is the pseudomain of its unit.
 */
bool is_pseudomain(borrowed_ptr<const php::Func>);

/*
 * Locals with certain special names can be set in the enclosing scope by
 * various php routines.  We don't attempt to track their types.  Furthermore,
 * in a pseudomain effectively all 'locals' are volatile, because any re-entry
 * could modify them through $GLOBALS, so in a pseudomain we don't track any
 * local types.
 */
bool is_volatile_local(borrowed_ptr<const php::Func>, LocalId);

/*
 * Given a function which is a memoize wrapper, return the name of the function
 * that the wrapper is wrapping.
 */
SString memoize_impl_name(borrowed_ptr<const php::Func>);

/*
 * Check that passing nArgs params to func has a chance of not warning.
 */
bool check_nargs_in_range(borrowed_ptr<const php::Func> func, uint32_t nArgs);


//////////////////////////////////////////////////////////////////////

}}

#endif
