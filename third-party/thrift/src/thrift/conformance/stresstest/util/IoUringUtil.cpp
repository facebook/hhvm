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

#include <thrift/conformance/stresstest/util/IoUringUtil.h>

DEFINE_bool(use_iouring_event_eventfd, true, "");
DEFINE_int32(io_capacity, 16'000, "");
DEFINE_int32(io_submit_sqe, -1, "");
DEFINE_int32(io_max_get, -1, "");
DEFINE_bool(set_iouring_defer_taskrun, true, "");
DEFINE_int32(io_max_submit, -1, "");
DEFINE_int32(io_registers, 16'000, "");
DEFINE_int32(io_prov_buffs_size, 131'072, "");
DEFINE_int32(io_prov_buffs, 2'000, "");

namespace apache {
namespace thrift {
namespace stress {
folly::IoUringBackend::Options getIoUringOptions() {
  folly::IoUringBackend::Options options;
  options.setRegisterRingFd(FLAGS_use_iouring_event_eventfd);

  if (FLAGS_io_prov_buffs_size > 0 && FLAGS_io_prov_buffs > 0) {
    options.setInitialProvidedBuffers(
        FLAGS_io_prov_buffs_size, FLAGS_io_prov_buffs);
  }

  if (FLAGS_io_registers > 0) {
    options.setUseRegisteredFds(static_cast<size_t>(FLAGS_io_registers));
  }

  if (FLAGS_io_capacity > 0) {
    options.setCapacity(static_cast<size_t>(FLAGS_io_capacity));
  }

  if (FLAGS_io_submit_sqe > 0) {
    options.setSqeSize(FLAGS_io_submit_sqe);
  }

  if (FLAGS_io_max_get > 0) {
    options.setMaxGet(static_cast<size_t>(FLAGS_io_max_get));
  }

  if (FLAGS_io_max_submit > 0) {
    options.setMaxSubmit(static_cast<size_t>(FLAGS_io_max_submit));
  }

  if (FLAGS_set_iouring_defer_taskrun) {
    if (folly::IoUringBackend::kernelSupportsDeferTaskrun()) {
      options.setDeferTaskRun(FLAGS_set_iouring_defer_taskrun);
    } else {
      LOG(ERROR) << "not setting DeferTaskRun as not supported on this kernel";
    }
  }
  return options;
}
} // namespace stress
} // namespace thrift
} // namespace apache
