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

#include <gflags/gflags.h>
#include <folly/experimental/io/IoUringBackend.h>

DECLARE_bool(use_iouring_event_eventfd);
DECLARE_int32(io_capacity);
DECLARE_int32(io_submit_sqe);
DECLARE_int32(io_max_get);
DECLARE_bool(set_iouring_defer_taskrun);
DECLARE_int32(io_max_submit);
DECLARE_int32(io_registers);
DECLARE_int32(io_prov_buffs_size);
DECLARE_int32(io_prov_buffs);
DECLARE_bool(io_zcrx);
DECLARE_int32(io_zcrx_num_pages);
DECLARE_int32(io_zcrx_refill_entries);
DECLARE_string(io_zcrx_ifname);
DECLARE_int32(io_zcrx_queue_id);

#if FOLLY_HAS_LIBURING

namespace apache::thrift::stress {
folly::IoUringBackend::Options getIoUringOptions();
} // namespace apache::thrift::stress

#endif
