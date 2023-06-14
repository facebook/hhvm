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

#include <memory>

#include <folly/io/IOBuf.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/ProxygenErrorEnum.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/transport/core/ThriftChannelIf.h>

namespace apache {
namespace thrift {

class H2ClientConnection;
class ThriftProcessor;

/**
 * Interface that specializes ThriftChannelIf for HTTP/2.  It supports
 * the translation between Thrift payloads and HTTP/2 streams.
 * Concrete implementations support different translation strategies.
 *
 * Objects of this class operate on a single HTTP/2 stream and run on
 * the thread managing the HTTP/2 network connection for that stream.
 *
 * Lifetime for objects of this class is managed via shared pointers:
 *
 * On the client side, a ThriftClient objects calls getChannel() on
 * its H2ClientConnection object.  The H2ClientConnection object
 * returns a shared pointer to an usable channel object (which may or
 * may not have been used for previous RPCs).  Thr ThriftClient
 * releases its shared pointer once its RPC is done, while the
 * H2ClientConnection releases its shared pointer once the channel
 * cannot be used for any more RPCs.
 *
 * On the server side, a H2RequestHandler object creates a shared
 * pointer of a channel object following which additional shared
 * pointers are passed to the single ThriftProcessor object using
 * shared_from_this().  When both the H2RequestHandler object and the
 * ThriftProcessor object are done with the channel object, all the
 * shared pointers are released and the channel object gets destroyed.
 *
 * Channel object methods must always be invoked on the event base
 * (thread) that manages the underlying connection.
 */
class H2Channel : public ThriftChannelIf {
 public:
  static constexpr folly::StringPiece RPC_KIND = "rpckind";

  H2Channel() = default;
  virtual ~H2Channel() = default;

  // Called from Proxygen at the beginning of the stream.
  virtual void onH2StreamBegin(
      std::unique_ptr<proxygen::HTTPMessage> headers) noexcept = 0;

  // Called from Proxygen whenever a body frame is available.
  virtual void onH2BodyFrame(
      std::unique_ptr<folly::IOBuf> contents) noexcept = 0;

  // Called from Proxygen at the end of the stream.  No more data will
  // be sent from Proxygen after this.
  virtual void onH2StreamEnd() noexcept = 0;

  // Called from Proxygen when the stream has closed.  Usually this is
  // called after all reading and writing to the stream have been
  // completed, but it could be called earlier in case of errors.  No
  // more writes to the stream should be performed after this point.
  // Also, after this call Proxygen will relinquish access to this
  // object.
  virtual void onH2StreamClosed(
      proxygen::ProxygenError /*error*/,
      std::string errorDescription) noexcept = 0;

  virtual void onMessageFlushed() noexcept {}

 protected:
  // Encodes Thrift headers to be HTTP compliant.
  void encodeHeaders(
      const transport::THeader::StringToStringMap& source,
      proxygen::HTTPMessage& dest) noexcept;

  // Decodes previously encoded HTTP compliant headers into Thrift
  // headers.
  static void decodeHeaders(
      const proxygen::HTTPMessage& source,
      transport::THeader::StringToStringMap& dest,
      RequestRpcMetadata* metadata) noexcept;

  static bool handleThriftMetadata(
      RequestRpcMetadata* metadata,
      proxygen::HTTPHeaderCode code,
      const std::string& name,
      const std::string& value) noexcept;
};

} // namespace thrift
} // namespace apache
