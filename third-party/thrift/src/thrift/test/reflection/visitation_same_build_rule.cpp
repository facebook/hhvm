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

#include <thrift/test/reflection/gen-cpp2/visitation_same_build_rule_for_each_field.h> // @manual=
#include <thrift/test/reflection/visitation_same_build_rule.h>

namespace apache::thrift::test {
std::vector<std::string> names() {
  std::vector<std::string> ret;
  for_each_field(TestStruct{}, [&](auto&& meta, auto&&) {
    ret.push_back(*meta.name_ref());
  });
  return ret;
}
} // namespace apache::thrift::test
