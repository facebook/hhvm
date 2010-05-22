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

#include <test/test_util.h>
#include <util/lfu_table.h>
#include <runtime/base/util/hphp_map.h>
#include <runtime/base/complex_types.h>
#include <util/logger.h>
#include <runtime/base/shared/shared_string.h>

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
  RUN_TEST(TestHphpMap);
  RUN_TEST(TestHphpVector);
  //RUN_TEST(TestLFUTable);
  RUN_TEST(TestSharedString);
  RUN_TEST(TestCanonicalize);
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

bool TestUtil::TestHphpMap() {
  {
    VS(Variant("0").toKey(), Variant(0));
    VS(Variant("10").toKey(), Variant(10));
    VS(Variant("00").toKey(), Variant("00"));
    VS(Variant("01").toKey(), Variant("01"));
    VS(Variant("name").toKey(), Variant("name"));
    VS(Variant("-0").toKey(), Variant("-0"));
    VS(Variant("-").toKey(), Variant("-"));
    VS(Variant("9223372036854775806").toKey(),
       Variant(9223372036854775806));
    VS(Variant("9223372036854775807").toKey(),
       Variant("9223372036854775807"));
    VS(Variant("-9223372036854775807").toKey(),
       Variant(-9223372036854775807));
    VS(Variant("-9223372036854775808").toKey(),
       Variant("-9223372036854775808"));
  }

  {
    HphpMap hmap;
    VERIFY_DUMP(hmap,hmap.size() == 0);
  }

  {
    HphpMap hmap;
    Variant k("key");
    int old;
    VERIFY_DUMP(hmap,hmap.insert(k, 10, old));
    VERIFY_DUMP(hmap,!hmap.insert(k, 11, old));
    VERIFY_DUMP(hmap,old == 10);
    old = 0;
    VERIFY_DUMP(hmap,hmap.find(k, old));
    VERIFY_DUMP(hmap,old == 10);
    VERIFY_DUMP(hmap,hmap.size() == 1);
    hmap.erase(k);
    old = 0;
    VERIFY_DUMP(hmap,!hmap.find(k, old));
    VERIFY_DUMP(hmap,hmap.insert(k, 11, old));
  }

  {
    HphpMap hmap;
    for (int i = 0; i < 1000; i++) {
      Variant k(i);
      int old;
      VERIFY_DUMP(hmap,hmap.insert(k, i*3, old));
    }
    VERIFY_DUMP(hmap,hmap.size() == 1000);
    for (int i = 0; i < 100; i++) {
      Variant k(i);
      int old;
      VERIFY_DUMP(hmap,!hmap.insert(k, i*4, old));
      VERIFY_DUMP(hmap,old == i*3);
    }

    for (int i = 0; i < 1000; i++) {
      Variant k(i);
      int old;
      VERIFY_DUMP(hmap,hmap.find(k, old));
      VERIFY_DUMP(hmap,old == i*3);
      hmap.erase(k);
    }
    VERIFY_DUMP(hmap,hmap.size() == 0);

  }

  {
    HphpMap hmap;
    for (int i = 0; i < 100; i++) {
      Variant k(i);
      int old;
      VERIFY_DUMP(hmap,hmap.insert(k, i*2, old));
    }
    HphpMap::iterator it =
      hmap.begin();
    for(; it != hmap.end(); ++it) {
      int i = it->key().toInt32();
      VERIFY_DUMP(hmap,i*2 == it->value());
    }
  }
  {
    HphpMap hmap;
    for (int i = 0; i < 100; i++) {
      Variant k(i);
      int old;
      VERIFY_DUMP(hmap,hmap.insert(k, i*2, old));
    }
    HphpMap::const_iterator it =
      hmap.begin();
    for(; it != hmap.end(); ++it) {
      int i = it->key().toInt32();
      VERIFY_DUMP(hmap,i*2 == it->value());
    }
  }
  {
    HphpMap hmap;
    for (int i = 0; i < 100; i++) {
      Variant k(i);
      hmap[k] = i*2;
    }
    for (int i = 0; i < 100; i++) {
      Variant k(i);
      VERIFY_DUMP(hmap,hmap[k] == i*2);
    }
  }
  {
    HphpMap hmap;
    Variant k1("n1");
    hmap[k1] = 0;
    VERIFY_DUMP(hmap,hmap[k1] == 0);
    Variant k2("n2");
    hmap[k2] = 1;
    VERIFY_DUMP(hmap,hmap[k2] == 1);
    HphpMap::iterator it =
      hmap.begin();
    for(; it != hmap.end(); ++it) {
      VERIFY_DUMP(hmap,it->value() < 2);
    }
  }
  {
   HphpMap hmap1;
   {
     HphpMap hmap2;
     hmap2[Variant(1)] = 1;
     hmap2[Variant(2)] = 2;
     hmap1 = hmap2;
     HphpMap hmap3(hmap2);
     VERIFY_DUMP(hmap3,hmap3[Variant(1)] == 1);
     VERIFY_DUMP(hmap3,hmap3[Variant(2)] == 2);
   }
   VERIFY_DUMP(hmap1,hmap1[Variant(1)] == 1);
   VERIFY_DUMP(hmap1,hmap1[Variant(2)] == 2);
  }

  return Count(true);
}

bool TestUtil::TestHphpVector() {
  {
    HphpVector<int64> items;
    items.push_back(1);
    items.push_back(2);
    for (unsigned int i = 0; i < items.size(); i++) {
      VERIFY(items[i] == i + 1);
    }
    VERIFY(items.size() == 2);

    items.insert(1, 3);
    VERIFY(items.size() == 3);
    VERIFY(items[0] == 1);
    VERIFY(items[1] == 3);
    VERIFY(items[2] == 2);
    VERIFY(items.back() == 2);

    items.reserve(10);

    items.resize(5);
    VERIFY(items.size() == 5);
    VERIFY(items[0] == 1);
    VERIFY(items[1] == 3);
    VERIFY(items[2] == 2);
    VERIFY(items[3] == 0);
    VERIFY(items[4] == 0);
    VERIFY(items.back() == 0);

    items.reserve(1);

    items.remove(2);
    VERIFY(items.size() == 4);
    VERIFY(items[0] == 1);
    VERIFY(items[1] == 3);
    VERIFY(items[2] == 0);
    VERIFY(items[3] == 0);
    VERIFY(items.back() == 0);

    HphpVector<int64> items2;
    items2.push_back(3);
    items2.push_back(4);
    items.swap(items2);
    VERIFY(items.size() == 2);
    VERIFY(items[0] == 3);
    VERIFY(items[1] == 4);

    items.append(items2, 1, 2);
    VERIFY(items.size() == 4);
    VERIFY(items[0] == 3);
    VERIFY(items[1] == 4);
    VERIFY(items[2] == 3);
    VERIFY(items[3] == 0);

    items.clear();
    VERIFY(items.size() == 0);
    VERIFY(items.empty());
  }
  {
    HphpVector<String> items;
    items.push_back("apple");
    items.push_back("orange");
    VERIFY(items.size() == 2);
    VERIFY(items[0] == "apple");
    VERIFY(items[1] == "orange");

    items.insert(1, "pear");
    VERIFY(items.size() == 3);
    VERIFY(items[0] == "apple");
    VERIFY(items[1] == "pear");
    VERIFY(items[2] == "orange");

    items.remove(2);
    VERIFY(items.size() == 2);
    VERIFY(items[0] == "apple");
    VERIFY(items[1] == "pear");

    HphpVector<String> items2;
    items2.push_back("banana");
    items2.push_back("grape");
    items.append(items2);
    VERIFY(items.size() == 4);
    VERIFY(items[0] == "apple");
    VERIFY(items[1] == "pear");
    VERIFY(items[2] == "banana");
    VERIFY(items[3] == "grape");
    String s = items[2];
    Variant b = s;

    items.swap(items2);
    VERIFY(items.size() == 2);
    VERIFY(items[0] == "banana");
    VERIFY(items[1] == "grape");

    items.append(items2, 1, 2);
    VERIFY(items.size() == 4);
    VERIFY(items[0] == "banana");
    VERIFY(items[1] == "grape");
    VERIFY(items[2] == "pear");
    VERIFY(items[3] == "banana");

    items.append(items2, 3);
    VERIFY(items.size() == 5);
    VERIFY(items[0] == "banana");
    VERIFY(items[1] == "grape");
    VERIFY(items[2] == "pear");
    VERIFY(items[3] == "banana");
    VERIFY(items[4] == "grape");

    VS(b, "banana");

    HphpVector<String> items3 = items;
    VERIFY(items3.size() == 5);
    VERIFY(items3[0] == "banana");
    VERIFY(items3[1] == "grape");
    VERIFY(items3[2] == "pear");
    VERIFY(items3[3] == "banana");
    VERIFY(items3[4] == "grape");
  }

  return Count(true);
}

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
