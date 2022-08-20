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

#include <thrift/conformance/cpp2/internal/AnyStructSerializer.h>

#include <folly/CPortability.h>
#include <thrift/conformance/cpp2/AnySerializer.h>

namespace apache::thrift::conformance {

// A serializer base class any thrift struct and protocol.
template <typename T, typename Reader, typename Writer>
class BaseAnyStructSerializer
    : public BaseTypedAnySerializer<
          T,
          BaseAnyStructSerializer<T, Reader, Writer>> {
  using Base =
      BaseTypedAnySerializer<T, BaseAnyStructSerializer<T, Reader, Writer>>;

 public:
  using Base::encode;
  void encode(const T& value, folly::io::QueueAppender&& appender) const;

  using Base::decode;
  void decode(folly::io::Cursor& cursor, T& value) const;
};

// A standard protocol serializer for any thrift struct.
template <typename T, StandardProtocol StdProtocol>
class AnyStandardSerializer : public BaseAnyStructSerializer<
                                  T,
                                  detail::protocol_reader_t<StdProtocol>,
                                  detail::protocol_writer_t<StdProtocol>> {
 public:
  const Protocol& getProtocol() const final {
    return getStandardProtocol<StdProtocol>();
  }
};

// Returns a static serializer for the given struct T and standard protocol.
template <typename T, StandardProtocol StdProtocol>
FOLLY_EXPORT const AnyStandardSerializer<T, StdProtocol>&
getAnyStandardSerializer() {
  static const AnyStandardSerializer<T, StdProtocol> kSerializer;
  return kSerializer;
}

// Implementation.

template <typename T, typename Reader, typename Writer>
void BaseAnyStructSerializer<T, Reader, Writer>::encode(
    const T& value, folly::io::QueueAppender&& appender) const {
  Writer writer;
  writer.setOutput(std::move(appender));
  value.write(&writer);
}

template <typename T, typename Reader, typename Writer>
void BaseAnyStructSerializer<T, Reader, Writer>::decode(
    folly::io::Cursor& cursor, T& value) const {
  Reader reader;
  reader.setInput(cursor);
  value.read(&reader);
  cursor = reader.getCursor();
}

} // namespace apache::thrift::conformance
