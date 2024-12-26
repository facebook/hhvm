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
#include <vector>

#include <folly/ExceptionWrapper.h>

namespace thrift::py3::exception {

// The only place this function is used it needs a shared_ptr, so may as
// well just return one instead of a unique_ptr
template <class T>
std::shared_ptr<T> try_make_shared_exception(
    const folly::exception_wrapper& exception) {
  auto e = exception.get_exception<T>();
  return e ? std::make_shared<T>(*e) : std::shared_ptr<T>();
}

} // namespace thrift::py3::exception
