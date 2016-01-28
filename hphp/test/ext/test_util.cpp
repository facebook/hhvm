/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/shared-string.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/config.h"

#define VERIFY_DUMP(map, exp)                                           \
  if (!(exp)) {                                                         \
    printf("%s:%d: [" #exp "] is false\n", __FILE__, __LINE__);         \
    map.dump();                                                         \
    return Count(false);                                                \
  }                                                                     \

///////////////////////////////////////////////////////////////////////////////

TestUtil::TestUtil() {
}

///////////////////////////////////////////////////////////////////////////////

bool TestUtil::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(TestSharedString);
  RUN_TEST(TestCanonicalize);
  RUN_TEST(TestHDF);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// data types

struct testhash {
  size_t operator()(const String& s) const {
    return hash_string_unsafe(s.data(), s.size());
  }
};

struct testeqstr {
  bool operator()(const String& s1, const String& s2) const {
    return string_strcmp(s1.data(), s1.size(), s2.data(), s2.size()) == 0;
  }
};

struct IHash {
  size_t operator()(int i) const { return i; }
};
struct IEq {
  bool operator()(int i, int j) const { return i == j; }
};

bool TestUtil::TestSharedString() {
  {
    SharedString foo = "foo";
    SharedString bah("bah");
    VERIFY(bah->getString() == "bah");
    {
      SharedString bah2 = bah;
      VERIFY(bah2->getString() == "bah");
      VERIFY(bah.get() == bah2.get());
      SharedString bah3 = "bah";
      VERIFY(bah.get() == bah3.get());
    }
    VERIFY(bah->getString() == "bah");
    VERIFY(foo->getString() == "foo");
  }
  {
    hphp_shared_string_map<int64_t> map;
    for (int i = 0; i < 100; i++) {
      std::string k("key");
      k += i;
      map[k] = i;
    }
    for (int i = 0; i < 100; i++) {
      std::string k("key");
      k += i;
      auto const it = map.find(k);
      VERIFY(it != map.end());
      VERIFY(it->second == i);
    }
  }

  return Count(true);
}

bool TestUtil::TestCanonicalize() {
  VERIFY(FileUtil::canonicalize(String("foo")) == String("foo"));
  VERIFY(FileUtil::canonicalize(String("/foo")) == String("/foo"));
  VERIFY(FileUtil::canonicalize(String("foo/bar")) == String("foo/bar"));
  VERIFY(FileUtil::canonicalize(String("foo/////bar")) == String("foo/bar"));
  VERIFY(FileUtil::canonicalize(String("foo/bar/")) == String("foo/bar/"));
  VERIFY(FileUtil::canonicalize(String("./foo")) == String("foo"));
  VERIFY(FileUtil::canonicalize(String(".")) == String("."));
  VERIFY(FileUtil::canonicalize(String("./")) == String("./"));
  VERIFY(FileUtil::canonicalize(String("././")) == String("./"));
  VERIFY(FileUtil::canonicalize(String("foo/./")) == String("foo/"));
  VERIFY(FileUtil::canonicalize(String("foo/../bar")) == String("bar"));
  VERIFY(FileUtil::canonicalize(String("./foo/../bar")) == String("bar"));
  VERIFY(FileUtil::canonicalize(String(".////foo/xyz////..////../bar"))
         == String("bar"));
  VERIFY(FileUtil::canonicalize(String("a/foo../bar"))
         == String("a/foo../bar"));
  VERIFY(FileUtil::canonicalize(String("a./foo/./bar"))
         == String("a./foo/bar"));
  VERIFY(FileUtil::canonicalize(String("////a/foo")) == String("/a/foo"));
  VERIFY(FileUtil::canonicalize(String("../foo")) == String("../foo"));
  VERIFY(FileUtil::canonicalize(String("foo/../../bar")) == String("../bar"));
  VERIFY(FileUtil::canonicalize(String("./../../")) == String("../../"));
  VERIFY(FileUtil::canonicalize(String("/test\0", 6, CopyString))
         == String("/test"));
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
