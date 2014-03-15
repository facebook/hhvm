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

#ifndef incl_HPHP_EXT_FILE_H_
#define incl_HPHP_EXT_FILE_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// constants

#define k_STDIN (BuiltinFiles::GetSTDIN())
#define k_STDOUT (BuiltinFiles::GetSTDOUT())
#define k_STDERR (BuiltinFiles::GetSTDERR())
extern const int64_t k_STREAM_URL_STAT_LINK;
extern const int64_t k_STREAM_URL_STAT_QUIET;

///////////////////////////////////////////////////////////////////////////////
// file handle based file operations

Variant f_fopen(
  const String& filename, const String& mode, bool use_include_path = false,
  const Variant& context = uninit_null());
Variant f_popen(const String& command, const String& mode);
bool f_fclose(const Resource& handle);
Variant f_pclose(const Resource& handle);
Variant f_fseek(const Resource& handle, int64_t offset, int64_t whence = k_SEEK_SET);
bool f_rewind(const Resource& handle);
Variant f_ftell(const Resource& handle);
bool f_feof(const Resource& handle);
Variant f_fstat(const Resource& handle);
Variant f_fread(const Resource& handle, int64_t length);
Variant f_fgetc(const Resource& handle);
Variant f_fgets(const Resource& handle, int64_t length = 0);
Variant f_fgetss(const Resource& handle, int64_t length = 0,
                const String& allowable_tags = null_string);
Variant f_fscanf(
  int _argc, const Resource& handle, const String& format, const Array& _argv = null_array);
Variant f_fpassthru(const Resource& handle);
Variant f_fwrite(const Resource& handle, const String& data, int64_t length = 0);
Variant f_fputs(const Resource& handle, const String& data, int64_t length = 0);
Variant f_fprintf(
  int _argc, const Resource& handle, const String& format, const Array& _argv = null_array);
Variant f_vfprintf(const Resource& handle, const String& format, const Array& args);
bool f_fflush(const Resource& handle);
bool f_ftruncate(const Resource& handle, int64_t size);
bool f_flock(
  const Resource& handle, int operation, VRefParam wouldblock = uninit_null());
Variant f_fputcsv(const Resource& handle, const Array& fields, const String& delimiter = ",",
                  const String& enclosure = "\"");
Variant f_fgetcsv(
  const Resource& handle, int64_t length = 0, const String& delimiter = ",",
  const String& enclosure = "\"", const String& escape = "\\");

///////////////////////////////////////////////////////////////////////////////
// file name based file operations

Variant f_file_get_contents(
  const String& filename, bool use_include_path = false,
  const Variant& context = uninit_null(),
  int64_t offset = -1, int64_t maxlen = -1);
Variant f_file_put_contents(const String& filename, const Variant& data, int flags = 0,
                            const Variant& context = uninit_null());
Variant f_file(
  const String& filename, int flags = 0, const Variant& context = uninit_null());
Variant f_readfile(const String& filename, bool use_include_path = false,
                   const Variant& context = uninit_null());
bool f_move_uploaded_file(const String& filename, const String& destination);
Variant f_parse_ini_file(const String& filename, bool process_sections = false,
                         int scanner_mode = k_INI_SCANNER_NORMAL);
Variant f_parse_ini_string(const String& ini, bool process_sections = false,
                           int scanner_mode = k_INI_SCANNER_NORMAL);
Variant f_md5_file(const String& filename, bool raw_output = false);
Variant f_sha1_file(const String& filename, bool raw_output = false);

///////////////////////////////////////////////////////////////////////////////
// shell commands

bool f_chmod(const String& filename, int64_t mode);
bool f_chown(const String& filename, const Variant& user);
bool f_lchown(const String& filename, const Variant& user);
bool f_chgrp(const String& filename, const Variant& group);
bool f_lchgrp(const String& filename, const Variant& group);
bool f_touch(const String& filename, int64_t mtime = 0, int64_t atime = 0);
bool f_copy(
  const String& source, const String& dest, const Variant& context = uninit_null());
bool f_rename(
  const String& oldname, const String& newname,
  const Variant& context = uninit_null());
int64_t f_umask(const Variant& mask = null_variant);
bool f_unlink(const String& filename, const Variant& context = uninit_null());
bool f_link(const String& target, const String& link);
bool f_symlink(const String& target, const String& link);
String f_basename(const String& path, const String& suffix = null_string);
bool f_fnmatch(const String& pattern, const String& filename, int flags = 0);
Variant f_glob(const String& pattern, int flags = 0);
Variant f_tempnam(const String& dir, const String& prefix);
Variant f_tmpfile();

///////////////////////////////////////////////////////////////////////////////
// stats functions

Variant f_fileperms(const String& filename);
Variant f_fileinode(const String& filename);
Variant f_filesize(const String& filename);
Variant f_fileowner(const String& filename);
Variant f_filegroup(const String& filename);
Variant f_fileatime(const String& filename);
Variant f_filemtime(const String& filename);
Variant f_filectime(const String& filename);
Variant f_filetype(const String& filename);
Variant f_linkinfo(const String& filename);
bool f_is_writable(const String& filename);
bool f_is_writeable(const String& filename);
bool f_is_readable(const String& filename);
bool f_is_executable(const String& filename);
bool f_is_file(const String& filename);
bool f_is_dir(const String& filename);
bool f_is_link(const String& filename);
bool f_is_uploaded_file(const String& filename);
bool f_file_exists(const String& filename);
Variant f_stat(const String& filename);
Variant f_lstat(const String& filename);
void f_clearstatcache(bool clear_realpath_cache = false,
                      const String& filename = null_string);
Variant f_readlink_internal(const String& path, bool warning_compliance);
Variant f_readlink(const String& path);
Variant f_realpath(const String& path);
Variant f_pathinfo(const String& path, int opt = 15);
Variant f_disk_free_space(const String& directory);
Variant f_diskfreespace(const String& directory);
Variant f_disk_total_space(const String& directory);

///////////////////////////////////////////////////////////////////////////////
// directory functions

bool f_mkdir(
  const String& pathname, int64_t mode = 0777, bool recursive = false,
  const Variant& context = uninit_null());
bool f_rmdir(const String& dirname, const Variant& context = uninit_null());
String f_dirname(const String& path);
Variant f_getcwd();
bool f_chdir(const String& directory);
bool f_chroot(const String& directory);
Variant f_dir(const String& directory);
Variant f_opendir(const String& path, const Variant& context = uninit_null());
Variant f_readdir(const Resource& dir_handle = null_resource);
void f_rewinddir(const Resource& dir_handle = null_resource);
Variant f_scandir(const String& directory, bool descending = false,
                  const Variant& context = uninit_null());
void f_closedir(const Resource& dir_handle = null_resource);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_FILE_H_
