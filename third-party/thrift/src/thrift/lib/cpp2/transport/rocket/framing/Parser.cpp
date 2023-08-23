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

// Flag to control whether or not the new parser memory manangement logic is
// enabled. (Do not hold buffer internally in rocket parser. Always hand
// payloads to application as soon as they are read from the socket.)
THRIFT_FLAG_DEFINE_bool(rocket_parser_dont_hold_buffer_enabled, false);

// Add a flag to enable strategy based parser. Default strategy right now uses.
THRIFT_FLAG_DEFINE_bool(rocket_strategy_parser, false);

// Add a flag to enable allocating strategy based parser.
THRIFT_FLAG_DEFINE_bool(rocket_allocating_strategy_parser, false);
