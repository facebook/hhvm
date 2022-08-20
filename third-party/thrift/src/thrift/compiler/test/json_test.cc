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

#include <thrift/compiler/generate/json.h>

#include <sstream>
#include <folly/portability/GTest.h>

using namespace apache::thrift::compiler;

TEST(JsonTest, json_quote_ascii_string) {
  auto actual = json_quote_ascii("the\bquick\"brown\nfox\001jumps\201over");
  EXPECT_EQ("\"the\\bquick\\\"brown\\nfox\\u0001jumps\\u0081over\"", actual);
}

TEST(JsonTest, json_quote_ascii_stream) {
  std::ostringstream actual;
  json_quote_ascii(actual, "the\bquick\"brown\nfox\001jumps\201over");
  EXPECT_EQ(
      "\"the\\bquick\\\"brown\\nfox\\u0001jumps\\u0081over\"", actual.str());
}
