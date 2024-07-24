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

// rocket_frame_parser flag specify which frame parser rocket transport decide
// to use. It can take three values
// - "strategy": rocket transport will use FrameLengthParserStrategy to parse
// rocket frames. This parsing strategy reads the rocket frame length (first 3
// bytes) and immediately hands off frames to downstream as soon as a full frame
// is available.
// - "allocating": frame parser that can take a custom allocator, we also
// guarantee buffer returned are not chained (meaning continuous). This can
// provide performance benefits when using cursor-based serialization.
THRIFT_FLAG_DEFINE_string(rocket_frame_parser, "strategy");

namespace apache::thrift::rocket::detail {
ParserAllocatorType& getDefaultAllocator() {
  static folly::Indestructible<ParserAllocatorType> defaultAllocator;
  return *defaultAllocator;
}

ParserMode stringToMode(const std::string& modeStr) noexcept {
  if (modeStr == "strategy") {
    return ParserMode::STRATEGY;
  } else if (modeStr == "allocating") {
    return ParserMode::ALLOCATING;
  }

  LOG(WARNING) << "Invalid parser mode: '" << modeStr
               << ", default to ParserMode::STRATEGY";
  return ParserMode::STRATEGY;
}
} // namespace apache::thrift::rocket::detail
