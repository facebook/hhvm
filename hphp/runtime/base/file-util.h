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

//////////////////////////////////////////////////////////////////////

#ifndef incl_HPHP_FILE_UTIL_H_
#define incl_HPHP_FILE_UTIL_H_

#include "hphp/util/file.h"

#include "hphp/runtime/base/type-string.h"

#include <set>
#include <string>
#include <vector>

namespace HPHP { namespace FileUtil {

///////////////////////////////////////////////////////////////////////////////

/**
 * Make sure path exists. Same as "mkdir -p", but "a/b" will only make sure
 * "a/" exists, treating "b" as a file name.
 */
bool mkdir(const std::string &path, int mode = 0777);

/**
 * Make dest directory look identical to src by copying files and directories,
 * without copying identical files (so they keep the same timestamp as before).
 */
void syncdir(const std::string &dest, const std::string &src,
             bool keepSrc = false);

/**
 * Copy srcfile to dstfile, return 0 on success, -1 otherwise
 */
int copy(const char *srcfile, const char *dstfile);

/**
 * Like copy but using little disk-cache
 */
int directCopy(const char *srcfile, const char *dstfile);

/**
 * Like rename(2), but takes care of cross-filesystem rename.
 */
int rename(const char *oldname, const char *newname);

/**
 * Like rename but using little disk-cache
 */
int directRename(const char *oldname, const char *newname);

/**
 * Like system(3), but automatically print errors if execution fails.
 */
int ssystem(const char *command);

/**
 * Find the relative path from a directory with trailing slash to the file
 */
String relativePath(const std::string& fromDir, const String& toFile);

/**
 * Canonicalize path to remove "..", "." and "\/", etc..
 */
String canonicalize(const String& path);
String canonicalize(const std::string& path);
String canonicalize(const char* path, size_t len,
                    bool collapse_slashes = true);

std::string expandUser(const std::string& path,
                       const std::string& sysUser = "");
/**
 * Makes sure there is ending slash by changing "path/name" to "path/name/".
 */
std::string normalizeDir(const std::string &dirname);

/**
 * Thread-safe dirname().
 */
String dirname(const String& path);

/**
 * Search for PHP or non-PHP files under a directory.
 */
void find(std::vector<std::string> &out,
          const std::string &root, const std::string& path, bool php,
          const std::set<std::string> *excludeDirs = nullptr,
          const std::set<std::string> *excludeFiles = nullptr);

/**
 * Search for PHP or non-PHP files under a directory, calling callback for
 * each one found.
 */
template <typename F>
void find(std::vector<std::string> &out,
          const std::string &root, const std::string& path, bool php,
          const F& callback);

/**
 * Determines if a given string is a valid path or not
 * (ie: contains no null bytes)
 */
bool isValidPath(const String& path);

/**
 * Helper functions for use with FileUtil::isValidPath
 */
bool checkPathAndWarn(const String& path,
                      const char* func_name,
                      int param_pos);
void checkPathAndError(const String& path,
                       const char* func_name,
                       int param_pos);

///////////////////////////////////////////////////////////////////////////////
}
}

#endif
