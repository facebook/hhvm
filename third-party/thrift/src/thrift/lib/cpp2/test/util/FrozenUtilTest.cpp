/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/File.h>
#include <folly/system/MemoryMapping.h>

#include <thrift/lib/cpp/util/FrozenTestUtil.h>
#include <thrift/lib/cpp/util/FrozenUtil.h>
#include <thrift/lib/cpp2/test/gen-cpp2/FrozenTypes_types.h>

#include <gtest/gtest.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::util;
using folly::File;
using folly::MemoryMapping;

namespace {

double filesize(int fd) {
  struct stat st;
  fstat(fd, &st);
  return st.st_size;
}

} // namespace

TEST(FrozenUtilTest, Set) {
  std::set<std::string> tset{"1", "3", "7", "5"};
  auto tempFrozen = freezeToTempFile(tset);
  MemoryMapping mapping(tempFrozen.fd());

  auto* pfset = mapFrozen<std::set<std::string>>(mapping);
  auto& fset = *pfset;
  EXPECT_EQ(1, fset.count("3"));
  EXPECT_EQ(0, fset.count("4"));
}

TEST(FrozenUtilTest, Vector) {
  std::vector<Person> people(3);
  *people[0].id() = 300;
  *people[1].id() = 301;
  *people[2].id() = 302;

  auto tempFrozen = freezeToTempFile(people);
  MemoryMapping mapping(tempFrozen.fd());

  auto* pfvect = mapFrozen<std::vector<Person>>(mapping);
  auto& fvect = *pfvect;
  EXPECT_EQ(300, fvect[0].id);
  EXPECT_EQ(302, fvect[2].id);
}

TEST(FrozenUtilTest, Shrink) {
  std::vector<Person> people(3);

  File f = File::temporary();

  size_t count = 1 << 16;
  for (size_t i = 0; i < count; ++i) {
    people.emplace_back();
    *people.back().id() = i + count;
  }

  freezeToFile(people, f.fd());
  EXPECT_NEAR(
      sizeof(Frozen<Person>) * count,
      filesize(f.fd()),
      sizeof(Frozen<Person>) * count);

  count /= 16;
  people.resize(count);

  freezeToFile(people, f.fd());
  EXPECT_NEAR(
      sizeof(Frozen<Person>) * count,
      filesize(f.fd()),
      sizeof(Frozen<Person>) * count);
}

TEST(FrozenUtilTest, Sparse) {
  std::vector<Person> people;

  size_t count = 1 << 20;
  for (size_t i = 0; i < count; ++i) {
    people.emplace_back();
    *people.back().id() = i + count;
  }

  File f = File::temporary();

  freezeToSparseFile(people, folly::File(f.fd()));

  EXPECT_NEAR(
      sizeof(Frozen<Person>) * count,
      filesize(f.fd()),
      sizeof(Frozen<Person>) * count);

  MemoryMapping mapping(f.fd());
  auto* pfvect = mapFrozen<std::vector<Person>>(mapping);
  auto& fvect = *pfvect;
  EXPECT_EQ(*people[100].id(), fvect[100].id);
  EXPECT_EQ(*people[9876].id(), fvect[9876].id);
}

TEST(FrozenUtilTest, KeepMapped) {
  Person p;
  *p.nums() = {9, 8, 7};
  *p.id() = 123;
  *p.name() = "Tom";

  File f = File::temporary();
  MemoryMapping mapping(
      folly::File(f.fd()), 0, frozenSize(p), MemoryMapping::writable());

  // also returns mapped addr
  auto* pfp = freezeToFile(p, mapping);
  auto& fp = *pfp;

  EXPECT_EQ(123, fp.id);
  EXPECT_EQ(1, fp.nums.count(8));
  EXPECT_EQ(3, fp.nums.size());
  EXPECT_EQ("Tom", fp.name.range());
}
