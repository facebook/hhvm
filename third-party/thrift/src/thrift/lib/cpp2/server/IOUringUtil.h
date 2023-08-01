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

#if __linux__ && !FOLLY_MOBILE && __has_include(<liburing.h>)

#define HAS_IO_URING 1

#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/experimental/io/IoUringBackend.h>

namespace apache {
namespace thrift {
namespace io_uring_util {

std::unique_ptr<folly::EventBaseBackendBase> getIOUringEventbaseBackendFunc();

folly::IoUringBackend::Options getDefaultIOUringOptions();

std::shared_ptr<folly::IOThreadPoolExecutorBase> getDefaultIOUringExecutor(
    bool enableThreadIdCollection = true);

bool validateExecutorSupportsIOUring(
    const std::shared_ptr<folly::IOThreadPoolExecutorBase>& executor);

} // namespace io_uring_util
} // namespace thrift
} // namespace apache

#endif
