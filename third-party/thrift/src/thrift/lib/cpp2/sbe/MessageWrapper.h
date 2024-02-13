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
#include <cstdint>
#include <optional>
#include <string_view>
#include <folly/io/IOBuf.h>
#include <folly/lang/SafeAssert.h>

namespace apache::thrift::sbe {

/**
 * A wrapper around a SBE flyweight that makes it easier to encode and decode.
 * Has helper methods to make it easier to deal with sub messages, i.e. a
 * message within in message. This class isn't mean to be passed around like a
 * Thrift struct. If you need to pass around an SBE message, you should pass
 * around the bytes that contain the message, and then use this class to wrap
 * the bytes for encoding or decoding.
 * @tparam Message The SBE message type
 * @tparam MessageHeader The SBE message header type associated with the message
 */
template <typename Message, typename MessageHeader>
class MessageWrapper {
 public:
  MessageWrapper() = default;

  MessageWrapper(MessageWrapper&&) = delete;
  MessageWrapper(const MessageWrapper&) = delete;
  MessageWrapper& operator=(MessageWrapper&&) = delete;
  MessageWrapper& operator=(const MessageWrapper&) = delete;

  operator Message&() { return message_; }
  Message* operator->() { return &message_; }

  void wrapForEncode(std::string_view& buf) {
    wrapForEncode(const_cast<char*>(buf.data()), buf.size());
  }

  void wrapForEncode(folly::IOBuf& buf, size_t offset = 0) {
    wrapForEncode(buf.writableData(), buf.capacity(), offset);
  }

  void wrapForEncoding(const void* buf, size_t length, size_t offset = 0) {
    wrapForEncode(const_cast<void*>(buf), length, offset);
  }

  void wrapForEncode(void* buf, size_t length, size_t offset = 0) {
    message_.wrapAndApplyHeader(static_cast<char*>(buf), offset, length);
  }

  void wrapForDecode(std::string_view& buf) {
    wrapForDecode(const_cast<char*>(buf.data()), buf.size());
  }

  void wrapForDecode(folly::IOBuf& buf, size_t offset = 0) {
    wrapForDecode(buf.writableData(), buf.capacity(), offset);
  }

  void wrapForDecode(const void* buf, size_t length, size_t offset = 0) {
    wrapForDecode(const_cast<void*>(buf), length, offset);
  }

  void wrapForDecode(void* buf, size_t length, size_t offset = 0) {
    char* buffer = static_cast<char*>(buf);
    header_.wrap(buffer, offset, Message::sbeSchemaVersion(), length);
    FOLLY_SAFE_DCHECK(
        header_.templateId() == Message::sbeTemplateId(),
        "Template Id in Buffer is not the same as the template id in the MessageWrapper");
    message_.wrapForDecode(
        buffer,
        offset + header_.encodedLength(),
        header_.blockLength(),
        header_.version(),
        length);
  }

  template <typename SubMessage, typename SubMessageHeader>
  std::uint32_t wrapSubMessageForEncode(
      MessageWrapper<SubMessage, SubMessageHeader>& message,
      const std::uint32_t subMessageEncodeLength,
      std::string_view (*bufferFunc)(Message&)) {
    encodeLength(subMessageEncodeLength);
    return wrapSubMessage(
        message,
        bufferFunc,
        &MessageWrapper<SubMessage, SubMessageHeader>::wrapForEncode);
  }

  template <typename SubMessage, typename SubMessageHeader>
  std::uint32_t wrapSubMessageForDecode(
      MessageWrapper<SubMessage, SubMessageHeader>& message,
      std::string_view (*bufferFunc)(Message&)) {
    return wrapSubMessage(
        message,
        bufferFunc,
        &MessageWrapper<SubMessage, SubMessageHeader>::wrapForDecode);
  }

  /**
   * Get the template id and buffer for the sub message. Use this method if you
   * sub message can have multiple types. This would be closest to a union in
   * Thrift. If your sub message only has one type use the other sub message
   * methods.
   */
  template <typename SubMessageHeader>
  std::optional<std::pair<std::string_view, std::uint16_t>>
  getSubMessageTemplateIdAndBuffer(std::string_view (*bufferFunc)(Message&)) {
    auto buf = bufferFunc(message_);
    auto size = buf.size();

    if (size) {
      char* bufPtr = const_cast<char*>(buf.data());
      SubMessageHeader header(bufPtr, 0, size, 0);
      std::uint16_t templateId = header.templateId();
      return std::make_pair(buf, templateId);
    } else {
      return std::nullopt;
    }
  }

  void completeEncoding() { message_.checkEncodingIsComplete(); }
  void completeEncoding(folly::IOBuf& buf) {
    completeEncoding();
    size_t length = message_.bufferLength() - message_.sbePosition();
    buf.append(length);
  }

 private:
  Message message_;
  MessageHeader header_;

  void encodeLength(const std::uint32_t length) {
    const std::uint32_t val = folly::Endian::little(length);
    auto buffer = message_.buffer();
    auto position = message_.sbePosition();
    std::memcpy(buffer + position, &val, sizeof(val));
  }

  template <typename SubMessage, typename SubMessageHeader>
  std::uint32_t wrapSubMessage(
      MessageWrapper<SubMessage, SubMessageHeader>& message,
      std::string_view (*bufferFunc)(Message&),
      void (MessageWrapper<SubMessage, SubMessageHeader>::*wrapFunc)(
          std::string_view&)) {
    auto buf = bufferFunc(message_);
    auto size = buf.size();
    if (size) {
      (message.*wrapFunc)(buf);
      return size;
    } else {
      return size;
    }
  }
};

} // namespace apache::thrift::sbe
