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

#include <test/test_util.h>
#include <util/logger.h>
#include <util/lfu_table.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/shared/shared_string.h>
#include <runtime/base/zend/zend_string.h>

using namespace std;

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
  //RUN_TEST(TestLFUTable);
  RUN_TEST(TestSharedString);
  RUN_TEST(TestCanonicalize);
  RUN_TEST(TestHDF);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// data types

struct testhash {
  size_t operator()(CStrRef s) const {
    return hash_string(s.data(), s.size());
  }
};

struct testeqstr {
  bool operator()(CStrRef s1, CStrRef s2) const {
    return string_strcmp(s1.data(), s1.size(), s2.data(), s2.size()) == 0;
  }
};

struct IHash {
  size_t operator()(int i) const { return i; }
};
struct IEq {
  bool operator()(int i, int j) const { return i == j; }
};

typedef LFUTable<int, int, IHash, IEq> TestMap;

bool TestUtil::TestLFUTable() {
  {
    TestMap table(1, 5, 2);
    table.insert(1,1);
    int r = 0;
    VERIFY(table.lookup(1, r) && r == 1);
    VERIFY(table.lookup(1, r) && r == 1);
    VERIFY(table.lookup(1, r) && r == 1);
  }
  {
    TestMap table(100, 5, 2);
    for (int i = 0; i < 10; i++) {
      table.insert(i,i);
    }
    VERIFY(table.check());
    int r = 0;
    VERIFY(!table.lookup(0, r));
    VERIFY(!table.lookup(1, r));
    VERIFY(!table.lookup(2, r));
    VERIFY(!table.lookup(3, r));
    VERIFY(!table.lookup(4, r));
    VERIFY(table.lookup(5, r) && r == 5);
    VERIFY(table.lookup(6, r) && r == 6);
    VERIFY(table.lookup(7, r) && r == 7);
    VERIFY(table.lookup(8, r) && r == 8);
    VERIFY(table.lookup(9, r) && r == 9);
    VERIFY(table.check());
  }
  {
    TestMap table(1, 5, 2);
    for (int i = 0; i < 5; i++) {
      table.insert(i,i);
    }
    sleep(1);
    for (int i = 5; i < 10; i++) {
      table.insert(i,i);
    }
    VERIFY(table.check());
    int r = 0;
    VERIFY(!table.lookup(0, r));
    VERIFY(!table.lookup(1, r));
    VERIFY(!table.lookup(2, r));
    VERIFY(!table.lookup(3, r));
    VERIFY(!table.lookup(4, r));
    VERIFY(table.lookup(5, r) && r == 5);
    VERIFY(table.lookup(6, r) && r == 6);
    VERIFY(table.lookup(7, r) && r == 7);
    VERIFY(table.lookup(8, r) && r == 8);
    VERIFY(table.lookup(9, r) && r == 9);
    VERIFY(table.check());
  }
  {
    TestMap table(1, 5, 2);
    for (int i = 0; i < 5; i++) {
      table.insert(i,i);
    }
    int r = 0;
    VERIFY(table.lookup(0, r) && r == 0);
    VERIFY(table.lookup(0, r) && r == 0);
    VERIFY(table.lookup(3, r) && r == 3);
    sleep(1);
    for (int i = 5; i < 8; i++) {
      table.insert(i,i);
    }
    VERIFY(table.lookup(0, r) && r == 0);
    VERIFY(!table.lookup(1, r));
    VERIFY(!table.lookup(2, r));
    VERIFY(table.lookup(3, r) && r == 3);
    VERIFY(!table.lookup(4, r));
    VERIFY(table.lookup(5, r) && r == 5);
    VERIFY(table.lookup(6, r) && r == 6);
    VERIFY(table.lookup(7, r) && r == 7);
    int ct = 0;
    for (TestMap::const_iterator it = table.begin();
         it != table.end(); ++it) {
      ct++;
      VERIFY(it.first() == it.second());
    }
    VERIFY(ct == 5);
  }
  {
    TestMap table(1, 5, 2);
    for (int i = 0; i < 5; i++) {
      table[i] = i;
    }
    int r = 0;
    VERIFY(table[0] == 0);
    VERIFY(table[0] == 0);
    VERIFY(table[3] == 3);
    sleep(1);
    for (int i = 5; i < 8; i++) {
      table[i] = i;
    }
    VERIFY(table.check());
    VERIFY(table.lookup(0, r) && r == 0);
    VERIFY(!table.lookup(1, r));
    VERIFY(!table.lookup(2, r));
    VERIFY(table.lookup(3, r) && r == 3);
    VERIFY(!table.lookup(4, r));
    VERIFY(table.lookup(5, r) && r == 5);
    VERIFY(table.lookup(6, r) && r == 6);
    VERIFY(table.lookup(7, r) && r == 7);
    VERIFY(table.check());
    int ct = 0;
    for (TestMap::const_iterator it = table.begin();
         it != table.end(); ++it) {
      ct++;
      VERIFY(it.first() == it.second());
    }
    VERIFY(ct == 5);
  }
  {
    class Reader : public TestMap::AtomicReader {
    public:
      void read(int const &k, int const &v) {

      }
    };
    class Updater : public TestMap::AtomicUpdater {
    public:
      bool update(int const &k, int &v, bool newly) {
        v = k;
        return false;
      }
    };
    Reader r;
    Updater u;
    TestMap table(1, 5, 2);
    for (int i = 0; i < 5; i++) {
      table.atomicUpdate(i, u, true);
    }
    for (int i = 0; i < 5; i++) {
      VERIFY(table.atomicRead(i, r));
    }
  }
  return Count(true);
}

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
    hphp_shared_string_map<int64> map;
    for (int i = 0; i < 100; i++) {
      string k("key");
      k += i;
      map[k] = i;
    }
    for (int i = 0; i < 100; i++) {
      string k("key");
      k += i;
      hphp_shared_string_map<int64>::const_iterator it = map.find(k);
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
  Hdf doc, node;
  node = doc["Node"];
  return Count(true);
}
