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

#include <thrift/lib/cpp/transport/TBufferTransports.h>
#include <thrift/lib/cpp/transport/THttpServer.h>
#include <thrift/lib/cpp/transport/TTransportException.h>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include <security/lionhead/utils/lib_ftest/ftest.h>
#include <folly/Range.h>

using namespace apache::thrift::transport;
using namespace facebook::security::lionhead::fdp;

namespace {

constexpr size_t kMaxRequestLen = 4096;
constexpr size_t kReadBufferLen = 256;
constexpr size_t kMaxReadCalls = 8;

void parseRequest(std::vector<uint8_t>& request) {
  if (request.empty()) {
    return;
  }

  auto input = std::make_shared<TMemoryBuffer>(
      request.data(),
      static_cast<uint32_t>(request.size()),
      TMemoryBuffer::COPY);
  THttpServer server(input);

  std::array<uint8_t, kReadBufferLen> output{};
  for (size_t i = 0; i < kMaxReadCalls; ++i) {
    if (server.read(output.data(), output.size()) == 0) {
      break;
    }
  }
  server.readEnd();
}

} // namespace

FUZZ(THttpServerTransport, FuzzRequest) {
  auto request = f.vec(
      "http_request",
      d_vec(d_u8(), d_len(kMaxRequestLen))
          .with_examples_from_str({
              folly::StringPiece(
                  "POST /thrift HTTP/1.1\r\nHost: example\r\nContent-Length: "
                  "0\r\n\r\n"),
              folly::StringPiece(
                  "POST /thrift HTTP/1.1\r\nHost: example\r\nContent-Length: "
                  "5\r\n\r\nhello"),
              folly::StringPiece(
                  "POST /thrift HTTP/1.1\r\nTransfer-Encoding: "
                  "chunked\r\n\r\n0\r\n\r\n"),
              folly::StringPiece(
                  "POST /thrift HTTP/1.1\r\nTransfer-Encoding: "
                  "chunked\r\n\r\n4\r\nping\r\n0\r\nX-Trailer: y\r\n\r\n"),
              folly::StringPiece(
                  "POST /thrift HTTP/1.1\r\nTransfer-Encoding: "
                  "chunked\r\nContent-Length: 1\r\n\r\n1;ext=x\r\na\r\n0\r\n\r\n"),
          }));

  try {
    parseRequest(request);
  } catch (const TTransportException&) {
  }
}
