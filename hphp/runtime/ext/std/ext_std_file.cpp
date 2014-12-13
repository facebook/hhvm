/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/std/ext_std_file.h"

#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/ext/ext_hash.h"
#include "hphp/runtime/ext/std/ext_std_options.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/zend-scanf.h"
#include "hphp/runtime/base/pipe.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/user-stream-wrapper.h"
#include "hphp/system/constants.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/runtime/base/file-util.h"
#include <folly/String.h>
#include <dirent.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/file.h>
#if defined(__FreeBSD__) || defined(__APPLE__)
# include <sys/mount.h>
#else
# include <sys/vfs.h>
#endif
#include <utime.h>
#include <grp.h>
#include <pwd.h>
#include <fnmatch.h>
#include <boost/algorithm/string/predicate.hpp>
#include <vector>


#define CHECK_HANDLE_BASE(handle, f, ret)               \
  File *f = handle.getTyped<File>(true, true);          \
  if (f == nullptr || f->isClosed()) {                  \
    raise_warning("Not a valid stream resource");       \
    return (ret);                                       \
  }                                                     \

#define CHECK_HANDLE(handle, f) \
  CHECK_HANDLE_BASE(handle, f, false)
#define CHECK_HANDLE_RET_NULL(handle, f) \
  CHECK_HANDLE_BASE(handle, f, null_variant)

#define CHECK_PATH_BASE(p, i, ret) \
  if (p.size() != strlen(p.data())) { \
    raise_warning( \
      "%s() expects parameter %d to be a valid path, string given", \
      __FUNCTION__ + 2, i \
    ); \
    return (ret); \
  }

#define CHECK_PATH(p, i) \
  CHECK_PATH_BASE(p, i, null_variant)
#define CHECK_PATH_FALSE(p, i) \
  CHECK_PATH_BASE(p, i, false)

#define CHECK_SYSTEM(exp)                                 \
  if ((exp) != 0) {                                       \
    raise_warning(                                        \
      "%s(): %s",                                         \
       __FUNCTION__ + 2,                                  \
       folly::errnoStr(errno).c_str()                     \
    );                                                    \
    return false;                                         \
  }                                                       \

#define CHECK_SYSTEM_SILENT(exp)                          \
  if ((exp) != 0) {                                       \
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,  \
                    folly::errnoStr(errno).c_str());      \
    return false;                                         \
  }                                                       \

// libxml/xpathInternals.h defines CHECK_ERROR,
// we need to undef it first
#ifdef CHECK_ERROR
#undef CHECK_ERROR
#endif
#define CHECK_ERROR(ret)                                 \
  check_error(__FUNCTION__, __LINE__, (ret))

#define PHP_FILE_USE_INCLUDE_PATH   1
#define PHP_FILE_IGNORE_NEW_LINES   2
#define PHP_FILE_SKIP_EMPTY_LINES   4
#define PHP_FILE_APPEND             8
#define PHP_FILE_NO_DEFAULT_CONTEXT 16

#ifndef GLOB_ONLYDIR
# define GLOB_ONLYDIR (1<<30)
# define GLOB_EMULATE_ONLYDIR
# define GLOB_FLAGMASK (~GLOB_ONLYDIR)
#else
# define GLOB_FLAGMASK (~0)
#endif

namespace HPHP {

static const StaticString s_STREAM_URL_STAT_LINK("STREAM_URL_STAT_LINK");
static const StaticString s_STREAM_URL_STAT_QUIET("STREAM_URL_STAT_QUIET");

///////////////////////////////////////////////////////////////////////////////
// helpers

static bool check_error(const char *function, int line, bool ret) {
  if (!ret) {
    Logger::Verbose("%s/%d: %s", function, line,
                    folly::errnoStr(errno).c_str());
  }
  return ret;
}

static int accessSyscall(
    const String& uri_or_path,
    int mode,
    bool useFileCache = false) {
  Stream::Wrapper* w = Stream::getWrapperFromURI(uri_or_path);
  if (!w) return -1;

  if (useFileCache && dynamic_cast<FileStreamWrapper*>(w)) {
    String path(uri_or_path);
    if (UNLIKELY(StringUtil::IsFileUrl(uri_or_path.data()))) {
      path = StringUtil::DecodeFileUrl(uri_or_path);
      if (path.empty()) {
        return -1;
      }
    }
    return ::access(File::TranslatePathWithFileCache(path).data(), mode);
  }
  return w->access(uri_or_path, mode);
}

static int statSyscall(
    const String& path,
    struct stat* buf,
    bool useFileCache = false) {
  bool isRelative = path.charAt(0) != '/';
  int pathIndex = 0;
  Stream::Wrapper* w = Stream::getWrapperFromURI(path, &pathIndex);
  if (!w) return -1;
  bool isFileStream = dynamic_cast<FileStreamWrapper*>(w);
  auto canUseFileCache = useFileCache && isFileStream;
  if (isRelative && !pathIndex) {
    auto fullpath = g_context->getCwd() + String::FromChar('/') + path;
    std::string realpath = StatCache::realpath(fullpath.data());
    // realpath will return an empty string for nonexistent files
    if (realpath.empty()) {
      return ENOENT;
    }
    auto translatedPath = canUseFileCache ?
      File::TranslatePathWithFileCache(realpath) :
      File::TranslatePath(realpath);
    return ::stat(translatedPath.data(), buf);
  }

  auto properPath = isFileStream ? path.substr(pathIndex) : path;
  if (canUseFileCache) {
    return ::stat(File::TranslatePathWithFileCache(properPath).data(), buf);
  }
  return w->stat(properPath, buf);
}

static int lstatSyscall(
    const String& path,
    struct stat* buf,
    bool useFileCache = false) {
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  if (!w) return -1;
  if (useFileCache && dynamic_cast<FileStreamWrapper*>(w)) {
    return ::lstat(File::TranslatePathWithFileCache(path).data(), buf);
  }
  return w->lstat(path, buf);
}

const StaticString
  s_dev("dev"),
  s_ino("ino"),
  s_mode("mode"),
  s_nlink("nlink"),
  s_uid("uid"),
  s_gid("gid"),
  s_rdev("rdev"),
  s_size("size"),
  s_atime("atime"),
  s_mtime("mtime"),
  s_ctime("ctime"),
  s_blksize("blksize"),
  s_blocks("blocks");

Array stat_impl(struct stat *stat_sb) {
  ArrayInit ret(26, ArrayInit::Mixed{});
  ret.append((int64_t)stat_sb->st_dev);
  ret.append((int64_t)stat_sb->st_ino);
  ret.append((int64_t)stat_sb->st_mode);
  ret.append((int64_t)stat_sb->st_nlink);
  ret.append((int64_t)stat_sb->st_uid);
  ret.append((int64_t)stat_sb->st_gid);
  ret.append((int64_t)stat_sb->st_rdev);
  ret.append((int64_t)stat_sb->st_size);
  ret.append((int64_t)stat_sb->st_atime);
  ret.append((int64_t)stat_sb->st_mtime);
  ret.append((int64_t)stat_sb->st_ctime);
  ret.append((int64_t)stat_sb->st_blksize);
  ret.append((int64_t)stat_sb->st_blocks);
  ret.set(s_dev,     (int64_t)stat_sb->st_dev);
  ret.set(s_ino,     (int64_t)stat_sb->st_ino);
  ret.set(s_mode,    (int64_t)stat_sb->st_mode);
  ret.set(s_nlink,   (int64_t)stat_sb->st_nlink);
  ret.set(s_uid,     (int64_t)stat_sb->st_uid);
  ret.set(s_gid,     (int64_t)stat_sb->st_gid);
  ret.set(s_rdev,    (int64_t)stat_sb->st_rdev);
  ret.set(s_size,    (int64_t)stat_sb->st_size);
  ret.set(s_atime,   (int64_t)stat_sb->st_atime);
  ret.set(s_mtime,   (int64_t)stat_sb->st_mtime);
  ret.set(s_ctime,   (int64_t)stat_sb->st_ctime);
  ret.set(s_blksize, (int64_t)stat_sb->st_blksize);
  ret.set(s_blocks,  (int64_t)stat_sb->st_blocks);
  return ret.toArray();
}

///////////////////////////////////////////////////////////////////////////////

const int64_t k_STREAM_URL_STAT_LINK = 1;
const int64_t k_STREAM_URL_STAT_QUIET = 2;

Variant HHVM_FUNCTION(fopen,
                      const String& filename,
                      const String& mode,
                      bool use_include_path /* = false */,
                      const Variant& context /* = null */) {
  CHECK_PATH_FALSE(filename, 1);
  if (!context.isNull() &&
      (!context.isResource() ||
       !context.toResource().getTyped<StreamContext>())) {
    raise_warning("$context must be a valid Stream Context or NULL");
    return false;
  }
  if (filename.empty()) {
    raise_warning("Filename cannot be empty");
    return false;
  }

  Resource resource = File::Open(filename, mode,
                                 use_include_path ? File::USE_INCLUDE_PATH : 0,
                                 context);
  if (resource.isNull()) {
    return false;
  }
  return resource;
}

Variant HHVM_FUNCTION(popen,
                      const String& command,
                      const String& mode) {
  CHECK_PATH_FALSE(command, 1);
  File *file = newres<Pipe>();
  Resource handle(file);
  bool ret = CHECK_ERROR(file->open(File::TranslateCommand(command), mode));
  if (!ret) {
    raise_warning("popen(%s,%s): Invalid argument",
                  command.data(), mode.data());
    return false;
  }
  return handle;
}

bool HHVM_FUNCTION(fclose,
                   const Resource& handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->close());
}

Variant HHVM_FUNCTION(pclose,
                      const Variant& handle) {
  CHECK_HANDLE(handle.toResource(), f);
  CHECK_ERROR(f->close());
  return s_pcloseRet;
}

Variant HHVM_FUNCTION(fseek,
                      const Resource& handle,
                      int64_t offset,
                      int64_t whence /* = k_SEEK_SET */) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->seek(offset, whence)) ? 0 : -1;
}

bool HHVM_FUNCTION(rewind,
                   const Resource& handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->rewind());
}

Variant HHVM_FUNCTION(ftell,
                      const Resource& handle) {
  CHECK_HANDLE(handle, f);
  int64_t ret = f->tell();
  if (!CHECK_ERROR(ret != -1)) {
    return false;
  }
  return ret;
}

bool HHVM_FUNCTION(feof,
                   const Resource& handle) {
  CHECK_HANDLE(handle, f);
  return f->eof();
}

Variant HHVM_FUNCTION(fstat,
                      const Resource& handle) {
  CHECK_HANDLE(handle, f);
  struct stat sb;
  if (!CHECK_ERROR(f->stat(&sb)))
    return false;
  return stat_impl(&sb);
}

Variant HHVM_FUNCTION(fread,
                      const Resource& handle,
                      int64_t length) {
  CHECK_HANDLE(handle, f);
  if (length < 1) {
    raise_warning(
      "fread(): Length parameter must be greater than 0"
    );
    return false;
  }
  return f->read(length);
}

Variant HHVM_FUNCTION(fgetc,
                      const Resource& handle) {
  CHECK_HANDLE(handle, f);
  int result = f->getc();
  if (result == EOF) {
    return false;
  }
  return String::FromChar(result);
}

Variant HHVM_FUNCTION(fgets,
                      const Resource& handle,
                      int64_t length /* = 0 */) {
  if (length < 0) {
    throw_invalid_argument("length (negative): %" PRId64, length);
    return false;
  }
  CHECK_HANDLE(handle, f);
  String line = f->readLine(length - 1);
  if (!line.isNull()) {
    return line;
  }
  return false;
}

Variant HHVM_FUNCTION(fgetss,
                      const Resource& handle,
                      int64_t length /* = 0 */,
                      const String& allowable_tags /* = null_string */) {
  Variant ret = HHVM_FN(fgets)(handle, length);
  if (!same(ret, false)) {
    return StringUtil::StripHTMLTags(ret.toString(), allowable_tags);
  }
  return ret;
}

Variant fscanfImpl(const Resource& handle,
                   const String& format,
                   const std::vector<Variant*>& args) {
  CHECK_HANDLE(handle, f);
  String line = f->readLine();
  if (line.length() == 0) {
    return false;
  }
  return sscanfImpl(line, format, args);
}

TypedValue* HHVM_FN(fscanf)(ActRec* ar) {
  Resource handle = getArg<KindOfResource>(ar, 0);
  if (ar->numArgs() < 1) {
    return arReturn(ar, init_null());
  }
  String format = getArg<KindOfString>(ar, 1);
  if (ar->numArgs() < 2) {
    return arReturn(ar, false);
  }

  std::vector<Variant*> args;
  for (int i = 2; i < ar->numArgs(); ++i) {
    args.push_back(&getArg<KindOfRef>(ar, i));
  }
  return arReturn(ar, fscanfImpl(handle, format, args));
}

Variant HHVM_FUNCTION(fpassthru,
                      const Resource& handle) {
  CHECK_HANDLE(handle, f);
  return f->print();
}

Variant HHVM_FUNCTION(fwrite,
                      const Resource& handle,
                      const String& data,
                      int64_t length /* = 0 */) {
  CHECK_HANDLE(handle, f);
  int64_t ret = f->write(data, length);
  if (ret < 0) {
    raise_notice("fwrite(): send of %d bytes failed with errno=%d %s",
                 data.size(), errno, folly::errnoStr(errno).c_str());
    ret = 0;
  }
  return ret;
}

Variant HHVM_FUNCTION(fputs,
                      const Resource& handle,
                      const String& data,
                      int64_t length /* = 0 */) {
  CHECK_HANDLE(handle, f);
  int64_t ret = f->write(data, length);
  if (ret < 0) ret = 0;
  return ret;
}

Variant HHVM_FUNCTION(fprintf,
                      const Variant& handle,
                      const String& format,
                      const Array& args /* = null_array */) {
  if (!handle.isResource()) {
    raise_param_type_warning("fprintf", 1, DataType::KindOfResource,
                             handle.getType());
    return false;
  }
  const Resource res = handle.toResource();
  CHECK_HANDLE(res, f);
  return f->printf(format, args);
}

Variant HHVM_FUNCTION(vfprintf,
                      const Variant& handle,
                      const Variant& format,
                      const Variant& args) {
  CHECK_HANDLE(handle.toResource(), f);
  return f->printf(format.toString(), args.toArray());
}

bool HHVM_FUNCTION(fflush,
                   const Resource& handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->flush());
}

bool HHVM_FUNCTION(ftruncate,
                   const Resource& handle,
                   int64_t size) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->truncate(size));
}

static int flock_values[] = { LOCK_SH, LOCK_EX, LOCK_UN };

bool HHVM_FUNCTION(flock,
                   const Resource& handle,
                   int operation,
                   VRefParam wouldblock /* = null */) {
  CHECK_HANDLE(handle, f);
  bool block = false;
  int act;

  act = operation & 3;
  if (act < 1 || act > 3) {
    throw_invalid_argument("operation: %d", operation);
    return false;
  }
  act = flock_values[act - 1] | (operation & 4 ? LOCK_NB : 0);
  bool ret = f->lock(act, block);
  wouldblock = block;
  return ret;
}

// match the behavior of PHP5
#define FCSV_CHECK_ARG(NAME)                            \
  if (NAME.size() == 0) {                               \
    throw_invalid_argument(#NAME ": %s", NAME.data());  \
    return false;                                       \
  } else if (NAME.size() > 1) {                         \
    raise_notice(#NAME " must be a single character");  \
  }                                                     \
  char NAME ## _char = NAME.charAt(0);                  \

Variant HHVM_FUNCTION(fputcsv,
                      const Resource& handle,
                      const Array& fields,
                      const String& delimiter /* = "," */,
                      const String& enclosure /* = "\"" */) {
  FCSV_CHECK_ARG(delimiter);
  FCSV_CHECK_ARG(enclosure);

  CHECK_HANDLE_RET_NULL(handle, f);
  return f->writeCSV(fields, delimiter_char, enclosure_char);
}

Variant HHVM_FUNCTION(fgetcsv,
                      const Resource& handle,
                      int64_t length /* = 0 */,
                      const String& delimiter /* = "," */,
                      const String& enclosure /* = "\"" */,
                      const String& escape /* = "\\" */) {
  if (length < 0) {
    throw_invalid_argument("Length parameter may not be negative");
    return false;
  }

  FCSV_CHECK_ARG(delimiter);
  FCSV_CHECK_ARG(enclosure);
  FCSV_CHECK_ARG(escape);

  CHECK_HANDLE_RET_NULL(handle, f);
  Array ret = f->readCSV(length, delimiter_char, enclosure_char, escape_char);
  if (!ret.isNull()) {
    return ret;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(file_get_contents,
                      const String& filename,
                      bool use_include_path /* = false */,
                      const Variant& context /* = null */,
                      int64_t offset /* = -1 */,
                      int64_t maxlen /* = -1 */) {
  CHECK_PATH(filename, 1);
  Variant stream = HHVM_FN(fopen)(filename, "rb", use_include_path, context);
  if (same(stream, false)) return false;
  return HHVM_FN(stream_get_contents)(stream.toResource(), maxlen, offset);
}

Variant HHVM_FUNCTION(file_put_contents,
                      const String& filename,
                      const Variant& data,
                      int flags /* = 0 */,
                      const Variant& context /* = null */) {
  CHECK_PATH(filename, 1);
  Resource resource = File::Open(
    filename, (flags & PHP_FILE_APPEND) ? "ab" : "wb", flags, context);
  File *f = resource.getTyped<File>(true);
  if (!f) {
    return false;
  }

  if (flags & LOCK_EX) {
    // Check to make sure we are dealing with a regular file
    if (!dynamic_cast<PlainFile*>(f)) {
      raise_warning(
        "%s(): Exclusive locks may only be set for regular files",
        __FUNCTION__ + 2);
      return false;
    }

    if (!f->lock(LOCK_EX)) {
      return false;
    }
  }

  int numbytes = 0;

  switch (data.getType()) {
    case KindOfResource: {
      File *fsrc = data.toResource().getTyped<File>(true, true);
      if (!fsrc) {
        raise_warning("Not a valid stream resource");
        return false;
      }
      while (true) {
        char buffer[1024];
        int len = fsrc->readImpl(buffer, sizeof(buffer));
        if (len == 0) break;
        numbytes += len;
        int written = f->writeImpl(buffer, len);
        if (written != len) {
          numbytes = -1;
          break;
        }
      }
      break;
    }

    case KindOfArray: {
      Array arr = data.toArray();
      for (ArrayIter iter(arr); iter; ++iter) {
        String value = iter.second();
        if (!value.empty()) {
          numbytes += value.size();
          int written = f->writeImpl(value.data(), value.size());
          if (written != value.size()) {
            numbytes = -1;
            break;
          }
        }
      }
      break;
    }

    case KindOfObject:
      if (!data.getObjectData()->hasToString()) {
        raise_warning("Not a valid stream resource");
        return false;
      }
      // fallthrough
    case KindOfUninit:
    case KindOfNull:
    case KindOfBoolean:
    case KindOfInt64:
    case KindOfDouble:
    case KindOfStaticString:
    case KindOfString:
    case KindOfRef: {
      String value = data.toString();
      if (!value.empty()) {
        numbytes += value.size();
        int written = f->writeImpl(value.data(), value.size());
        if (written != value.size()) {
          numbytes = -1;
        }
      }
      break;
    }

    case KindOfClass:
      not_reached();
  }

  // like fwrite(), fclose() can error when fflush()ing
  if (numbytes < 0 || !f->close()) {
    return false;
  }

  return numbytes;
}

Variant HHVM_FUNCTION(file,
                      const String& filename,
                      int flags /* = 0 */,
                      const Variant& context /* = null */) {
  CHECK_PATH(filename, 1);
  Variant contents = HHVM_FN(file_get_contents)(filename,
                                         flags & PHP_FILE_USE_INCLUDE_PATH,
                                         context);
  if (same(contents, false)) {
    return false;
  }
  String content = contents.toString();
  Array ret = Array::Create();
  if (content.empty()) {
    return ret;
  }

  char eol_marker = '\n';
  bool include_new_line = !(flags & PHP_FILE_IGNORE_NEW_LINES);
  bool skip_blank_lines = flags & PHP_FILE_SKIP_EMPTY_LINES;
  const char *s = content.data();
  const char *e = s + content.size();

  int i = 0;
  const char *p = (const char *)memchr(s, '\n', content.size());
  if (!p) {
    p = e;
    goto parse_eol;
  }

  if (include_new_line) {
    do {
      p++;
    parse_eol:
      ret.set(i++, String(s, p-s, CopyString));
      s = p;
    } while ((p = (const char *)memchr(p, eol_marker, (e-p))));
  } else {
    do {
      int windows_eol = 0;
      if (p != content.data() && eol_marker == '\n' && *(p - 1) == '\r') {
        windows_eol++;
      }

      if (skip_blank_lines && !(p-s-windows_eol)) {
        s = ++p;
        continue;
      }
      ret.set(i++, String(s, p-s-windows_eol, CopyString));
      s = ++p;
    } while ((p = (const char *)memchr(p, eol_marker, (e-p))));
  }

  /* handle any left overs of files without new lines */
  if (s != e) {
    p = e;
    goto parse_eol;
  }
  return ret;
}

Variant HHVM_FUNCTION(readfile,
                      const String& filename,
                      bool use_include_path /* = false */,
                      const Variant& context /* = null */) {
  CHECK_PATH_FALSE(filename, 1);
  Variant f = HHVM_FN(fopen)(filename, "rb", use_include_path, context);
  if (same(f, false)) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    folly::errnoStr(errno).c_str());
    return false;
  }
  Variant ret = HHVM_FN(fpassthru)(f.toResource());
  return ret;
}

bool HHVM_FUNCTION(move_uploaded_file,
                   const String& filename,
                   const String& destination) {
  Transport *transport = g_context->getTransport();
  if (!transport || !transport->isUploadedFile(filename)) {
    return false;
  }

  if (HHVM_FN(rename)(filename, destination)) {
    return true;
  }

  // If rename didn't work, fall back to copy followed by unlink
  if (!HHVM_FN(copy)(filename, destination)) {
    return false;
  }
  HHVM_FN(unlink)(filename);

  return true;
}

// Resolves a filename for the `parse_ini_file`, considering
// CWD, containing file name, and include paths.
static String resolve_parse_ini_filename(const String& filename) {
  String resolved = File::TranslatePath(filename);

  if (!resolved.empty() && HHVM_FN(file_exists)(resolved)) {
    return resolved;
  }

  // Don't resolve further absolute paths.
  if (filename[0] == '/') {
    return null_string;
  }

  // Try to infer from containing file name.
  auto const cfd = String::attach(g_context->getContainingFileName());
  if (!cfd.empty()) {
    int npos = cfd.rfind('/');
    if (npos >= 0) {
      resolved = cfd.substr(0, npos + 1) + filename;
      if (HHVM_FN(file_exists)(resolved)) {
        return resolved;
      }
    }
  }

  // Next, see if include path was set in the ini settings.
  auto const includePaths = ThreadInfo::s_threadInfo.getNoCheck()->
    m_reqInjectionData.getIncludePaths();

  unsigned int pathCount = includePaths.size();
  for (int i = 0; i < (int)pathCount; i++) {
    resolved = includePaths[i] + '/' + filename;
    if (HHVM_FN(file_exists)(resolved)) {
      return resolved;
    }
  }

  return null_string;
}

Variant HHVM_FUNCTION(parse_ini_file,
                      const String& filename,
                      bool process_sections /* = false */,
                      int scanner_mode /* = k_INI_SCANNER_NORMAL */) {
  CHECK_PATH_FALSE(filename, 1);
  if (filename.empty()) {
    throw_invalid_argument("Filename cannot be empty!");
    return false;
  }

  String resolved = resolve_parse_ini_filename(filename);
  if (resolved == null_string) {
    raise_warning("No such file or directory");
    return false;
  }

  Variant content = HHVM_FN(file_get_contents)(resolved);
  if (same(content, false)) return false;
  return IniSetting::FromString(content.toString(), filename, process_sections,
                                scanner_mode);
}

Variant HHVM_FUNCTION(parse_ini_string,
                      const String& ini,
                      bool process_sections /* = false */,
                      int scanner_mode /* = k_INI_SCANNER_NORMAL */) {
  return IniSetting::FromString(ini, "", process_sections, scanner_mode);
}

Variant HHVM_FUNCTION(md5_file,
                      const String& filename,
                      bool raw_output /* = false */) {
  CHECK_PATH(filename, 1);
  return HHVM_FN(hash_file)("md5", filename, raw_output);
}

Variant HHVM_FUNCTION(sha1_file,
                      const String& filename,
                      bool raw_output /* = false */) {
  CHECK_PATH(filename, 1);
  return HHVM_FN(hash_file)("sha1", filename, raw_output);
}

///////////////////////////////////////////////////////////////////////////////
// stats functions

Variant HHVM_FUNCTION(fileperms,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_mode;
}

Variant HHVM_FUNCTION(fileinode,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb));
  return (int64_t)sb.st_ino;
}

Variant HHVM_FUNCTION(filesize,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  if (StaticContentCache::TheFileCache) {
    int64_t size =
      StaticContentCache::TheFileCache->fileSize(filename.data(),
        filename.data()[0] != '/');
    if (size >= 0) return size;
  }
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_size;
}

Variant HHVM_FUNCTION(fileowner,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_uid;
}

Variant HHVM_FUNCTION(filegroup,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_gid;
}

Variant HHVM_FUNCTION(fileatime,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_atime;
}

Variant HHVM_FUNCTION(filemtime,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_mtime;
}

Variant HHVM_FUNCTION(filectime,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_ctime;
}

Variant HHVM_FUNCTION(filetype,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(lstatSyscall(filename, &sb));

  switch (sb.st_mode & S_IFMT) {
  case S_IFLNK:  return "link";
  case S_IFIFO:  return "fifo";
  case S_IFCHR:  return "char";
  case S_IFDIR:  return "dir";
  case S_IFBLK:  return "block";
  case S_IFREG:  return "file";
  case S_IFSOCK: return "socket";
  }
  return "unknown";
}

Variant HHVM_FUNCTION(linkinfo,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb));
  return (int64_t)sb.st_dev;
}

bool HHVM_FUNCTION(is_writable,
                   const String& filename) {
  CHECK_PATH_FALSE(filename, 1);
  if (filename.empty()) {
    return false;
  }
  CHECK_SYSTEM_SILENT(accessSyscall(filename, W_OK));
  return true;
  /*
  int mask = S_IWOTH;
  if (sb.st_uid == getuid()) {
    mask = S_IWUSR;
  } else if (sb.st_gid == getgid()) {
    mask = S_IWGRP;
  } else {
    int groups = getgroups(0, NULL);
    if (groups > 0) {
      gid_t *gids = (gid_t *)malloc(groups * sizeof(gid_t));
      int n = getgroups(groups, gids);
      for (int i = 0; i < n; i++) {
        if (sb.st_gid == gids[i]) {
          mask = S_IWGRP;
          break;
        }
      }
      free(gids);
    }
  }
  return sb.st_mode & mask;
  */
}

bool HHVM_FUNCTION(is_writeable,
                   const String& filename) {
  CHECK_PATH_FALSE(filename, 1);
  return HHVM_FN(is_writable)(filename);
}

bool HHVM_FUNCTION(is_readable,
                   const String& filename) {
  CHECK_PATH_FALSE(filename, 1);
  if (filename.empty()) {
    return false;
  }
  CHECK_SYSTEM_SILENT(accessSyscall(filename, R_OK, true));
  return true;
  /*
  int mask = S_IROTH;
  if (sb.st_uid == getuid()) {
    mask = S_IRUSR;
  } else if (sb.st_gid == getgid()) {
    mask = S_IRGRP;
  } else {
    int groups = getgroups(0, NULL);
    if (groups > 0) {
      gid_t *gids = (gid_t *)malloc(groups * sizeof(gid_t));
      int n = getgroups(groups, gids);
      for (int i = 0; i < n; i++) {
        if (sb.st_gid == gids[i]) {
          mask = S_IRGRP;
          break;
        }
      }
      free(gids);
    }
  }
  return sb.st_mode & mask;
  */
}

bool HHVM_FUNCTION(is_executable,
                   const String& filename) {
  CHECK_PATH_FALSE(filename, 1);
  if (filename.empty()) {
    return false;
  }
  CHECK_SYSTEM_SILENT(accessSyscall(filename, X_OK));
  return true;
  /*
  int mask = S_IXOTH;
  if (sb.st_uid == getuid()) {
    mask = S_IXUSR;
  } else if (sb.st_gid == getgid()) {
    mask = S_IXGRP;
  } else {
    int groups = getgroups(0, NULL);
    if (groups > 0) {
      gid_t *gids = (gid_t *)malloc(groups * sizeof(gid_t));
      int n = getgroups(groups, gids);
      for (int i = 0; i < n; i++) {
        if (sb.st_gid == gids[i]) {
          mask = S_IXGRP;
          break;
        }
      }
      free(gids);
    }
  }
  return (sb.st_mode & mask) && (sb.st_mode & S_IFMT) != S_IFDIR;
  */
}

bool HHVM_FUNCTION(is_file,
                   const String& filename) {
  CHECK_PATH_FALSE(filename, 1);
  struct stat sb;
  CHECK_SYSTEM_SILENT(statSyscall(filename, &sb, true));
  return (sb.st_mode & S_IFMT) == S_IFREG;
}

bool HHVM_FUNCTION(is_dir,
                   const String& filename) {
  CHECK_PATH_FALSE(filename, 1);
  String cwd;
  if (filename.empty()) {
    return false;
  }
  bool isRelative = (filename.charAt(0) != '/');
  if (isRelative) cwd = g_context->getCwd();
  if (!isRelative || cwd == String(RuntimeOption::SourceRoot)) {
    if (File::IsVirtualDirectory(filename)) {
      return true;
    }
  }

  struct stat sb;
  CHECK_SYSTEM_SILENT(statSyscall(filename, &sb));
  return (sb.st_mode & S_IFMT) == S_IFDIR;
}

bool HHVM_FUNCTION(is_link,
                   const String& filename) {
  CHECK_PATH_FALSE(filename, 1);
  struct stat sb;
  CHECK_SYSTEM_SILENT(lstatSyscall(filename, &sb));
  return (sb.st_mode & S_IFMT) == S_IFLNK;
}

bool HHVM_FUNCTION(is_uploaded_file,
                   const String& filename) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    return transport->isUploadedFile(filename);
  }
  return false;
}

bool HHVM_FUNCTION(file_exists,
                   const String& filename) {
  CHECK_PATH_FALSE(filename, 1);
  if (filename.empty() ||
      (accessSyscall(filename, F_OK, true)) < 0) {
    return false;
  }
  return true;
}

Variant HHVM_FUNCTION(stat,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  if (filename.empty()) {
    return false;
  }

  struct stat sb;
  if (statSyscall(filename, &sb, true) != 0) {
    raise_warning(
      "stat(): stat failed for %s",
       filename.c_str()
    );
    return false;
  }
  return stat_impl(&sb);
}

Variant HHVM_FUNCTION(lstat,
                      const String& filename) {
  CHECK_PATH(filename, 1);
  if (filename.empty()) {
    return false;
  }

  struct stat sb;
  CHECK_SYSTEM(lstatSyscall(filename, &sb, true));
  return stat_impl(&sb);
}

void HHVM_FUNCTION(clearstatcache,
                   bool clear_realpath_cache /* = false */,
                   const Variant& filename /* = null_variant */) {
  // we are not having a cache for file stats, so do nothing here
}

Variant HHVM_FUNCTION(readlink_internal,
                      const String& path,
                      bool warning_compliance) {
  char buff[PATH_MAX];
  int ret = readlink(File::TranslatePath(path).data(), buff, PATH_MAX-1);
  if (ret < 0) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    folly::errnoStr(errno).c_str());
    if (warning_compliance) {
      raise_warning("readlink(): No such file or directory %s",path.c_str());
    }
    return false;
  }
  buff[ret] = '\0';
  return String(buff, ret, CopyString);
}

Variant HHVM_FUNCTION(readlink,
                      const String& path) {
  return HHVM_FN(readlink_internal)(path, true);
}

Variant HHVM_FUNCTION(realpath,
                      const String& path) {
  CHECK_PATH(path, 1);
  String translated = File::TranslatePath(path);
  if (translated.empty()) {
    return false;
  }
  if (StaticContentCache::TheFileCache &&
      StaticContentCache::TheFileCache->exists(translated.data(), false)) {
    return translated;
  }
  // Zend doesn't support streams in realpath
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  if (!w || !dynamic_cast<FileStreamWrapper*>(w)) {
    return false;
  }
  char resolved_path[PATH_MAX];
  if (!realpath(translated.c_str(), resolved_path)) {
    return false;
  }
  return String(resolved_path, CopyString);
}

#define PHP_PATHINFO_DIRNAME    1
#define PHP_PATHINFO_BASENAME   2
#define PHP_PATHINFO_EXTENSION  4
#define PHP_PATHINFO_FILENAME   8

const StaticString
  s_dirname("dirname"),
  s_basename("basename"),
  s_extension("extension"),
  s_filename("filename");

Variant HHVM_FUNCTION(pathinfo,
                      const String& path,
                      int opt /* = 15 */) {
  ArrayInit ret(4, ArrayInit::Map{});

  if (opt == 0) {
    return empty_string_variant();
  }

  if ((opt & PHP_PATHINFO_DIRNAME) == PHP_PATHINFO_DIRNAME) {
    String dirname = HHVM_FN(dirname)(path);
    if (opt == PHP_PATHINFO_DIRNAME) {
      return dirname;
    }
    if (!dirname.equal(staticEmptyString())) {
      ret.set(s_dirname, dirname);
    }
  }

  String basename = HHVM_FN(basename)(path);
  if ((opt & PHP_PATHINFO_BASENAME) == PHP_PATHINFO_BASENAME) {
    if (opt == PHP_PATHINFO_BASENAME) {
      return basename;
    }
    ret.set(s_basename, basename);
  }

  if ((opt & PHP_PATHINFO_EXTENSION) == PHP_PATHINFO_EXTENSION) {
    int pos = basename.rfind('.');
    String extension(empty_string());
    if (pos >= 0) {
      extension = basename.substr(pos + 1);
      ret.set(s_extension, extension);
    }
    if (opt == PHP_PATHINFO_EXTENSION) {
      return extension;
    }
  }

  if ((opt & PHP_PATHINFO_FILENAME) == PHP_PATHINFO_FILENAME) {
    int pos = basename.rfind('.');
    String filename(empty_string());
    if (pos >= 0) {
      filename = basename.substr(0, pos);
    } else {
      filename = basename;
    }
    if (opt == PHP_PATHINFO_FILENAME) {
      return filename;
    }
    ret.set(s_filename, filename);
  }

  return ret.toVariant();
}

Variant HHVM_FUNCTION(disk_free_space,
                      const String& directory) {
  CHECK_PATH(directory, 1);
  struct statfs buf;
  String translated = File::TranslatePath(directory);
  CHECK_SYSTEM(statfs(translated.c_str(), &buf));
  return (double)buf.f_bsize * (double)buf.f_bavail;
}

Variant HHVM_FUNCTION(diskfreespace,
                      const String& directory) {
  CHECK_PATH(directory, 1);
  return HHVM_FN(disk_free_space)(directory);
}

Variant HHVM_FUNCTION(disk_total_space,
                      const String& directory) {
  CHECK_PATH(directory, 1);
  struct statfs buf;
  String translated = File::TranslatePath(directory);
  CHECK_SYSTEM(statfs(translated.c_str(), &buf));
  return (double)buf.f_bsize * (double)buf.f_blocks;
}

///////////////////////////////////////////////////////////////////////////////
// system wrappers

bool HHVM_FUNCTION(chmod,
                   const String& filename,
                   int64_t mode) {
  CHECK_PATH_FALSE(filename, 1);
  String translated = File::TranslatePath(filename);

  // If filename points to a user file, invoke UserStreamWrapper::chmod(..)
  Stream::Wrapper* w = Stream::getWrapperFromURI(filename);
  auto usw = dynamic_cast<UserStreamWrapper*>(w);
  if (usw != nullptr) {
    return usw->chmod(filename, mode);
  }

  CHECK_SYSTEM(chmod(translated.c_str(), mode));
  return true;
}

static int get_uid(const Variant& user) {
  int uid;
  if (user.isString()) {
    String suser = user.toString();
    struct passwd *pw = getpwnam(suser.data());
    if (!pw) {
      Logger::Verbose("%s/%d: Unable to find uid for %s",
                      __FUNCTION__, __LINE__, suser.data());
      return -1;
    }
    uid = pw->pw_uid;
  } else {
    uid = user.toInt32();
  }
  return uid;
}

bool HHVM_FUNCTION(chown,
                   const String& filename,
                   const Variant& user) {
  CHECK_PATH_FALSE(filename, 1);

  // If filename points to a user file, invoke UserStreamWrapper::chown(..)
  Stream::Wrapper* w = Stream::getWrapperFromURI(filename);
  auto usw = dynamic_cast<UserStreamWrapper*>(w);
  if (usw != nullptr) {
    if (user.isInteger()) {
      return usw->chown(filename, user.toInt64());
    } else if (user.isString()) {
      return usw->chown(filename, user.toString());
    }
    raise_warning("chown(): parameter 2 should be string or integer, %s given",
                  getDataTypeString(user.getType()).c_str());
    return false;
  }

  int uid = get_uid(user);
  if (uid == -1) {
    raise_warning("chown(): Unable to find uid for %s",
                  user.toString().c_str());
    return false;
  }
  CHECK_SYSTEM(chown(File::TranslatePath(filename).data(), uid, (gid_t)-1));
  return true;
}

bool HHVM_FUNCTION(lchown,
                   const String& filename,
                   const Variant& user) {
  CHECK_PATH_FALSE(filename, 1);

  // If filename points to a user file, invoke UserStreamWrapper::chown(..)
  Stream::Wrapper* w = Stream::getWrapperFromURI(filename);
  auto usw = dynamic_cast<UserStreamWrapper*>(w);
  if (usw != nullptr) {
    if (user.isInteger()) {
      return usw->chown(filename, user.toInt64());
    } else if (user.isString()) {
      return usw->chown(filename, user.toString());
    }
    raise_warning("lchown(): parameter 2 should be string or integer, %s given",
                  getDataTypeString(user.getType()).c_str());
    return false;
  }

  int uid = get_uid(user);
  if (uid == -1) {
    raise_warning("lchown(): Unable to find uid for %s",
                  user.toString().c_str());
    return false;
  }
  CHECK_SYSTEM(lchown(File::TranslatePath(filename).data(), uid, (gid_t)-1));
  return true;
}

static int get_gid(const Variant& group) {
  int gid;
  if (group.isString()) {
    String sgroup = group.toString();
    struct group *gr = getgrnam(sgroup.data());
    if (!gr) {
      Logger::Verbose("%s/%d: Unable to find gid for %s",
                      __FUNCTION__, __LINE__, sgroup.data());
      return -1;
    }
    gid = gr->gr_gid;
  } else {
    gid = group.toInt32();
  }
  return gid;
}

bool HHVM_FUNCTION(chgrp,
                   const String& filename,
                   const Variant& group) {
  CHECK_PATH_FALSE(filename, 1);

  // If filename points to a user file, invoke UserStreamWrapper::chgrp(..)
  Stream::Wrapper* w = Stream::getWrapperFromURI(filename);
  auto usw = dynamic_cast<UserStreamWrapper*>(w);
  if (usw != nullptr) {
    if (group.isInteger()) {
      return usw->chgrp(filename, group.toInt64());
    } else if (group.isString()) {
      return usw->chgrp(filename, group.toString());
    }
    raise_warning("chgrp(): parameter 2 should be string or integer, %s given",
                  getDataTypeString(group.getType()).c_str());
    return false;
  }

  int gid = get_gid(group);
  if (gid == -1) return false;
  CHECK_SYSTEM(chown(File::TranslatePath(filename).data(), (uid_t)-1, gid));
  return true;
}

bool HHVM_FUNCTION(lchgrp,
                   const String& filename,
                   const Variant& group) {
  CHECK_PATH_FALSE(filename, 1);

  // If filename points to a user file, invoke UserStreamWrapper::chgrp(..)
  Stream::Wrapper* w = Stream::getWrapperFromURI(filename);
  auto usw = dynamic_cast<UserStreamWrapper*>(w);
  if (usw != nullptr) {
    if (group.isInteger()) {
      return usw->chgrp(filename, group.toInt64());
    } else if (group.isString()) {
      return usw->chgrp(filename, group.toString());
    }
    raise_warning("lchgrp(): parameter 2 should be string or integer, %s given",
                  getDataTypeString(group.getType()).c_str());
    return false;
  }

  int gid = get_gid(group);
  if (gid == 0) return false;
  CHECK_SYSTEM(lchown(File::TranslatePath(filename).data(), (uid_t)-1, gid));
  return true;
}

bool HHVM_FUNCTION(touch,
                   const String& filename,
                   int64_t mtime /* = 0 */,
                   int64_t atime /* = 0 */) {
  CHECK_PATH_FALSE(filename, 1);

  // If filename points to a user file, invoke UserStreamWrapper::touch(..)
  Stream::Wrapper* w = Stream::getWrapperFromURI(filename);
  auto usw = dynamic_cast<UserStreamWrapper*>(w);
  if (usw != nullptr) {
    return usw->touch(filename, mtime, atime);
  }

  String translated = File::TranslatePath(filename);

  /* create the file if it doesn't exist already */
  if (accessSyscall(translated, F_OK)) {
    FILE *f = fopen(translated.data(), "w");
    if (!f) {
      raise_warning(
        "touch(): Unable to create file %s because %s",
        translated.data(), folly::errnoStr(errno).c_str()
      );
      return false;
    }
    fclose(f);
  }

  if (mtime == 0 && atime == 0) {
    // It is important to pass nullptr so that the OS sets mtime and atime
    // to the current time with maximum precision (more precise then seconds)
    CHECK_SYSTEM(utime(translated.data(), nullptr));
  } else {
    struct utimbuf newtime;
    newtime.actime = atime ? atime : mtime;
    newtime.modtime = mtime;
    CHECK_SYSTEM(utime(translated.data(), &newtime));
  }

  return true;
}

bool HHVM_FUNCTION(copy,
                   const String& source,
                   const String& dest,
                   const Variant& context /* = null */) {
  CHECK_PATH_FALSE(source, 1);
  CHECK_PATH_FALSE(dest, 2);
  if (!context.isNull() || !File::IsPlainFilePath(source) ||
      !File::IsPlainFilePath(dest)) {
    Variant sfile = HHVM_FN(fopen)(source, "r", false, context);
    if (same(sfile, false)) {
      return false;
    }
    Variant dfile = HHVM_FN(fopen)(dest, "w", false, context);
    if (same(dfile, false)) {
      return false;
    }

    if (!HHVM_FN(stream_copy_to_stream)(sfile.toResource(),
                                        dfile.toResource()).toBoolean()) {
      return false;
    }

    return HHVM_FN(fclose)(dfile.toResource());
  } else {
    int ret =
      RuntimeOption::UseDirectCopy ?
      FileUtil::directCopy(File::TranslatePath(source).data(),
          File::TranslatePath(dest).data())
      :
      FileUtil::copy(File::TranslatePath(source).data(),
          File::TranslatePath(dest).data());
    return (ret == 0);
  }
}

bool HHVM_FUNCTION(rename,
                   const String& oldname,
                   const String& newname,
                   const Variant& context /* = null */) {
  CHECK_PATH_FALSE(oldname, 1);
  CHECK_PATH_FALSE(newname, 2);
  Stream::Wrapper* w = Stream::getWrapperFromURI(oldname);
  if (!w) return false;
  if (w != Stream::getWrapperFromURI(newname)) {
    raise_warning("Can't rename a file on different streams");
    return false;
  }
  CHECK_SYSTEM(w->rename(oldname, newname));
  return true;
}

int64_t HHVM_FUNCTION(umask,
                      const Variant& mask /* = null_variant */) {
  int oldumask = umask(077);
  if (mask.isNull()) {
    umask(oldumask);
  } else {
    umask(mask.toInt32());
  }
  return oldumask;
}

bool HHVM_FUNCTION(unlink,
                   const String& filename,
                   const Variant& context /* = null */) {
  CHECK_PATH_FALSE(filename, 1);
  Stream::Wrapper* w = Stream::getWrapperFromURI(filename);
  if (!w) return false;
  CHECK_SYSTEM_SILENT(w->unlink(filename));
  return true;
}

bool HHVM_FUNCTION(link,
                   const String& target,
                   const String& link) {
  CHECK_PATH_FALSE(target, 1);
  CHECK_PATH_FALSE(link, 2);
  CHECK_SYSTEM(::link(File::TranslatePath(target).data(),
                      File::TranslatePath(link).data()));
  return true;
}

bool HHVM_FUNCTION(symlink,
                   const String& target,
                   const String& link) {
  CHECK_PATH_FALSE(target, 1);
  CHECK_PATH_FALSE(link, 2);
  CHECK_SYSTEM(symlink(File::TranslatePathKeepRelative(target).data(),
                       File::TranslatePath(link).data()));
  return true;
}

String HHVM_FUNCTION(basename,
                     const String& path,
                     const String& suffix /* = null_string */) {
  int state = 0;
  const char *c = path.data();
  const char *comp, *cend;
  comp = cend = c;
  for (int cnt = path.size(); cnt > 0; --cnt, ++c) {
    if (*c == '/') {
      if (state == 1) {
        state = 0;
        cend = c;
      }
    } else if (state == 0) {
      comp = c;
      state = 1;
    }
  }

  if (state == 1) {
    cend = c;
  }
  int sufflen = suffix.size();
  if (!suffix.empty() && sufflen < (int)(cend - comp) &&
      memcmp(cend - sufflen, suffix.data(), sufflen) == 0) {
    cend -= sufflen;
  }
  return String(comp, cend - comp, CopyString);
}

bool HHVM_FUNCTION(fnmatch,
                   const String& pattern,
                   const String& filename, int flags /* = 0 */) {
  CHECK_PATH_FALSE(pattern, 1);
  CHECK_PATH_FALSE(filename, 2);
  if (filename.size() >= PATH_MAX) {
    raise_warning(
      "Filename exceeds the maximum allowed length of %d characters",
      PATH_MAX);
    return false;
  }
  if (pattern.size() >= PATH_MAX) {
    raise_warning("Path exceeds the maximum allowed length of %d characters",
                  PATH_MAX);
    return false;
  }

  return fnmatch(pattern.data(), filename.data(), flags) == 0;
}

Variant HHVM_FUNCTION(glob,
                      const String& pattern,
                      int flags /* = 0 */) {
  glob_t globbuf;
  int cwd_skip = 0;
  memset(&globbuf, 0, sizeof(glob_t));
  globbuf.gl_offs = 0;
  String work_pattern;

  if (pattern.size() >= PATH_MAX) {
    raise_warning("Pattern exceeds the maximum allowed length of %d characters",
                  PATH_MAX);
    return false;
  }

  if (pattern.charAt(0) == '/') {
    work_pattern = pattern;
  } else {
    String cwd = g_context->getCwd();
    if (!cwd.empty() && cwd[cwd.length() - 1] == '/') {
      work_pattern = cwd + pattern;
      cwd_skip = cwd.length();
    } else {
      work_pattern = cwd + "/" + pattern;
      cwd_skip = cwd.length() + 1;
    }
  }
  int nret = glob(work_pattern.data(), flags & GLOB_FLAGMASK, NULL, &globbuf);
  if (nret == GLOB_NOMATCH) {
    return empty_array();
  }

  if (!globbuf.gl_pathc || !globbuf.gl_pathv) {
    if (ThreadInfo::s_threadInfo->m_reqInjectionData.hasSafeFileAccess()) {
      if (!HHVM_FN(is_dir)(work_pattern)) {
        return false;
      }
    }
    return empty_array();
  }

  if (nret) {
    return false;
  }

  Array ret;
  bool basedir_limit = false;
  for (int n = 0; n < (int)globbuf.gl_pathc; n++) {
    String translated = File::TranslatePath(globbuf.gl_pathv[n]);
    if (translated.empty()) {
      basedir_limit = true;
      continue;
    }
    /* we need to do this everytime since GLOB_ONLYDIR does not guarantee that
     * all directories will be filtered. GNU libc documentation states the
     * following:
     * If the information about the type of the file is easily available
     * non-directories will be rejected but no extra work will be done to
     * determine the information for each file. I.e., the caller must still be
     * able to filter directories out.
     */
    if ((flags & GLOB_ONLYDIR) && !HHVM_FN(is_dir)(globbuf.gl_pathv[n])) {
      continue;
    }
    ret.append(String(globbuf.gl_pathv[n] + cwd_skip, CopyString));
  }

  globfree(&globbuf);

  if (basedir_limit && ret.empty()) {
    return false;
  }
  // php's glob always produces an array, but Variant::Variant(CArrRef)
  // will produce KindOfNull if given a SmartPtr wrapped around null.
  if (ret.isNull()) {
    return empty_array();
  }
  return ret;
}

Variant HHVM_FUNCTION(tempnam,
                      const String& dir,
                      const String& prefix) {
  CHECK_PATH(dir, 1);
  String tmpdir = dir, trailing_slash = "/";
  if (tmpdir.empty() || !HHVM_FN(is_dir)(tmpdir) ||
      !HHVM_FN(is_writable)(tmpdir)) {
    tmpdir = HHVM_FN(sys_get_temp_dir)();
  }
  tmpdir = File::TranslatePath(tmpdir);
  String pbase = HHVM_FN(basename)(prefix);
  if (pbase.size() > 64) pbase = pbase.substr(0, 63);
  if ((tmpdir.length() > 0) && (tmpdir[tmpdir.length() - 1] == '/')) {
    trailing_slash = "";
  }
  String templ = tmpdir + trailing_slash + pbase + "XXXXXX";
  char buf[PATH_MAX + 1];
  strcpy(buf, templ.data());
  int fd = mkstemp(buf);
  if (fd < 0) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    folly::errnoStr(errno).c_str());
    return false;
  }

  close(fd);
  return String(buf, CopyString);
}

Variant HHVM_FUNCTION(tmpfile) {
  FILE *f = tmpfile();
  if (f) {
    return Resource(newres<PlainFile>(f));
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// directory functions

bool HHVM_FUNCTION(mkdir,
                   const String& pathname,
                   int64_t mode /* = 0777 */,
                   bool recursive /* = false */,
                   const Variant& context /* = null */) {
  CHECK_PATH_FALSE(pathname, 1);
  Stream::Wrapper* w = Stream::getWrapperFromURI(pathname);
  if (!w) return false;
  int options = recursive ? k_STREAM_MKDIR_RECURSIVE : 0;
  CHECK_SYSTEM_SILENT(w->mkdir(pathname, mode, options));
  return true;
}

bool HHVM_FUNCTION(rmdir,
                   const String& dirname,
                   const Variant& context /* = null */) {
  Stream::Wrapper* w = Stream::getWrapperFromURI(dirname);
  if (!w) return false;
  int options = 0;
  CHECK_SYSTEM_SILENT(w->rmdir(dirname, options));
  return true;
}

String HHVM_FUNCTION(dirname,
                     const String& path) {
  char *buf = strndup(path.data(), path.size());
  int len = FileUtil::dirname_helper(buf, path.size());
  return String(buf, len, AttachString);
}

Variant HHVM_FUNCTION(getcwd) {
  return g_context->getCwd();
}

bool HHVM_FUNCTION(chdir,
                   const String& directory) {
  CHECK_PATH_FALSE(directory, 1);
  if (HHVM_FN(is_dir)(directory)) {
    g_context->setCwd(HHVM_FN(realpath)(directory));
    return true;
  }
  raise_warning("No such file or directory (errno 2)");
  return false;
}

bool HHVM_FUNCTION(chroot,
                   const String& directory) {
  CHECK_PATH_FALSE(directory, 1);
  CHECK_SYSTEM(chroot(File::TranslatePath(directory).data()));
  CHECK_SYSTEM(chdir("/"));
  return true;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * A stack maintains the states of nested structures.
 */
struct directory_data {
  Resource defaultDirectory;
  ~directory_data() {
    defaultDirectory.detach();
  }
};
IMPLEMENT_THREAD_LOCAL(directory_data, s_directory_data);
InitFiniNode file_init([]() {
  s_directory_data->defaultDirectory.detach();
}, InitFiniNode::When::ThreadInit);

const StaticString
  s_handle("handle"),
  s_path("path");

static Directory *get_dir(const Resource& dir_handle) {
  if (dir_handle.isNull()) {
    auto defaultDir = s_directory_data->defaultDirectory;
    if (defaultDir.isNull()) {
      raise_warning("no Directory resource supplied");
      return nullptr;
    }
    return get_dir(defaultDir);
  }

  Directory *d = dir_handle.getTyped<Directory>(true, true);
  if (!d) {
    raise_warning("Not a valid directory resource");
    return nullptr;
  }
  return d;
}

Variant HHVM_FUNCTION(dir,
                      const String& directory) {
  Variant dir = HHVM_FN(opendir)(directory);
  if (same(dir, false)) {
    return false;
  }
  ObjectData* d = SystemLib::AllocDirectoryObject();
  *(d->o_realProp(s_path, 0)) = directory;
  *(d->o_realProp(s_handle, 0)) = dir;
  return d;
}

Variant HHVM_FUNCTION(opendir,
                      const String& path,
                      const Variant& context /* = null */) {
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  if (!w) return false;
  Directory *p = w->opendir(path);
  if (!p) {
    return false;
  }
  s_directory_data->defaultDirectory = p;
  return Resource(p);
}

Variant HHVM_FUNCTION(readdir,
                      const Variant& dir_handle /* = null */) {
  const Resource& res_dir_handle = dir_handle.isNull()
                                 ? null_resource
                                 : dir_handle.toResource();
  Directory *dir = get_dir(res_dir_handle);
  if (!dir) {
    return false;
  }
  return dir->read();
}

void HHVM_FUNCTION(rewinddir,
                   const Variant& dir_handle /* = null */) {
  const Resource& res_dir_handle = dir_handle.isNull()
                                 ? null_resource
                                 : dir_handle.toResource();
  Directory *dir = get_dir(res_dir_handle);
  if (!dir) {
    return;
  }
  dir->rewind();
}

static bool StringDescending(const String& s1, const String& s2) {
  return s1.more(s2);
}

static bool StringAscending(const String& s1, const String& s2) {
  return s1.less(s2);
}

Variant HHVM_FUNCTION(scandir,
                      const String& directory,
                      bool descending /* = false */,
                      const Variant& context /* = null */) {
  if (directory.empty()) {
    raise_warning("scandir(): Directory name cannot be empty");
    return false;
  }

  CHECK_PATH(directory, 1);
  Stream::Wrapper* w = Stream::getWrapperFromURI(directory);
  if (!w) return false;
  Directory *dir = w->opendir(directory);
  if (!dir) {
    return false;
  }
  Resource deleter(dir);

  std::vector<String> names;
  while (true) {
    auto name = dir->read();
    if (same(name, false)) {
      break;
    }
    names.push_back(name.toString());
  }
  dir->close();

  if (descending) {
    sort(names.begin(), names.end(), StringDescending);
  } else {
    sort(names.begin(), names.end(), StringAscending);
  }

  Array ret;
  for (unsigned int i = 0; i < names.size(); i++) {
    ret.append(names[i]);
  }
  return ret;
}

void HHVM_FUNCTION(closedir,
                   const Variant& dir_handle /* = null */) {
  const Resource& res_dir_handle = dir_handle.isNull()
                                 ? null_resource
                                 : dir_handle.toResource();
  Directory *d = get_dir(res_dir_handle);
  if (!d) {
    return;
  }
  if (same(s_directory_data->defaultDirectory, d)) {
    s_directory_data->defaultDirectory = nullptr;
  }
  d->close();
}

///////////////////////////////////////////////////////////////////////////////

void StandardExtension::initFile() {
  Native::registerConstant<KindOfInt64>(s_STREAM_URL_STAT_LINK.get(),
                                        k_STREAM_URL_STAT_LINK);
  Native::registerConstant<KindOfInt64>(s_STREAM_URL_STAT_QUIET.get(),
                                        k_STREAM_URL_STAT_QUIET);

  HHVM_FE(fopen);
  HHVM_FE(popen);
  HHVM_FE(fclose);
  HHVM_FE(pclose);
  HHVM_FE(fseek);
  HHVM_FE(rewind);
  HHVM_FE(ftell);
  HHVM_FE(feof);
  HHVM_FE(fstat);
  HHVM_FE(fread);
  HHVM_FE(fgetc);
  HHVM_FE(fgets);
  HHVM_FE(fgetss);
  HHVM_FE(fscanf);
  HHVM_FE(fpassthru);
  HHVM_FE(fwrite);
  HHVM_FE(fputs);
  HHVM_FE(fprintf);
  HHVM_FE(vfprintf);
  HHVM_FE(fflush);
  HHVM_FE(ftruncate);
  HHVM_FE(flock);
  HHVM_FE(fputcsv);
  HHVM_FE(fgetcsv);
  HHVM_FE(file_get_contents);
  HHVM_FE(file_put_contents);
  HHVM_FE(file);
  HHVM_FE(readfile);
  HHVM_FE(move_uploaded_file);
  HHVM_FE(parse_ini_file);
  HHVM_FE(parse_ini_string);
  HHVM_FE(md5_file);
  HHVM_FE(sha1_file);
  HHVM_FE(chmod);
  HHVM_FE(chown);
  HHVM_FE(lchown);
  HHVM_FE(chgrp);
  HHVM_FE(lchgrp);
  HHVM_FE(touch);
  HHVM_FE(copy);
  HHVM_FE(rename);
  HHVM_FE(umask);
  HHVM_FE(unlink);
  HHVM_FE(link);
  HHVM_FE(symlink);
  HHVM_FE(basename);
  HHVM_FE(fnmatch);
  HHVM_FE(glob);
  HHVM_FE(tempnam);
  HHVM_FE(tmpfile);
  HHVM_FE(fileperms);
  HHVM_FE(fileinode);
  HHVM_FE(filesize);
  HHVM_FE(fileowner);
  HHVM_FE(filegroup);
  HHVM_FE(fileatime);
  HHVM_FE(filemtime);
  HHVM_FE(filectime);
  HHVM_FE(filetype);
  HHVM_FE(linkinfo);
  HHVM_FE(is_writable);
  HHVM_FE(is_writeable);
  HHVM_FE(is_readable);
  HHVM_FE(is_executable);
  HHVM_FE(is_file);
  HHVM_FE(is_dir);
  HHVM_FE(is_link);
  HHVM_FE(is_uploaded_file);
  HHVM_FE(file_exists);
  HHVM_FE(stat);
  HHVM_FE(lstat);
  HHVM_FE(clearstatcache);
  HHVM_FE(readlink_internal);
  HHVM_FE(readlink);
  HHVM_FE(realpath);
  HHVM_FE(pathinfo);
  HHVM_FE(disk_free_space);
  HHVM_FE(diskfreespace);
  HHVM_FE(disk_total_space);
  HHVM_FE(mkdir);
  HHVM_FE(rmdir);
  HHVM_FE(dirname);
  HHVM_FE(getcwd);
  HHVM_FE(chdir);
  HHVM_FE(chroot);
  HHVM_FE(dir);
  HHVM_FE(opendir);
  HHVM_FE(readdir);
  HHVM_FE(rewinddir);
  HHVM_FE(scandir);
  HHVM_FE(closedir);

  loadSystemlib("std_file");
}

///////////////////////////////////////////////////////////////////////////////
}
