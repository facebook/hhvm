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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/ast/visitor.h>

using namespace apache::thrift::compiler;

namespace {

class InterleavedVisitorTest : public testing::Test {};

} // namespace

TEST_F(InterleavedVisitorTest, mixed) {
  struct tracking_visitor : visitor {
    tracking_visitor(bool ret) : ret_(ret) {}
    bool visit(t_program*) override { return ret_; }
    bool visit(t_service* const service) override {
      visited_services.push_back(service);
      return true;
    }
    std::vector<const t_service*> visited_services;
    bool ret_;
  };

  auto tv_f = tracking_visitor(false);
  auto tv_t = tracking_visitor(true);
  auto vtor = interleaved_visitor({&tv_f, &tv_t});

  auto program = t_program("path/to/module.thrift");
  auto service = std::make_unique<t_service>(&program, "MyService");
  auto service_ptr = service.get();
  program.add_service(std::move(service));
  vtor.traverse(&program);
  EXPECT_THAT(tv_f.visited_services, testing::ElementsAre());
  EXPECT_THAT(tv_t.visited_services, testing::ElementsAre(service_ptr));
}
