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

#include <test/test_ext_file.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_output.h>
#include <runtime/ext/ext_string.h>
#include <runtime/base/runtime_option.h>
#include <util/light_process.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtFile::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_fopen);
  RUN_TEST(test_fpassthru);
  RUN_TEST(test_fputcsv);
  RUN_TEST(test_fputs);
  RUN_TEST(test_fread);
  RUN_TEST(test_fscanf);
  RUN_TEST(test_fseek);
  RUN_TEST(test_fstat);
  RUN_TEST(test_ftell);
  RUN_TEST(test_ftruncate);
  RUN_TEST(test_fwrite);
  RUN_TEST(test_read_write);
  RUN_TEST(test_fprintf);
  RUN_TEST(test_vfprintf);
  RUN_TEST(test_fclose);
  RUN_TEST(test_feof);
  RUN_TEST(test_fflush);
  RUN_TEST(test_fgetc);
  RUN_TEST(test_fgetcsv);
  RUN_TEST(test_fgets);
  RUN_TEST(test_fgetss);
  RUN_TEST(test_flock);
  RUN_TEST(test_rewind);
  RUN_TEST(test_popen);
  RUN_TEST(test_pclose);
  RUN_TEST(test_file_exists);
  RUN_TEST(test_file_get_contents);
  RUN_TEST(test_file_put_contents);
  RUN_TEST(test_file);
  RUN_TEST(test_readfile);
  RUN_TEST(test_move_uploaded_file);
  RUN_TEST(test_parse_ini_file);
  RUN_TEST(test_parse_ini_string);
  RUN_TEST(test_parse_hdf_file);
  RUN_TEST(test_parse_hdf_string);
  RUN_TEST(test_write_hdf_file);
  RUN_TEST(test_write_hdf_string);
  RUN_TEST(test_md5_file);
  RUN_TEST(test_sha1_file);
  RUN_TEST(test_chmod);
  RUN_TEST(test_chown);
  RUN_TEST(test_touch);
  RUN_TEST(test_copy);
  RUN_TEST(test_rename);
  RUN_TEST(test_umask);
  RUN_TEST(test_unlink);
  RUN_TEST(test_chgrp);
  RUN_TEST(test_link);
  RUN_TEST(test_symlink);
  RUN_TEST(test_lchgrp);
  RUN_TEST(test_lchown);
  RUN_TEST(test_basename);
  RUN_TEST(test_fnmatch);
  RUN_TEST(test_glob);
  RUN_TEST(test_tempnam);
  RUN_TEST(test_tmpfile);
  RUN_TEST(test_clearstatcache);
  RUN_TEST(test_stat);
  RUN_TEST(test_lstat);
  RUN_TEST(test_is_dir);
  RUN_TEST(test_is_executable);
  RUN_TEST(test_is_file);
  RUN_TEST(test_is_link);
  RUN_TEST(test_is_readable);
  RUN_TEST(test_is_uploaded_file);
  RUN_TEST(test_is_writable);
  RUN_TEST(test_is_writeable);
  RUN_TEST(test_fileatime);
  RUN_TEST(test_filectime);
  RUN_TEST(test_filegroup);
  RUN_TEST(test_fileinode);
  RUN_TEST(test_filemtime);
  RUN_TEST(test_fileowner);
  RUN_TEST(test_fileperms);
  RUN_TEST(test_filesize);
  RUN_TEST(test_filetype);
  RUN_TEST(test_readlink);
  RUN_TEST(test_realpath);
  RUN_TEST(test_linkinfo);
  RUN_TEST(test_disk_free_space);
  RUN_TEST(test_diskfreespace);
  RUN_TEST(test_disk_total_space);
  RUN_TEST(test_pathinfo);
  RUN_TEST(test_mkdir);
  RUN_TEST(test_rmdir);
  RUN_TEST(test_dirname);
  RUN_TEST(test_chdir);
  RUN_TEST(test_chroot);
  RUN_TEST(test_dir);
  RUN_TEST(test_getcwd);
  RUN_TEST(test_opendir);
  RUN_TEST(test_readdir);
  RUN_TEST(test_rewinddir);
  RUN_TEST(test_scandir);
  RUN_TEST(test_closedir);

  LightProcess::Initialize(RuntimeOption::LightProcessFilePrefix,
                           RuntimeOption::LightProcessCount);
  RUN_TEST(test_popen);
  RUN_TEST(test_pclose);
  LightProcess::Close();

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

#define VF(f, s)                                                        \
  if (!VerifyFile(f, s)) {                                              \
    printf("%s:%d: VerifyFile failed.\n", __FILE__, __LINE__);          \
    return Count(false);                                                \
  }                                                                     \

bool TestExtFile::VerifyFile(CVarRef f, CStrRef contents) {
  f_ob_start();
  f_fpassthru(f);
  VS(f_ob_get_clean(), contents);
  f_ob_end_clean();
  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtFile::test_fopen() {
  Variant f = f_fopen("test/test_ext_file.txt", "r");
  VERIFY(!same(f, false));
  return Count(true);
}

bool TestExtFile::test_fpassthru() {
  Variant f = f_fopen("test/test_ext_file.txt", "r");
  f_ob_start();
  VS(f_fpassthru(f), 17);
  VS(f_ob_get_clean(), "Testing Ext File\n");
  f_ob_end_clean();
  return Count(true);
}

bool TestExtFile::test_fputcsv() {
  Array fields = CREATE_VECTOR2("apple", "\"banana\"");
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputcsv(f, fields);
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VF(f, "apple,\"\"\"banana\"\"\"\n");
  return Count(true);
}

bool TestExtFile::test_fputs() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing fputs");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VF(f, "testing fputs");
  return Count(true);
}

bool TestExtFile::test_fread() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing fread");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fread(f, 7), "testing");
  VS(f_fread(f, 100), " fread");
  return Count(true);
}

bool TestExtFile::test_fscanf() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing fscanf");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fscanf(2, f, "%s %s"), CREATE_VECTOR2("testing", "fscanf"));
  return Count(true);
}

bool TestExtFile::test_fseek() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing fseek");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  f_fseek(f, -5, k_SEEK_END);
  VS(f_fread(f, 7), "fseek");

  f_fseek(f, 7);
  VS(f_fread(f, 7), " fseek");
  return Count(true);
}

bool TestExtFile::test_fstat() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing fstat");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fstat(f)["size"], 13);
  return Count(true);
}

bool TestExtFile::test_ftell() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing ftell");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  f_fseek(f, -5, k_SEEK_END);
  VS(f_ftell(f), 8);
  return Count(true);
}

bool TestExtFile::test_ftruncate() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing ftruncate");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r+");
  f_ftruncate(f, 7);
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fread(f, 20), "testing");
  return Count(true);
}

bool TestExtFile::test_fwrite() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fwrite(f, "testing fwrite", 7);
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VF(f, "testing");
  return Count(true);
}

bool TestExtFile::test_read_write() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fwrite(f, "testing read/write");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r+");
  f_fseek(f, 8);
  f_fwrite(f, "succeeds");
  f_fseek(f, 8);
  VS(f_fread(f, 8), "succeeds");
  return Count(true);
}

bool TestExtFile::test_fprintf() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fprintf(4, f, "%s %s", CREATE_VECTOR2("testing", "fprintf"));
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VF(f, "testing fprintf");
  return Count(true);
}

bool TestExtFile::test_vfprintf() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_vfprintf(f, "%s %s", CREATE_VECTOR2("testing", "vfprintf"));
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VF(f, "testing vfprintf");
  return Count(true);
}

bool TestExtFile::test_fclose() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fclose(f);
  return Count(true);
}

bool TestExtFile::test_feof() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing feof");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VERIFY(!f_feof(f));
  VS(f_fread(f, 20), "testing feof");
  VERIFY(f_feof(f));
  return Count(true);
}

bool TestExtFile::test_fflush() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing fflush");
  f_fflush(f);
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fread(f, 20), "testing fflush");
  return Count(true);
}

bool TestExtFile::test_fgetc() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing fgetc");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fgetc(f), "t");
  VS(f_fgetc(f), "e");
  VS(f_fgetc(f), "s");
  VS(f_fgetc(f), "t");
  return Count(true);
}

bool TestExtFile::test_fgetcsv() {
  Array fields = CREATE_VECTOR2("a", "b");
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputcsv(f, fields);
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  Array read = f_fgetcsv(f);
  VS(read, fields);
  return Count(true);
}

bool TestExtFile::test_fgets() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing\nfgets\n\n");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fgets(f), "testing\n");
  VS(f_fgets(f), "fgets\n");
  VS(f_fgets(f), "\n");
  return Count(true);
}

bool TestExtFile::test_fgetss() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "<html><head>testing</head><body> fgetss</body></html>\n");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fgetss(f), "testing fgetss\n");
  return Count(true);
}

bool TestExtFile::test_flock() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w+");
  VERIFY(f_flock(f, k_LOCK_EX));
  f_flock(f, k_LOCK_UN);
  return Count(true);
}

bool TestExtFile::test_rewind() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing rewind");
  f_fclose(f);

  f = f_fopen("test/test_ext_file.tmp", "r");
  VS(f_fread(f, 7), "testing");
  VS(f_fread(f, 100), " rewind");
  VS(f_fread(f, 7), "");
  VERIFY(f_rewind(f));
  VS(f_fread(f, 7), "testing");
  VS(f_fread(f, 100), " rewind");
  return Count(true);
}

bool TestExtFile::test_popen() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing popen");
  f_fclose(f);

  f = f_popen("cat test/test_ext_file.tmp", "r");
  VS(f_fread(f, 20), "testing popen");
  return Count(true);
}

bool TestExtFile::test_pclose() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing pclose");
  f_fclose(f);

  f = f_popen("cat test/test_ext_file.tmp", "r");
  VS(f_fread(f, 20), "testing pclose");
  f_pclose(f);
  return Count(true);
}

bool TestExtFile::test_file_exists() {
  VERIFY(f_file_exists("test/test_ext_file.txt"));
  VERIFY(!f_file_exists(""));
  return Count(true);
}

bool TestExtFile::test_file_get_contents() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing file_get_contents");
  f_fclose(f);

  VS(f_file_get_contents("test/test_ext_file.tmp"),
     "testing file_get_contents");

  VS(f_unserialize(f_file_get_contents("compress.zlib://test/test_zlib_file")),
     CREATE_VECTOR1("rblock:216105"));
  return Count(true);
}

bool TestExtFile::test_file_put_contents() {
  f_file_put_contents("test/test_ext_file.tmp", "testing file_put_contents");
  VS(f_file_get_contents("test/test_ext_file.tmp"),
     "testing file_put_contents");
  return Count(true);
}

bool TestExtFile::test_file() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing\nfile\n");
  f_fclose(f);

  Variant items = f_file("test/test_ext_file.tmp");
  VS(items, CREATE_VECTOR2("testing\n", "file\n"));
  return Count(true);
}

bool TestExtFile::test_readfile() {
  f_ob_start();
  VS(f_readfile("test/test_ext_file.txt"), 17);
  VS(f_ob_get_clean(), "Testing Ext File\n");
  f_ob_end_clean();
  return Count(true);
}

bool TestExtFile::test_move_uploaded_file() {
  VS(f_move_uploaded_file("", ""), false);
  return Count(true);
}

bool TestExtFile::test_parse_ini_file() {
  // tested with test_parse_ini_string()
  return Count(true);
}

bool TestExtFile::test_parse_ini_string() {
  String ini
    (";;; Created on Tuesday, October 27, 2009 at 12:01 PM GMT\n"
     "[GJK_Browscap_Version]\n"
     "Version=4520\n"
     "Released=Tue, 27 Oct 2009 12:01:07 -0000\n"
     "\n"
     "\n;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DefaultProperties\n"
     "\n"
     "[DefaultProperties]\n"
     "Browser=\"DefaultProperties\"\n"
     "Version=0\n"
     "Platform=unknown\n"
     "Beta=false\n");
  VS(f_parse_ini_string(ini),
     CREATE_MAP5("Version", "0",
                 "Released", "Tue, 27 Oct 2009 12:01:07 -0000",
                 "Browser", "DefaultProperties",
                 "Platform", "unknown",
                 "Beta", ""));
  VS(f_parse_ini_string(ini, true),
     CREATE_MAP2("GJK_Browscap_Version",
                 CREATE_MAP2("Version", "4520",
                             "Released", "Tue, 27 Oct 2009 12:01:07 -0000"),
                 "DefaultProperties",
                 CREATE_MAP4("Browser", "DefaultProperties",
                             "Version", "0",
                             "Platform", "unknown",
                             "Beta", "")));
  return Count(true);
}

bool TestExtFile::test_parse_hdf_file() {
  // tested with test_parse_hdf_string()
  return Count(true);
}

bool TestExtFile::test_parse_hdf_string() {
  Array hdf;
  hdf.set("bool", false);
  hdf.set("string", "text");
  hdf.set("num", 12345);
  Array arr = CREATE_MAP3("bool", false, "string", "anothertext", "num", 6789);
  hdf.set("arr", arr);

  VS(f_parse_hdf_string("bool = false\n"
                        "string = text\n"
                        "num = 12345\n"
                        "arr {\n"
                        "  bool = false\n"
                        "  string = anothertext\n"
                        "  num = 6789\n"
                        "}\n"), hdf);
  return Count(true);
}

bool TestExtFile::test_write_hdf_file() {
  // tested with test_write_hdf_string()
  return Count(true);
}

bool TestExtFile::test_write_hdf_string() {
  Array hdf;
  hdf.set("bool", false);
  hdf.set("string", "text");
  hdf.set("num", 12345);
  Array arr = CREATE_MAP3("bool", false, "string", "anothertext", "num", 6789);
  hdf.set("arr", arr);

  String str = f_write_hdf_string(hdf);
  VS(str,
     "bool = false\n"
     "string = text\n"
     "num = 12345\n"
     "arr {\n"
     "  bool = false\n"
     "  string = anothertext\n"
     "  num = 6789\n"
     "}\n");

  return Count(true);
}

bool TestExtFile::test_md5_file() {
  VS(f_md5_file("test/test_ext_file.txt"),
     "f53a9b64dc3846f27ee10848d0493320");
  return Count(true);
}

bool TestExtFile::test_sha1_file() {
  VS(f_sha1_file("test/test_ext_file.txt"),
     "3f0f6cb904835174e35697d8262170aa060be3a5");
  return Count(true);
}

bool TestExtFile::test_chmod() {
  VERIFY(f_chmod("test/test_ext_file.txt", 0777));
  return Count(true);
}

bool TestExtFile::test_chown() {
  Variant f = f_fopen("test/test_ext_file.tmp", "w");
  f_fputs(f, "testing\nchown\n");
  f_fclose(f);
  VERIFY(f_chmod("test/test_ext_file.txt", 0777));
  //VERIFY(f_chown("test/test_ext_file.tmp", "hzhao"));
  f_unlink("test/test_ext_file.tmp");
  return Count(true);
}

bool TestExtFile::test_touch() {
  if (f_file_exists("test/test_ext_file.tmp")) {
    f_unlink("test/test_ext_file.tmp");
    VERIFY(!f_file_exists("test/test_ext_file.tmp"));
  }
  f_touch("test/test_ext_file.tmp");
  VERIFY(f_file_exists("test/test_ext_file.tmp"));
  return Count(true);
}

bool TestExtFile::test_copy() {
  if (f_file_exists("test/test_ext_file2.tmp")) {
    f_unlink("test/test_ext_file2.tmp");
    VERIFY(!f_file_exists("test/test_ext_file2.tmp"));
  }
  f_touch("test/test_ext_file.tmp");
  f_copy("test/test_ext_file.tmp", "test/test_ext_file2.tmp");
  VERIFY(f_file_exists("test/test_ext_file2.tmp"));
  VERIFY(f_file_exists("test/test_ext_file.tmp"));
  return Count(true);
}

bool TestExtFile::test_rename() {
  if (f_file_exists("test/test_ext_file2.tmp")) {
    f_unlink("test/test_ext_file2.tmp");
    VERIFY(!f_file_exists("test/test_ext_file2.tmp"));
  }
  f_touch("test/test_ext_file.tmp");
  f_rename("test/test_ext_file.tmp", "test/test_ext_file2.tmp");
  VERIFY(f_file_exists("test/test_ext_file2.tmp"));
  VERIFY(!f_file_exists("test/test_ext_file.tmp"));
  return Count(true);
}

bool TestExtFile::test_umask() {
  f_umask();
  return Count(true);
}

bool TestExtFile::test_unlink() {
  f_touch("test/test_ext_file.tmp");
  VERIFY(f_file_exists("test/test_ext_file.tmp"));
  f_unlink("test/test_ext_file.tmp");
  VERIFY(!f_file_exists("test/test_ext_file.tmp"));
  return Count(true);
}

bool TestExtFile::test_chgrp() {
  f_touch("test/test_ext_file.tmp");
  f_chgrp("test/test_ext_file.tmp", "root");
  f_unlink("test/test_ext_file.tmp");
  return Count(true);
}

bool TestExtFile::test_link() {
  if (f_file_exists("test/test_ext_file2.tmp")) {
    f_unlink("test/test_ext_file2.tmp");
    VERIFY(!f_file_exists("test/test_ext_file2.tmp"));
  }
  f_touch("test/test_ext_file.tmp");
  f_link("test/test_ext_file.tmp", "test/test_ext_file2.tmp");
  VERIFY(f_file_exists("test/test_ext_file2.tmp"));
  VERIFY(f_file_exists("test/test_ext_file.tmp"));
  return Count(true);
}

bool TestExtFile::test_symlink() {
  if (f_file_exists("test/test_ext_file2.tmp")) {
    f_unlink("test/test_ext_file2.tmp");
    VERIFY(!f_file_exists("test/test_ext_file2.tmp"));
  }
  f_touch("test/test_ext_file.tmp");
  f_symlink("test/test_ext_file.tmp", "test/test_ext_file2.tmp");
  VERIFY(f_file_exists("test/test_ext_file2.tmp"));
  VERIFY(f_file_exists("test/test_ext_file.tmp"));
  return Count(true);
}

bool TestExtFile::test_lchgrp() {
  f_touch("test/test_ext_file.tmp");
  f_lchgrp("test/test_ext_file.tmp", "root");
  return Count(true);
}

bool TestExtFile::test_lchown() {
  f_touch("test/test_ext_file.tmp");
  f_lchown("test/test_ext_file.tmp", "root");
  return Count(true);
}

bool TestExtFile::test_basename() {
  VS(f_basename("test/test_ext_file.tmp"), "test_ext_file.tmp");
  VS(f_basename("test/test_ext_file.tmp", ".tmp"), "test_ext_file");
  return Count(true);
}

bool TestExtFile::test_fnmatch() {
  VERIFY(f_fnmatch("test/test_*_file.tmp", "test/test_ext_file.tmp"));
  return Count(true);
}

bool TestExtFile::test_glob() {
  f_touch("test/test_ext_file.tmp");
  VS(f_glob("test/test_*_file.tmp"), CREATE_VECTOR1("test/test_ext_file.tmp"));
  return Count(true);
}

bool TestExtFile::test_tempnam() {
  String name = f_tempnam("/tmp", "test_ext_file.tmp");
  int size = String("/tmp/test_ext_file.tmp").size();
  VERIFY(name.size() > size);
  VS(name.substr(0, size), "/tmp/test_ext_file.tmp");
  unlink(name.data());
  return Count(true);
}

bool TestExtFile::test_tmpfile() {
  Variant f = f_tmpfile();
  f_fputs(f, "testing tmpfile");
  f_fclose(f);
  return Count(true);
}

bool TestExtFile::test_clearstatcache() {
  f_clearstatcache();
  return Count(true);
}

bool TestExtFile::test_stat() {
  VS(f_stat("test/test_ext_file.txt")["size"], 17);
  return Count(true);
}

bool TestExtFile::test_lstat() {
  VS(f_lstat("test/test_ext_file.txt")["size"], 17);
  return Count(true);
}

bool TestExtFile::test_is_dir() {
  f_mkdir("test/tmp_dir");
  VERIFY(!f_is_dir("test/test_ext_file.txt"));
  VERIFY(f_is_dir("test/tmp_dir"));
  VERIFY(!f_is_dir("tmp_dir"));
  f_chdir("test");
  VERIFY(!f_is_dir("test/tmp_dir"));
  VERIFY(f_is_dir("tmp_dir"));
  f_rmdir("test/tmp_dir");
  f_chdir("..");
  return Count(true);
}

bool TestExtFile::test_is_executable() {
  VERIFY(f_chmod("test/test_ext_file.txt", 0777));
  VERIFY(f_is_executable("test/test_ext_file.txt"));
  return Count(true);
}

bool TestExtFile::test_is_file() {
  VERIFY(f_is_file("test/test_ext_file.txt"));
  return Count(true);
}

bool TestExtFile::test_is_link() {
  VERIFY(!f_is_link("test/test_ext_file.txt"));
  return Count(true);
}

bool TestExtFile::test_is_readable() {
  VERIFY(f_is_readable("test/test_ext_file.txt"));
  return Count(true);
}

bool TestExtFile::test_is_uploaded_file() {
  VS(f_is_uploaded_file("test/test_ext_file.txt"), false);
  return Count(true);
}

bool TestExtFile::test_is_writable() {
  VERIFY(f_is_writable("test/test_ext_file.txt"));
  return Count(true);
}

bool TestExtFile::test_is_writeable() {
  VERIFY(f_is_writeable("test/test_ext_file.txt"));
  return Count(true);
}

bool TestExtFile::test_fileatime() {
  VERIFY(more(f_fileatime("test/test_ext_file.txt"), 0));
  return Count(true);
}

bool TestExtFile::test_filectime() {
  VERIFY(more(f_filectime("test/test_ext_file.txt"), 0));
  return Count(true);
}

bool TestExtFile::test_filegroup() {
  f_filegroup("test/test_ext_file.txt");
  return Count(true);
}

bool TestExtFile::test_fileinode() {
  VERIFY(more(f_fileinode("test/test_ext_file.txt"), 0));
  return Count(true);
}

bool TestExtFile::test_filemtime() {
  VERIFY(more(f_filemtime("test/test_ext_file.txt"), 0));
  return Count(true);
}

bool TestExtFile::test_fileowner() {
  f_fileowner("test/test_ext_file.txt");
  return Count(true);
}

bool TestExtFile::test_fileperms() {
  VERIFY(more(f_fileperms("test/test_ext_file.txt"), 0));
  return Count(true);
}

bool TestExtFile::test_filesize() {
  VS(f_filesize("test/test_ext_file.txt"), 17);
  return Count(true);
}

bool TestExtFile::test_filetype() {
  VS(f_filetype("test/test_ext_file.txt"), "file");
  return Count(true);
}

bool TestExtFile::test_readlink() {
  if (f_file_exists("test/test_ext_file2.tmp")) {
    f_unlink("test/test_ext_file2.tmp");
    VERIFY(!f_file_exists("test/test_ext_file2.tmp"));
  }
  f_touch("test/test_ext_file.tmp");
  f_symlink("test/test_ext_file.tmp", "test/test_ext_file2.tmp");
  String resolved = f_readlink("test/test_ext_file2.tmp");
  VS(resolved.substr(resolved.size() - 22), "test/test_ext_file.tmp");
  return Count(true);
}

bool TestExtFile::test_realpath() {
  f_touch("test/test_ext_file.tmp");
  String resolved = f_realpath("test/test_ext_file.tmp");
  VERIFY(resolved.size() > 22);
  VS(resolved.substr(resolved.size() - 22), "test/test_ext_file.tmp");
  return Count(true);
}

bool TestExtFile::test_linkinfo() {
  if (f_file_exists("test/test_ext_file2.tmp")) {
    f_unlink("test/test_ext_file2.tmp");
    VERIFY(!f_file_exists("test/test_ext_file2.tmp"));
  }
  f_touch("test/test_ext_file.tmp");
  f_symlink("test/test_ext_file.tmp", "test/test_ext_file2.tmp");
  VERIFY(more(f_linkinfo("test/test_ext_file2.tmp"), 0));
  return Count(true);
}

bool TestExtFile::test_disk_free_space() {
  VERIFY(more(f_disk_free_space("test"), 0));
  return Count(true);
}

bool TestExtFile::test_diskfreespace() {
  VERIFY(more(f_diskfreespace("test"), 0));
  return Count(true);
}

bool TestExtFile::test_disk_total_space() {
  VERIFY(more(f_disk_total_space("test"), 0));
  return Count(true);
}

bool TestExtFile::test_pathinfo() {
  VS(f_print_r(f_pathinfo("test/test_ext_file.txt"), true),
     "Array\n"
     "(\n"
     "    [dirname] => test\n"
     "    [basename] => test_ext_file.txt\n"
     "    [extension] => txt\n"
     "    [filename] => test_ext_file\n"
     ")\n");
  return Count(true);
}

bool TestExtFile::test_mkdir() {
  f_mkdir("test/tmp_dir");
  f_rmdir("test/tmp_dir");
  return Count(true);
}

bool TestExtFile::test_rmdir() {
  f_mkdir("test/tmp_dir");
  f_rmdir("test/tmp_dir");
  return Count(true);
}

bool TestExtFile::test_dirname() {
  VS(f_dirname("test/test_ext_file.txt"), "test");
  return Count(true);
}

bool TestExtFile::test_chdir() {
  f_chdir("test");
  f_chdir("..");
  return Count(true);
}

bool TestExtFile::test_chroot() {
  //f_chroot("test");
  return Count(true);
}

bool TestExtFile::test_dir() {
  Variant d = f_dir("test");
  VS(d.toArray()["path"], "test");
  Variant entry;
  bool seen = false;
  while (!same(entry = f_readdir(d.toArray()["handle"]), false)) {
    if (same(entry, "test_ext_file.txt")) {
      seen = true;
    }
  }
  f_closedir(d);
  VERIFY(seen);
  return Count(true);
}

bool TestExtFile::test_getcwd() {
  VERIFY(!f_getcwd().toString().empty());
  return Count(true);
}

bool TestExtFile::test_opendir() {
  Variant d = f_opendir("test");
  Variant entry;
  bool seen = false;
  while (!same(entry = f_readdir(d), false)) {
    if (same(entry, "test_ext_file.txt")) {
      seen = true;
    }
  }
  f_closedir(d);
  VERIFY(seen);
  return Count(true);
}

bool TestExtFile::test_readdir() {
  Variant d = f_opendir("test");
  Variant entry;
  bool seen = false;
  while (!same(entry = f_readdir(d), false)) {
    if (same(entry, "test_ext_file.txt")) {
      seen = true;
    }
  }
  f_closedir(d);
  VERIFY(seen);
  return Count(true);
}

bool TestExtFile::test_rewinddir() {
  Variant d = f_opendir("test");
  Variant entry;
  bool seen = false;
  while (!same(entry = f_readdir(d), false)) {
    if (same(entry, "test_ext_file.txt")) {
      seen = true;
    }
  }
  VERIFY(seen);

  seen = false;
  f_rewinddir(d);
  while (!same(entry = f_readdir(d), false)) {
    if (same(entry, "test_ext_file.txt")) {
      seen = true;
    }
  }
  VERIFY(seen);

  f_closedir(d);
  return Count(true);
}

bool TestExtFile::test_scandir() {
  VERIFY(!f_scandir("test").toArray().empty());
  return Count(true);
}

bool TestExtFile::test_closedir() {
  f_closedir(f_opendir("test"));
  return Count(true);
}
