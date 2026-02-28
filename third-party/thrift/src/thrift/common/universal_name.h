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
#include <string_view>
#include <vector>

namespace apache::thrift::detail {

/**
 * Checks that the given (split) labels form a valid Thrift URI domain.
 *
 * @throws std::invalid_argument on failure
 */
void check_univeral_name_domain(const std::vector<std::string>& domain);

/**
 * Checks that the given (split) segments form a valid Thrift URI path.
 *
 * @throws std::invalid_argument on failure
 */
void check_universal_name_path(const std::vector<std::string>& path);

} // namespace apache::thrift::detail

namespace apache::thrift {
/**
 * Validates that uri is a valid Thrift universal name (aka Thrift URI) of the
 * form: {domain}/{path}/{type}.
 *
 * For example: facebook.com/thrift/Value.
 *
 * The URI scheme "fbthrift://"" is implied and must not be included in the uri.
 *
 * @throws std::invalid_argument on failure.
 */
void validate_universal_name(std::string_view uri);

} // namespace apache::thrift
