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

#include <filesystem>
#include <string_view>
#include <thrift/compiler/generate/templates.h>

namespace apache::thrift::compiler {

namespace {

bool is_last_char(std::string_view data, char c) {
  return !data.empty() && data.back() == c;
}
void chomp_last_char(std::string* data, char c) {
  if (is_last_char(*data, c)) {
    data->pop_back();
  }
}

} // namespace

templates_map create_templates_by_path() {
  templates_map result;
  for (std::size_t i = 0; i < detail::templates_size; ++i) {
    auto name = std::filesystem::path(
        detail::templates_name_datas[i],
        detail::templates_name_datas[i] + detail::templates_name_sizes[i]);
    name = name.parent_path() / name.stem();

    auto tpl = std::string(
        detail::templates_content_datas[i],
        detail::templates_content_datas[i] +
            detail::templates_content_sizes[i]);
    // Remove a single '\n' or '\r\n' or '\r' at end, if present.
    chomp_last_char(&tpl, '\n');
    chomp_last_char(&tpl, '\r');
    result.emplace(name.generic_string(), std::move(tpl));
  }
  return result;
}

} // namespace apache::thrift::compiler
