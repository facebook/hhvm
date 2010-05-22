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

#ifndef __EXT_FILE_H__
#define __EXT_FILE_H__

#include <runtime/base/base_includes.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// file handle based file operations

Variant f_fopen(CStrRef filename, CStrRef mode, bool use_include_path = false,
                CObjRef context = null_object);
Variant f_popen(CStrRef command, CStrRef mode);
bool f_fclose(CObjRef handle);
Variant f_pclose(CObjRef handle);
Variant f_fseek(CObjRef handle, int64 offset, int64 whence = SEEK_SET);
bool f_rewind(CObjRef handle);
Variant f_ftell(CObjRef handle);
bool f_feof(CObjRef handle);
Variant f_fstat(CObjRef handle);
Variant f_fread(CObjRef handle, int64 length);
Variant f_fgetc(CObjRef handle);
Variant f_fgets(CObjRef handle, int64 length = 1024);
Variant f_fgetss(CObjRef handle, int64 length = 0,
                CStrRef allowable_tags = null_string);
Variant f_fscanf(int _argc, CObjRef handle, CStrRef format, CArrRef _argv = null_array);
Variant f_fpassthru(CObjRef handle);
Variant f_fwrite(CObjRef handle, CStrRef data, int64 length = 0);
Variant f_fputs(CObjRef handle, CStrRef data, int64 length = 0);
Variant f_fprintf(int _argc, CObjRef handle, CStrRef format, CArrRef _argv = null_array);
Variant f_vfprintf(CObjRef handle, CStrRef format, CArrRef args);
bool f_fflush(CObjRef handle);
bool f_ftruncate(CObjRef handle, int64 size);
bool f_flock(CObjRef handle, int operation, Variant wouldblock = null);
Variant f_fputcsv(CObjRef handle, CArrRef fields, CStrRef delimiter = ",",
                  CStrRef enclosure = "\"");
Variant f_fgetcsv(CObjRef handle, int64 length = 0, CStrRef delimiter = ",",
                  CStrRef enclosure = "\"");

///////////////////////////////////////////////////////////////////////////////
// file name based file operations

Variant f_file_get_contents(CStrRef filename, bool use_include_path = false,
                            CObjRef context = null_object, int64 offset = 0,
                            int64 maxlen = 0);
Variant f_file_put_contents(CStrRef filename, CVarRef data, int flags = 0,
                            CObjRef context = null_object);
Variant f_file(CStrRef filename, int flags = 0, CObjRef context = null_object);
Variant f_readfile(CStrRef filename, bool use_include_path = false,
                   CObjRef context = null_object);
bool f_move_uploaded_file(CStrRef filename, CStrRef destination);
Variant f_parse_ini_file(CStrRef filename, bool process_sections = false,
                         int scanner_mode = k_INI_SCANNER_NORMAL);
Variant f_parse_ini_string(CStrRef ini, bool process_sections = false,
                           int scanner_mode = k_INI_SCANNER_NORMAL);
Variant f_parse_hdf_file(CStrRef filename);
Variant f_parse_hdf_string(CStrRef input);
bool f_write_hdf_file(CArrRef data, CStrRef filename);
String f_write_hdf_string(CArrRef data);
Variant f_md5_file(CStrRef filename, bool raw_output = false);
Variant f_sha1_file(CStrRef filename, bool raw_output = false);

///////////////////////////////////////////////////////////////////////////////
// shell commands

bool f_chmod(CStrRef filename, int64 mode);
bool f_chown(CStrRef filename, CVarRef user);
bool f_lchown(CStrRef filename, CVarRef user);
bool f_chgrp(CStrRef filename, CVarRef group);
bool f_lchgrp(CStrRef filename, CVarRef group);
bool f_touch(CStrRef filename, int64 mtime = 0, int64 atime = 0);
bool f_copy(CStrRef source, CStrRef dest, CObjRef context = null_object);
bool f_rename(CStrRef oldname, CStrRef newname, CObjRef context = null_object);
int f_umask(CVarRef mask = null_variant);
bool f_unlink(CStrRef filename, CObjRef context = null_object);
bool f_link(CStrRef target, CStrRef link);
bool f_symlink(CStrRef target, CStrRef link);
String f_basename(CStrRef path, CStrRef suffix = null_string);
bool f_fnmatch(CStrRef pattern, CStrRef filename, int flags = 0);
Variant f_glob(CStrRef pattern, int flags = 0);
Variant f_tempnam(CStrRef dir, CStrRef prefix);
Variant f_tmpfile();

///////////////////////////////////////////////////////////////////////////////
// stats functions

Variant f_fileperms(CStrRef filename);
Variant f_fileinode(CStrRef filename);
Variant f_filesize(CStrRef filename);
Variant f_fileowner(CStrRef filename);
Variant f_filegroup(CStrRef filename);
Variant f_fileatime(CStrRef filename);
Variant f_filemtime(CStrRef filename);
Variant f_filectime(CStrRef filename);
Variant f_filetype(CStrRef filename);
Variant f_linkinfo(CStrRef filename);
bool f_is_writable(CStrRef filename);
bool f_is_writeable(CStrRef filename);
bool f_is_readable(CStrRef filename);
bool f_is_executable(CStrRef filename);
bool f_is_file(CStrRef filename);
bool f_is_dir(CStrRef filename);
bool f_is_link(CStrRef filename);
bool f_is_uploaded_file(CStrRef filename);
bool f_file_exists(CStrRef filename);
Variant f_stat(CStrRef filename);
Variant f_lstat(CStrRef filename);
void f_clearstatcache();
Variant f_readlink_internal(CStrRef path, bool warning_compliance);
Variant f_readlink(CStrRef path);
Variant f_realpath(CStrRef path);
Variant f_pathinfo(CStrRef path, int opt = 15);
Variant f_disk_free_space(CStrRef directory);
Variant f_diskfreespace(CStrRef directory);
Variant f_disk_total_space(CStrRef directory);

///////////////////////////////////////////////////////////////////////////////
// directory functions

bool f_mkdir(CStrRef pathname, int64 mode = 0777, bool recursive = false,
             CObjRef context = null_object);
bool f_rmdir(CStrRef dirname, CObjRef context = null_object);
String f_dirname(CStrRef path);
Variant f_getcwd();
bool f_chdir(CStrRef directory);
bool f_chroot(CStrRef directory);
Variant f_dir(CStrRef directory);
Variant f_opendir(CStrRef path, CObjRef context = null);
Variant f_readdir(CObjRef dir_handle);
void f_rewinddir(CObjRef dir_handle);
Variant f_scandir(CStrRef directory, bool descending = false,
                  CObjRef context = null);
void f_closedir(CObjRef dir_handle);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_FILE_H__
