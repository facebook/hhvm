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

#ifndef incl_HPHP_FAST_STAT_CACHE_H_
#define incl_HPHP_FAST_STAT_CACHE_H_

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <sstream>
#include <unordered_map>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * FastStatCache cache the result of stat/lstat/access/readlink/realpath system call
 * in the memory for fast access.
 * The result may not be the latest status of the file system. If the file system changes, 
 * the cache will update within Server.FastStatCacheTTL seconds.
 */
class FastStatCache {
  public:
    static int stat(const char* path, struct stat* buf);
    static int lstat(const char* path, struct stat* buf);
    static int access(const char* path, int mode);
    static std::string readlink(const char* path);
    static std::string realpath(const char* path);

    static void clearAllCache();
    static void getStatistics(std::stringstream& ss);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_FAST_STAT_CACHE_H_
