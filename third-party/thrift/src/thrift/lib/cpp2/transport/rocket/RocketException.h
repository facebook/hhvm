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

#include <exception>
#include <memory>
#include <utility>

#include <folly/Range.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>

namespace apache::thrift::rocket {

class FOLLY_EXPORT RocketException : public std::exception {
 public:
  explicit RocketException(ErrorCode errorCode)
      : rsocketErrorCode_(errorCode) {}

  RocketException(ErrorCode errorCode, std::unique_ptr<folly::IOBuf> errorData)
      : rsocketErrorCode_(errorCode), errorData_(std::move(errorData)) {}

  RocketException(ErrorCode errorCode, folly::StringPiece errorData)
      : RocketException(errorCode, folly::IOBuf::copyBuffer(errorData)) {}

  RocketException(RocketException&&) = default;
  RocketException(const RocketException& other)
      : rsocketErrorCode_(other.rsocketErrorCode_),
        errorData_(other.errorData_ ? other.errorData_->clone() : nullptr) {}

  ErrorCode getErrorCode() const noexcept { return rsocketErrorCode_; }

  const char* what() const noexcept override { return "RocketException"; }

  std::unique_ptr<folly::IOBuf> moveErrorData() {
    return std::move(errorData_);
  }

  bool hasErrorData() const noexcept {
    return !!errorData_ && !errorData_->empty();
  }

 private:
  ErrorCode rsocketErrorCode_{ErrorCode::RESERVED};
  std::unique_ptr<folly::IOBuf> errorData_;
};

} // namespace apache::thrift::rocket
