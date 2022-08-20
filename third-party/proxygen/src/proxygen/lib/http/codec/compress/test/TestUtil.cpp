/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/test/TestUtil.h>

#include <proxygen/lib/http/codec/compress/test/TestStreamingCallback.h>

#include <folly/io/Cursor.h>
#include <folly/portability/GTest.h>
#include <fstream>
#include <glog/logging.h>
#include <proxygen/lib/http/codec/compress/Logging.h>

using folly::IOBuf;
using std::ofstream;
using std::string;
using std::unique_ptr;
using std::vector;

namespace proxygen { namespace hpack {

void dumpToFile(const string& filename, const IOBuf* buf) {
  ofstream outfile(filename, ofstream::binary);
  if (buf) {
    const IOBuf* p = buf;
    do {
      outfile.write((const char*)p->data(), p->length());
      p = p->next();
    } while (p->next() != buf);
  }
  outfile.close();
}

void verifyHeaders(vector<HPACKHeader>& headers,
                   vector<HPACKHeader>& decodedHeaders) {
  EXPECT_EQ(headers.size(), decodedHeaders.size());
  std::sort(decodedHeaders.begin(), decodedHeaders.end());
  std::sort(headers.begin(), headers.end());
  if (headers.size() != decodedHeaders.size()) {
    std::cerr << printDelta(decodedHeaders, headers);
    CHECK(false) << "Mismatched headers size";
  }
  EXPECT_EQ(headers, decodedHeaders);
  if (headers != decodedHeaders) {
    std::cerr << printDelta(headers, decodedHeaders);
    CHECK(false) << "Mismatched headers";
  }
}

unique_ptr<IOBuf> encodeDecode(vector<HPACKHeader>& headers,
                               HPACKEncoder& encoder,
                               HPACKDecoder& decoder) {
  unique_ptr<IOBuf> encoded = encoder.encode(headers);
  auto decodedHeaders = hpack::decode(decoder, encoded.get());
  CHECK(!decoder.hasError());

  verifyHeaders(headers, *decodedHeaders);

  // header tables should look the same
  CHECK(encoder.getTable() == decoder.getTable());
  EXPECT_EQ(encoder.getTable(), decoder.getTable());

  return encoded;
}

void encodeDecode(vector<HPACKHeader>& headers,
                  QPACKEncoder& encoder,
                  QPACKDecoder& decoder) {
  auto encoded = encoder.encode(headers, 0, 1);
  TestStreamingCallback cb;
  if (encoded.control) {
    decoder.decodeEncoderStream(std::move(encoded.control));
    encoder.decodeDecoderStream(decoder.encodeInsertCountInc());
  }
  CHECK(encoded.stream);
  auto length = encoded.stream->computeChainDataLength();
  decoder.decodeStreaming(1, std::move(encoded.stream), length, &cb);
  CHECK(!cb.hasError());
  auto decodedHeaders = cb.hpackHeaders();
  verifyHeaders(headers, *decodedHeaders);
  encoder.decodeDecoderStream(decoder.encodeHeaderAck(1));

  // header tables should look the same
  CHECK(encoder.getTable() == decoder.getTable());
  EXPECT_EQ(encoder.getTable(), decoder.getTable());
}

unique_ptr<HPACKDecoder::headers_t> decode(HPACKDecoder& decoder,
                                           const IOBuf* buffer) {
  auto headers = std::make_unique<HPACKDecoder::headers_t>();
  folly::io::Cursor cursor(buffer);
  uint32_t totalBytes = buffer ? cursor.totalLength() : 0;
  TestStreamingCallback cb;
  decoder.decodeStreaming(cursor, totalBytes, &cb);
  if (cb.hasError()) {
    return headers;
  }
  return cb.hpackHeaders();
}

vector<compress::Header> headersFromArray(vector<vector<string>>& a) {
  vector<compress::Header> headers;
  for (auto& ha : a) {
    headers.push_back(compress::Header::makeHeaderForTest(ha[0], ha[1]));
  }
  return headers;
}

vector<compress::Header> basicHeaders() {
  static vector<vector<string>> headersStrings = {
      {":path", "/index.php"},
      {":authority", "www.facebook.com"},
      {":method", "GET"},
      {":scheme", "https"},
      {"Host", "www.facebook.com"},
      {"accept-encoding", "gzip"}};
  static vector<compress::Header> headers = headersFromArray(headersStrings);
  return headers;
}

}} // namespace proxygen::hpack
