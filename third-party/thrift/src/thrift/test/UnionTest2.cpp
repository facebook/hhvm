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

#include <thrift/lib/cpp/util/EnumUtils.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/test/gen-cpp2/UnionTest2_types.h>
#include <thrift/test/gen-cpp2/UnionTest3_types.h>

#include <gtest/gtest.h>

using namespace thrift::test::debug::cpp2;
using namespace apache::thrift;
using namespace apache::thrift::util;
using namespace testing;

using apache::thrift::TEnumTraits;

template <class UnionType>
UnionType serializeDeserialize(const UnionType& val) {
  BinaryProtocolWriter prot;
  size_t bufSize = val.serializedSize(&prot);
  IOBufQueue queue(IOBufQueue::cacheChainLength());

  prot.setOutput(&queue, bufSize);
  val.write(&prot);

  bufSize = queue.chainLength();
  auto buf = queue.move();

  UnionType out;
  BinaryProtocolReader protReader;
  protReader.setInput(buf.get());
  out.read(&protReader);

  return out;
}

template <class UnionType>
void testSerializeDeserialize(const UnionType& val) {
  EXPECT_EQ(val, serializeDeserialize(val));
}

class UnionTestFixture : public Test {
 public:
  void serializeDeserialize(TestUnion& val) { testSerializeDeserialize(val); }
};

class TerseUnionTestFixture : public Test {
 public:
  void serializeDeserialize(TerseTestUnion& val) {
    testSerializeDeserialize(val);
  }
};

TEST_F(UnionTestFixture, Constructors) {
  auto f = [](const TestUnion& u) {
    EXPECT_EQ(TestUnion::Type::i32_field, u.getType());
    EXPECT_EQ(100, u.get_i32_field());
  };

  TestUnion u;
  u.set_i32_field(100);
  f(u);

  auto v1(u);
  f(v1);

  auto v2 = u;
  f(v2);

  auto v3(std::move(u));
  f(v3);

  auto v4 = std::move(v2);
  f(v4);
}

TEST_F(UnionTestFixture, ChangeType) {
  TestUnion u;
  u.set_i32_field(100);
  EXPECT_EQ(TestUnion::Type::i32_field, u.getType());
  EXPECT_EQ(100, u.get_i32_field());

  CHECK_EQ(TestUnion::Type::i32_field, u.getType());
  EXPECT_DEATH(
      CHECK_EQ(TestUnion::Type::other_i32_field, u.getType()),
      "other_i32_field vs. i32_field");
  if (folly::kIsDebug) {
    EXPECT_DEATH(
        DCHECK_EQ(TestUnion::Type::other_i32_field, u.getType()),
        "other_i32_field vs. i32_field");
  } else {
    DCHECK_EQ(TestUnion::Type::other_i32_field, u.getType());
  }

  CHECK_NE(TestUnion::Type::other_i32_field, u.getType());
  EXPECT_DEATH(
      CHECK_NE(TestUnion::Type::i32_field, u.getType()),
      "i32_field vs. i32_field");
  if (folly::kIsDebug) {
    EXPECT_DEATH(
        DCHECK_NE(TestUnion::Type::i32_field, u.getType()),
        "i32_field vs. i32_field");
  } else {
    DCHECK_NE(TestUnion::Type::i32_field, u.getType());
  }

  u.set_other_i32_field(200);
  EXPECT_EQ(TestUnion::Type::other_i32_field, u.getType());
  EXPECT_EQ(200, u.get_other_i32_field());

  u.set_string_field("str");
  EXPECT_EQ(TestUnion::Type::string_field, u.getType());
  EXPECT_EQ("str", u.get_string_field());

  u.set_struct_list(std::vector<RandomStuff>());
  EXPECT_EQ(TestUnion::Type::struct_list, u.getType());
  EXPECT_EQ(std::vector<RandomStuff>(), u.get_struct_list());

  u.set_ref_field(OneOfEach());
  EXPECT_EQ(TestUnion::Type::ref_field, u.getType());
  EXPECT_EQ(OneOfEach(), *u.get_ref_field());
}

TEST_F(UnionTestFixture, SerdeTest) {
  TestUnion u;
  serializeDeserialize(u);

  u.set_i32_field(100);
  serializeDeserialize(u);

  u.set_other_i32_field(200);
  serializeDeserialize(u);

  u.set_string_field("str");
  serializeDeserialize(u);

  u.set_struct_list(std::vector<RandomStuff>());
  serializeDeserialize(u);
}

TEST_F(UnionTestFixture, TypeEnumTest) {
  TestUnion u;
  EXPECT_EQ(TEnumTraits<TestUnion::Type>::size, 6);
  EXPECT_EQ(
      enumName(TestUnion::Type::string_field), std::string{"string_field"});
  EXPECT_EQ(
      enumName(TestUnion::Type::struct_field), std::string{"struct_field"});
  EXPECT_EQ(enumName(TestUnion::Type::struct_list), std::string{"struct_list"});
  EXPECT_EQ(enumName(TestUnion::Type::ref_field), std::string{"ref_field"});
  EXPECT_EQ(enumName(TestUnion::Type::__EMPTY__), (const char*)nullptr);
  EXPECT_EQ(enumName(static_cast<TestUnion::Type>(-10)), (const char*)nullptr);
  TestUnion::Type t;
  EXPECT_TRUE(tryParseEnum("string_field", &t));
  EXPECT_EQ((int)t, 1);
  EXPECT_TRUE(tryParseEnum("other_i32_field", &t));
  EXPECT_EQ((int)t, 5);

  EXPECT_FALSE(tryParseEnum("__EMPTY__", &t));
  EXPECT_FALSE(tryParseEnum("foo_field", &t));
  EXPECT_FALSE(tryParseEnum("bar_field", &t));
}

TEST_F(TerseUnionTestFixture, SerializeDeserializeTest) {
  TerseTestUnion u;
  serializeDeserialize(u);

  I32Stuff i32St;
  *i32St.a() = 100;
  u.set_i32_field(i32St);
  serializeDeserialize(u);

  StringStuff stringSt;
  *stringSt.a() = "str";
  u.set_string_field(stringSt);
  serializeDeserialize(u);
}

TEST(NonCopyableUnion, Simple) {
  NonCopyableUnion a;
  a.set_a(42);
  auto b = serializeDeserialize(a);
  EXPECT_TRUE(b.getType() == NonCopyableUnion::Type::a);
  EXPECT_EQ(42, b.get_a());

  const char buf[] = "hello world";
  a.set_buf(folly::IOBuf(folly::IOBuf::COPY_BUFFER, buf, sizeof(buf)));
  b = serializeDeserialize(a);
  EXPECT_TRUE(b.getType() == NonCopyableUnion::Type::buf);
  auto& iob = b.get_buf();
  EXPECT_EQ(sizeof(buf), iob.length());
  EXPECT_EQ(0, memcmp(buf, iob.data(), sizeof(buf)));
}

TEST(NonCopyableUnion, NoncopyableMember) {
  NonCopyableStruct s;
  *s.num() = 42;
  NonCopyableUnion u;
  u.set_ncs(std::move(s));
  EXPECT_EQ(*u.get_ncs().num(), 42);
}

TEST(NoExceptMoveUnion, Constructor) {
  EXPECT_TRUE(std::is_nothrow_move_constructible<NoExceptMoveUnion>::value);
  NoExceptMoveUnion u1;
  u1.set_string_field("hello world");
  NoExceptMoveUnion u2(std::move(u1));
  EXPECT_EQ(u2.get_string_field(), "hello world");
}

TEST(NoExceptMoveUnion, MoveOperator) {
  EXPECT_TRUE(std::is_nothrow_move_assignable<NoExceptMoveUnion>::value);
  NoExceptMoveUnion u1;
  u1.set_string_field("hello world");
  NoExceptMoveUnion u2;
  u2 = std::move(u1);
  EXPECT_EQ(u2.get_string_field(), "hello world");
}

TEST(CppRefContainers, Simple) {
  CppRefContainers v;
  CppRefContainers v1;
  CppRefContainers v2;
  v1.set_data("v1");
  v2.set_data("v2");
  v.set_values({v1, v2});

  EXPECT_EQ(v.getType(), CppRefContainers::Type::values);
  EXPECT_EQ((*v.get_values())[0].get_data(), "v1");
  EXPECT_EQ((*v.get_values())[1].get_data(), "v2");
}

TEST(Empty, Simple) {
  TestUnion u;
  EXPECT_TRUE(apache::thrift::empty(u));
  u.i32_field() = 0;
  EXPECT_FALSE(apache::thrift::empty(u));
  u = TestUnion{};
  EXPECT_TRUE(apache::thrift::empty(u));
  u.string_field() = "";
  EXPECT_FALSE(apache::thrift::empty(u));
  apache::thrift::clear(u);
  EXPECT_TRUE(apache::thrift::empty(u));
}
