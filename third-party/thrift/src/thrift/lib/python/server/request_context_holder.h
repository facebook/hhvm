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

#include <atomic>
#include <memory>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift::python {

// Shared-ownership holder for Cpp2RequestContext* that safely invalidates the
// pointer when the C++ request lifecycle ends, preventing dangling access from
// Python background tasks or leaked references.
struct Cpp2RequestContextHolder {
  std::atomic<Cpp2RequestContext*> ctx;
  std::string methodName;

  explicit Cpp2RequestContextHolder(Cpp2RequestContext* c);

  Cpp2RequestContext* get_ctx() const;
  bool is_valid() const;
  void invalidate();
};

void installInvalidator(
    Cpp2RequestContext* ctx,
    const std::shared_ptr<Cpp2RequestContextHolder>& holder);

} // namespace apache::thrift::python
