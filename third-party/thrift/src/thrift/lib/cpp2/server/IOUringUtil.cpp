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

#include <thrift/lib/cpp2/server/IOUringUtil.h>

#ifdef HAS_IO_URING
#include <folly/experimental/io/IoUringEventBaseLocal.h>
#include <folly/system/HardwareConcurrency.h>

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

std::shared_ptr<folly::IOThreadPoolExecutorBase> getDefaultIOUringExecutor(
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
      folly::hardware_concurrency(),
      std::make_shared<folly::NamedThreadFactory>("ThriftIO"),
      &ebm,
      folly::IOThreadPoolExecutor::Options().setEnableThreadIdCollection(
          enableThreadIdCollection));
}

bool validateExecutorSupportsIOUring(
    const std::shared_ptr<folly::IOThreadPoolExecutorBase>& executor) {
  VLOG(1) << "checking to see if supports io_uring";
  try {
    if (auto eventBase =
            static_cast<folly::IOExecutor*>(executor.get())->getEventBase()) {
      VLOG(1) << "checking found event base";
      auto t = dynamic_cast<folly::IoUringBackend*>(eventBase->getBackend());
      VLOG(1) << "event base supports io_uring " << t;
      return t;
    }
  } catch (const std::exception& e) {
    VLOG(1) << "error getting executor, configuring default: " << e.what();
    return false;
  }

  VLOG(1) << "doesn't support io_uring";
  return false;
}

} // namespace io_uring_util
} // namespace thrift
} // namespace apache

#endif
