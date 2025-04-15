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

#include <folly/observer/Observer.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorQualifiedName.h>

namespace apache::thrift {

/**
 * A ServiceInterceptorControl handles listening to the thrift flag
 * disabled_service_interceptors. It checks whether or not the interceptor's
 * qualified name is in the set of disabled interceptors.
 *
 * This is initialized when a server's modules are collected and processed,
 * since the fully qualified name is not known until then.
 */
class ServiceInterceptorControl {
 public:
  explicit ServiceInterceptorControl(
      const ServiceInterceptorQualifiedName& name);

  bool isDisabled() const;

 private:
  folly::observer::AtomicObserver<bool> observer_;
};

} // namespace apache::thrift
