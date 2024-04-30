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

// NOTE: rocket_frame_parser is aimed at replacing rocket_strategy_parser and
// rocket_allocating_strategy_parser boolean flags.
// Should take one of the following three values:
// - "legacy" - use legacy parser which is the legacy implementation currently
// in Parser-inl.h
// - "strategy" - use strategy parser which we intend to replace legacy parser.
// Currently in FrameLengthParserStrategy.cpp
// - "allocating" - use allocating strategy parser which allow usage of custom
// allocator. Currently in AllocatingParserStrategy.cpp
// Note the current way to configure which parser to use is error prone as some
// one may specify rocket_strategy_parser and rocket_allocating_strategy_parser
// to be both true and it is ambiguous which one to use.
THRIFT_FLAG_DEFINE_string(rocket_frame_parser, "legacy");
