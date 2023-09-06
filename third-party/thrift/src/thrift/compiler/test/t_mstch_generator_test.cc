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

#include <string>

#include <folly/portability/GTest.h>

#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/generate/t_mstch_generator.h>

namespace apache {
namespace thrift {
namespace compiler {

TEST(t_mstch_generator_test, cache_leaks) {
  class leaky_program : public mstch_program {
   public:
    leaky_program(
        const t_program* program,
        mstch_context& ctx,
        mstch_element_position pos,
        int* object_count)
        : mstch_program(program, ctx, pos), object_count_(object_count) {
      ++*object_count_;
    }
    virtual ~leaky_program() override { --*object_count_; }

   private:
    int* object_count_;
  };

  class leaky_generator : public t_mstch_generator {
   public:
    leaky_generator(t_program& program, int* object_count, t_program_bundle& pb)
        : t_mstch_generator(program, source_mgr_, pb),
          object_count_(object_count) {}

    std::string template_prefix() const override { return "."; }

    void generate_program() override {
      mstch_context_.add<leaky_program>(object_count_);
      std::shared_ptr<mstch_base> prog = cached_program(get_program());
    }

   private:
    source_manager source_mgr_;
    int* object_count_;
  };

  int object_count = 0;
  {
    t_program_bundle pb(std::make_unique<t_program>("my_leak.thrift"));
    leaky_generator generator(*pb.get_root_program(), &object_count, pb);
    generator.generate_program();
  }

  EXPECT_EQ(object_count, 0);
}

} // namespace compiler
} // namespace thrift
} // namespace apache
