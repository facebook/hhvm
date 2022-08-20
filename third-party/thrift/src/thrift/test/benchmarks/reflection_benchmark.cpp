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

#include <string_view>

#include <glog/logging.h>
#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/lang/Pretty.h>
#include <thrift/test/testset/gen-cpp2/testset_fatal_all.h>
#include <thrift/test/testset/gen-cpp2/testset_for_each_field.h>

namespace apache::thrift::test {
namespace {

template <class Struct>
void add_benchmark() {
  if constexpr (!is_thrift_union_v<Struct>) {
    static_assert(
        fatal::size<typename reflect_struct<Struct>::members>::value == 1);
  } else {
    static_assert(
        fatal::size<
            typename reflect_variant<Struct>::traits::descriptors>::value == 2);
  }

  std::string name = folly::pretty_name<Struct>();
  name = name.substr(name.rfind("::") + 2);

  if constexpr (!is_thrift_union_v<Struct>) {
    static Struct s;
    folly::addBenchmark(__FILE__, name + "_value_reflection", [] {
      fatal::foreach<typename reflect_struct<Struct>::members>([](auto tag) {
        using Member = decltype(fatal::tag_type(tag));
        typename Member::field_ref_getter()(s) = "a";
        folly::doNotOptimizeAway(s);
      });
      return 1;
    });
  }

  {
    static Struct s;
    folly::addBenchmark(__FILE__, name + "_value_visitation", [] {
      for_each_field(s, [](auto&&, auto ref) {
        ref = "a";
        folly::doNotOptimizeAway(ref);
      });
      return 1;
    });
  }

  {
    static Struct s;
    folly::addBenchmark(__FILE__, name + "_value_baseline", [] {
      s.field_1_ref() = "a";
      folly::doNotOptimizeAway(s);
      return 1;
    });
  }

  if constexpr (!is_thrift_union_v<Struct>) {
    folly::addBenchmark(__FILE__, name + "_name_reflection", [] {
      int k = 0;
      fatal::foreach<typename reflect_struct<Struct>::members>([&k](auto tag) {
        using Member = decltype(fatal::tag_type(tag));
        const char* name = fatal::z_data<typename Member::name>();
        for (char c : std::string_view(name)) {
          k += c;
        }
      });
      folly::doNotOptimizeAway(k);
      return 1;
    });
  }

  {
    static Struct s;
    folly::addBenchmark(__FILE__, name + "_name_visitation", [] {
      int k = 0;
      for_each_field(s, [&k](auto&& meta, auto&&) {
        for (char c : *meta.name_ref()) {
          k += c;
        }
      });
      folly::doNotOptimizeAway(k);
      return 1;
    });
  }

  folly::addBenchmark(__FILE__, name + "_name_baseline", [] {
    int k = 0;
    for (char c : "field_1") {
      k += c;
    }
    folly::doNotOptimizeAway(k);
    return 1;
  });
}
} // namespace

void addReflectionBenchmarks() {
  add_benchmark<testset::struct_string>();
  add_benchmark<testset::struct_optional_string>();
  add_benchmark<testset::struct_required_string>();
  add_benchmark<testset::union_string>();
}

} // namespace apache::thrift::test

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  apache::thrift::test::addReflectionBenchmarks();
  folly::runBenchmarks();
  return 0;
}
