/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/shared-string.h"
#include "hphp/runtime/base/zend-string.h"

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
    return hash_string(s.data(), s.size());
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
      string k("key");
      k += i;
      map[k] = i;
    }
    for (int i = 0; i < 100; i++) {
      string k("key");
      k += i;
      hphp_shared_string_map<int64_t>::const_iterator it = map.find(k);
      VERIFY(it != map.end());
      VERIFY(it->second == i);
    }
  }

  return Count(true);
}

bool TestUtil::TestCanonicalize() {
  VERIFY(Util::canonicalize("foo") == "foo");
  VERIFY(Util::canonicalize("/foo") == "/foo");
  VERIFY(Util::canonicalize("./foo") == "foo");
  VERIFY(Util::canonicalize("foo/bar") == "foo/bar");
  VERIFY(Util::canonicalize("foo/////bar") == "foo/bar");
  VERIFY(Util::canonicalize("foo/bar/") == "foo/bar/");
  VERIFY(Util::canonicalize("foo/../bar") == "bar");
  VERIFY(Util::canonicalize("./foo/../bar") == "bar");
  VERIFY(Util::canonicalize(".////foo/xyz////..////../bar") == "bar");
  VERIFY(Util::canonicalize("a/foo../bar") == "a/foo../bar");
  VERIFY(Util::canonicalize("a./foo/./bar") == "a./foo/bar");
  VERIFY(Util::canonicalize("////a/foo") == "/a/foo");
  VERIFY(Util::canonicalize("../foo") == "../foo");
  VERIFY(Util::canonicalize("foo/../../bar") == "../bar");
  VERIFY(Util::canonicalize("./../../") == "../../");
  return Count(true);
}

bool TestUtil::TestHDF() {
  // This was causing a crash
  {
    Hdf doc, node;
    node = doc["Node"];
  }

  {
    Hdf doc;
    doc.fromString(
      "node.* {\n"
      "  name = value\n"
      "}");
    VS(doc["node"][0]["name"].getString(), "value");
  }

  return Count(true);
}
