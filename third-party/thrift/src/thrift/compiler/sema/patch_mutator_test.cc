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

#include <memory>
#include <thrift/compiler/sema/patch_mutator.h>

#include <folly/portability/GTest.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/sema/diagnostic_context.h>

namespace apache::thrift::compiler {

class PatchGeneratorTest : public ::testing::Test {
 public:
  PatchGeneratorTest() : ctx_(source_mgr_, results_) {}

  void SetUp() override {
    ctx_.begin_visit(program_);
    gen_ = std::make_unique<patch_generator>(ctx_, program_);
  }

  // Give tests access to this funciton.
  template <typename... Args>
  std::string prefix_uri_name(Args&&... args) {
    return patch_generator::prefix_uri_name(std::forward<Args>(args)...);
  }

 protected:
  t_program program_{"path/to/file.thrift"};
  source_manager source_mgr_;
  diagnostic_results results_;
  diagnostic_context ctx_;
  std::unique_ptr<patch_generator> gen_;
};

namespace {

TEST_F(PatchGeneratorTest, Empty) {
  // We do not error when the patch types cannot be found.
  EXPECT_FALSE(results_.has_error());
  // Failures/warnings are produced lazily.
  EXPECT_EQ(results_.count(diagnostic_level::warning), 0);
}

TEST_F(PatchGeneratorTest, PrefixUriName) {
  EXPECT_EQ(prefix_uri_name("", "Foo"), "");
  EXPECT_EQ(prefix_uri_name("Bar", "Foo"), "");
  EXPECT_EQ(prefix_uri_name("Baz/Bar", "Foo"), "Baz/FooBar");
  EXPECT_EQ(prefix_uri_name("Buk/Baz/Bar", "Foo"), "Buk/Baz/FooBar");
}

} // namespace
} // namespace apache::thrift::compiler
