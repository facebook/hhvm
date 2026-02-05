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

#include <cstddef>

#include <thrift/lib/cpp2/async/InterceptorFrameworkMetadata.h>
#include <thrift/lib/cpp2/util/AllocationColocator.h>
#include <thrift/lib/cpp2/util/TypeErasedTupleRef.h>
#include <thrift/lib/cpp2/util/TypeErasedValue.h>

namespace apache::thrift::detail {

using ServiceInterceptorOnRequestStorage =
    util::TypeErasedValue<64, alignof(std::max_align_t)>;

struct ServiceInterceptorRequestStorageContext {
  std::size_t count = 0;
  util::AllocationColocator<>::ArrayPtr<ServiceInterceptorOnRequestStorage>
      onRequest = nullptr;
  InterceptorFrameworkMetadataStorage frameworkMetadata = {};
};

using ServiceInterceptorOnConnectionStorage =
    util::TypeErasedValue<128, alignof(std::max_align_t)>;

using StreamId = uint32_t;

using ServiceInterceptorOnRequestArguments = util::TypeErasedTupleRef;

} // namespace apache::thrift::detail
