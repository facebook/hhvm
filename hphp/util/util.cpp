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

#include "util.h"
#include "hphp/util/base.h"
#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/exception.h"
#include "hphp/util/network.h"
#include "folly/String.h"

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static Mutex s_file_mutex;

void Util::split(char delimiter, const char *s, vector<string> &out,
                 bool ignoreEmpty /* = false */) {
  assert(s);

  const char *start = s;
  const char *p = s;
  for (; *p; p++) {
    if (*p == delimiter) {
      if (!ignoreEmpty || p > start) {
        out.push_back(string(start, p - start));
      }
      start = p + 1;
    }
  }
  if (!ignoreEmpty || p > start) {
    out.push_back(string(start, p - start));
  }
}

void Util::replaceAll(string &s, const char *from, const char *to) {
  assert(from && *from);
  assert(to);

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
      return false;
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
  if (f1 == nullptr) {
    Logger::Error("unable to read %s", file1);
    return false;
  }
  FILE *f2 = fopen(file2, "r");
  if (f2 == nullptr) {
    fclose(f1);
    Logger::Error("unable to read %s", file2);
    return false;
  }

  bool ret = false;
  char buf1[8192];
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
  if (ddest == nullptr) {
    Logger::Error("syncdir: unable to open dest %s", dest.c_str());
    return;
  }

  DIR *dsrc = opendir(src.c_str());
  if (dsrc == nullptr) {
    closedir(ddest);
    Logger::Error("syncdir: unable to open src %s", src.c_str());
    return;
  }

  dirent *e;

  std::set<string> todelete;
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
    for (std::set<string>::const_iterator iter = todelete.begin();
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

int Util::copy(const char *srcfile, const char *dstfile) {
  int srcFd = open(srcfile, O_RDONLY);
  if (srcFd == -1) return -1;
  int dstFd = open(dstfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (dstFd == -1) return -1;

  while (1) {
    char buf[8192];
    bool err = false;
    ssize_t rbytes = read(srcFd, buf, sizeof(buf));
    ssize_t wbytes;
    if (rbytes == 0) break;
    if (rbytes == -1) {
      err = true;
      Logger::Error("read failed: %s", folly::errnoStr(errno).c_str());
    } else if ((wbytes = write(dstFd, buf, rbytes)) != rbytes) {
      err = true;
      Logger::Error("write failed: %zd, %s", wbytes,
                    folly::errnoStr(errno).c_str());
    }
    if (err) {
      close(srcFd);
      close(dstFd);
      return -1;
    }
  }
  close(srcFd);
  close(dstFd);
  return 0;
}

static int force_sync(int fd) {
#if defined(__FreeBSD__) || defined(__APPLE__)
  return fsync(fd);
#else
  return fdatasync(fd);
#endif
}

int Util::drop_cache(int fd, off_t len /* = 0 */) {
#if defined(__FreeBSD__) || defined(__APPLE__)
  return 0;
#else
  return posix_fadvise(fd, 0, len, POSIX_FADV_DONTNEED);
#endif
}

int Util::drop_cache(FILE *f, off_t len /* = 0 */) {
  return drop_cache(fileno(f), len);
}


int LogFileFlusher::DropCacheChunkSize = (1 << 20);
void LogFileFlusher::recordWriteAndMaybeDropCaches(int fd, int bytes) {
  int oldBytesWritten = m_bytesWritten.fetch_add(bytes);
  int newBytesWritten = oldBytesWritten + bytes;

  if (!(newBytesWritten > DropCacheChunkSize &&
        oldBytesWritten <= DropCacheChunkSize)) {
    return;
  }

  off_t offset = lseek(fd, 0, SEEK_CUR);
  if (offset > kDropCacheTail) {
    dropCache(fd, offset - kDropCacheTail);
  }

  m_bytesWritten = 0;
}

int Util::directCopy(const char *srcfile, const char *dstfile) {
  int srcFd = open(srcfile, O_RDONLY);
  if (srcFd == -1) return -1;
  int dstFd = open(dstfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (dstFd == -1) return -1;

#if defined(__APPLE__)
  fcntl(srcFd, F_NOCACHE, 1);
  fcntl(dstFd, F_NOCACHE, 1);
#endif

  while (1) {
    char buf[1 << 20];
    bool err = false;
    ssize_t rbytes = read(srcFd, buf, sizeof(buf));
    ssize_t wbytes;
    if (rbytes == 0) break;

    if (rbytes == -1) {
      err = true;
      Logger::Error("read failed: %s", folly::errnoStr(errno).c_str());
    } else if (force_sync(srcFd) == -1) {
      err = true;
      Logger::Error("read sync failed: %s",
                    folly::errnoStr(errno).c_str());
    } else if (drop_cache(srcFd) == -1) {
      err = true;
      Logger::Error("read cache drop failed: %s",
                    folly::errnoStr(errno).c_str());
    } else if ((wbytes = write(dstFd, buf, rbytes)) != rbytes) {
      err = true;
      Logger::Error("write failed: %zd, %s", wbytes,
                    folly::errnoStr(errno).c_str());
    } else if (force_sync(dstFd) == -1) {
      err = true;
      Logger::Error("write sync failed: %s",
                    folly::errnoStr(errno).c_str());
    } else if (drop_cache(dstFd) == -1) {
      err = true;
      Logger::Error("write cache drop failed: %s",
                    folly::errnoStr(errno).c_str());
    }
    if (err) {
      close(srcFd);
      close(dstFd);
      return -1;
    }
  }
  close(srcFd);
  close(dstFd);
  return 0;
}

int Util::rename(const char *oldname, const char *newname) {
  int ret = ::rename(oldname, newname);
  if (ret == 0) return 0;
  if (errno != EXDEV) return -1;

  copy(oldname, newname);
  unlink(oldname);
  return 0;
}

int Util::directRename(const char *oldname, const char *newname) {
  int ret = ::rename(oldname, newname);
  if (ret == 0) return 0;
  if (errno != EXDEV) return -1;

  directCopy(oldname, newname);
  unlink(oldname);
  return 0;
}

int Util::ssystem(const char* command) {
  int ret = system(command);
  if (ret == -1) {
    Logger::Error("system(\"%s\"): %s", command,
                  folly::errnoStr(errno).c_str());
  } else if (ret != 0) {
    Logger::Error("command failed: \"%s\"", command);
  }
  return ret;
}

size_t Util::dirname_helper(char *path, int len) {
  if (len == 0) {
    /* Illegal use of this function */
    return 0;
  }

  /* Strip trailing slashes */
  register char *end = path + len - 1;
  while (end >= path && *end == '/') {
    end--;
  }
  if (end < path) {
    /* The path only contained slashes */
    path[0] = '/';
    path[1] = '\0';
    return 1;
  }

  /* Strip filename */
  while (end >= path && *end != '/') {
    end--;
  }
  if (end < path) {
    /* No slash found, therefore return '.' */
    path[0] = '.';
    path[1] = '\0';
    return 1;
  }

  /* Strip slashes which came before the file name */
  while (end >= path && *end == '/') {
    end--;
  }
  if (end < path) {
    path[0] = '/';
    path[1] = '\0';
    return 1;
  }
  *(end+1) = '\0';

  return end + 1 - path;
}

std::string Util::safe_dirname(const char *path, int len) {
  char* tmp_path = (char*)malloc(len+1);
  memcpy(tmp_path, path, len);
  tmp_path[len] = '\0';
  size_t newLen = dirname_helper(tmp_path, len);
  std::string ret;
  ret.assign(tmp_path, newLen);
  free((void *) tmp_path);
  return ret;
}

std::string Util::safe_dirname(const char *path) {
  int len = strlen(path);
  return safe_dirname(path, len);
}

std::string Util::safe_dirname(const std::string& path) {
  return safe_dirname(path.c_str(), path.size());
}

std::string Util::relativePath(const std::string fromDir,
  const std::string toFile) {

  size_t maxlen = (fromDir.size() + toFile.size()) * 3;

  // Sanity checks
  if (fromDir[0] != '/' || toFile[0] != '/' ||
      fromDir[fromDir.size() - 1] != '/') {
    return "";
  }

  // Maybe we're lucky and this is an easy case
  int from_len = fromDir.size();
  if (strncmp(toFile.c_str(), fromDir.c_str(), from_len) == 0) {
    return toFile.substr(from_len);
  }

  char* path = (char*) malloc(maxlen);
  int path_len = 0;

  const char* from_dir = fromDir.c_str();
  const char* to_file = toFile.c_str();

  const char* from_start = from_dir + 1;
  const char* to_start = to_file + 1;

  while (true) {
    int seg_len = 0;
    char cur = from_start[0];
    while (cur && cur != '/') {
      ++seg_len;
      cur = from_start[seg_len];
    }

    if (memcmp(from_start, to_start, seg_len + 1)) {
      break;
    }
    from_start += seg_len + 1;
    to_start += seg_len + 1;
  }

  // Now to build the path
  char cur = *from_start;
  char* path_end = path;
  while (cur) {
    if (cur == '/') {
      strcpy(path_end, "../");
      path_len += 3;
      maxlen -= 3;
      path_end += 3;
    }
    ++from_start;
    cur = *from_start;
  }

  if (from_start[-1] != '/') {
    strcpy(path_end, "../");
    path_len += 3;
    maxlen -= 3;
    path_end += 3;
  }

  strncpy(path_end, to_start, maxlen - 1);

  string p(path);
  free((void*) path);

  return p;
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

char* Util::canonicalize(const char *addpath, size_t addlen,
                         bool collapse_slashes /* = true */) {
  assert(strlen(addpath) == addlen);
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

  if (addpath[0] == '/' && collapse_slashes) {
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

    if (seglen == 0) {
      /* / */
      if (!collapse_slashes) {
        path[pathlen++] = '/';
      }
    } else if (seglen == 1 && addpath[0] == '.') {
      /* ./ */
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
        /* otherwise crop the prior segment */
        do {
          --pathlen;
        } while (pathlen && path[pathlen - 1] != '/');
      }
    } else {
      /* An actual segment, append it to the destination path */
      if (*next) {
        seglen++;
      }
      memcpy(path + pathlen, addpath, seglen);
      pathlen += seglen;
    }

    /* Skip over trailing slash to the next segment */
    if (*next) {
      ++next;
    }

    addpath = next;
  }
  path[pathlen] = '\0';
  return path;
}

std::string Util::normalizeDir(const std::string &dirname) {
  string ret = Util::canonicalize(dirname);
  if (!ret.empty() && ret[ret.length() - 1] != '/') {
    ret += '/';
  }
  return ret;
}

std::string Util::escapeStringForCPP(const char *input, int len,
                                     bool* binary /* = NULL */) {
  if (binary) *binary = false;
  string ret;
  ret.reserve((len << 1) + 2);
  for (int i = 0; i < len; i++) {
    unsigned char ch = input[i];
    switch (ch) {
      case '\n': ret += "\\n";  break;
      case '\r': ret += "\\r";  break;
      case '\t': ret += "\\t";  break;
      case '\a': ret += "\\a";  break;
      case '\b': ret += "\\b";  break;
      case '\f': ret += "\\f";  break;
      case '\v': ret += "\\v";  break;
      case '\0': ret += "\\000"; if (binary) *binary = true; break;
      case '\"': ret += "\\\""; break;
      case '\\': ret += "\\\\"; break;
      case '?':  ret += "\\?";  break; // avoiding trigraph errors
      default:
        if (isprint(ch)) {
          ret += ch;
        } else {
          // output in octal notation
          char buf[10];
          snprintf(buf, sizeof(buf), "\\%03o", ch);
          ret += buf;
        }
        break;
    }
  }
  return ret;
}

std::string Util::escapeStringForPHP(const char *input, int len) {
  string output;
  output.reserve((len << 1) + 2);
  output = "'";
  for (int i = 0; i < len; i++) {
    unsigned char ch = input[i];
    switch (ch) {
    case '\n': output += "'.\"\\n\".'";  break;
    case '\r': output += "'.\"\\r\".'";  break;
    case '\t': output += "'.\"\\t\".'";  break;
    case '\'': output += "'.\"'\".'";    break;
    case '\\': output += "'.\"\\\\\".'"; break;
    default:
      output += ch;
      break;
    }
  }
  output += "'";
  replaceAll(output, ".''.", ".");
  replaceAll(output, "''.", "");
  replaceAll(output, ".''", "");
  replaceAll(output, "\".\"", "");
  return output;
}

const void *Util::buffer_duplicate(const void *src, int size) {
  char *s = (char *)malloc(size + 1); // '\0' in the end
  memcpy(s, src, size);
  s[size] = '\0';
  return s;
}

const void *Util::buffer_append(const void *buf1, int size1,
                                const void *buf2, int size2) {
  char *s = (char *)realloc(const_cast<void *>(buf1), size1 + size2 + 1);
  memcpy((char *)s + size1, buf2, size2);
  s[size1 + size2] = '\0';
  return s;
}

void Util::string_printf(std::string &msg, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  string_vsnprintf(msg, fmt, ap);
  va_end(ap);
}

void Util::string_vsnprintf(std::string &msg, const char *fmt, va_list ap) {
  int i = 0;
  for (int len = 1024; msg.empty(); len <<= 1) {
    va_list v;
    va_copy(v, ap);

    char *buf = (char*)malloc(len);
    if (vsnprintf(buf, len, fmt, v) < len) {
      msg = buf;
    }
    free(buf);

    va_end(v);
    if (++i > 10) break;
  }
}

void Util::find(std::vector<std::string> &out,
                const std::string &root, const char *path, bool php,
                const std::set<std::string> *excludeDirs /* = NULL */,
                const std::set<std::string> *excludeFiles /* = NULL */) {
  if (!path) path = "";
  if (*path == '/') path++;

  string spath = path;
  if (spath.length() && spath[spath.length() - 1] != '/') {
    spath += '/';
  }
  if (excludeDirs && excludeDirs->find(spath) != excludeDirs->end()) {
    return;
  }

  string fullPath = root + path;
  if (fullPath.empty()) {
    return;
  }
  DIR *dir = opendir(fullPath.c_str());
  if (dir == nullptr) {
    Logger::Error("Util::find(): unable to open directory %s",
                  fullPath.c_str());
    return;
  }
  if (fullPath[fullPath.length() - 1] != '/') {
    fullPath += '/';
  }

  dirent *e;
  while ((e = readdir(dir))) {
    char *ename = e->d_name;

    // skipping .  .. hidden files
    if (ename[0] == '.' || !*ename) {
      continue;
    }
    string fe = fullPath + ename;
    struct stat se;
    if (stat(fe.c_str(), &se)) {
      Logger::Error("Util::find(): unable to stat %s", fe.c_str());
      continue;
    }

    if ((se.st_mode & S_IFMT) == S_IFDIR) {
      string subdir = spath + ename;
      find(out, root, subdir.c_str(), php, excludeDirs, excludeFiles);
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
        string line;
        std::ifstream fin(fe.c_str());
        if (std::getline(fin, line)) {
          if (line[0] == '#' && line[1] == '!' &&
              line.find("php") != string::npos) {
            isPHP = true;
          }
        }
      } catch (...) {
        Logger::Error("Util::find(): unable to read %s", fe.c_str());
      }
    }

    if (isPHP == php &&
        (!excludeFiles ||
         excludeFiles->find(spath + ename) == excludeFiles->end())) {
      out.push_back(fe);
    }
  }

  closedir(dir);
}

std::string Util::format_pattern(const std::string &pattern,
                                 bool prefixSlash) {
  if (pattern.empty()) return pattern;

  std::string ret = "#";
  for (unsigned int i = 0; i < pattern.size(); i++) {
    char ch = pattern[i];

    // apache rewrite rules don't require initial slash
    if (prefixSlash && i == 0 && ch == '^') {
      char ch1 = pattern[1];
      if (ch1 != '/' && ch1 != '(') {
        ret += "^/";
        continue;
      }
    }

    if (ch == '#') {
      ret += "\\#";
    } else {
      ret += ch;
    }
  }
  ret += '#';
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
