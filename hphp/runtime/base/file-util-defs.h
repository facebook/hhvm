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

#ifndef incl_HPHP_FILE_UTIL_DEFS_H_
#define incl_HPHP_FILE_UTIL_DEFS_H_

#include "hphp/runtime/base/file-util.h"
#include "hphp/util/logger.h"

#include <string>
#include <fstream>

namespace HPHP { namespace FileUtil {

///////////////////////////////////////////////////////////////////////////////

template <typename F>
void find(const std::string &root, const std::string& path, bool php,
          const F& callback) {
  auto spath = path.empty() || !isDirSeparator(path[0]) ?
    path : path.substr(1);

  if (!spath.empty() && !isDirSeparator(spath.back())) {
    spath += getDirSeparator();
  }
  auto fullPath = root + spath;
  if (fullPath.empty()) {
    return;
  }
  if (!callback(spath, true)) {
    return;
  }

  DIR *dir = opendir(fullPath.c_str());
  if (dir == nullptr) {
    Logger::Error("FileUtil::find(): unable to open directory %s",
                  fullPath.c_str());
    return;
  }

  dirent *e;
  while ((e = readdir(dir))) {
    char *ename = e->d_name;

    // skipping .  .. hidden files
    if (ename[0] == '.' || !*ename) {
      continue;
    }
    auto fe = fullPath + ename;
    struct stat se;
    if (stat(fe.c_str(), &se)) {
      Logger::Error("FileUtil::find(): unable to stat %s", fe.c_str());
      continue;
    }

    if ((se.st_mode & S_IFMT) == S_IFDIR) {
      find(root, spath + ename, php, callback);
      continue;
    }

    // skipping "tags" files
    if (strcmp(ename, "tags") == 0) {
      continue;
    }

    // skipping emacs leftovers
    char last = ename[strlen(ename) - 1];
    if (last == '~' || last == '#') {
      continue;
    }

    bool isPHP = false;
    const char *p = strrchr(ename, '.');
    if (p) {
      isPHP = (strncmp(p + 1, "php", 3) == 0);
    } else {
      try {
        std::string line;
        std::ifstream fin(fe.c_str());
        if (std::getline(fin, line)) {
          if (line[0] == '#' && line[1] == '!' &&
              line.find("php") != std::string::npos) {
            isPHP = true;
          }
        }
      } catch (...) {
        Logger::Error("FileUtil::find(): unable to read %s", fe.c_str());
      }
    }

    if (isPHP == php) {
      callback(spath + ename, false);
    }
  }

  closedir(dir);
}

///////////////////////////////////////////////////////////////////////////////
}
}

#endif
