/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_string.h>
#include <runtime/ext/ext_stream.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/ini_setting.h>
#include <runtime/base/array/array_util.h>
#include <runtime/base/util/http_client.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/server/static_content_cache.h>
#include <runtime/base/zend/zend_scanf.h>
#include <runtime/base/file/pipe.h>
#include <util/logger.h>
#include <util/util.h>
#include <util/process.h>
#include <dirent.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/vfs.h>
#include <utime.h>
#include <grp.h>
#include <pwd.h>
#include <fnmatch.h>

#define CHECK_HANDLE(handle, f)                         \
  File *f = handle.getTyped<File>(true, true);          \
  if (f == NULL) {                                      \
    raise_warning("Not a valid stream resource");       \
    return false;                                       \
  }                                                     \

#define CHECK_SYSTEM(exp)                                 \
  if ((exp) != 0) {                                       \
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,  \
                    Util::safe_strerror(errno).c_str());  \
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

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// helpers

static bool check_error(const char *function, int line, bool ret) {
  if (!ret) {
    Logger::Verbose("%s/%d: %s", function, line,
                    Util::safe_strerror(errno).c_str());
  }
  return ret;
}

static Array stat_impl(struct stat *stat_sb) {
  Array ret;

  ret.append((int64)stat_sb->st_dev);
  ret.append((int64)stat_sb->st_ino);
  ret.append((int64)stat_sb->st_mode);
  ret.append((int64)stat_sb->st_nlink);
  ret.append((int64)stat_sb->st_uid);
  ret.append((int64)stat_sb->st_gid);
  ret.append((int64)stat_sb->st_rdev);
  ret.append((int64)stat_sb->st_size);
  ret.append((int64)stat_sb->st_atime);
  ret.append((int64)stat_sb->st_mtime);
  ret.append((int64)stat_sb->st_ctime);
  ret.append((int64)stat_sb->st_blksize);
  ret.append((int64)stat_sb->st_blocks);

  ret.set("dev",     (int64)stat_sb->st_dev);
  ret.set("ino",     (int64)stat_sb->st_ino);
  ret.set("mode",    (int64)stat_sb->st_mode);
  ret.set("nlink",   (int64)stat_sb->st_nlink);
  ret.set("uid",     (int64)stat_sb->st_uid);
  ret.set("gid",     (int64)stat_sb->st_gid);
  ret.set("rdev",    (int64)stat_sb->st_rdev);
  ret.set("size",    (int64)stat_sb->st_size);
  ret.set("atime",   (int64)stat_sb->st_atime);
  ret.set("mtime",   (int64)stat_sb->st_mtime);
  ret.set("ctime",   (int64)stat_sb->st_ctime);
  ret.set("blksize", (int64)stat_sb->st_blksize);
  ret.set("blocks",  (int64)stat_sb->st_blocks);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

Variant f_fopen(CStrRef filename, CStrRef mode,
                bool use_include_path /* = false */,
                CObjRef context /* = null_object */) {
  Array options;
  if (!context.isNull()) {
    StreamContext *streamContext = context.getTyped<StreamContext>();
    options = streamContext->m_options;
  }
  return File::Open(filename, mode, options);
}

Variant f_popen(CStrRef command, CStrRef mode) {
  File *file = NEW(Pipe)();
  Object handle(file);
  bool ret = CHECK_ERROR(file->open(File::TranslateCommand(command), mode));
  if (!ret) {
    return false;
  }
  return handle;
}

bool f_fclose(CObjRef handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->close());
}

Variant f_pclose(CObjRef handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->close());
}

Variant f_fseek(CObjRef handle, int64 offset, int64 whence /* = SEEK_SET */) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->seek(offset, whence)) ? 0 : -1;
}

bool f_rewind(CObjRef handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->rewind());
}

Variant f_ftell(CObjRef handle) {
  CHECK_HANDLE(handle, f);
  int64 ret = f->tell();
  if (!CHECK_ERROR(ret != -1)) {
    return false;
  }
  return ret;
}

bool f_feof(CObjRef handle) {
  CHECK_HANDLE(handle, f);
  return f->eof();
}

Variant f_fstat(CObjRef handle) {
  PlainFile *file = handle.getTyped<PlainFile>(true, true);
  if (file == NULL) {
    raise_warning("Not a valid stream resource");
    return false;
  }
  struct stat sb;
  CHECK_SYSTEM(fstat(file->fd(), &sb));
  return stat_impl(&sb);
}

Variant f_fread(CObjRef handle, int64 length) {
  CHECK_HANDLE(handle, f);
  return f->read(length);
}

Variant f_fgetc(CObjRef handle) {
  CHECK_HANDLE(handle, f);
  int result = f->getc();
  if (result == EOF) {
    return false;
  }
  return String::FromChar(result);
}

Variant f_fgets(CObjRef handle, int64 length /* = 1024 */) {
  if (length < 0) {
    throw_invalid_argument("length (negative): %d", length);
    return false;
  }
  CHECK_HANDLE(handle, f);
  return f->readLine(length);
}

Variant f_fgetss(CObjRef handle, int64 length /* = 0 */,
                CStrRef allowable_tags /* = null_string */) {
  String ret = f_fgets(handle, length);
  if (!ret.empty()) {
    return StringUtil::StripHTMLTags(ret, allowable_tags);
  }
  return ret;
}

Variant f_fscanf(int _argc, CObjRef handle, CStrRef format, CArrRef _argv /* = null_array */) {
  CHECK_HANDLE(handle, f);
  StringBuffer str;
  str.read(f);
  return f_sscanf(_argc, str.detach(), format, _argv);
}

Variant f_fpassthru(CObjRef handle) {
  CHECK_HANDLE(handle, f);
  return f->print();
}

Variant f_fwrite(CObjRef handle, CStrRef data, int64 length /* = 0 */) {
  CHECK_HANDLE(handle, f);
  int64 ret = f->write(data, length);
  if (ret < 0) ret = 0;
  return ret;
}

Variant f_fputs(CObjRef handle, CStrRef data, int64 length /* = 0 */) {
  CHECK_HANDLE(handle, f);
  int64 ret = f->write(data, length);
  if (ret < 0) ret = 0;
  return ret;
}

Variant f_fprintf(int _argc, CObjRef handle, CStrRef format, CArrRef _argv /* = null_array */) {
  CHECK_HANDLE(handle, f);
  return f->printf(format, _argv);
}

Variant f_vfprintf(CObjRef handle, CStrRef format, CArrRef args) {
  CHECK_HANDLE(handle, f);
  return f->printf(format, args);
}

bool f_fflush(CObjRef handle) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->flush());
}

bool f_ftruncate(CObjRef handle, int64 size) {
  CHECK_HANDLE(handle, f);
  return CHECK_ERROR(f->truncate(size));
}

bool f_flock(CObjRef handle, int operation, Variant wouldblock /* = null */) {
  CHECK_HANDLE(handle, f);
  bool block = false;
  bool ret = f->lock(operation, block);
  wouldblock = block;
  return ret;
}

Variant f_fputcsv(CObjRef handle, CArrRef fields, CStrRef delimiter /* = "," */,
                  CStrRef enclosure /* = "\"" */) {
  if (delimiter.size() != 1) {
    throw_invalid_argument("delimiter: %s", delimiter.data());
  }
  if (enclosure.size() != 1) {
    throw_invalid_argument("enclosure: %s", enclosure.data());
  }
  CHECK_HANDLE(handle, f);
  return f->writeCSV(fields, delimiter.charAt(0), enclosure.charAt(0));
}

Variant f_fgetcsv(CObjRef handle, int64 length /* = 0 */,
                  CStrRef delimiter /* = "," */,
                  CStrRef enclosure /* = "\"" */) {
  if (delimiter.size() != 1) {
    throw_invalid_argument("delimiter: %s", delimiter.data());
  }
  if (enclosure.size() != 1) {
    throw_invalid_argument("enclosure: %s", enclosure.data());
  }
  CHECK_HANDLE(handle, f);
  Array ret = f->readCSV(length, delimiter.charAt(0), enclosure.charAt(0));
  if (!ret.isNull()) {
    return ret;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Variant f_file_get_contents(CStrRef filename,
                            bool use_include_path /* = false */,
                            CObjRef context /* = null_object */,
                            int64 offset /* = 0 */,
                            int64 maxlen /* = 0 */) {
  Variant stream = f_fopen(filename, "rb");
  if (same(stream, false)) return false;
  return f_stream_get_contents(stream, maxlen, offset);
}

Variant f_file_put_contents(CStrRef filename, CVarRef data,
                            int flags /* = 0 */,
                            CObjRef context /* = null_object */) {
  FILE *f = fopen(File::TranslatePath(filename).data(),
                  (flags & PHP_FILE_APPEND) ? "ab" : "wb");
  Object closer(NEW(PlainFile)(f));
  if (!f) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    Util::safe_strerror(errno).c_str());
    return false;
  }

  if ((flags & LOCK_EX) && flock(fileno(f), LOCK_EX)) {
    return false;
  }

  int numbytes = 0;
  switch (data.getType()) {
  case KindOfObject:
    {
      File *fsrc = data.toObject().getTyped<File>(true, true);
      if (fsrc == NULL) {
        raise_warning("Not a valid stream resource");
        return false;
      }
      while (true) {
        char buffer[1024];
        int len = fsrc->readImpl(buffer, sizeof(buffer));
        if (len == 0) break;
        numbytes += len;
        int written = fwrite(buffer, 1, len, f);
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
          int written = fwrite(value.data(), 1, value.size(), f);
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
      numbytes += value.size();
      int written = fwrite(value.data(), 1, value.size(), f);
      if (written != value.size()) {
        numbytes = -1;
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
               CObjRef context /* = null_object */) {
  Variant contents = f_file_get_contents(filename,
                                         flags & PHP_FILE_USE_INCLUDE_PATH);
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
      if (skip_blank_lines && !(p-s)) {
        s = ++p;
        continue;
      }
      ret.set(i++, String(s, p-s, CopyString));
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
                   CObjRef context /* = null_object */) {
  Variant f = f_fopen(filename, "rb");
  if (same(f, false)) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    Util::safe_strerror(errno).c_str());
    return false;
  }
  Variant ret = f_fpassthru(f.toObject());
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
      String cfd = FrameInjection::GetContainingFileName(true);
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
  return IniSetting::FromString(content, filename, process_sections,
                                scanner_mode);
}

Variant f_parse_ini_string(CStrRef ini, bool process_sections /* = false */,
                           int scanner_mode /* = k_INI_SCANNER_NORMAL */) {
  return IniSetting::FromString(ini, "", process_sections, scanner_mode);
}

Variant f_parse_hdf_file(CStrRef filename) {
  Variant content = f_file_get_contents(filename);
  if (same(content, false)) return false;
  return f_parse_hdf_string(content);
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
  String str = f_file_get_contents(filename);
  return StringUtil::MD5(str, raw_output);
}

Variant f_sha1_file(CStrRef filename, bool raw_output /* = false */) {
  String str = f_file_get_contents(filename);
  return StringUtil::SHA1(str, raw_output);
}

///////////////////////////////////////////////////////////////////////////////
// stats functions

Variant f_fileperms(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return (int64)sb.st_mode;
}

Variant f_fileinode(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename).data(), &sb));
  return (int64)sb.st_ino;
}

Variant f_filesize(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return (int64)sb.st_size;
}

Variant f_fileowner(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return (int64)sb.st_uid;
}

Variant f_filegroup(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return (int64)sb.st_gid;
}

Variant f_fileatime(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return (int64)sb.st_atime;
}

Variant f_filemtime(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return (int64)sb.st_mtime;
}

Variant f_filectime(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return (int64)sb.st_ctime;
}

Variant f_filetype(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(lstat(File::TranslatePath(filename).data(), &sb));

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
  CHECK_SYSTEM(stat(File::TranslatePath(filename).data(), &sb));
  return (int64)sb.st_dev;
}

bool f_is_writable(CStrRef filename) {
  struct stat sb;
  if (stat(File::TranslatePath(filename).data(), &sb)) {
    return false;
  }
  CHECK_SYSTEM(access(File::TranslatePath(filename).data(), W_OK));
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
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  CHECK_SYSTEM(access(File::TranslatePath(filename, true).data(), R_OK));
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
  CHECK_SYSTEM(stat(File::TranslatePath(filename).data(), &sb));
  CHECK_SYSTEM(access(File::TranslatePath(filename).data(), X_OK));
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
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return (sb.st_mode & S_IFMT) == S_IFREG;
}

bool f_is_dir(CStrRef filename) {
  String cwd;
  bool isRelative = (filename.charAt(0) != '/');
  if (isRelative) cwd = g_context->getCwd();
  if (!isRelative || cwd == RuntimeOption::SourceRoot.c_str()) {
    if (File::IsVirtualDirectory(filename)) {
      return true;
    }
  }

  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename).data(), &sb));
  return (sb.st_mode & S_IFMT) == S_IFDIR;
}

bool f_is_link(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(lstat(File::TranslatePath(filename).data(), &sb));
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
      (access(File::TranslatePath(filename, true).data(), F_OK)) < 0) {
    return false;
  }
  return true;
}

Variant f_stat(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(stat(File::TranslatePath(filename, true).data(), &sb));
  return stat_impl(&sb);
}

Variant f_lstat(CStrRef filename) {
  struct stat sb;
  CHECK_SYSTEM(lstat(File::TranslatePath(filename, true).data(), &sb));
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
                    Util::safe_strerror(errno).c_str());
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
  if (access(translated.data(), F_OK) == 0) {
    char resolved_path[PATH_MAX];
    if (!realpath(translated, resolved_path)) {
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

Variant f_pathinfo(CStrRef path, int opt /* = 15 */) {
  Array ret;

  if ((opt & PHP_PATHINFO_DIRNAME) == PHP_PATHINFO_DIRNAME) {
    String dirname = f_dirname(path);
    if (opt == PHP_PATHINFO_DIRNAME) {
      return dirname;
    }
    ret.set("dirname", dirname);
  }

  String basename = f_basename(path);
  if ((opt & PHP_PATHINFO_BASENAME) == PHP_PATHINFO_BASENAME) {
    if (opt == PHP_PATHINFO_BASENAME) {
      return basename;
    }
    ret.set("basename", basename);
  }

  if ((opt & PHP_PATHINFO_EXTENSION) == PHP_PATHINFO_EXTENSION) {
    int pos = basename.rfind('.');
    String extension;
    if (pos >= 0) {
      extension = basename.substr(pos + 1);
    }
    if (opt == PHP_PATHINFO_EXTENSION) {
      return extension;
    }
    ret.set("extension", extension);
  }

  if ((opt & PHP_PATHINFO_FILENAME) == PHP_PATHINFO_FILENAME) {
    int pos = basename.rfind('.');
    String filename;
    if (pos >= 0) {
      filename = basename.substr(0, pos);
    } else {
      filename = basename;
    }
    if (opt == PHP_PATHINFO_FILENAME) {
      return filename;
    }
    ret.set("filename", filename);
  }

  return ret;
}

Variant f_disk_free_space(CStrRef directory) {
  struct statfs buf;
  CHECK_SYSTEM(statfs(File::TranslatePath(directory), &buf));
  return (double)buf.f_bsize * (double)buf.f_bavail;
}

Variant f_diskfreespace(CStrRef directory) {
  return f_disk_free_space(directory);
}

Variant f_disk_total_space(CStrRef directory) {
  struct statfs buf;
  CHECK_SYSTEM(statfs(File::TranslatePath(directory), &buf));
  return (double)buf.f_bsize * (double)buf.f_blocks;
}

///////////////////////////////////////////////////////////////////////////////
// system wrappers

bool f_chmod(CStrRef filename, int64 mode) {
  CHECK_SYSTEM(chmod(File::TranslatePath(filename).data(), mode));
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

bool f_touch(CStrRef filename, int64 mtime /* = 0 */, int64 atime /* = 0 */) {
  String translated = File::TranslatePath(filename);

  /* create the file if it doesn't exist already */
  if (access(translated.data(), F_OK)) {
    FILE *f = fopen(translated.data(), "w");
    if (f == NULL) {
      Logger::Verbose("%s/%d: Unable to create file %s because %s",
                      __FUNCTION__, __LINE__, translated.data(),
                      Util::safe_strerror(errno).c_str());
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
            CObjRef context /* = null_object */) {
  Variant content = f_file_get_contents(source);
  if (same(content, false)) {
    return false;
  }
  Variant ret = f_file_put_contents(File::TranslatePath(dest), content);
  return !same(ret, false);
}

bool f_rename(CStrRef oldname, CStrRef newname,
              CObjRef context /* = null_object */) {
  int ret = Util::rename(File::TranslatePath(oldname).data(),
                         File::TranslatePath(newname).data());
  return (ret == 0);
}

int f_umask(CVarRef mask /* = null_variant */) {
  int oldumask = umask(077);
  if (mask.isNull()) {
    umask(oldumask);
  } else {
    umask(mask.toInt32());
  }
  return oldumask;
}

bool f_unlink(CStrRef filename, CObjRef context /* = null_object */) {
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
  int len = cend - comp;
  char *ret = (char *)malloc(len + 1);
  memcpy(ret, comp, len);
  ret[len] = '\0';
  return String(ret, len, AttachString);
}

bool f_fnmatch(CStrRef pattern, CStrRef filename, int flags /* = 0 */) {
  return fnmatch(pattern.data(), filename.data(), flags) == 0;
}

Variant f_glob(CStrRef pattern, int flags /* = 0 */) {
  glob_t globbuf;
  int cwd_skip = 0;
  memset(&globbuf, 0, sizeof(glob_t));
  globbuf.gl_offs = 0;
  String work_pattern;

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
  int nret = glob(work_pattern.data(), flags, NULL, &globbuf);
  if (nret == GLOB_NOMATCH) {
    if (!f_is_dir(work_pattern)) {
      return false;
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
  return ret;
}

Variant f_tempnam(CStrRef dir, CStrRef prefix) {
  String rootDir = File::TranslatePath(dir);
  String pbase = f_basename(prefix);
  if (pbase.size() > 64) pbase = pbase.substr(0, 63);
  String templ = rootDir + "/" + pbase + "XXXXXX";
  char buf[PATH_MAX + 1];
  strcpy(buf, templ.data());
  int fd = mkstemp(buf);
  if (fd < 0) {
    Logger::Verbose("%s/%d: %s", __FUNCTION__, __LINE__,
                    Util::safe_strerror(errno).c_str());
    return false;
  }

  close(fd);
  return String(buf, CopyString);
}

Variant f_tmpfile() {
  FILE *f = tmpfile();
  if (f) {
    return Object(NEW(PlainFile)(f));
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// directory functions

bool f_mkdir(CStrRef pathname, int64 mode /* = 0777 */,
             bool recursive /* = false */, CObjRef context /* = null_object */) {
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

bool f_rmdir(CStrRef dirname, CObjRef context /* = null_object */) {
  CHECK_SYSTEM(rmdir(File::TranslatePath(dirname).data()));
  return true;
}

static size_t php_dirname(char *path, int len) {
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

String f_dirname(CStrRef path) {
  char *buf = strndup(path.data(), path.size());
  int len = php_dirname(buf, path.size());
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
  Directory(DIR *handle) : dir(handle) {
    ASSERT(handle);
  }

  ~Directory() {
    close();
  }

  // overriding ResourceData
  const char *o_getClassName() const { return "Directory";}

  void close() {
    if (dir) {
      closedir(dir);
      dir = NULL;
    }
  }

  DIR *dir;
};

class DirectoryRequestData : public RequestEventHandler {
public:
  virtual void requestInit() {
    defaultDirectory.reset();
  }

  virtual void requestShutdown() {
    defaultDirectory.reset();
  }

  Object defaultDirectory;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(DirectoryRequestData, s_directory_data);

static DIR *get_dir(CObjRef dir_handle) {
  Object obj;
  if (dir_handle.isNull()) {
    obj = s_directory_data->defaultDirectory;
  } else {
    Array arr = dir_handle.toArray();
    if (arr.exists("handle")) {
      obj = arr["handle"].toObject();
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
  c_directory *c_d = NEW(c_directory)();
  c_d->m_path = directory;
  c_d->m_handle = dir;
  return c_d;
}

Variant f_opendir(CStrRef path, CObjRef context /* = null */) {
  DIR *dir = opendir(File::TranslatePath(path).data());
  if (dir == NULL) {
    return false;
  }

  Directory *p = new Directory(dir);
  s_directory_data->defaultDirectory = p;
  return Object(p);
}

Variant f_readdir(CObjRef dir_handle) {
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

void f_rewinddir(CObjRef dir_handle) {
  DIR *dir = get_dir(dir_handle);
  if (dir) {
    rewinddir(dir);
  }
}

static bool StringDescending(CStrRef s1, CStrRef s2) {
  return s1.more(s2);
}

Variant f_scandir(CStrRef directory, bool descending /* = false */,
                  CObjRef context /* = null */) {
  DIR *dir = opendir(File::TranslatePath(directory).data());
  if (dir == NULL) {
    return false;
  }
  Object deleter(new Directory(dir));

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
  }

  Array ret;
  for (unsigned int i = 0; i < names.size(); i++) {
    ret.append(names[i]);
  }
  return ret;
}

void f_closedir(CObjRef dir_handle) {
  if (!dir_handle.isNull()) {
    Object obj;
    Array arr = dir_handle.toArray();
    if (arr.exists("handle")) {
      obj = arr["handle"].toObject();
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
