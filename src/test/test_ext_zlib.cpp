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

#include <test/test_ext_zlib.h>
#include <runtime/ext/ext_zlib.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_output.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtZlib::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_readgzfile);
  RUN_TEST(test_gzfile);
  RUN_TEST(test_gzcompress);
  RUN_TEST(test_gzuncompress);
  RUN_TEST(test_gzdeflate);
  RUN_TEST(test_gzinflate);
  RUN_TEST(test_gzencode);
  RUN_TEST(test_gzdecode);
  RUN_TEST(test_zlib_get_coding_type);
  RUN_TEST(test_gzopen);
  RUN_TEST(test_gzclose);
  RUN_TEST(test_gzrewind);
  RUN_TEST(test_gzeof);
  RUN_TEST(test_gzgetc);
  RUN_TEST(test_gzgets);
  RUN_TEST(test_gzgetss);
  RUN_TEST(test_gzread);
  RUN_TEST(test_gzpassthru);
  RUN_TEST(test_gzseek);
  RUN_TEST(test_gztell);
  RUN_TEST(test_gzwrite);
  RUN_TEST(test_gzputs);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtZlib::test_readgzfile() {
  f_ob_start();
  f_readgzfile("test/test_ext_zlib.gz");
  VS(f_ob_get_clean(), "Testing Ext Zlib\n");
  f_ob_end_clean();
  return Count(true);
}

bool TestExtZlib::test_gzfile() {
  VS(f_gzfile("test/test_ext_zlib.gz"), CREATE_VECTOR1("Testing Ext Zlib\n"));
  return Count(true);
}

bool TestExtZlib::test_gzcompress() {
  VS(f_gzuncompress(f_gzcompress("testing gzcompress")), "testing gzcompress");
  return Count(true);
}

bool TestExtZlib::test_gzuncompress() {
  // tested in gzcompress
  return Count(true);
}

bool TestExtZlib::test_gzdeflate() {
  VS(f_gzinflate(f_gzdeflate("testing gzdeflate")), "testing gzdeflate");
  return Count(true);
}

bool TestExtZlib::test_gzinflate() {
  // tested in gzdeflate
  return Count(true);
}

bool TestExtZlib::test_gzencode() {
  Variant zipped = f_gzencode("testing gzencode");
  Variant f = f_fopen("test/test_ext_zlib.tmp", "w");
  f_fwrite(f, zipped);
  f_fclose(f);

  f_ob_start();
  f_readgzfile("test/test_ext_zlib.tmp");
  VS(f_ob_get_clean(), "testing gzencode");
  f_ob_end_clean();
  return Count(true);
}

bool TestExtZlib::test_gzdecode() {
  Variant zipped = f_gzencode("testing gzencode");
  VS(f_gzdecode(zipped), "testing gzencode");
  return Count(true);
}

bool TestExtZlib::test_zlib_get_coding_type() {
  try {
    f_zlib_get_coding_type();
  } catch (NotSupportedException e) {
    return Count(true);
  }
  return Count(false);
}

bool TestExtZlib::test_gzopen() {
  Variant f = f_gzopen("test/test_ext_zlib.tmp", "w");
  VERIFY(!same(f, false));
  f_gzputs(f, "testing gzputs\n");
  f_gzwrite(f, "<html>testing gzwrite</html>\n");
  f_gzclose(f);

  f = f_gzopen("test/test_ext_zlib.tmp", "r");
  VS(f_gzread(f, 7), "testing");
  VS(f_gzgetc(f), " ");
  VS(f_gzgets(f), "gzputs\n");
  VS(f_gzgetss(f), "testing gzwrite\n");
  VS(f_gztell(f), 44);
  VERIFY(f_gzeof(f));
  VERIFY(f_gzrewind(f));
  VS(f_gztell(f), 0);
  VERIFY(!f_gzeof(f));
  f_gzseek(f, -7, k_SEEK_END);
  VS(f_gzgets(f), "testing gzputs\n");
  f_gzclose(f);

  return Count(true);
}

bool TestExtZlib::test_gzclose() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzrewind() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzeof() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzgetc() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzgets() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzgetss() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzread() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzpassthru() {
  Variant f = f_gzopen("test/test_ext_zlib.gz", "r");
  f_ob_start();
  f_gzpassthru(f);
  VS(f_ob_get_clean(), "Testing Ext Zlib\n");
  f_ob_end_clean();
  return Count(true);
}

bool TestExtZlib::test_gzseek() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gztell() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzwrite() {
  // tested in gzopen
  return Count(true);
}

bool TestExtZlib::test_gzputs() {
  // tested in gzopen
  return Count(true);
}
