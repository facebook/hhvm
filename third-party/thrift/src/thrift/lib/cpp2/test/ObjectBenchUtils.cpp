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

#include <folly/BenchmarkUtil.h>

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

std::size_t read_all(const folly::F14FastSet<protocol::detail::Value>& s) {
  std::size_t res = 0;
  for (const auto& val : s) {
    res += read_all(val);
  }
  return res;
}

std::size_t read_all(const std::vector<protocol::detail::Value>& l) {
  std::size_t res = 0;
  for (const auto& val : l) {
    res += read_all(val);
  }
  return res;
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

// ----- Access a sparse subset of data within a thrift hierarchy ---- //

#define READ_SOME_IS_ALL(TYPE)                         \
  std::size_t read_some(SparseAccess, const TYPE& t) { \
    return read_all(t);                                \
  }

READ_SOME_IS_ALL(bool)
READ_SOME_IS_ALL(int8_t)
READ_SOME_IS_ALL(int16_t)
READ_SOME_IS_ALL(int32_t)
READ_SOME_IS_ALL(int64_t)
READ_SOME_IS_ALL(float)
READ_SOME_IS_ALL(double)
READ_SOME_IS_ALL(std::string)
READ_SOME_IS_ALL(folly::IOBuf)

#undef READ_SOME_IS_ALL

template <typename T>
std::size_t read_some_map_iter(SparseAccess access, const T& iterable);
template <typename T>
std::size_t read_some_iter(SparseAccess access, const T& iterable);

template <typename T>
std::size_t read_some_iter(SparseAccess access, const T& iterable) {
  std::size_t res = 0;
  switch (access) {
    case SparseAccess::First: {
      const auto it = iterable.begin();
      if (it != iterable.end()) {
        res += read_some(access, *it);
      }
      break;
    }
    case SparseAccess::Half: {
      bool skip = false;
      for (auto it = iterable.begin(); it != iterable.end(); ++it) {
        if (skip) {
          skip = false;
        } else {
          res += read_some(access, *it);
          skip = true;
        }
      }
    }
  }
  return res;
}

template <typename T>
std::size_t read_some_map_iter(SparseAccess access, const T& iterable) {
  std::size_t res = 0;
  switch (access) {
    case SparseAccess::First: {
      const auto it = iterable.begin();
      if (it != iterable.end()) {
        const auto& [key, value] = *it;
        res += read_some(access, key);
        res += read_some(access, value);
      }
      break;
    }
    case SparseAccess::Half: {
      bool skip = false;
      for (auto it = iterable.begin(); it != iterable.end(); ++it) {
        if (skip) {
          skip = false;
        } else {
          const auto& [key, value] = *it;
          res += read_some(access, key);
          res += read_some(access, value);
          skip = true;
        }
      }
    }
  }
  return res;
}

template <typename T>
std::size_t read_some(SparseAccess access, const folly::F14FastSet<T>& set) {
  return read_some_iter(access, set);
}

std::size_t read_some(
    SparseAccess access, const std::vector<protocol::Value>& l) {
  return read_some_iter(access, l);
}

std::size_t read_some(
    SparseAccess access,
    const folly::F14FastMap<protocol::detail::Value, protocol::detail::Value>&
        m) {
  return read_some_map_iter(access, m);
}

std::size_t read_some(
    SparseAccess access, const folly::F14FastSet<protocol::Value>& set) {
  return read_some_iter(access, set);
}

std::size_t read_some(SparseAccess access, const protocol::Object& obj) {
  return read_some_map_iter(access, obj);
}

std::size_t read_some(SparseAccess access, const protocol::detail::Value& val) {
  if (const auto* v = val.if_bool()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_byte()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_i16()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_i32()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_i64()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_float()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_double()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_string()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_binary()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_list()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_set()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_map()) {
    return read_some(access, *v);
  }
  if (const auto* v = val.if_object()) {
    return read_some(access, *v);
  }
  throw std::runtime_error("unknown value type");
}

} // namespace apache::thrift::test::utils
