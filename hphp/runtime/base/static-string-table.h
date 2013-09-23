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
#ifndef incl_HPHP_STATIC_STRING_TABLE_H_
#define incl_HPHP_STATIC_STRING_TABLE_H_

#include <string>

#include "hphp/util/slice.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct StringData;
struct String;
struct Array;

//////////////////////////////////////////////////////////////////////

/*
 * Process-lifetime strings are allocated using a table managed
 * through this api.
 *
 * We refer to these strings as "static strings"---they may be passed
 * around like request local strings, but have a bit set in their
 * reference count which indicates they should not actually be
 * incref'd or decref'd, and therefore are never freed.
 *
 * Note that when a static string is in a TypedValue, it may or may
 * not have KindOfStaticString.  (But no non-static strings will ever
 * have KindOfStaticString.)
 *
 * Because all constants defined in hhvm programs create a
 * process-lifetime string for the constant name, this module also
 * manages a mapping from constant names to target cache offsets.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Attempt to lookup a string (specified in various ways) in the
 * static string table.  If it's not there, create a new static string
 * and return it.
 */
StringData* makeStaticString(const StringData* str);
StringData* makeStaticString(StringSlice);
StringData* makeStaticString(const std::string& str);
StringData* makeStaticString(const String& str);
StringData* makeStaticString(const char* str, size_t len);
StringData* makeStaticString(const char* str);

/*
 * Lookup static strings for single character strings.  (We pre-create
 * static strings for all 256 characters at process startup.)
 */
StringData* makeStaticString(char c);

/*
 * Attempt to look up a static string for `str' if it exists, without
 * inserting it if not.
 *
 * Returns: a string that isStatic(), or nullptr if there was none.
 *
 * TODO(#2880477): can this have a precondition that str is not
 * static?  Also can't it assume the static string map is already
 * allocated...
 */
StringData* lookupStaticString(const StringData* str);

/*
 * Return the number of static strings in the process.
 */
size_t makeStaticStringCount();

/*
 * Functions mapping constants to target cache offsets.
 *
 * TODO(#2879005): replace uint32_t with TargetCache::CacheHandle.
 */
uint32_t lookupCnsHandle(const StringData* cnsName);
uint32_t makeCnsHandle(const StringData* cnsName, bool persistent);

/*
 * Return an array of all the defined constants in the current
 * execution context.
 */
Array lookupDefinedConstants(bool categorize = false);

//////////////////////////////////////////////////////////////////////

}

#endif
