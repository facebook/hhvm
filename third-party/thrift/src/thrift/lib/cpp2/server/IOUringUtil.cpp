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

#ifdef __linux__

#include <folly/experimental/io/IoUringEventBaseLocal.h>
#include <thrift/lib/cpp2/server/IOUringUtil.h>

namespace apache {
namespace thrift {
namespace io_uring_util {

std::unique_ptr<folly::EventBaseBackendBase> getIOUringEventbaseBackendFunc() {
  try {
    return std::make_unique<folly::IoUringBackend>(getDefaultIOUringOptions());
  } catch (const std::exception& ex) {
    LOG(FATAL) << "Failed to create io_uring backend: "
               << folly::exceptionStr(ex);
  }
}

folly::IoUringBackend::Options getDefaultIOUringOptions() {
  folly::IoUringBackend::Options options;
  options.setRegisterRingFd(true)
      .setInitialProvidedBuffers(2048, 2000)
      .setUseRegisteredFds(2048)
      .setDeferTaskRun(true)
      .setCapacity(512);
  return options;
}

std::shared_ptr<folly::IOThreadPoolExecutor> getDefaultIOUringExecutor(
    bool enableThreadIdCollection) {
  folly::EventBaseBackendBase::FactoryFunc func(getIOUringEventbaseBackendFunc);
  static folly::EventBaseManager ebm(
      folly::EventBase::Options().setBackendFactory(std::move(func)));

  auto* evb = folly::EventBaseManager::get()->getEventBase();
  // use the same EventBase for the main thread
  if (evb) {
    ebm.setEventBase(evb, false);
  }
  return std::make_shared<folly::IOThreadPoolExecutor>(
      0,
      std::make_shared<folly::NamedThreadFactory>("ThriftIO"),
      folly::EventBaseManager::get(),
      folly::IOThreadPoolExecutor::Options().setEnableThreadIdCollection(
          enableThreadIdCollection));
}

bool validateExecutorSupportsIOUring(
    const std::shared_ptr<folly::IOThreadPoolExecutor>& executor) {
  if (auto eventBase = executor->getEventBase()) {
    return dynamic_cast<folly::IoUringBackend*>(eventBase->getBackend());
  }
  return false;
}

} // namespace io_uring_util
} // namespace thrift
} // namespace apache

#endif
