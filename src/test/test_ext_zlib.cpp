/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
  RUN_TEST(test_qlzcompress);
  RUN_TEST(test_qlzuncompress);
  RUN_TEST(test_sncompress);
  RUN_TEST(test_snuncompress);
  RUN_TEST(test_nzcompress);
  RUN_TEST(test_nzuncompress);
  RUN_TEST(test_lz4compress);
  RUN_TEST(test_lz4hccompress);
  RUN_TEST(test_lz4uncompress);

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

bool TestExtZlib::test_qlzcompress() {
  // tested in test_qlzuncompress();
  return Count(true);
}

bool TestExtZlib::test_qlzuncompress() {
  try {
    VS(f_qlzuncompress(f_qlzcompress("testing gzcompress", 1), 1),
       "testing gzcompress");
    VS(f_qlzuncompress(f_qlzcompress("testing gzcompress", 2), 2),
       "testing gzcompress");
    VS(f_qlzuncompress(f_qlzcompress("testing gzcompress", 3), 3),
       "testing gzcompress");
  } catch (NotSupportedException e) {
    SKIP("no qlzcompress() support");
  }
  return Count(true);
}

bool TestExtZlib::test_sncompress() {
  // tested in test_sncompress();
  return Count(true);
}

bool TestExtZlib::test_snuncompress() {
  try {
    VS(f_snuncompress(f_sncompress("testing sncompress")),
       "testing sncompress");
  } catch (NotSupportedException e) {
    SKIP("No sncompress() support");
  }
  return Count(true);
}

bool TestExtZlib::test_nzcompress() {
  char compressable[1025];
  memset(compressable, 'A', 1024);
  compressable[1024] = '\0';
  String s(compressable, CopyString);
  String t(f_nzcompress(s).asStrRef());
  if (s.size() <= t.size()) {
    return Count(false);
  }
  String u(f_nzuncompress(t).asStrRef());
  if (s != u) {
    return Count(false);
  }
  char *p = (char*)malloc(t.size());
  if (p == NULL) {
    return Count(false);
  }

  memset(compressable, '\0', 1025);
  String bs(compressable, 1024, CopyString);
  String bt(f_nzcompress(bs).asStrRef());
  if (bs.size() <= bt.size()) {
    return Count(false);
  }
  String bu(f_nzuncompress(bt).asStrRef());
  if (bu != bs || bu.size() != 1024) {
    return Count(false);
  }

  return Count(true);
}

bool TestExtZlib::test_nzuncompress() {
  String s("garbage stuff", AttachLiteral);
  Variant v = f_nzuncompress(s);
  if (v != Variant(false)) {
    return Count(false);
  }

  String empty("", AttachLiteral);
  String c(f_nzcompress(empty).asStrRef());
  String d(f_nzuncompress(c).asStrRef());
  if (d != empty) {
    return Count(false);
  }

  return Count(true);
}

bool TestExtZlib::test_lz4compress() {
  VS(f_lz4uncompress(f_lz4compress("testing lz4compress")),
      "testing lz4compress");
  return Count(true);
}

bool TestExtZlib::test_lz4hccompress() {
  VS(f_lz4uncompress(f_lz4hccompress("testing lz4hccompress")),
      "testing lz4hccompress");
  return Count(true);
}

bool TestExtZlib::test_lz4uncompress() {
  // first test uncompressing invalid string
  String s("invalid compressed string", AttachLiteral);
  Variant v = f_lz4uncompress(s);
  if (v != Variant(false)) {
    return Count(false);
  }
  // try uncompressing empty string
  String empty("", AttachLiteral);
  v = f_lz4uncompress(empty);
  if (v != Variant(false)) {
    return Count(false);
  }

  // compress and uncompress empty string
  String c(f_lz4compress(empty).asStrRef());
  String d(f_lz4uncompress(c).asStrRef());
  if (d != empty) {
    return Count(false);
  }

  return Count(true);
}
