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

#ifndef incl_HPHP_UNIT_CACHE_H_
#define incl_HPHP_UNIT_CACHE_H_

#include <string>

struct stat;

namespace HPHP {

struct Unit;
struct String;
struct StringData;

//////////////////////////////////////////////////////////////////////

/*
 * Try to get a Unit* for a php file, given a path and directory.  The actual
 * path to try to find the file at is located using these arguments,
 * resolveVmInclude, and possibly StatCache::realpath calls.
 *
 * In RepoAuthoritative mode, this will only find Units that were compiled into
 * the repo ahead of time.  Otherwise, this function may invoke the compiler to
 * create a new Unit for a file.  Units are cached across requests in either
 * case, so even in non-RepoAuthoritative mode this function won't need to
 * compile it over and over, but it does need to check whether the file has
 * changed.
 *
 * The `initial_opt' argument performs two functions, if non-null:
 *
 *    o It is an output param indicating whether whether this was the first
 *      time this unit was found in this request.  (For example, if you are
 *      doing an include_once you need to know whether it was already
 *      included.)
 *
 *    o It means we should count this lookup as a request-local lookup for the
 *      current request.
 *
 * May return nullptr if the Unit can't be loaded, and may throw exceptions or
 * fatal errors.
 */
Unit* lookupUnit(StringData* path, const char* currentDir, bool* initial_opt);

/*
 * Mangle a file's md5sum with runtime options that affect the Unit output.
 * The parser and this module need to agree on how this is done.
 */
std::string mangleUnitMd5(const std::string& fileMd5);

/*
 * Return the number of php files that are currently loaded in this process.
 * Exported for the admin request handler.
 */
size_t numLoadedUnits();

/*
 * Resolve an include path, for the supplied path and directory, using the same
 * rules as PHP's fopen() or include.  May return a null String if the path
 * would not be includable.  File stat information is returned in `s'.
 *
 * If `allow_dir' is true, this resolves the path even if it is naming a
 * directory.  Otherwise for directories a null String is returned.
 *
 * Note: it's unclear what's "vm" about this, and why it's not just
 * resolve_include.  (Likely naming relic from hphpc days.)
 */
String resolveVmInclude(StringData* path,
                        const char* currentDir,
                        struct stat* s,  // out
                        bool allow_dir = false);

void preloadRepo();

//////////////////////////////////////////////////////////////////////

}

#endif
