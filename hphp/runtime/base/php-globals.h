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
 * semantics.  (I.e. if the global is a KindOfRef, this will set the
 * value in its RefData.)
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
 *     auto arr = php_global_exchange(s_MyKey, Variant());
 *     arr.set(123, "foo");
 *     php_globals_set(s_MyKey, std::move(arr));
 */
Variant php_global_exchange(const StaticString& key, Variant newV);

/*
 * Strong bind a value to $GLOBALS, making the value into a Ref if it
 * isn't one already.
 */
void php_global_bind(const StaticString&, Variant&);

/*
 * Read a variable from $GLOBALS, returning a temporary for read-only
 * access.
 *
 * Returns a KindOfNull if the global didn't exist, without changing
 * $GLOBALS.
 */
Variant php_global(const StaticString&);

/*
 * Return access to $GLOBALS as an Array.
 *
 * Note that the $GLOBALS Array behaves a bit differently than normal
 * PHP arrays: it doesn't do COW, and you need to be much more careful
 * with things like lvalAt or getValueRef, since anything that can
 * invoke arbitrary php-code (e.g. a set invoking an object
 * destructor) could turn around and clobber your pointer, where
 * normally you were protected from that by the COW/value semantics of
 * php arrays.  (If this doesn't make sense, please don't try to use
 * any of those APIs with this array.)
 *
 * Sometimes, however, it's still necessary to deal with globals using
 * a generic Array in extension code.  This function gives you that,
 * but keep in mind that if you are just doing lookups, the above
 * functions will be more efficient.
 */
Array php_globals_as_array();

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/php-globals-inl.h"

#endif
