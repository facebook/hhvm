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

#include <array>
#include <cstdint>
#include <string_view>

#include <folly/CPortability.h>
#include <folly/Indestructible.h>
#include <folly/Range.h>
#include <folly/lang/Exception.h>

FOLLY_GNU_DISABLE_WARNING("-Woverlength-strings")
FOLLY_GNU_DISABLE_WARNING("-Wtrigraphs")

namespace apache::thrift::detail::mc {

::std::string_view readSchema(::std::string_view (*access)());

::std::string_view readSchemaInclude(
    ::folly::Range<const ::std::string_view*> (*access)(), ::std::size_t index);

} // namespace apache::thrift::detail::mc
