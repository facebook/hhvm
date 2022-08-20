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

#include <thrift/lib/cpp/transport/THttpClient.h>

#include <cstdlib>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <thrift/lib/cpp/transport/TSocket.h>

namespace apache {
namespace thrift {
namespace transport {

using std::string;

const string THttpClient::kAcceptHeader = "Accept";
const string THttpClient::kConnectionHeader = "Connection";
const string THttpClient::kContentLengthHeader = "Content-Length";
const string THttpClient::kContentTypeHeader = "Content-Type";
const string THttpClient::kHostHeader = "Host";
const string THttpClient::kTransferEncodingHeader = "Transfer-Encoding";
const string THttpClient::kUserAgentHeader = "User-Agent";

THttpClient::THttpClient(
    const std::shared_ptr<TTransport>& transport,
    const string& host,
    const string& path)
    : THttpTransport(transport),
      host_(host),
      path_(path),
      connectionClosedByServer_(false) {
  setHeader(kUserAgentHeader, "C++/THttpClient");
  setHeader(kContentTypeHeader, "application/x-thrift");
  setHeader(kAcceptHeader, "application/x-thrift");
}

THttpClient::THttpClient(const string& host, int port, const string& path)
    : THttpTransport(std::shared_ptr<TTransport>(new TSocket(host, port))),
      host_(host),
      path_(path),
      connectionClosedByServer_(false) {
  setHeader(kUserAgentHeader, "C++/THttpClient");
  setHeader(kContentTypeHeader, "application/x-thrift");
  setHeader(kAcceptHeader, "application/x-thrift");
}

THttpClient::~THttpClient() {}

void THttpClient::beginParsingHeaders() {
  responseHeaders_.clear();
}

void THttpClient::parseHeader(char* header) {
  const char* colon = strchr(header, ':');
  if (colon == nullptr || colon == header) {
    return;
  }
  const char* value = colon + 1;
  while (*value == ' ') {
    value++;
  }

  string name(header, colon - header);

  // If the same header appears twice, we simply overwrite it
  responseHeaders_[name] = value;

  if (boost::iequals(name, kTransferEncodingHeader)) {
    if (boost::iequals(value, "chunked")) {
      chunked_ = true;
    }
  } else if (boost::iequals(name, kContentLengthHeader)) {
    chunked_ = false;
    contentLength_ = atoi(value);
  } else if (boost::iequals(name, kConnectionHeader)) {
    if (boost::iequals(value, "close")) {
      connectionClosedByServer_ = true;
    }
  }
}

bool THttpClient::parseStatusLine(char* status) {
  const char* http = status;

  // Skip over the "HTTP/<version>" string.
  // TODO: we should probably check that the version is 1.0 or 1.1
  const char* code = strchr(http, ' ');
  if (code == nullptr) {
    throw TTransportException(string("Bad Status: ") + status);
  }

  // RFC 2616 requires exactly 1 space between the HTTP version and the status
  // code. Skip over it.
  ++code;

  // RFC 2616 requires exactly 1 space between the the status code and reason
  // phrase.
  const char* endCode = strchr(code, ' ');
  if (endCode == nullptr) {
    throw TTransportException(string("Bad Status: ") + status);
  }

  try {
    statusCode_ = boost::lexical_cast<uint16_t>(string(code, endCode - code));
    if (statusCode_ == 100) {
      // HTTP 100 = continue, just keep reading
      return false;
    }
    return true;
  } catch (boost::bad_lexical_cast&) {
    throw TTransportException(string("Bad Status: ") + status);
  }
}

void THttpClient::endParsingHeaders() {
  if (statusCode_ != 200) {
    std::stringstream msg;
    msg << "Server returned HTTP status code " << statusCode_;
    throw TTransportException(msg.str());
  }
}

void THttpClient::flush() {
  if (connectionClosedByServer_) {
    close();
    open();
    connectionClosedByServer_ = false;
  }

  // Fetch the contents of the write buffer
  uint8_t* buf;
  uint32_t len;
  writeBuffer_.getBuffer(&buf, &len);

  // Construct the HTTP header
  std::ostringstream h;
  h << "POST " << path_ << " HTTP/1.1" << CRLF << kHostHeader << ": " << host_
    << CRLF;
  for (const auto& header : requestHeaders_) {
    h << header.first << ": " << header.second << CRLF;
  }
  h << kContentLengthHeader << ": " << len << CRLF << CRLF;
  string header = h.str();

  // Write the header, then the data, then flush
  transport_->write((const uint8_t*)header.c_str(), header.size());
  transport_->write(buf, len);
  transport_->flush();

  // Reset the buffer
  writeBuffer_.resetBuffer();
}

void THttpClient::setHeader(const std::string& name, const std::string& value) {
  requestHeaders_[name] = value;
}

void THttpClient::setUserAgent(const std::string& userAgent) {
  setHeader(kUserAgentHeader, userAgent);
}

} // namespace transport
} // namespace thrift
} // namespace apache
