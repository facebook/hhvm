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

#include <cstdint>

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"

namespace HPHP::HHBBC {
namespace php { struct Func; struct Local; struct Block; }

//////////////////////////////////////////////////////////////////////

/*
 * Return the number of use-vars for a closure body.
 *
 * Pre: f->isClosureBody
 */
uint32_t closure_num_use_vars(const php::Func*);

/*
 * Locals with certain special names can be set in the enclosing scope by
 * various php routines.  We don't attempt to track their types.  Furthermore,
 * in a pseudomain effectively all 'locals' are volatile, because any re-entry
 * could modify them through $GLOBALS, so in a pseudomain we don't track any
 * local types.
 */
bool is_volatile_local(const php::Func*, LocalId);

/*
 * Given a function which is a memoize wrapper, return the name of the function
 * that the wrapper is wrapping.
 */
SString memoize_impl_name(const php::Func*);

/*
 * Check that passing nArgs params to func has a chance of not warning.
 */
bool check_nargs_in_range(const php::Func* func, uint32_t nArgs);

/*
 * Append the body of src to dst, such that all returns from dst
 * branch to src instead.
 */
bool append_func(php::Func* dst, const php::Func& src);

/*
 * Append the cases of the switch from one 86cinit to another 86cinit.
 */
bool append_86cinit(php::Func* dst, const php::Func& src);

/*
 * Create a block similar to another block (but with no bytecode in it yet).
 *
 * It will have the same exnNodeId, and throw exit block.
 */
BlockId make_block(php::WideFunc& func, const php::Block* srcBlk);

/*
 * Based on runtime options returns action that should be taken on dynamic
 * call to function: 0 - no action, 1 - notice, 2 -throw exception.
*/
int dyn_call_error_level(const php::Func*);

/*
 * Does this function have a coeffects local?
 */
bool has_coeffects_local(const php::Func*);

std::string func_fullname(const php::Func&);

/*
 * Whether this function is one of the special 86*init functions on a
 * class.
 */
bool is_86init_func(const php::Func&);

//////////////////////////////////////////////////////////////////////

}
