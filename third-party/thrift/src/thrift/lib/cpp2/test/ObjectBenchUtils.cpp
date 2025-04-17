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

#include <cstdint>

#include <folly/Benchmark.h>

#include <thrift/lib/cpp2/test/ObjectBenchUtils.h>

namespace apache::thrift::test::utils {

template <typename T>
constexpr bool is_pod_v = std::is_same_v<T, bool> ||
    std::is_same_v<T, std::int8_t> || std::is_same_v<T, std::int16_t> ||
    std::is_same_v<T, std::int32_t> || std::is_same_v<T, std::int64_t> ||
    std::is_same_v<T, float> || std::is_same_v<T, double>;

std::size_t read_all(const bool& b) {
  return b ? 1 : 0;
}
std::size_t read_all(const int8_t& i) {
  return static_cast<std::size_t>(i);
}
std::size_t read_all(const int16_t& i) {
  return static_cast<std::size_t>(i);
}

std::size_t read_all(const int32_t& i) {
  return static_cast<std::size_t>(i);
}

std::size_t read_all(const int64_t& i) {
  return static_cast<std::size_t>(i);
}

std::size_t read_all(const float& f) {
  return static_cast<std::size_t>(f);
}

size_t read_all(const double& d) {
  return static_cast<std::size_t>(d);
}

std::size_t read_all(const std::string& s) {
  return s.size() == 0 ? 0 : static_cast<std::size_t>(s[0]);
}

std::size_t read_all(const folly::IOBuf& b) {
  return b.length() == 0 ? 0 : static_cast<std::size_t>(b.data()[0]);
}

std::size_t read_all(
    const folly::F14FastMap<protocol::detail::Value, protocol::detail::Value>&
        m) {
  std::size_t res = 0;
  for (const auto& [key, val] : m) {
    res += read_all(key);
    res += read_all(val);
  }
  return res;
}

template <typename T>
std::size_t read_all_iter(const T& iterable) {
  std::size_t res = 0;
  for (const auto& val : iterable) {
    res += read_all(val);
  }
  return res;
}

std::size_t read_all(const folly::F14VectorSet<protocol::detail::Value>& s) {
  return read_all_iter(s);
}

std::size_t read_all(const std::vector<protocol::detail::Value>& l) {
  return read_all_iter(l);
}

std::size_t read_all(const protocol::detail::Value& val) {
  if (const auto* v = val.if_bool()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_byte()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_i16()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_i32()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_i64()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_float()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_double()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_string()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_binary()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_list()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_set()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_map()) {
    return read_all(*v);
  }
  if (const auto* v = val.if_object()) {
    return read_all(*v);
  }
  throw std::runtime_error("unknown value type");
}

std::size_t read_all(const protocol::Object& obj) {
  std::size_t res = 0;
  for (const auto& [field_id, field] : obj) {
    res += read_all(field);
  }
  return res;
}

} // namespace apache::thrift::test::utils
