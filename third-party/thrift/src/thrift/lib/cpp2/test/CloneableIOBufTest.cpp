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

#include <thrift/lib/cpp2/CloneableIOBuf.h>

#include <gtest/gtest.h>

using namespace folly;
using namespace apache::thrift;

TEST(CloneableIOBufTest, UniquePtrCompatibility) {
  CloneableIOBuf ptr(IOBuf::copyBuffer("Test string"));
  EXPECT_NE(nullptr, ptr.get());
  CloneableIOBuf ptr2(std::move(ptr));
  EXPECT_EQ(nullptr, ptr.get());
  EXPECT_NE(nullptr, ptr2.get());
  swap(ptr, ptr2);
  EXPECT_NE(nullptr, ptr.get());
  EXPECT_EQ(nullptr, ptr2.get());
}

TEST(CloneableIOBufTest, CopyConstructorAndAssignment) {
  const std::string s = "Test string";
  const std::string s2 = "Foo bar";
  CloneableIOBuf ptr(IOBuf::wrapBuffer(s.data(), s.size()));
  CloneableIOBuf ptr2(ptr);
  CloneableIOBuf ptr3(IOBuf::wrapBuffer(s2.data(), s2.size()));
  EXPECT_NE(nullptr, ptr.get());
  EXPECT_NE(nullptr, ptr2.get());
  EXPECT_NE(nullptr, ptr3.get());
  EXPECT_NE(ptr.get(), ptr2.get());
  EXPECT_TRUE(ptr->isShared());
  EXPECT_EQ(ptr->buffer(), ptr2->buffer());
  EXPECT_NE(ptr->buffer(), ptr3->buffer());

  ptr2 = ptr3;
  EXPECT_NE(nullptr, ptr.get());
  EXPECT_NE(nullptr, ptr2.get());
  EXPECT_NE(nullptr, ptr3.get());
  EXPECT_NE(ptr.get(), ptr2.get());
  EXPECT_NE(ptr2.get(), ptr3.get());
  EXPECT_NE(ptr->buffer(), ptr2->buffer());
  EXPECT_EQ(ptr2->buffer(), ptr3->buffer());
}

TEST(CloneableIOBufTest, Swap) {
  const std::string s = "Test string";
  const std::string s2 = "Foo bar";
  CloneableIOBuf ptr(IOBuf::wrapBuffer(s.data(), s.size()));
  CloneableIOBuf ptr2(IOBuf::wrapBuffer(s2.data(), s2.size()));
  EXPECT_NE(nullptr, ptr.get());
  EXPECT_NE(nullptr, ptr2.get());
  EXPECT_NE(ptr.get(), ptr2.get());
  EXPECT_NE(ptr->buffer(), ptr2->buffer());
  EXPECT_STREQ(s.c_str(), (const char*)ptr->data());
  EXPECT_STREQ(s2.c_str(), (const char*)ptr2->data());

  swap(ptr, ptr2);
  EXPECT_NE(nullptr, ptr.get());
  EXPECT_NE(nullptr, ptr2.get());
  EXPECT_NE(ptr.get(), ptr2.get());
  EXPECT_NE(ptr->buffer(), ptr2->buffer());
  EXPECT_STREQ(s2.c_str(), (const char*)ptr->data());
  EXPECT_STREQ(s.c_str(), (const char*)ptr2->data());
}

TEST(CloneableIOBufTest, NullPtr) {
  const std::string s = "Test string";
  CloneableIOBuf ptr(IOBuf::wrapBuffer(s.data(), s.size()));
  CloneableIOBuf ptr2;
  EXPECT_NE(nullptr, ptr.get());
  EXPECT_EQ(nullptr, ptr2.get());
  CloneableIOBuf ptr3(ptr2);
  EXPECT_EQ(nullptr, ptr3.get());
  ptr = ptr2;
  EXPECT_EQ(nullptr, ptr.get());
}
