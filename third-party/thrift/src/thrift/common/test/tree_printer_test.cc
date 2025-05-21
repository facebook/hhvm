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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/common/tree_printer.h>

namespace apache::thrift {

TEST(TreePrinterTest, basic) {
  tree_printer::scope root = tree_printer::scope::make_root("encyclopedia");
  {
    tree_printer::scope& culture = root.make_child("culture");
    culture.make_child("art");
    culture.make_child("craft");
  }
  {
    tree_printer::scope& science = root.make_child("science");
    science.make_child("physics");
    science.make_child("chemistry");
  }
  EXPECT_EQ(
      tree_printer::to_string(root),
      "encyclopedia\n"
      "├─ culture\n"
      "│  ├─ art\n"
      "│  ╰─ craft\n"
      "╰─ science\n"
      "   ├─ physics\n"
      "   ╰─ chemistry\n");
}

} // namespace apache::thrift
