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

#include <string>
#include <boost/algorithm/string.hpp>

namespace apache::thrift::compiler {

inline bool is_reserved_identifier_name(std::string_view name) {
  const std::string_view prefix = "fbthrift";

  auto pos = name.find_first_not_of("_");
  if (pos == std::string_view::npos) {
    return false;
  }

  auto after_underscores =
      std::string_view(name.data() + pos, name.size() - pos);

  return boost::algorithm::istarts_with(
      after_underscores, prefix, std::locale::classic());
}

} // namespace apache::thrift::compiler
