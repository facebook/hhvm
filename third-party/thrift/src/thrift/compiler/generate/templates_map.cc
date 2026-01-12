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
#include <thrift/compiler/generate/templates.h>

namespace apache::thrift::compiler {

templates_map create_templates_by_path() {
  templates_map result;
  for (std::size_t i = 0; i < detail::templates_size; ++i) {
    std::filesystem::path name(
        detail::templates_name_datas[i],
        detail::templates_name_datas[i] + detail::templates_name_sizes[i]);
    name = name.parent_path() / name.stem();

    std::string tpl(
        detail::templates_content_datas[i],
        detail::templates_content_datas[i] +
            detail::templates_content_sizes[i]);
    // Remove a single '\n' or '\r\n' or '\r' at end, if present.
    if (tpl.ends_with('\n')) {
      tpl.pop_back();
    }
    if (tpl.ends_with('\r')) {
      tpl.pop_back();
    }
    result.emplace(name.generic_string(), std::move(tpl));
  }
  return result;
}

} // namespace apache::thrift::compiler
