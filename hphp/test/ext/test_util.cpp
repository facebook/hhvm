/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/test/ext/test_util.h"
#include "hphp/util/logger.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/config.h"

#define VERIFY_DUMP(map, exp)                                           \
  if (!(exp)) {                                                         \
    printf("%s:%d: [" #exp "] is false\n", __FILE__, __LINE__);         \
    map.dump();                                                         \
    return Count(false);                                                \
  }                                                                     \

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

TestUtil::TestUtil() {
}

///////////////////////////////////////////////////////////////////////////////

bool TestUtil::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestCanonicalize);
  RUN_TEST(TestHDF);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// data types

struct testhash {
  size_t operator()(const HPHP::OptString& s) const {
    return HPHP::hash_string_i_unsafe(s.data(), s.size());
  }
};

struct testeqstr {
  bool operator()(const HPHP::OptString& s1, const HPHP::OptString& s2) const {
    return HPHP::string_strcmp(s1.data(), s1.size(), s2.data(), s2.size()) == 0;
  }
};

struct IHash {
  size_t operator()(int i) const { return i; }
};
struct IEq {
  bool operator()(int i, int j) const { return i == j; }
};

bool TestUtil::TestCanonicalize() {
  VERIFY(HPHP::FileUtil::canonicalize(HPHP::OptString("foo")) == HPHP::OptString("foo"));
  VERIFY(FileUtil::canonicalize(OptString("/foo")) == OptString("/foo"));
  VERIFY(FileUtil::canonicalize(OptString("foo/bar")) == OptString("foo/bar"));
  VERIFY(FileUtil::canonicalize(OptString("foo/////bar")) == OptString("foo/bar"));
  VERIFY(FileUtil::canonicalize(OptString("foo/bar/")) == OptString("foo/bar/"));
  VERIFY(FileUtil::canonicalize(OptString("./foo")) == OptString("foo"));
  VERIFY(FileUtil::canonicalize(OptString(".")) == OptString("."));
  VERIFY(FileUtil::canonicalize(OptString("./")) == OptString("./"));
  VERIFY(FileUtil::canonicalize(OptString("././")) == OptString("./"));
  VERIFY(FileUtil::canonicalize(OptString("foo/./")) == OptString("foo/"));
  VERIFY(FileUtil::canonicalize(OptString("foo/../bar")) == OptString("bar"));
  VERIFY(FileUtil::canonicalize(OptString("./foo/../bar")) == OptString("bar"));
  VERIFY(FileUtil::canonicalize(OptString(".////foo/xyz////..////../bar"))
         == OptString("bar"));
  VERIFY(FileUtil::canonicalize(OptString("a/foo../bar"))
         == OptString("a/foo../bar"));
  VERIFY(FileUtil::canonicalize(OptString("a./foo/./bar"))
         == OptString("a./foo/bar"));
  VERIFY(FileUtil::canonicalize(OptString("////a/foo")) == OptString("/a/foo"));
  VERIFY(FileUtil::canonicalize(OptString("../foo")) == OptString("../foo"));
  VERIFY(FileUtil::canonicalize(OptString("foo/../../bar")) == OptString("../bar"));
  VERIFY(FileUtil::canonicalize(OptString("./../../")) == OptString("../../"));
  VERIFY(FileUtil::canonicalize(OptString("/test\0", 6, CopyString))
         == OptString(""));
  VERIFY(FileUtil::canonicalize(OptString("/test\0test", 10, CopyString))
         == OptString(""));
  return Count(true);
}

bool TestUtil::TestHDF() {
  // This was causing a crash
  {
    Hdf doc, node;
    node = doc["Node"];
  }

  {
    IniSetting::Map ini = IniSetting::Map::object;
    Hdf doc;
    doc.fromString(
      "node.* {\n"
      "  name = value\n"
      "}");
    VS(Config::GetString(ini, doc, "node.0.name"), "value");
  }

  return Count(true);
}
