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

#pragma once

#include <initializer_list>
#include <string>

namespace apache {
namespace thrift {
namespace compiler {
namespace gen {
namespace cpp {
namespace detail {

template <typename C, typename K = typename C::key_type, typename G>
auto& get_or_gen(C& cache, const K& key, const G& gen_func) {
  auto itr = cache.find(key);
  if (itr == cache.end()) {
    itr = cache.emplace(key, gen_func()).first;
  }
  return itr->second;
}

std::string gen_template_type(
    std::string template_name, std::initializer_list<std::string> args);

} // namespace detail
} // namespace cpp
} // namespace gen
} // namespace compiler
} // namespace thrift
} // namespace apache
