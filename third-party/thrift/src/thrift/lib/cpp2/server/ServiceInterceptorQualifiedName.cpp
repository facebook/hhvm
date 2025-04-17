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

#include <fmt/format.h>
#include <glog/logging.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorQualifiedName.h>

namespace apache::thrift {

namespace {

void fatalIfAlreadyIntialized(const std::string& qualifiedName) {
  CHECK(qualifiedName.empty())
      << "Invariant violation - cannot set interceptor name / module name twice";
}

void fatalIfModuleNameIsInvalid(const std::string& moduleName) {
  CHECK(!moduleName.empty()) << "ServerModule names cannot be empty";
  CHECK_EQ(moduleName.find_first_of(".,"), std::string::npos)
      << "ServerModule name cannot contain '.' or ',' - got: " << moduleName;
}

void fatalIfInterceptorNameIsInvalid(const std::string& interceptorName) {
  CHECK(!interceptorName.empty()) << "Interceptor names cannot be empty";
  CHECK_EQ(interceptorName.find_first_of(".,"), std::string::npos)
      << "ServerModule name cannot contain '.' or ',' - got: "
      << interceptorName;
}

void fatalIfUninitialized(const std::string& qualifiedName) {
  CHECK(!qualifiedName.empty())
      << "Invariant violation - interceptor was never initialized properly";
}

} // namespace

void ServiceInterceptorQualifiedName::setName(
    std::string moduleName, std::string interceptorName) {
  fatalIfAlreadyIntialized(qualifiedName_);
  fatalIfModuleNameIsInvalid(moduleName);
  fatalIfInterceptorNameIsInvalid(interceptorName);

  auto moduleNameLength = moduleName.size();
  auto interceptorNameLength = interceptorName.size();
  qualifiedName_ =
      fmt::format("{}.{}", std::move(moduleName), std::move(interceptorName));
  moduleName_ = std::string_view(qualifiedName_.c_str(), moduleNameLength);
  // The additional offset is for the '.'
  interceptorName_ = std::string_view(
      qualifiedName_.c_str() + moduleNameLength + 1, interceptorNameLength);
}

std::string_view ServiceInterceptorQualifiedName::get() const {
  fatalIfUninitialized(qualifiedName_);
  return qualifiedName_;
}

std::string_view ServiceInterceptorQualifiedName::getModuleName() const {
  fatalIfUninitialized(qualifiedName_);
  return moduleName_;
}

std::string_view ServiceInterceptorQualifiedName::getInterceptorName() const {
  fatalIfUninitialized(qualifiedName_);
  return interceptorName_;
}

} // namespace apache::thrift
