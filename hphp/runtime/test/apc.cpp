/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <gtest/gtest.h>

#include <memory>
#include <cstring>

#include <folly/Memory.h>
#include <folly/Format.h>

#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/concurrent-shared-store.h"
#include "hphp/runtime/base/shared-store-base.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

using Store = ConcurrentTableSharedStore;
using PrimePair = Store::KeyValuePair;

char* alloc_leaked_string(const char* source) {
  return strdup(source);
}

template<class Create>
std::vector<PrimePair> primable_n(Store& store,
                                  const char* prefix,
                                  Create create) {
  std::vector<PrimePair> pairs(10);
  auto counter = int64_t{0};
  for (auto& pair : pairs) {
    auto key = folly::sformat("{}_{}", prefix, counter);
    pair.key = alloc_leaked_string(key.c_str());
    pair.len = key.size();

    Variant v(create(counter));
    store.constructPrime(v, pair);

    ++counter;
  }
  return pairs;
}

/*
 * Sets int_n -> KindOfInt64 n
 *
 * Note that ints are never file-backed in apc right now.
 */
std::vector<PrimePair> primable_ints(Store& store) {
  return primable_n(store, "int", [&] (int64_t n) {
    return n;
  });
}

/*
 * Sets obj_n -> a stdClass
 */
std::vector<PrimePair> primable_objs(Store& store) {
  return primable_n(store, "obj", [&] (int64_t n) {
    return Variant(SystemLib::AllocStdClassObject());
  });
}

/*
 * Just an empty table.
 */
std::unique_ptr<Store> new_store() {
  return folly::make_unique<Store>();
}

/*
 * Make an APC table with some things primed for tests to use.
 */
std::unique_ptr<Store> new_primed_store() {
  s_apc_file_storage.enable("/tmp/apc_unit_test", 1ul << 20, 1ul << 32);

  auto ret = folly::make_unique<Store>();
  ret->prime(primable_ints(*ret));
  ret->prime(primable_objs(*ret));
  ret->primeDone();
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

TEST(APC, Basic) {
  auto store = new_store();

  const StaticString s_key("key");
  const StaticString s_value1("value1");

  EXPECT_EQ(store->add(s_key, Variant(s_value1), 1500), true);
  EXPECT_EQ(store->exists(s_key), true);
}

TEST(APC, BasicPrimeStuff) {
  auto store = new_primed_store();
  Variant val;

  EXPECT_TRUE(store->get("int_2", val));
  EXPECT_TRUE(cellSame(*val.asCell(), make_tv<KindOfInt64>(2)));

  bool found = false;
  EXPECT_EQ(store->inc("int_3", 1, found), 4);
  EXPECT_TRUE(found);
  EXPECT_FALSE(store->get("int_200", val));

  EXPECT_EQ(store->cas("obj_1", 1, 2), true); // stdclass converts to 1
  EXPECT_EQ(store->cas("obj_2", 4, 5), false);
  EXPECT_EQ(store->cas("int_4", 4, 5), true);
  EXPECT_EQ(store->cas("int_5", 4, 5), false);
}

//////////////////////////////////////////////////////////////////////

}
