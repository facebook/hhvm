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

#include <cstdint>

#include <thrift/lib/cpp2/protocol/Object.h>

namespace apache::thrift::test::utils {

// ----- Access 'all' data within a thrift hierarchy ---- //

std::size_t read_all(const bool& b);
std::size_t read_all(const std::int8_t& i);
std::size_t read_all(const std::int16_t& i);
std::size_t read_all(const std::int32_t& i);
std::size_t read_all(const std::int64_t& i);
std::size_t read_all(const float& f);
std::size_t read_all(const double& d);
std::size_t read_all(const std::string& s);
std::size_t read_all(const folly::IOBuf& b);
std::size_t read_all(const std::vector<protocol::detail::Value>& l);
std::size_t read_all(const folly::F14FastSet<protocol::detail::Value>& s);
std::size_t read_all(
    const folly::F14FastMap<protocol::detail::Value, protocol::detail::Value>&
        m);
std::size_t read_all(const ::apache::thrift::protocol::Object& obj);
std::size_t read_all(const ::apache::thrift::protocol::detail::Value& val);

// ----- Access a sparse subset of data within a thrift hierarchy ---- //

enum class SparseAccess {
  // Reads every other property
  Half,
  // Reads a random property at every level.
  // Note: Always reads the first field for a struct
  SingleRandom,
};

std::size_t read_some(
    SparseAccess access,
    const std::vector<::apache::thrift::protocol::detail::Value>& l);
std::size_t read_some(
    SparseAccess access,
    const folly::F14FastSet<::apache::thrift::protocol::detail::Value>& s);
std::size_t read_some(
    SparseAccess access,
    const folly::F14FastMap<
        ::apache::thrift::protocol::detail::Value,
        ::apache::thrift::protocol::detail::Value>& m);
std::size_t read_some(
    SparseAccess access, const ::apache::thrift::protocol::Object& obj);
std::size_t read_some(
    SparseAccess access, const ::apache::thrift::protocol::detail::Value& val);

} // namespace apache::thrift::test::utils
