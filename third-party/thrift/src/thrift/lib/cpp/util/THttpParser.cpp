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

#include <thrift/lib/cpp/util/THttpParser.h>

#include <thrift/lib/cpp/transport/TTransportException.h>

#include <fmt/core.h>
#include <folly/Conv.h>
#include <folly/String.h>
#include <folly/container/F14Map.h>

#include <cassert>
#include <cstdlib>
#include <sstream>

namespace apache::thrift::util {

using namespace std;
using namespace folly;
using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::transport::TTransportException;

const int THttpParser::CRLF_LEN = 2;
const char* const CRLF = "\r\n";

THttpParser::THttpParser()
    : httpPos_(0),
      httpBufLen_(0),
      httpBufSize_(1024),
      state_(HTTP_PARSE_START),
      maxSize_(0x7fffffff),
      dataBuf_(nullptr) {
  httpBuf_ = (char*)std::malloc(httpBufSize_ + 1);
  if (httpBuf_ == nullptr) {
    throw std::bad_alloc();
  }
  httpBuf_[httpBufLen_] = '\0';
}

THttpParser::~THttpParser() {
  if (httpBuf_ != nullptr) {
    std::free(httpBuf_);
    httpBuf_ = nullptr;
  }
}

void THttpParser::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  assert(httpBufLen_ <= httpBufSize_);
  uint32_t avail = httpBufSize_ - httpBufLen_;
  if (avail <= (httpBufSize_ / 4)) {
    httpBufSize_ *= 2;
    httpBuf_ = (char*)std::realloc(httpBuf_, httpBufSize_ + 1);
    if (httpBuf_ == nullptr) {
      throw std::bad_alloc();
    }
  }
  *bufReturn = httpBuf_ + httpBufLen_;
  *lenReturn = httpBufSize_ - httpBufLen_;
}

bool THttpParser::readDataAvailable(size_t len) {
  assert(httpBufLen_ + len <= httpBufSize_);
  httpBufLen_ += len;
  httpBuf_[httpBufLen_] = '\0';

  while (true) {
    THttpParser::HttpParseResult result;
    switch (state_) {
      case HTTP_PARSE_START:
        result = parseStart();
        break;

      case HTTP_PARSE_HEADER:
        result = parseHeader();
        break;

      case HTTP_PARSE_CHUNK:
        result = parseChunk();
        break;

      case HTTP_PARSE_CONTENT:
        result = parseContent();
        break;

      case HTTP_PARSE_CHUNKFOOTER:
        result = parseChunkFooter();
        break;

      case HTTP_PARSE_TRAILING:
        result = parseTrailing();
        break;

      default:
        throw TTransportException("Unknown state");
    }

    if (result == HTTP_PARSE_RESULT_CONTINUE) {
      if (state_ == HTTP_PARSE_START) {
        // parse the whole message
        return true;
      }
    } else {
      // need read more data
      assert(result == HTTP_PARSE_RESULT_BLOCK);
      assert(httpBufLen_ >= httpPos_);
      checkMessageSize(httpBufLen_ - httpPos_, false);
      return false;
    }
  }
}

int THttpParser::getMinBytesRequired() {
  size_t avail;
  switch (state_) {
    case HTTP_PARSE_START:
      return 0;

    case HTTP_PARSE_HEADER:
    case HTTP_PARSE_CHUNK:
    case HTTP_PARSE_CHUNKFOOTER:
    case HTTP_PARSE_TRAILING:
      // Don't know exactly how much we'll need, but at least 1 more byte
      return 1;

    case HTTP_PARSE_CONTENT:
      CHECK_LE(httpPos_, httpBufLen_);
      avail = httpBufLen_ - httpPos_;
      return std::max<int>(0, contentLength_ - avail);
  }

  throw TTransportException("Unknown state");
}

char* THttpParser::readLine() {
  char* eol = nullptr;
  eol = strstr(httpBuf_ + httpPos_, CRLF);
  if (eol != nullptr) {
    *eol = '\0';
    char* line = httpBuf_ + httpPos_;
    uint32_t oldHttpPos = httpPos_;
    httpPos_ = (eol - httpBuf_) + CRLF_LEN;
    assert(httpPos_ >= oldHttpPos);
    checkMessageSize(httpPos_ - oldHttpPos, true);
    assert(httpPos_ <= httpBufLen_);
    return line;
  } else {
    shift();
    return nullptr;
  }
}

void THttpParser::checkMessageSize(uint32_t more, bool added) {
  uint32_t messageSize = partialMessageSize_ + more;
  if (messageSize > maxSize_) {
    T_ERROR("THttpParser: partial message size of %d rejected", messageSize);
    throw TTransportException(
        TTransportException::CORRUPTED_DATA,
        "rejected overly large  http message");
  }
  if (added) {
    partialMessageSize_ = messageSize;
  }
}

void THttpParser::shift() {
  assert(httpBufLen_ >= httpPos_);
  if (httpBufLen_ > httpPos_) {
    // Shift down remaining data and read more
    uint32_t length = httpBufLen_ - httpPos_;
    memmove(httpBuf_, httpBuf_ + httpPos_, length);
    httpBufLen_ = length;
  } else {
    httpBufLen_ = 0;
  }
  httpPos_ = 0;
  httpBuf_[httpBufLen_] = '\0';
}

THttpParser::HttpParseResult THttpParser::parseStart() {
  contentLength_ = 0;
  chunked_ = false;

  statusLine_ = true;
  finished_ = false;
  readHeaders_.clear();

  partialMessageSize_ = 0;

  state_ = THttpParser::HTTP_PARSE_HEADER;
  return THttpParser::HTTP_PARSE_RESULT_CONTINUE;
}

THttpParser::HttpParseResult THttpParser::parseHeader() {
  // Loop until headers are finished
  while (true) {
    const auto lineStr = readLine();

    // No line is found, need wait for more data.
    if (lineStr == nullptr) {
      return HTTP_PARSE_RESULT_BLOCK;
    }

    const folly::StringPiece line = lineStr;

    if (line.empty()) {
      if (finished_) {
        // go to the next state
        if (chunked_) {
          state_ = THttpParser::HTTP_PARSE_CHUNK;
        } else {
          state_ = THttpParser::HTTP_PARSE_CONTENT;
        }
        return THttpParser::HTTP_PARSE_RESULT_CONTINUE;
      } else {
        // Must have been an HTTP 100, keep going for another status line
        statusLine_ = true;
      }
    } else {
      if (statusLine_) {
        statusLine_ = false;
        finished_ = parseStatusLine(line);
      } else {
        parseHeaderLine(line);
      }
    }
  }
}

THttpParser::HttpParseResult THttpParser::parseChunk() {
  char* line = readLine();
  if (line == nullptr) {
    return THttpParser::HTTP_PARSE_RESULT_BLOCK;
  }

  char* semi = strchr(line, ';');
  if (semi != nullptr) {
    *semi = '\0';
  }
  uint32_t size = 0;
  sscanf(line, "%x", &size);
  if (size == 0) {
    state_ = THttpParser::HTTP_PARSE_CHUNKFOOTER;
  } else {
    contentLength_ = size;
    state_ = THttpParser::HTTP_PARSE_CONTENT;
  }
  return THttpParser::HTTP_PARSE_RESULT_CONTINUE;
}

THttpParser::HttpParseResult THttpParser::parseChunkFooter() {
  // End of data, read footer lines until a blank one appears
  while (true) {
    char* line = readLine();
    if (line == nullptr) {
      return THttpParser::HTTP_PARSE_RESULT_BLOCK;
    }
    if (strlen(line) == 0) {
      state_ = THttpParser::HTTP_PARSE_START;
      break;
    }
  }
  return THttpParser::HTTP_PARSE_RESULT_CONTINUE;
}

THttpParser::HttpParseResult THttpParser::parseContent() {
  assert(httpPos_ <= httpBufLen_);
  size_t avail = httpBufLen_ - httpPos_;
  if (avail > 0 && avail >= contentLength_) {
    // copy all of them to the data buf
    assert(dataBuf_ != nullptr);
    dataBuf_->write((uint8_t*)(httpBuf_ + httpPos_), contentLength_);
    httpPos_ += contentLength_;
    checkMessageSize(contentLength_, true);
    contentLength_ = 0;
    shift();
    if (chunked_) {
      state_ = THttpParser::HTTP_PARSE_TRAILING;
    } else {
      state_ = THttpParser::HTTP_PARSE_START;
    }

    return THttpParser::HTTP_PARSE_RESULT_CONTINUE;
  } else {
    return THttpParser::HTTP_PARSE_RESULT_BLOCK;
  }
}

THttpParser::HttpParseResult THttpParser::parseTrailing() {
  assert(chunked_);
  char* line = readLine();
  if (line == nullptr) {
    return THttpParser::HTTP_PARSE_RESULT_BLOCK;
  } else {
    state_ = THttpParser::HTTP_PARSE_CHUNK;
  }
  return THttpParser::HTTP_PARSE_RESULT_CONTINUE;
}

void THttpClientParser::parseHeaderLine(folly::StringPiece header) {
  const auto colonPos = header.find(':');
  if (colonPos == std::string::npos) {
    return;
  }

  const auto value = folly::ltrimWhitespace(header.subpiece(colonPos + 1));

  readHeaders_.emplace(header.subpiece(0, colonPos).str(), value.str());

  const folly::AsciiCaseInsensitive i{};

  if (header.startsWith("Transfer-Encoding", i)) {
    if (value.endsWith("chunked", i)) {
      chunked_ = true;
    }
  } else if (header.startsWith("Content-Length", i)) {
    chunked_ = false;
    contentLength_ = atoi(value.begin());
  } else if (header.startsWith("Connection", i)) {
    if (header.endsWith("close", i)) {
      connectionClosedByServer_ = true;
    }
  }
}

bool THttpClientParser::parseStatusLine(folly::StringPiece status) {
  const auto badStatus = [&] {
    return TTransportException(fmt::format("Bad Status: {}", status));
  };

  // Skip over the "HTTP/<version>" string.
  // TODO: we should probably check that the version is 1.0 or 1.1
  const auto spacePos = status.find(' ');
  if (spacePos == std::string::npos) {
    throw badStatus();
  }

  // RFC 2616 requires exactly 1 space between the HTTP version and the status
  // code.  Skip over it.
  const auto codeStart = status.subpiece(spacePos + 1);

  // Find the status code.  It must be followed by a space.
  const auto nextSpacePos = codeStart.find(' ');
  if (nextSpacePos == std::string::npos) {
    throw badStatus();
  }
  const auto code = codeStart.subpiece(0, nextSpacePos);

  if (code == "200") {
    // HTTP 200 = OK, we got the response.
    return true;
  }
  if (code == "100") {
    // HTTP 100 = continue, just keep reading.
    return false;
  }

  throw badStatus();
}

void THttpClientParser::resetConnectClosedByServer() {
  connectionClosedByServer_ = false;
}

bool THttpClientParser::isConnectClosedByServer() {
  return connectionClosedByServer_;
}

unique_ptr<IOBuf> THttpClientParser::constructHeader(unique_ptr<IOBuf> buf) {
  folly::F14NodeMap<std::string, std::string> empty;
  return constructHeader(std::move(buf), empty, &empty);
}

unique_ptr<IOBuf> THttpClientParser::constructHeader(
    unique_ptr<IOBuf> buf,
    const folly::F14NodeMap<std::string, std::string>& writeHeaders,
    const folly::F14NodeMap<std::string, std::string>* extraWriteHeaders) {
  IOBufQueue queue;
  queue.append("POST ");
  queue.append(path_);
  queue.append(" HTTP/1.1");
  queue.append(CRLF);
  queue.append("Host: ");
  queue.append(host_);
  queue.append(CRLF);
  queue.append("Content-Type: application/x-thrift");
  queue.append(CRLF);
  queue.append("Accept: application/x-thrift");
  queue.append(CRLF);
  queue.append("User-Agent: ");
  queue.append(userAgent_);
  queue.append(CRLF);
  queue.append("Content-Length: ");
  string contentLen = folly::to<std::string>(buf->computeChainDataLength());
  queue.append(contentLen);
  queue.append(CRLF);

  THttpClientParser::appendHeadersToQueue(queue, writeHeaders);
  if (extraWriteHeaders) {
    THttpClientParser::appendHeadersToQueue(queue, *extraWriteHeaders);
  }

  queue.append(CRLF);

  auto res = queue.move();
  res->prependChain(std::move(buf));
  return res;
}

void THttpClientParser::appendHeadersToQueue(
    folly::IOBufQueue& queue,
    const folly::F14NodeMap<std::string, std::string>& headersToAppend) {
  for (const auto& headerToAppend : headersToAppend) {
    if (headerToAppend.first.find(CRLF) != std::string::npos ||
        headerToAppend.second.find(CRLF) != std::string::npos) {
      throw TTransportException(
          fmt::format(
              "HTTP Headers cannot contain \\r\\n. Header: {}:{}",
              folly::cEscape<std::string>(headerToAppend.first),
              folly::cEscape<std::string>(headerToAppend.second)));
    }
    queue.append(headerToAppend.first);
    queue.append(": ");
    queue.append(headerToAppend.second);
    queue.append(CRLF);
  }
}

} // namespace apache::thrift::util
