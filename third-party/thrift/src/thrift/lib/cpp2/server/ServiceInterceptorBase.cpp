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

#include <thrift/lib/cpp2/server/ServiceInterceptorBase.h>

#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

#if FOLLY_HAS_COROUTINES

namespace apache::thrift {

folly::coro::Task<void> ServiceInterceptorBase::co_onStartServing(InitParams) {
  co_return;
}

} // namespace apache::thrift

#endif // FOLLY_HAS_COROUTINES
