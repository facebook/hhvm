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
#include <string>

#include <folly/io/IOBuf.h>

namespace apache::thrift::python {

class PythonUserException final : public std::exception {
 public:
  PythonUserException(
      std::string type, std::string reason, std::unique_ptr<folly::IOBuf> buf);
  PythonUserException(const PythonUserException& ex);
  PythonUserException& operator=(const PythonUserException& ex);

  const std::string& type() const { return type_; }
  const std::string& reason() const { return reason_; }
  const folly::IOBuf* buf() const { return buf_.get(); }
  const char* what() const noexcept override { return reason_.c_str(); }

 private:
  std::string type_;
  std::string reason_;
  std::unique_ptr<folly::IOBuf> buf_;
};

} // namespace apache::thrift::python
