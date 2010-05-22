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

#include <test/test_ext_splfile.h>
#include <runtime/ext/ext_splfile.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtSplfile::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_hphp_splfileinfo___construct);
  RUN_TEST(test_hphp_splfileinfo_get_atime);
  RUN_TEST(test_hphp_splfileinfo_get_basename);
  RUN_TEST(test_hphp_splfileinfo_get_ctime);
  RUN_TEST(test_hphp_splfileinfo_get_fileinfo);
  RUN_TEST(test_hphp_splfileinfo_get_filename);
  RUN_TEST(test_hphp_splfileinfo_get_group);
  RUN_TEST(test_hphp_splfileinfo_get_inode);
  RUN_TEST(test_hphp_splfileinfo_get_linktarget);
  RUN_TEST(test_hphp_splfileinfo_get_mtime);
  RUN_TEST(test_hphp_splfileinfo_get_owner);
  RUN_TEST(test_hphp_splfileinfo_get_path);
  RUN_TEST(test_hphp_splfileinfo_get_pathinfo);
  RUN_TEST(test_hphp_splfileinfo_get_pathname);
  RUN_TEST(test_hphp_splfileinfo_get_perms);
  RUN_TEST(test_hphp_splfileinfo_get_realpath);
  RUN_TEST(test_hphp_splfileinfo_get_size);
  RUN_TEST(test_hphp_splfileinfo_get_type);
  RUN_TEST(test_hphp_splfileinfo_is_dir);
  RUN_TEST(test_hphp_splfileinfo_is_executable);
  RUN_TEST(test_hphp_splfileinfo_is_file);
  RUN_TEST(test_hphp_splfileinfo_is_link);
  RUN_TEST(test_hphp_splfileinfo_is_readable);
  RUN_TEST(test_hphp_splfileinfo_is_writable);
  RUN_TEST(test_hphp_splfileinfo_open_file);
  RUN_TEST(test_hphp_splfileinfo_set_file_class);
  RUN_TEST(test_hphp_splfileinfo_set_info_class);
  RUN_TEST(test_hphp_splfileinfo___tostring);
  RUN_TEST(test_hphp_splfileobject___construct);
  RUN_TEST(test_hphp_splfileobject_current);
  RUN_TEST(test_hphp_splfileobject_eof);
  RUN_TEST(test_hphp_splfileobject_fflush);
  RUN_TEST(test_hphp_splfileobject_fgetc);
  RUN_TEST(test_hphp_splfileobject_fgetcsv);
  RUN_TEST(test_hphp_splfileobject_fgets);
  RUN_TEST(test_hphp_splfileobject_fgetss);
  RUN_TEST(test_hphp_splfileobject_flock);
  RUN_TEST(test_hphp_splfileobject_fpassthru);
  RUN_TEST(test_hphp_splfileobject_fscanf);
  RUN_TEST(test_hphp_splfileobject_fseek);
  RUN_TEST(test_hphp_splfileobject_fstat);
  RUN_TEST(test_hphp_splfileobject_ftell);
  RUN_TEST(test_hphp_splfileobject_ftruncate);
  RUN_TEST(test_hphp_splfileobject_fwrite);
  RUN_TEST(test_hphp_splfileobject_get_cvscontrol);
  RUN_TEST(test_hphp_splfileobject_get_flags);
  RUN_TEST(test_hphp_splfileobject_get_maxlinelen);
  RUN_TEST(test_hphp_splfileobject_haschildren);
  RUN_TEST(test_hphp_splfileobject_key);
  RUN_TEST(test_hphp_splfileobject_next);
  RUN_TEST(test_hphp_splfileobject_rewind);
  RUN_TEST(test_hphp_splfileobject_seek);
  RUN_TEST(test_hphp_splfileobject_setcsvcontrol);
  RUN_TEST(test_hphp_splfileobject_set_flags);
  RUN_TEST(test_hphp_splfileobject_set_maxlinelen);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtSplfile::test_hphp_splfileinfo___construct() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_atime() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_basename() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_ctime() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_fileinfo() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_filename() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_group() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_inode() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_linktarget() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_mtime() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_owner() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_path() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_pathinfo() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_pathname() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_perms() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_realpath() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_size() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_get_type() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_is_dir() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_is_executable() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_is_file() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_is_link() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_is_readable() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_is_writable() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_open_file() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_set_file_class() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo_set_info_class() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileinfo___tostring() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject___construct() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_current() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_eof() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fflush() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fgetc() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fgetcsv() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fgets() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fgetss() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_flock() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fpassthru() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fscanf() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fseek() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fstat() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_ftell() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_ftruncate() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_fwrite() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_get_cvscontrol() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_get_flags() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_get_maxlinelen() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_haschildren() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_key() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_next() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_rewind() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_seek() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_setcsvcontrol() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_set_flags() {
  return Count(true);
}

bool TestExtSplfile::test_hphp_splfileobject_set_maxlinelen() {
  return Count(true);
}
