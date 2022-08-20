/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef THRIFT_TRANSPORT_THTTPTRANSPORT_H_
#define THRIFT_TRANSPORT_THTTPTRANSPORT_H_ 1

#include <thrift/lib/cpp/transport/TBufferTransports.h>
#include <thrift/lib/cpp/transport/TVirtualTransport.h>

namespace apache {
namespace thrift {
namespace transport {

/**
 * HTTP implementation of the thrift transport. This was irritating
 * to write, but the alternatives in C++ land are daunting. Linking CURL
 * requires 23 dynamic libraries last time I checked (WTF?!?). All we have
 * here is a VERY basic HTTP/1.1 client which supports HTTP 100 Continue,
 * chunked transfer encoding, keepalive, etc. Tested against Apache.
 */
class THttpTransport : public TVirtualTransport<THttpTransport> {
 public:
  explicit THttpTransport(std::shared_ptr<TTransport> transport);

  ~THttpTransport() override;

  void open() override { transport_->open(); }

  bool isOpen() override { return transport_->isOpen(); }

  bool peek() override { return transport_->peek(); }

  void close() override { transport_->close(); }

  uint32_t read(uint8_t* buf, uint32_t len);

  uint32_t readEnd() override;

  void write(const uint8_t* buf, uint32_t len);

  void flush() override = 0;

  std::shared_ptr<TTransport> getUnderlyingTransport() { return transport_; }

 protected:
  std::shared_ptr<TTransport> transport_;

  TMemoryBuffer writeBuffer_;
  TMemoryBuffer readBuffer_;

  bool readHeaders_;
  bool chunked_;
  bool chunkedDone_;
  uint32_t chunkSize_;
  uint32_t contentLength_;

  char* httpBuf_;
  uint32_t httpPos_;
  uint32_t httpBufLen_;
  uint32_t httpBufSize_;

  virtual void init();

  uint32_t readMoreData();
  char* readLine();

  void readHeaders();
  virtual void beginParsingHeaders() {}
  virtual void parseHeader(char* header) = 0;
  virtual bool parseStatusLine(char* status) = 0;
  virtual void endParsingHeaders() {}

  uint32_t readChunked();
  void readChunkedFooters();
  uint32_t parseChunkSize(char* line);

  uint32_t readContent(uint32_t size);

  virtual void refill();
  void shift();

  static const char* CRLF;
  static const int CRLF_LEN;
};

} // namespace transport
} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_TRANSPORT_THTTPCLIENT_H_
