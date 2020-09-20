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

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {
namespace php { struct Func; struct Class; }

struct Type;

//////////////////////////////////////////////////////////////////////

/*
 * Hardcoded information about builtin functions. Right now this just
 * encodes the behavior of collection methods that return $this.
 */
bool is_collection_method_returning_this(const php::Class* cls,
                                         const php::Func* func);

/*
 * Given an HNI function, figure out the real return type.
 */
Type native_function_return_type(const php::Func* func);

/*
 * Returns the type of the index-th inout value pushed by HNI function func.
 */
Type native_function_out_type(const php::Func* func, uint32_t index);

//////////////////////////////////////////////////////////////////////

}}

