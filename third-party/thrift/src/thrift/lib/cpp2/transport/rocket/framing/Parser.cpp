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

#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>

// timeout value for true resizng timer mechanism, if 0 fallback to
// the old mechanism
THRIFT_FLAG_DEFINE_int64(rocket_parser_resize_period_seconds, 3);

// Flag to control whether or not the new parser memory manangement logic is
// enabled. (Do not hold buffer internally in rocket parser. Always hand
// payloads to application as soon as they are read from the socket.)
THRIFT_FLAG_DEFINE_bool(rocket_parser_dont_hold_buffer_enabled, false);

// Enable hybrid buffer parser logic. In hybrid mode, parser uses a small, fixed
// size buffer for incoming frames. If a frame exceeds the buffer size or page
// alignment is requested, a dedicated buffer is allocated and ownership is
// passed to the application.
THRIFT_FLAG_DEFINE_bool(rocket_parser_hybrid_buffer_enabled, false);
