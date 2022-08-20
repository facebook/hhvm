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

#include <thrift/lib/cpp2/reflection/demo/json_print.h>

#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_constants.h>
#include <thrift/lib/cpp2/reflection/demo/gen-cpp2/data_fatal_types.h>

int main() {
  std::cerr << "example 1:\n";
  print(static_reflection::demo::data_constants::example_1());

  std::cerr << "example 2:\n";
  print(static_reflection::demo::data_constants::example_2());

  std::cerr << "example 3:\n";
  print(static_reflection::demo::data_constants::example_3());

  return 0;
}
