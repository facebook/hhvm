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

#include <thrift/lib/cpp2/transcode/TranscodeErrc.h>

namespace apache::thrift::transcode {

std::string_view toString(TranscodeErrc code) noexcept {
  switch (code) {
    case TranscodeErrc::Ok:
      return "Ok";
    case TranscodeErrc::Truncated:
      return "Truncated";
    case TranscodeErrc::Malformed:
      return "Malformed";
    case TranscodeErrc::Overflow:
      return "Overflow";
    case TranscodeErrc::LimitExceeded:
      return "LimitExceeded";
    case TranscodeErrc::Unsupported:
      return "Unsupported";
    case TranscodeErrc::SchemaMismatch:
      return "SchemaMismatch";
    case TranscodeErrc::DuplicateArgument:
      return "DuplicateArgument";
    case TranscodeErrc::MissingArgument:
      return "MissingArgument";
    case TranscodeErrc::Oom:
      return "Oom";
    case TranscodeErrc::Internal:
      return "Internal";
  }
  return "Unknown";
}

} // namespace apache::thrift::transcode
