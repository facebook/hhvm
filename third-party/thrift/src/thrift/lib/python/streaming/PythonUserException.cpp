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

#include <thrift/lib/python/streaming/PythonUserException.h>

namespace apache::thrift::python {

PythonUserException::PythonUserException(
    std::string type, std::string reason, std::unique_ptr<folly::IOBuf> buf)
    : type_(std::move(type)),
      reason_(std::move(reason)),
      buf_(std::move(buf)) {}

PythonUserException::PythonUserException(const PythonUserException& ex)
    : type_(ex.type_), reason_(ex.reason_), buf_(ex.buf_->clone()) {}

PythonUserException& PythonUserException::operator=(
    const PythonUserException& ex) {
  type_ = ex.type_;
  reason_ = ex.reason_;
  buf_ = ex.buf_->clone();
  return *this;
}

std::unique_ptr<folly::IOBuf> extractBufFromPythonUserException(
    folly::exception_wrapper& ew) {
  if (auto ex_ptr = ew.get_mutable_exception<PythonUserException>()) {
    return std::move(*ex_ptr).extractBuf();
  }

  return nullptr;
}

} // namespace apache::thrift::python
