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
#ifndef incl_HPHP_PHP_GLOBALS_H_
#define incl_HPHP_PHP_GLOBALS_H_

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct StaticString;
struct Variant;
struct TypedValue;
struct Array;

//////////////////////////////////////////////////////////////////////

/*
 * Set a $GLOBALS[foo] to the supplied value, using normal php set
 * semantics.
 */
void php_global_set(const StaticString&, Variant);

/*
 * Exchange a value in $GLOBALS.
 *
 * This sets $GLOBALS[key] to newV, and returns its previous value.
 * This is sometimes particularly useful if you want to temporarily
 * take an Array out of $GLOBALS so it can be modified with its
 * reference count as one, and then stored back in.
 *
 * For example:
 *
 *     auto arr = php_global_exchange(s_MyKey, uninit_null());
 *     arr.set(123, "foo");
 *     php_global_set(s_MyKey, std::move(arr));
 */
Variant php_global_exchange(const StaticString& key, Variant newV);

/*
 * Read a variable from $GLOBALS, returning a temporary for read-only
 * access.
 *
 * Returns a KindOfNull if the global didn't exist, without changing
 * $GLOBALS.
 */
Variant php_global(const StaticString&);

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/php-globals-inl.h"

#endif
