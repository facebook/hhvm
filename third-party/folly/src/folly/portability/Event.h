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

#ifdef _MSC_VER
// This needs to be before the libevent include.
#include <folly/portability/Windows.h>
#endif

#include <event.h>

#ifdef _MSC_VER
#include <event2/event_compat.h> // @manual
// The signal_set macro from libevent 2 compat conflicts with the
// boost::asio::signal_set function
#undef signal_set
#include <folly/portability/Fcntl.h>
#endif

// The signal_set macro from libevent 1.4.14b-stable conflicts with the
// boost::asio::signal_set function
#if _EVENT_NUMERIC_VERSION == 0x01040e00
#undef signal_set
#endif

#include <folly/net/detail/SocketFileDescriptorMap.h>

namespace folly {
using libevent_fd_t = decltype(event::ev_fd);
} // namespace folly
