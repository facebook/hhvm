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

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/compiler/detail/mustache/mstch.h>

using namespace apache::thrift;

mstch::node test_data(size_t count) {
  mstch::array a;
  a.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    a.push_back(mstch::map{{"index", std::to_string(i)}});
  }
  return mstch::map{{"indexes", a}};
}

BENCHMARK(mstch_render_section_tiny) {
  mstch::node data = test_data(1000);
  mstch::render("indexes:\n{{#indexes}}* {{index}}\n{{/indexes}}", data);
}

BENCHMARK(mstch_render_section_small) {
  mstch::node data = test_data(10000);
  mstch::render("indexes:\n{{#indexes}}* {{index}}\n{{/indexes}}", data);
}

BENCHMARK(mstch_render_section_large) {
  mstch::node data = test_data(100000);
  mstch::render("indexes:\n{{#indexes}}* {{index}}\n{{/indexes}}", data);
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
}
