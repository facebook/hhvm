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

#include <folly/lang/Bits.h>
#include <thrift/lib/cpp2/op/Encode.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

namespace apache::thrift {
namespace detail {
template <typename T>
T& maybe_emplace(T& t) {
  return t;
}
template <typename T>
T& maybe_emplace(std::optional<T>& t) {
  return t.emplace();
}

class BaseCursorReader {
 protected:
  BinaryProtocolReader protocol_;
  enum class State {
    Active,
    Child, // Reading a nested struct or container
    Done,
  };
  State state_ = State::Active;

  void checkState(State expected) {
    if (state_ != expected) {
      folly::throw_exception<std::runtime_error>([&]() {
        switch (state_) {
          case State::Active:
            return "Reader not yet finalized";
          case State::Child:
            return "Child not yet finalized";
          case State::Done:
            return "Reader already finalized";
        }
      }());
    }
  }

  explicit BaseCursorReader(BinaryProtocolReader&& p)
      : protocol_(std::move(p)) {}
  BaseCursorReader() = default;

  ~BaseCursorReader() {
    DCHECK(state_ == State::Done) << "Reader must be passed to endRead";
  }

  BaseCursorReader(BaseCursorReader&& other) noexcept {
    protocol_ = std::move(other.protocol_);
    state_ = other.state_;
    other.state_ = State::Done;
  }
  BaseCursorReader& operator=(BaseCursorReader&& other) noexcept {
    if (this != &other) {
      DCHECK(state_ == State::Done) << "Reader must be passed to endRead";
      protocol_ = std::move(other.protocol_);
      state_ = other.state_;
      other.state_ = State::Done;
    }
    return *this;
  }

 public:
  BaseCursorReader(const BaseCursorReader&) = delete;
  BaseCursorReader& operator=(const BaseCursorReader&) = delete;
};

} // namespace detail

template <typename T>
class StructuredCursorReader;
} // namespace apache::thrift
