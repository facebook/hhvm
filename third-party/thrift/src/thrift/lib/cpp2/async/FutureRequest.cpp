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

#include <thrift/lib/cpp2/async/FutureRequest.h>

template class apache::thrift::detail::FutureCallbackHelper<folly::Unit>;
template class apache::thrift::detail::FutureCallbackHelper<bool>;
template class apache::thrift::detail::FutureCallbackHelper<std::int8_t>;
template class apache::thrift::detail::FutureCallbackHelper<std::int16_t>;
template class apache::thrift::detail::FutureCallbackHelper<std::int32_t>;
template class apache::thrift::detail::FutureCallbackHelper<float>;
template class apache::thrift::detail::FutureCallbackHelper<double>;
template class apache::thrift::detail::FutureCallbackHelper<std::string>;
template class apache::thrift::detail::FutureCallbackHelper<
    std::unique_ptr<folly::IOBuf>>;
