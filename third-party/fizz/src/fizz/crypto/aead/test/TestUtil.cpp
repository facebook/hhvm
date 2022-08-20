/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/aead/test/TestUtil.h>

#include <fizz/crypto/aead/IOBufUtil.h>

using namespace folly;

namespace fizz {
namespace test {

std::unique_ptr<folly::IOBuf> createBufExact(size_t len) {
  return IOBuf::takeOwnership(malloc(len), len, (size_t)0);
}

std::unique_ptr<folly::IOBuf> defaultCreator(size_t len, size_t) {
  // Manual allocation to control exact sizing
  return createBufExact(len);
}

// Converts the hex encoded string to an IOBuf.
std::unique_ptr<folly::IOBuf>
toIOBuf(std::string hexData, size_t headroom, size_t tailroom) {
  std::string out;
  CHECK(folly::unhexlify(hexData, out));
  // Manually allocate it to control exact sizing
  size_t bufSize = out.size() + headroom + tailroom;
  char* buf = static_cast<char*>(malloc(bufSize));
  memcpy(buf + headroom, out.data(), out.size());
  auto ret = folly::IOBuf::takeOwnership(buf, bufSize);
  ret->trimStart(headroom);
  ret->trimEnd(tailroom);
  return ret;
}

std::unique_ptr<IOBuf>
chunkIOBuf(std::unique_ptr<IOBuf> input, size_t chunks, BufCreator creator) {
  if (!creator) {
    creator = defaultCreator;
  }
  // create IOBuf chunks
  size_t inputLen = input->computeChainDataLength();
  size_t chunkLen = floor((double)inputLen / (double)chunks);
  std::unique_ptr<IOBuf> chunked;

  for (size_t i = 0; i < chunks - 1; ++i) {
    auto buf = creator(chunkLen, i);
    buf->append(chunkLen);
    if (!chunked) {
      chunked = std::move(buf);
    } else {
      chunked->prependChain(std::move(buf));
    }
  }

  size_t remainLen = inputLen - (chunks - 1) * chunkLen;
  auto remain = creator(remainLen, chunks - 1);
  remain->append(remainLen);
  chunked->prependChain(std::move(remain));

  transformBuffer(
      *input, *chunked, [](uint8_t* out, const uint8_t* in, size_t len) {
        memcpy(out, in, len);
      });

  CHECK_EQ(chunks, chunked->countChainElements());
  return chunked;
}
} // namespace test
} // namespace fizz
