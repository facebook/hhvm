/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/ext/ext_string.h"
#include "hphp/runtime/ext/ext_stream.h"
#include "hphp/runtime/ext/ext_options.h"
#include "hphp/runtime/ext/ext_hash.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/array-util.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/zend-scanf.h"
#include "hphp/runtime/base/pipe.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/logger.h"
#include "hphp/util/util.h"
#include "hphp/util/process.h"
#include "folly/String.h"
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

#define CHECK_HANDLE(handle, f)                         \
  File *f = handle.getTyped<File>(true, true);          \
  if (f == NULL || f->isClosed()) {                     \
    raise_warning("Not a valid stream resource");       \
    return false;                                       \
  }                                                     \

#define CHECK_SYSTEM(exp)                                 \
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
    CStrRef path,
    int mode,
    bool useFileCache = false) {
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  if (useFileCache && dynamic_cast<FileStreamWrapper*>(w)) {
    return ::access(File::TranslatePathWithFileCache(path).data(), mode);
  }
  return w->access(path, mode);
}

static int statSyscall(
    CStrRef path,
    struct stat* buf,
    bool useFileCache = false) {
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
  if (useFileCache && dynamic_cast<FileStreamWrapper*>(w)) {
    return ::stat(File::TranslatePathWithFileCache(path).data(), buf);
  }
  return w->stat(path, buf);
}

static int lstatSyscall(
    CStrRef path,
    struct stat* buf,
    bool useFileCache = false) {
  Stream::Wrapper* w = Stream::getWrapperFromURI(path);
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
  ArrayInit ret(26);
  ret.set((int64_t)stat_sb->st_dev);
  ret.set((int64_t)stat_sb->st_ino);
  ret.set((int64_t)stat_sb->st_mode);
  ret.set((int64_t)stat_sb->st_nlink);
  ret.set((int64_t)stat_sb->st_uid);
  ret.set((int64_t)stat_sb->st_gid);
  ret.set((int64_t)stat_sb->st_rdev);
  ret.set((int64_t)stat_sb->st_size);
  ret.set((int64_t)stat_sb->st_atime);
  ret.set((int64_t)stat_sb->st_mtime);
  ret.set((int64_t)stat_sb->st_ctime);
  ret.set((int64_t)stat_sb->st_blksize);
  ret.set((int64_t)stat_sb->st_blocks);
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
  return ret.create();
}

///////////////////////////////////////////////////////////////////////////////

Variant f_fopen(CStrRef filename, CStrRef mode,
                bool use_include_path /* = false */,
                CVarRef context /* = null */) {
  if (!context.isNull() &&
      (!context.isResource() ||
       !context.toResource().getTyped<StreamContext>())) {
    raise_warning("$context must be a valid Stream Context or NULL");
    return false;
  }

  return File::Open(filename, mode,
                    use_include_path ? File::USE_INCLUDE_PATH : 0,
                    context);
}

Variant f_popen(CStrRef command, CStrRef mode) {
  File *file = NEWOBJ(Pipe)();
  Resource handle(file);
  bool ret = CHECK_ERROR(file->open(File::TranslateCommand(command), mode));
  if (!ret) {
    return false;
  }
  return handle;
}

bool f_fclose(CResRef handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->close());
}

Variant f_pclose(CResRef handle) {
  CHECK_HANDLE(handle, f);
  CHECK_ERROR(f->close());
  return s_file_data->m_pcloseRet;
}

Variant f_fseek(CResRef handle, int64_t offset,
                int64_t whence /* = k_SEEK_SET */) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->seek(offset, whence)) ? 0 : -1;
}

bool f_rewind(CResRef handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->rewind());
}

Variant f_ftell(CResRef handle) {
  CHECK_HANDLE(handle, f);
  int64_t ret = f->tell();
  if (!CHECK_ERROR(ret != -1)) {
    return false;
  }
  return ret;
}

bool f_feof(CResRef handle) {
  CHECK_HANDLE(handle, f);
  return f->eof();
}

Variant f_fstat(CResRef handle) {
  PlainFile *file = handle.getTyped<PlainFile>(true, true);
  if (file == NULL) {
    raise_warning("Not a valid stream resource");
    return false;
  }
  struct stat sb;
  CHECK_SYSTEM(fstat(file->fd(), &sb));
  return stat_impl(&sb);
}

Variant f_fread(CResRef handle, int64_t length) {
  CHECK_HANDLE(handle, f);
  return f->read(length);
}

Variant f_fgetc(CResRef handle) {
  CHECK_HANDLE(handle, f);
  int result = f->getc();
  if (result == EOF) {
    return false;
  }
  return String::FromChar(result);
}

Variant f_fgets(CResRef handle, int64_t length /* = 0 */) {
  if (length < 0) {
    throw_invalid_argument("length (negative): %" PRId64, length);
    return false;
  }
  CHECK_HANDLE(handle, f);
  String line = f->readLine(length);
  if (!line.isNull()) {
    return line;
  }
  return false;
}

Variant f_fgetss(CResRef handle, int64_t length /* = 0 */,
                 CStrRef allowable_tags /* = null_string */) {
  Variant ret = f_fgets(handle, length);
  if (!same(ret, false)) {
    return StringUtil::StripHTMLTags(ret.toString(), allowable_tags);
  }
  return ret;
}

Variant f_fscanf(int _argc, CResRef handle, CStrRef format,
                 CArrRef _argv /* = null_array */) {
  CHECK_HANDLE(handle, f);
  return f_sscanf(_argc, f->readLine(), format, _argv);
}

Variant f_fpassthru(CResRef handle) {
  CHECK_HANDLE(handle, f);
  return f->print();
}

Variant f_fwrite(CResRef handle, CStrRef data, int64_t length /* = 0 */) {
  CHECK_HANDLE(handle, f);
  int64_t ret = f->write(data, length);
  if (ret < 0) ret = 0;
  return ret;
}

Variant f_fputs(CResRef handle, CStrRef data, int64_t length /* = 0 */) {
  CHECK_HANDLE(handle, f);
  int64_t ret = f->write(data, length);
  if (ret < 0) ret = 0;
  return ret;
}

Variant f_fprintf(int _argc, CResRef handle, CStrRef format,
                  CArrRef _argv /* = null_array */) {
  CHECK_HANDLE(handle, f);
  return f->printf(format, _argv);
}

Variant f_vfprintf(CResRef handle, CStrRef format, CArrRef args) {
  CHECK_HANDLE(handle, f);
  return f->printf(format, args);
}

bool f_fflush(CResRef handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->flush());
}

bool f_ftruncate(CResRef handle, int64_t size) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->truncate(size));
}

static int flock_values[] = { LOCK_SH, LOCK_EX, LOCK_UN };

bool f_flock(CResRef handle, int operation, VRefParam wouldblock /* = null */) {
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

Variant f_fputcsv(CResRef handle, CArrRef fields, CStrRef delimiter /* = "," */,
                  CStrRef enclosure /* = "\"" */) {
  if (delimiter.size() != 1) {
    throw_invalid_argument("delimiter: %s", delimiter.data());
    return false;
  }
  if (enclosure.size() != 1) {
    throw_invalid_argument("enclosure: %s", enclosure.data());
    return false;
  }
  CHECK_HANDLE(handle, f);
  return f->writeCSV(fields, delimiter.charAt(0), enclosure.charAt(0));
}

Variant f_fgetcsv(CResRef handle, int64_t length /* = 0 */,
                  CStrRef delimiter /* = "," */,
                  CStrRef enclosure /* = "\"" */,
                  CStrRef escape /* = "\\" */) {
  if (delimiter.size() != 1) {
    throw_invalid_argument("delimiter: %s", delimiter.data());
    return false;
  }
  if (enclosure.size() != 1) {
    throw_invalid_argument("enclosure: %s", enclosure.data());
    return false;
  }
  if (escape.size() != 1) {
    throw_invalid_argument("escape: %s", enclosure.data());
    return false;
  }
  CHECK_HANDLE(handle, f);
  Array ret = f->readCSV(length, delimiter.charAt(0), enclosure.charAt(0),
                         escape.charAt(0));
  if (!ret.isNull()) {
    return ret;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Variant f_file_get_contents(CStrRef filename,
                            bool use_include_path /* = false */,
                            CVarRef context /* = null */,
                            int64_t offset /* = 0 */,
                            int64_t maxlen /* = 0 */) {
  Variant stream = f_fopen(filename, "rb", use_include_path, context);
  if (same(stream, false)) return false;
  return f_stream_get_contents(stream.toResource(), maxlen, offset);
}

Variant f_file_put_contents(CStrRef filename, CVarRef data,
                            int flags /* = 0 */,
                            CVarRef context /* = null */) {
  Variant fvar = File::Open(filename, (flags & PHP_FILE_APPEND) ? "ab" : "wb");
  if (!fvar.toBoolean()) {
    return false;
  }
  File *f = fvar.asResRef().getTyped<File>();

  if ((flags & LOCK_EX) && flock(f->fd(), LOCK_EX)) {
    return false;
  }

  int numbytes = 0;
  switch (data.getType()) {
  case KindOfObject:
    {
      raise_warning("Not a valid stream resource");
      return false;
    }
    break;
  case KindOfResource:
    {
      File *fsrc = data.toResource().getTyped<File>(true, true);
      if (fsrc == NULL) {
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
    }
    break;
  case KindOfArray:
    {
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
    }
    break;
  default:
    {
      String value = data.toString();
      if (!value.empty()) {
        numbytes += value.size();
        int written = f->writeImpl(value.data(), value.size());
        if (written != value.size()) {
          numbytes = -1;
        }
      }
    }
    break;
  }

  if (numbytes < 0) {
    return false;
  }
  return numbytes;
}

Variant f_file(CStrRef filename, int flags /* = 0 */,
               CVarRef context /* = null */) {
  Variant contents = f_file_get_contents(filename,
                                         flags & PHP_FILE_USE_INCLUDE_PATH,
                                         context);
  if (same(contents, false)) {
    return false;
  }
  String content = contents.toString();
  Array ret;
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

Variant f_readfile(CStrRef filename, bool use_include_path /* = false */,
                   CVarRef context /* = null */) {
  Variant f = f_fopen(filename, "rb", use_include_path, context);
  if (same(f, false)) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    folly::errnoStr(errno).c_str());
    return false;
  }
  Variant ret = f_fpassthru(f.toResource());
  return ret;
}

bool f_move_uploaded_file(CStrRef filename, CStrRef destination) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    return transport->moveUploadedFile(filename, destination);
  }
  return false;
}

Variant f_parse_ini_file(CStrRef filename, bool process_sections /* = false */,
                         int scanner_mode /* = k_INI_SCANNER_NORMAL */) {
  String translated = File::TranslatePath(filename);
  if (translated.empty() || !f_file_exists(translated)) {
    if (filename[0] != '/') {
      String cfd = g_vmContext->getContainingFileName();
      if (!cfd.empty()) {
        int npos = cfd.rfind('/');
        if (npos >= 0) {
          translated = cfd.substr(0, npos + 1) + filename;
        }
      }
    }
  }
  Variant content = f_file_get_contents(translated);
  if (same(content, false)) return false;
  return IniSetting::FromString(content.toString(), filename, process_sections,
                                scanner_mode);
}

Variant f_parse_ini_string(CStrRef ini, bool process_sections /* = false */,
                           int scanner_mode /* = k_INI_SCANNER_NORMAL */) {
  return IniSetting::FromString(ini, "", process_sections, scanner_mode);
}

Variant f_parse_hdf_file(CStrRef filename) {
  Variant content = f_file_get_contents(filename);
  if (same(content, false)) return false;
  return f_parse_hdf_string(content.toString());
}

Variant f_parse_hdf_string(CStrRef input) {
  Hdf hdf;
  hdf.fromString(input.data());
  return ArrayUtil::FromHdf(hdf);
}

bool f_write_hdf_file(CArrRef data, CStrRef filename) {
  Hdf hdf;
  ArrayUtil::ToHdf(data, hdf);
  const char *str = hdf.toString();
  Variant ret = f_file_put_contents(filename, str);
  return !same(ret, false);
}

String f_write_hdf_string(CArrRef data) {
  Hdf hdf;
  ArrayUtil::ToHdf(data, hdf);
  const char *str = hdf.toString();
  return String(str, CopyString);
}

Variant f_md5_file(CStrRef filename, bool raw_output /* = false */) {
  return f_hash_file("md5", filename, raw_output);
}

Variant f_sha1_file(CStrRef filename, bool raw_output /* = false */) {
  return f_hash_file("sha1", filename, raw_output);
}

///////////////////////////////////////////////////////////////////////////////
// stats functions

Variant f_fileperms(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_mode;
}

Variant f_fileinode(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb));
  return (int64_t)sb.st_ino;
}

Variant f_filesize(CStrRef filename) {
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

Variant f_fileowner(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_uid;
}

Variant f_filegroup(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_gid;
}

Variant f_fileatime(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_atime;
}

Variant f_filemtime(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_mtime;
}

Variant f_filectime(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (int64_t)sb.st_ctime;
}

Variant f_filetype(CStrRef filename) {
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

Variant f_linkinfo(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb));
  return (int64_t)sb.st_dev;
}

bool f_is_writable(CStrRef filename) {
  struct stat sb;
  if (statSyscall(filename, &sb)) {
    return false;
  }
  CHECK_SYSTEM(accessSyscall(filename, W_OK));
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

bool f_is_writeable(CStrRef filename) {
  return f_is_writable(filename);
}

bool f_is_readable(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  CHECK_SYSTEM(accessSyscall(filename, R_OK, true));
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

bool f_is_executable(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb));
  CHECK_SYSTEM(accessSyscall(filename, X_OK));
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

bool f_is_file(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return (sb.st_mode & S_IFMT) == S_IFREG;
}

bool f_is_dir(CStrRef filename) {
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
  CHECK_SYSTEM(statSyscall(filename, &sb));
  return (sb.st_mode & S_IFMT) == S_IFDIR;
}

bool f_is_link(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(lstatSyscall(filename, &sb));
  return (sb.st_mode & S_IFMT) == S_IFLNK;
}

bool f_is_uploaded_file(CStrRef filename) {
  Transport *transport = g_context->getTransport();
  if (transport) {
    return transport->isUploadedFile(filename);
  }
  return false;
}

bool f_file_exists(CStrRef filename) {
  if (filename.empty() ||
      (accessSyscall(filename, F_OK, true)) < 0) {
    return false;
  }
  return true;
}

Variant f_stat(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(statSyscall(filename, &sb, true));
  return stat_impl(&sb);
}

Variant f_lstat(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(lstatSyscall(filename, &sb, true));
  return stat_impl(&sb);
}

void f_clearstatcache() {
  // we are not having a cache for file stats, so do nothing here
}

Variant f_readlink_internal(CStrRef path, bool warning_compliance) {
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

Variant f_readlink(CStrRef path) {
  return f_readlink_internal(path, true);
}

Variant f_realpath(CStrRef path) {
  String translated = File::TranslatePath(path);
  if (translated.empty()) {
    return false;
  }
  if (StaticContentCache::TheFileCache &&
      StaticContentCache::TheFileCache->exists(translated.data(), false)) {
    return translated;
  }
  if (accessSyscall(path, F_OK) == 0) {
    char resolved_path[PATH_MAX];
    if (!realpath(translated.c_str(), resolved_path)) {
      return false;
    }
    return String(resolved_path, CopyString);
  }
  return false;
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

Variant f_pathinfo(CStrRef path, int opt /* = 15 */) {
  ArrayInit ret(4);

  if (opt == 0) {
    return empty_string;
  }

  if ((opt & PHP_PATHINFO_DIRNAME) == PHP_PATHINFO_DIRNAME) {
    String dirname = f_dirname(path);
    if (opt == PHP_PATHINFO_DIRNAME) {
      return dirname;
    }
    if (!dirname.equal(empty_string)) {
      ret.set(s_dirname, dirname);
    }
  }

  String basename = f_basename(path);
  if ((opt & PHP_PATHINFO_BASENAME) == PHP_PATHINFO_BASENAME) {
    if (opt == PHP_PATHINFO_BASENAME) {
      return basename;
    }
    ret.set(s_basename, basename);
  }

  if ((opt & PHP_PATHINFO_EXTENSION) == PHP_PATHINFO_EXTENSION) {
    int pos = basename.rfind('.');
    String extension(empty_string);
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
    String filename(empty_string);
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

  return ret.create();
}

Variant f_disk_free_space(CStrRef directory) {
  struct statfs buf;
  String translated = File::TranslatePath(directory);
  CHECK_SYSTEM(statfs(translated.c_str(), &buf));
  return (double)buf.f_bsize * (double)buf.f_bavail;
}

Variant f_diskfreespace(CStrRef directory) {
  return f_disk_free_space(directory);
}

Variant f_disk_total_space(CStrRef directory) {
  struct statfs buf;
  String translated = File::TranslatePath(directory);
  CHECK_SYSTEM(statfs(translated.c_str(), &buf));
  return (double)buf.f_bsize * (double)buf.f_blocks;
}

///////////////////////////////////////////////////////////////////////////////
// system wrappers

bool f_chmod(CStrRef filename, int64_t mode) {
  String translated = File::TranslatePath(filename);
  CHECK_SYSTEM(chmod(translated.c_str(), mode));
  return true;
}

static int get_uid(CVarRef user) {
  int uid;
  if (user.isString()) {
    String suser = user.toString();
    struct passwd *pw = getpwnam(suser.data());
    if (!pw) {
      Logger::Verbose("%s/%d: Unable to find uid for %s",
                      __FUNCTION__, __LINE__, suser.data());
      return 0;
    }
    uid = pw->pw_uid;
  } else {
    uid = user.toInt32();
  }
  return uid;
}

bool f_chown(CStrRef filename, CVarRef user) {
  int uid = get_uid(user);
  if (uid == 0) return false;
  CHECK_SYSTEM(chown(File::TranslatePath(filename).data(), uid, (gid_t)-1));
  return true;
}

bool f_lchown(CStrRef filename, CVarRef user) {
  int uid = get_uid(user);
  if (uid == 0) return false;
  CHECK_SYSTEM(lchown(File::TranslatePath(filename).data(), uid, (gid_t)-1));
  return true;
}

static int get_gid(CVarRef group) {
  int gid;
  if (group.isString()) {
    String sgroup = group.toString();
    struct group *gr = getgrnam(sgroup.data());
    if (!gr) {
      Logger::Verbose("%s/%d: Unable to find gid for %s",
                      __FUNCTION__, __LINE__, sgroup.data());
      return 0;
    }
    gid = gr->gr_gid;
  } else {
    gid = group.toInt32();
  }
  return gid;
}

bool f_chgrp(CStrRef filename, CVarRef group) {
  int gid = get_gid(group);
  if (gid == 0) return false;
  CHECK_SYSTEM(chown(File::TranslatePath(filename).data(), (uid_t)-1, gid));
  return true;
}

bool f_lchgrp(CStrRef filename, CVarRef group) {
  int gid = get_gid(group);
  if (gid == 0) return false;
  CHECK_SYSTEM(lchown(File::TranslatePath(filename).data(), (uid_t)-1, gid));
  return true;
}

bool f_touch(CStrRef filename, int64_t mtime /* = 0 */, int64_t atime /* = 0 */) {
  String translated = File::TranslatePath(filename);

  /* create the file if it doesn't exist already */
  if (accessSyscall(translated, F_OK)) {
    FILE *f = fopen(translated.data(), "w");
    if (f == NULL) {
      Logger::Verbose("%s/%d: Unable to create file %s because %s",
                      __FUNCTION__, __LINE__, translated.data(),
                      folly::errnoStr(errno).c_str());
      return false;
    }
    fclose(f);
  }

  if (mtime == 0 || atime == 0) {
    int now = time(0);
    if (mtime == 0) mtime = now;
    if (atime == 0) atime = now;
  }
  struct utimbuf newtime;
  newtime.actime = atime;
  newtime.modtime = mtime;
  CHECK_SYSTEM(utime(translated.data(), &newtime));
  return true;
}

bool f_copy(CStrRef source, CStrRef dest,
            CVarRef context /* = null */) {
  if (!context.isNull() || !File::IsPlainFilePath(source) ||
      !File::IsPlainFilePath(dest)) {
    Variant sfile = f_fopen(source, "r", false, context);
    if (same(sfile, false)) {
      return false;
    }
    Variant dfile = f_fopen(dest, "w", false, context);
    if (same(dfile, false)) {
      return false;
    }

    return f_stream_copy_to_stream(sfile.toResource(),
      dfile.toResource()).toBoolean();
  } else {
    int ret =
      RuntimeOption::UseDirectCopy ?
      Util::directCopy(File::TranslatePath(source).data(),
          File::TranslatePath(dest).data())
      :
      Util::copy(File::TranslatePath(source).data(),
          File::TranslatePath(dest).data());
    return (ret == 0);
  }
}

bool f_rename(CStrRef oldname, CStrRef newname,
              CVarRef context /* = null */) {
  int ret =
    RuntimeOption::UseDirectCopy ?
      Util::directRename(File::TranslatePath(oldname).data(),
                         File::TranslatePath(newname).data())
                                 :
      Util::rename(File::TranslatePath(oldname).data(),
                   File::TranslatePath(newname).data());
  return (ret == 0);
}

int64_t f_umask(CVarRef mask /* = null_variant */) {
  int oldumask = umask(077);
  if (mask.isNull()) {
    umask(oldumask);
  } else {
    umask(mask.toInt32());
  }
  return oldumask;
}

bool f_unlink(CStrRef filename, CVarRef context /* = null */) {
  CHECK_SYSTEM(unlink(File::TranslatePath(filename).data()));
  return true;
}

bool f_link(CStrRef target, CStrRef link) {
  CHECK_SYSTEM(::link(File::TranslatePath(target).data(),
                      File::TranslatePath(link).data()));
  return true;
}

bool f_symlink(CStrRef target, CStrRef link) {
  CHECK_SYSTEM(symlink(File::TranslatePath(target).data(),
                       File::TranslatePath(link).data()));
  return true;
}

String f_basename(CStrRef path, CStrRef suffix /* = null_string */) {
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

bool f_fnmatch(CStrRef pattern, CStrRef filename, int flags /* = 0 */) {
  if (filename.size() >= PATH_MAX) {
    raise_warning("Filename exceeds the maximum allowed length of %d characters",
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

Variant f_glob(CStrRef pattern, int flags /* = 0 */) {
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
  if (nret == GLOB_NOMATCH || !globbuf.gl_pathc || !globbuf.gl_pathv) {
    if (RuntimeOption::SafeFileAccess) {
      if (!f_is_dir(work_pattern)) {
        return false;
      }
    }
    return Array::Create();
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
    if ((flags & GLOB_ONLYDIR) && !f_is_dir(globbuf.gl_pathv[n])) {
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
    return Array::Create();
  }
  return ret;
}

Variant f_tempnam(CStrRef dir, CStrRef prefix) {
  String tmpdir = dir;
  if (tmpdir.empty() || !f_is_dir(tmpdir) || !f_is_writable(tmpdir)) {
    tmpdir = f_sys_get_temp_dir();
  }
  tmpdir = File::TranslatePath(tmpdir);
  String pbase = f_basename(prefix);
  if (pbase.size() > 64) pbase = pbase.substr(0, 63);
  String templ = tmpdir + "/" + pbase + "XXXXXX";
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

Variant f_tmpfile() {
  FILE *f = tmpfile();
  if (f) {
    return Resource(NEWOBJ(PlainFile)(f));
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// directory functions

bool f_mkdir(CStrRef pathname, int64_t mode /* = 0777 */,
             bool recursive /* = false */, CVarRef context /* = null */) {
  if (recursive) {
    String path = File::TranslatePath(pathname);
    if (path.empty()) return false;
    if (path.charAt(path.size() - 1) != '/') {
      path += "/";
    }
    return Util::mkdir(path.data(), mode);
  }
  CHECK_SYSTEM(mkdir(File::TranslatePath(pathname).data(), mode));
  return true;
}

bool f_rmdir(CStrRef dirname, CVarRef context /* = null */) {
  CHECK_SYSTEM(rmdir(File::TranslatePath(dirname).data()));
  return true;
}

String f_dirname(CStrRef path) {
  char *buf = strndup(path.data(), path.size());
  int len = Util::dirname_helper(buf, path.size());
  return String(buf, len, AttachString);
}

Variant f_getcwd() {
  return g_context->getCwd();
}

bool f_chdir(CStrRef directory) {
  if (f_is_dir(directory)) {
    g_context->setCwd(File::TranslatePath(directory));
    return true;
  }
  raise_warning("No such file or directory (errno 2)");
  return false;
}

bool f_chroot(CStrRef directory) {
  CHECK_SYSTEM(chroot(File::TranslatePath(directory).data()));
  CHECK_SYSTEM(chdir("/"));
  return true;
}

///////////////////////////////////////////////////////////////////////////////

class Directory : public SweepableResourceData {
public:
  explicit Directory(DIR *handle) : dir(handle) {
    assert(handle);
  }

  ~Directory() {
    close();
  }

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassNameHook() const { return s_class_name; }

  void close() {
    if (dir) {
      closedir(dir);
      dir = NULL;
    }
  }

  DIR *dir;
};

StaticString Directory::s_class_name("Directory");

class DirectoryRequestData : public RequestEventHandler {
public:
  virtual void requestInit() {
    defaultDirectory.reset();
  }

  virtual void requestShutdown() {
    defaultDirectory.reset();
  }

  Resource defaultDirectory;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(DirectoryRequestData, s_directory_data);

const StaticString
  s_handle("handle"),
  s_path("path");

static DIR *get_dir(CResRef dir_handle) {
  Resource obj;
  if (dir_handle.isNull()) {
    obj = s_directory_data->defaultDirectory;
  } else {
    Array arr = dir_handle.toArray();
    if (arr.exists(s_handle)) {
      obj = arr[s_handle].toResource();
    } else {
      obj = dir_handle;
    }
  }
  if (obj.get()) {
    Directory *d = obj.getTyped<Directory>(true, true);
    if (d == NULL) {
      raise_warning("Not a valid directory");
      return NULL;
    }
    return d->dir;
  }
  return NULL;
}

Variant f_dir(CStrRef directory) {
  Variant dir = f_opendir(directory);
  if (same(dir, false)) {
    return false;
  }
  ObjectData* d = SystemLib::AllocDirectoryObject();
  *(d->o_realProp(s_path, 0)) = directory;
  *(d->o_realProp(s_handle, 0)) = dir;
  return d;
}

Variant f_opendir(CStrRef path, CVarRef context /* = null */) {
  DIR *dir = opendir(File::TranslatePath(path).data());
  if (dir == NULL) {
    return false;
  }

  Directory *p = new Directory(dir);
  s_directory_data->defaultDirectory = p;
  return Resource(p);
}

Variant f_readdir(CResRef dir_handle) {
  DIR *dir = get_dir(dir_handle);
  if (dir) {
    struct dirent entry;
    struct dirent *result;
    CHECK_SYSTEM(readdir_r(dir, &entry, &result));
    if (result) {
      return String(entry.d_name, CopyString);
    }
  }
  return false;
}

void f_rewinddir(CResRef dir_handle) {
  DIR *dir = get_dir(dir_handle);
  if (dir) {
    rewinddir(dir);
  }
}

static bool StringDescending(CStrRef s1, CStrRef s2) {
  return s1.more(s2);
}

static bool StringAscending(CStrRef s1, CStrRef s2) {
  return s1.less(s2);
}

Variant f_scandir(CStrRef directory, bool descending /* = false */,
                  CVarRef context /* = null */) {
  DIR *dir = opendir(File::TranslatePath(directory).data());
  if (dir == NULL) {
    return false;
  }
  Resource deleter(new Directory(dir));

  std::vector<String> names;
  while (true) {
    struct dirent entry;
    struct dirent *result;
    CHECK_SYSTEM(readdir_r(dir, &entry, &result));
    if (result == NULL) {
      break;
    }
    names.push_back(String(entry.d_name, CopyString));
  }

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

void f_closedir(CResRef dir_handle) {
  if (!dir_handle.isNull()) {
    Resource obj;
    Array arr = dir_handle.toArray();
    if (arr.exists(s_handle)) {
      obj = arr[s_handle].toResource();
    } else {
      obj = dir_handle;
    }
    if (same(s_directory_data->defaultDirectory, obj)) {
      s_directory_data->defaultDirectory = NULL;
    }
    Directory *d = obj.getTyped<Directory>(true, true);
    if (d == NULL) {
      raise_warning("Not a valid directory");
    } else {
      d->close();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}
