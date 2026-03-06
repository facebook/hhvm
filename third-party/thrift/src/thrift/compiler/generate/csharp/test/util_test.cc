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

#include <gtest/gtest.h>
#include <thrift/compiler/generate/csharp/util.h>

namespace apache::thrift::compiler::csharp {

// === get_csharp_property_name ===

TEST(CSharpUtilTest, PropertyName_PassesThrough) {
  // Property names pass through unchanged — CS0542 collisions are
  // banned at the IDL level rather than silently mangled.
  EXPECT_EQ(get_csharp_property_name("my_field"), "my_field");
  EXPECT_EQ(get_csharp_property_name("id"), "id");
  EXPECT_EQ(get_csharp_property_name("class"), "class");
  EXPECT_EQ(get_csharp_property_name("namespace"), "namespace");
  EXPECT_EQ(get_csharp_property_name("path"), "path");
}

// === quote_csharp_string ===

TEST(CSharpUtilTest, QuoteString_Basic) {
  EXPECT_EQ(quote_csharp_string("hello"), "\"hello\"");
  EXPECT_EQ(quote_csharp_string(""), "\"\"");
}

TEST(CSharpUtilTest, QuoteString_EscapesSpecialChars) {
  EXPECT_EQ(quote_csharp_string("a\"b"), "\"a\\\"b\"");
  EXPECT_EQ(quote_csharp_string("a\\b"), "\"a\\\\b\"");
  EXPECT_EQ(quote_csharp_string("a\nb"), "\"a\\nb\"");
  EXPECT_EQ(quote_csharp_string("a\tb"), "\"a\\tb\"");
  EXPECT_EQ(quote_csharp_string("a\rb"), "\"a\\rb\"");
}

TEST(CSharpUtilTest, QuoteString_Utf8PassThrough) {
  // UTF-8 bytes for "é" (U+00E9) should pass through unchanged
  // since C# source files are UTF-8 encoded
  EXPECT_EQ(quote_csharp_string("café"), "\"café\"");
  EXPECT_EQ(quote_csharp_string("日本語"), "\"日本語\"");

  // Control characters should still be escaped
  EXPECT_EQ(escape_csharp_string("\x01"), "\\u0001");
  EXPECT_EQ(escape_csharp_string("\x1f"), "\\u001f");
}

} // namespace apache::thrift::compiler::csharp
