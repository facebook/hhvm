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

namespace apache::thrift {

/**
 * Service Interceptors are uniquely identified by
 * {moduleName}.{interceptorName} at runtime. Module Name is the the name of
 * module that installed the interceptor, and Interceptor Name is the result
 * of calling the getName() method on the interceptor.
 */
class ServiceInterceptorQualifiedName {
 public:
  /**
   * This is called by the ThriftServer when installing Service interceptors,
   * since that is the earliest point when the fully qualfied name can be known.
   * This method will fatal if moduleName or interceptorName are empty or
   * contain restricted characters '.' and ','. This method is only expected
   * to be called once - calling it more than that will also be a fatal error.
   */
  void setName(std::string moduleName, std::string interceptorName);

  /**
   * Get the fully qualified name of the intercepor i.e
   * {moduleName}.{interceptorName}
   */
  std::string_view get() const;

  /**
   * Get the module name, i.e the first part of the fully qualified name
   */
  std::string_view getModuleName() const;

  /**
   * Get the interceptor name, i.e the second part of the fully qualified name
   */
  std::string_view getInterceptorName() const;

 private:
  std::string_view moduleName_;
  std::string_view interceptorName_;
  std::string qualifiedName_;
};

} // namespace apache::thrift
