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

#include <type_traits>

namespace whisker::detail {

// std::remove_cvref_t was added in C++20
template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T, template <typename...> typename Template>
constexpr inline bool is_specialization_v = false;
template <template <typename...> typename Template, typename... Types>
constexpr inline bool is_specialization_v<Template<Types...>, Template> = true;

template <typename T, template <typename...> typename Template>
concept specialization = is_specialization_v<T, Template>;

} // namespace whisker::detail
