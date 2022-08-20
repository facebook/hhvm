/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <thrift/lib/cpp2/reflection/indenter.h>

#include <sstream>
#include <string>

#include <folly/portability/GTest.h>

TEST(fatal_indenter, arbitrary_push_depth) {
  std::ostringstream out;
  auto indenter = apache::thrift::make_indenter(out, "  ");

  std::string expected =
      "  1.2\n"
      "    2.2\n"
      "    3.2\n"
      "        4.4\n"
      "      5.3\n"
      "6.0";

  indenter.push();
  indenter << "1.2";
  indenter.newline();
  indenter.push();
  indenter << "2.2";
  indenter.newline();
  indenter << "3.2";
  indenter.newline();
  indenter.push(2);
  indenter << "4.4";
  indenter.newline();
  indenter.pop();
  indenter << "5.3";
  indenter.newline();
  indenter.pop(3);
  indenter << "6.0";

  auto actual = out.str();
  EXPECT_EQ(expected, actual);
}

TEST(fatal_indenter, set_margin) {
  std::ostringstream out;
  auto indenter = apache::thrift::make_indenter(out, "  ");

  std::string expected =
      "a  1.2\n"
      "a    2.2\n"
      "b    3.2\n"
      "         4.4\n"
      "      5.3\n"
      "c6.0";

  indenter.push();
  indenter.set_margin("a");
  indenter << "1.2";
  indenter.newline();
  indenter.push();
  indenter << "2.2";
  indenter.newline();
  indenter.set_margin("b");
  indenter << "3.2";
  indenter.newline();
  indenter.push(2);
  indenter.set_margin(" ");
  indenter << "4.4";
  indenter.newline();
  indenter.pop();
  indenter.set_margin("");
  indenter << "5.3";
  indenter.newline();
  indenter.pop(3);
  indenter.set_margin("c");
  indenter << "6.0";

  auto actual = out.str();
  EXPECT_EQ(expected, actual);
}
