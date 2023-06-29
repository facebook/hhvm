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

#ifndef THRIFT_TRANSPORT_THTTPPARSER_H_
#define THRIFT_TRANSPORT_THTTPPARSER_H_ 1

#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/transport/TBufferTransports.h>

namespace apache {
namespace thrift {
namespace util {

class THttpParser {
 protected:
  enum HttpParseState {
    HTTP_PARSE_START = 0,
    HTTP_PARSE_HEADER,
    HTTP_PARSE_CHUNK,
    HTTP_PARSE_CONTENT,
    HTTP_PARSE_CHUNKFOOTER,
    HTTP_PARSE_TRAILING
  };

  enum HttpParseResult {
    HTTP_PARSE_RESULT_CONTINUE,
    HTTP_PARSE_RESULT_BLOCK,
  };

 public:
  THttpParser();
  virtual ~THttpParser();

  void getReadBuffer(void** bufReturn, size_t* lenReturn);
  bool readDataAvailable(size_t len);
  int getMinBytesRequired();
  uint32_t getUnparsedDataLen() const { return httpBufLen_ - httpPos_; }
  void setDataBuffer(apache::thrift::transport::TMemoryBuffer* buffer) {
    dataBuf_ = buffer;
  }
  void unsetDataBuffer() { dataBuf_ = nullptr; }
  void setMaxSize(uint32_t size) { maxSize_ = size; }
  uint32_t getMaxSize() { return maxSize_; }
  bool hasReadAheadData() {
    return (state_ == HTTP_PARSE_START) && (httpBufLen_ > httpPos_);
  }
  bool hasPartialMessage() { return partialMessageSize_ > 0; }
  const folly::F14NodeMap<std::string, std::string>& getReadHeaders() {
    return readHeaders_;
  }
  folly::F14NodeMap<std::string, std::string> moveReadHeaders() {
    return std::move(readHeaders_);
  }
  virtual std::unique_ptr<folly::IOBuf> constructHeader(
      std::unique_ptr<folly::IOBuf> buf) = 0;
  virtual std::unique_ptr<folly::IOBuf> constructHeader(
      std::unique_ptr<folly::IOBuf> buf,
      const folly::F14NodeMap<std::string, std::string>& writeHeaders,
      const folly::F14NodeMap<std::string, std::string>* extraWriteHeaders) = 0;

 protected:
  HttpParseResult parseStart();
  HttpParseResult parseHeader();
  HttpParseResult parseContent();
  HttpParseResult parseChunk();
  HttpParseResult parseChunkFooter();
  HttpParseResult parseTrailing();

  virtual bool parseStatusLine(folly::StringPiece) = 0;
  virtual void parseHeaderLine(folly::StringPiece) = 0;

  void shift();
  char* readLine();
  void checkMessageSize(uint32_t more, bool added);

  char* httpBuf_;
  uint32_t httpPos_;
  uint32_t httpBufLen_;
  uint32_t httpBufSize_;

  HttpParseState state_;

  // for read headers
  bool statusLine_;
  bool finished_;
  bool chunked_;
  folly::F14NodeMap<std::string, std::string> readHeaders_;

  size_t contentLength_;

  // max http message size
  uint32_t maxSize_;
  uint32_t partialMessageSize_;

  apache::thrift::transport::TMemoryBuffer* dataBuf_;

  static const int CRLF_LEN;
};

class THttpClientParser : public THttpParser {
 public:
  THttpClientParser() {}
  THttpClientParser(std::string host, std::string path)
      : host_{std::move(host)},
        path_{std::move(path)},
        userAgent_{"C++/THttpClient"} {}

  void setHost(const std::string& host) { host_ = host; }
  void setPath(const std::string& path) { path_ = path; }
  void resetConnectClosedByServer();
  bool isConnectClosedByServer();
  void setUserAgent(std::string userAgent) { userAgent_ = userAgent; }
  std::unique_ptr<folly::IOBuf> constructHeader(
      std::unique_ptr<folly::IOBuf> buf) override;
  std::unique_ptr<folly::IOBuf> constructHeader(
      std::unique_ptr<folly::IOBuf> buf,
      const folly::F14NodeMap<std::string, std::string>& writeHeaders,
      const folly::F14NodeMap<std::string, std::string>* extraWriteHeaders)
      override;

 protected:
  void parseHeaderLine(folly::StringPiece) override;
  bool parseStatusLine(folly::StringPiece) override;

 private:
  static void appendHeadersToQueue(
      folly::IOBufQueue& queue,
      const folly::F14NodeMap<std::string, std::string>& headersToAppend);

  bool connectionClosedByServer_;
  std::string host_;
  std::string path_;
  std::string userAgent_;
};

} // namespace util
} // namespace thrift
} // namespace apache

#endif // #ifndef THRIFT_TRANSPORT_THTTPPARSER_H_
