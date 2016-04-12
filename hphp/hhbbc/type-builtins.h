/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HHBBC_TYPE_ARITH_BUILTINS_H_
#define incl_HHBBC_TYPE_ARITH_BUILTINS_H_

#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {
namespace php { struct Func; struct Class; }

struct Type;

//////////////////////////////////////////////////////////////////////

/*
 * Hardcoded information about builtin functions. Right now this just
 * encodes the behavior of collection methods that return $this.
 */
bool is_collection_method_returning_this(borrowed_ptr<php::Class> cls,
                                         borrowed_ptr<php::Func> func);

/*
 * Given an HNI or IDL function, figure out the real return type. Thanks to
 * ParamCoerceMode, this will either be a nullable or falsable version
 * of the declared return type.
 */
Type native_function_return_type(borrowed_ptr<const php::Func> func);

//////////////////////////////////////////////////////////////////////

}}

#endif
