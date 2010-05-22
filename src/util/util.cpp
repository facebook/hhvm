/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "util.h"
#include "base.h"
#include "logger.h"
#include "exception.h"
#include "network.h"

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void Util::split(char delimiter, const char *s, vector<string> &out,
                 bool ignoreEmpty /* = false */) {
  ASSERT(s);

  string token;
  for (const char *p = s; *p; p++) {
    if (*p == delimiter) {
      if (!ignoreEmpty || !token.empty()) {
        out.push_back(token);
      }
      token.clear();
    } else {
      token += *p;
    }
  }
  if (!ignoreEmpty || !token.empty()) {
    out.push_back(token);
  }
}

void Util::replaceAll(string &s, const char *from, const char *to) {
  ASSERT(from && *from);
  ASSERT(to);

  string::size_type lenFrom = strlen(from);
  string::size_type lenTo = strlen(to);
  for (string::size_type pos = s.find(from);
       pos != string::npos;
       pos = s.find(from, pos + lenTo)) {
    s.replace(pos, lenFrom, to);
  }
}

std::string Util::toLower(const std::string &s) {
  unsigned int len = s.size();
  string ret;
  if (len) {
    ret.reserve(len);
    for (unsigned int i = 0; i < len; i++) {
      ret += tolower(s[i]);
    }
  }
  return ret;
}

std::string Util::toUpper(const std::string &s) {
  unsigned int len = s.size();
  string ret;
  ret.reserve(len);
  for (unsigned int i = 0; i < len; i++) {
    ret += toupper(s[i]);
  }
  return ret;
}

std::string Util::getIdentifier(const std::string &fileName) {
  string ret = "hphp_" + fileName;
  replaceAll(ret, "/", "__");
  replaceAll(ret, ".", "__");
  replaceAll(ret, "-", "__");
  return ret;
}

bool Util::mkdir(const std::string &path, int mode /* = 0777 */) {
  if (path.empty()) {
    return false;
  }
  size_t pos = path.rfind('/');
  if (pos != string::npos) {
    // quick test whole path exists
    if (access(path.substr(0, pos).c_str(), F_OK) >= 0) {
      return true;
    }
    for (pos = path.find('/'); pos != string::npos;
         pos = path.find('/', pos + 1)) {
      string subpath = path.substr(0, pos);
      if (subpath.empty()) continue;
      if (access(subpath.c_str(), F_OK) < 0 &&
          ::mkdir(subpath.c_str(), mode) < 0) {
        Logger::Error("unable to mkdir %s", subpath.c_str());
        return false;
      }
    }
  }
  return true;
}

static bool same(const char *file1, const char *file2) {
  FILE *f1 = fopen(file1, "r");
  if (f1 == NULL) {
    Logger::Error("unable to read %s", file1);
    return false;
  }
  FILE *f2 = fopen(file2, "r");
  if (f2 == NULL) {
    fclose(f1);
    Logger::Error("unable to read %s", file2);
    return false;
  }

  bool ret = false;
  char buf1[8096];
  char buf2[sizeof(buf1)];
  int n1;
  while ((n1 = fread(buf1, 1, sizeof(buf1), f1))) {
    int toread = n1;
    int pos = 0;
    while (toread) {
      int n2 = fread(buf2 + pos, 1, toread, f2);
      if (n2 <= 0) {
        goto exit_false;
      }
      toread -= n2;
      pos += n2;
    }
    if (memcmp(buf1, buf2, n1) != 0) {
      goto exit_false;
    }
  }
  if (fread(buf2, 1, 1, f2) == 0) {
    ret = true;
  }
 exit_false:
  fclose(f2);
  fclose(f1);
  return ret;
}

void Util::syncdir(const std::string &dest_, const std::string &src_,
                   bool keepSrc /* = false */) {
  if (src_.empty() || dest_.empty()) return;

  string src = src_;
  if (src[src.size() - 1] != '/') src += '/';
  string dest = dest_;
  if (dest[dest.size() - 1] != '/') dest += '/';

  DIR *ddest = opendir(dest.c_str());
  if (ddest == NULL) {
    Logger::Error("syncdir: unable to open dest %s", dest.c_str());
    return;
  }

  DIR *dsrc = opendir(src.c_str());
  if (dsrc == NULL) {
    closedir(ddest);
    Logger::Error("syncdir: unable to open src %s", src.c_str());
    return;
  }

  dirent *e;

  set<string> todelete;
  while ((e = readdir(ddest))) {
    if (strcmp(e->d_name, ".") == 0 ||
        strcmp(e->d_name, "..") == 0) {
      continue;
    }
    string fsrc = src + e->d_name;
    string fdest = dest + e->d_name;

    // delete files/directories that are only in dest
    if (access(fsrc.c_str(), F_OK) < 0) {
      size_t pos = fdest.rfind('.');
      if (pos != string::npos) {
        string ext = fdest.substr(pos + 1);
        // do not delete intermediate files
        if (ext == "o" || ext == "d") {
          continue;
        }
      }
      todelete.insert(fdest);
      continue;
    }

    // delete mismatched types so to copy over new ones
    struct stat sb1, sb2;
    stat(fsrc.c_str(), &sb1);
    stat(fdest.c_str(), &sb2);
    if ((sb1.st_mode & S_IFMT) != (sb2.st_mode & S_IFMT)) {
      todelete.insert(fdest.c_str());
      continue;
    }

    // updates
    if ((sb1.st_mode & S_IFMT) == S_IFDIR) {
      syncdir(fdest, fsrc);
    } else if (sb1.st_size != sb2.st_size ||
               !same(fsrc.c_str(), fdest.c_str())) {
      todelete.insert(fdest);
    }
  }

  // delete the ones to delete
  if (!todelete.empty()) {
    for (set<string>::const_iterator iter = todelete.begin();
         iter != todelete.end(); ++iter) {
      Logger::Info("sync: deleting %s", iter->c_str());
      boost::filesystem::remove_all(*iter);
    }
  }

  // insert new ones
  while ((e = readdir(dsrc))) {
    string fdest = dest + e->d_name;
    if (access(fdest.c_str(), F_OK) < 0) {
      Logger::Info("sync: updating %s", fdest.c_str());
      if (keepSrc) {
        ssystem((string("cp -R ") + src + e->d_name + " " + dest).c_str());
      } else {
        rename((src + e->d_name).c_str(), (dest + e->d_name).c_str());
      }
    }
  }

  closedir(dsrc);
  closedir(ddest);
}

int Util::rename(const char *oldname, const char *newname) {
  int ret = ::rename(oldname, newname);
  if (ret == 0) return 0;
  if (errno != EXDEV) return -1;

  int oldFd = open(oldname, O_RDONLY);
  if (oldFd == -1) return -1;
  int newFd = open(newname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (newFd == -1) return -1;
  char buf[8192];
  ssize_t bytes;

  while (1) {
    bool err = false;
    bytes = read(oldFd, buf, sizeof(buf));
    if (bytes == 0) break;
    if (bytes == -1) {
      err = true;
      Logger::Error("read failed: %s", safe_strerror(errno).c_str());
    } else if (write(newFd, buf, bytes) == -1) {
      err = true;
      Logger::Error("write failed: %s", safe_strerror(errno).c_str());
    }
    if (err) {
      close(oldFd);
      close(newFd);
      return -1;
    }
  }
  close(oldFd);
  close(newFd);
  unlink(oldname);
  return 0;
}

int Util::ssystem(const char* command) {
  int ret = system(command);
  if (ret == -1) {
    Logger::Error("system(\"%s\"): %s", command, safe_strerror(errno).c_str());
  } else if (ret != 0) {
    Logger::Error("command failed: \"%s\"", command);
  }
  return ret;
}

std::string Util::safe_strerror(int errnum) {
  char buf[1024];
  return strerror_r(errnum, buf, sizeof(buf));
}

bool Util::isPowerOfTwo(int value) {
  return (value > 0 && (value & (value-1)) == 0);
}

int Util::roundUpToPowerOfTwo(int value) {
  ASSERT(value > 0);
  --value;
  for (unsigned int i = 1; i < sizeof(int)*8; i <<= 1)
    value |= value >> i;
  ++value;
  return (value);
}

std::string Util::canonicalize(const std::string &path) {
  const char *r = canonicalize(path.c_str(), path.size());
  string res(r);
  free((void*)r);
  return res;
}

/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

const char *Util::canonicalize(const char *addpath, size_t addlen) {
  // 4 for slashes at start, after root, and at end, plus trailing
  // null
  size_t maxlen = addlen + 4;
  size_t pathlen = 0; // is the length of the result path
  size_t seglen;  // is the end of the current segment

  /* Treat null as an empty path.
   */
  if (!addpath)
    addpath = "";

  char *path = (char *)malloc(maxlen);

  if (addpath[0] == '/') {
    /* Ignore the given root path, strip off leading
     * '/'s to a single leading '/' from the addpath,
     * and leave addpath at the first non-'/' character.
     */
    while (addpath[0] == '/')
      ++addpath;
    path[0] = '/';
    pathlen = 1;
  }

  while (*addpath) {
    /* Parse each segment, find the closing '/'
     */
    const char *next = addpath;
    while (*next && (*next != '/')) {
      ++next;
    }
    seglen = next - addpath;

    if (seglen == 0 || (seglen == 1 && addpath[0] == '.')) {
      /* noop segment (/ or ./) so skip it
       */
    } else if (seglen == 2 && addpath[0] == '.' && addpath[1] == '.') {
      /* backpath (../) */
      if (pathlen == 1 && path[0] == '/') {
      } else if (pathlen == 0
                 || (pathlen == 3
                     && !memcmp(path + pathlen - 3, "../", 3))
                 || (pathlen  > 3
                     && !memcmp(path + pathlen - 4, "/../", 4))) {
        /* Append another backpath, including
         * trailing slash if present.
         */
        memcpy(path + pathlen, "../", *next ? 3 : 2);
        pathlen += *next ? 3 : 2;
      } else {
        /* otherwise crop the prior segment
         */
        do {
          --pathlen;
        } while (pathlen && path[pathlen - 1] != '/');
      }
    } else {
      /* An actual segment, append it to the destination path
       */
      if (*next) {
        seglen++;
      }
      memcpy(path + pathlen, addpath, seglen);
      pathlen += seglen;
    }

    /* Skip over trailing slash to the next segment
     */
    if (*next) {
      ++next;
    }

    addpath = next;
  }
  path[pathlen] = '\0';
  return path;
}

///////////////////////////////////////////////////////////////////////////////
}
