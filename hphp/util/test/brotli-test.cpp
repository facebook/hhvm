/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/brotli.h"
#include "hphp/util/alloc.h"
#include <brotli/decode.h>
#include <brotli/encode.h>
#include <folly/ScopeGuard.h>
#include <folly/Range.h>

#include <gtest/gtest.h>

using namespace std;
using namespace folly;

namespace HPHP {
namespace {
struct BrotliDecDeleter {
  void operator()(BrotliDecoderState *d) {
    BrotliDecoderDestroyInstance(d);
  }
};
}

TEST(BrotliTest, Chunks) {

  vector<string> chunks = {
      "<!DOCTYPE html>\n"
      "<html>\n"
      "    <head>\n"
      "        <meta http-equiv=\"Content-Type\" content=\"text/html; "
      "charset=UTF-8\">\n"
      "        <title>Title</title>\n"
      "    </head>\n"
      "<body>\n"
      "Sending data chunk 1 of 1000 <br />\r\n",
      "Sending data chunk 2 of 1000 <br />\r\n",
      "Sending data chunk 3 of 1000 <br />\r\n",
      "Sending data chunk 4 of 1000 <br />\r\n",
      "Sending data chunk 5 of 1000 <br />\r\n"
      "    </body>\n"
      "</html>\n",
  };


  std::unique_ptr<BrotliDecoderState, BrotliDecDeleter> decState(
      BrotliDecoderCreateInstance(nullptr, nullptr, nullptr));

  ASSERT_TRUE(decState != nullptr);

  BrotliCompressor compressor(BROTLI_MODE_GENERIC, 5, BROTLI_MIN_INPUT_BLOCK_BITS);

  // generate a huge chunk. use block size + 20
  // blockSize is roughly (3 << BROTLI_MIN_INPUT_BLOCK_BITS)
  size_t hugeSize = (3 << BROTLI_MIN_INPUT_BLOCK_BITS) + 20;
  string hugeChunk;
  hugeChunk.reserve(hugeSize);
  string filler = "The quick brown fox jumps over the lazy dog.";
  while (hugeChunk.size() != hugeSize) {
    hugeChunk.append(
        filler, 0, std::min(filler.size(), hugeSize - hugeChunk.size()));
  }
  chunks.emplace_back(std::move(hugeChunk));


  int i = 0;
  for (auto& chunk : chunks) {
    size_t size = chunk.size();
    bool last = ++i == chunks.size();

    auto compressed = compressor.compress(chunk.data(), size, last);
    EXPECT_TRUE(compressed.data() != nullptr);

    size_t decompressedBufferSize = chunk.size();
    auto decompressedBuffer = std::make_unique<uint8_t[]>(decompressedBufferSize);

    uint8_t* decompressedPos = decompressedBuffer.get();
    size_t decompressedAvailable = decompressedBufferSize;

    auto compressedPos = reinterpret_cast<const uint8_t*>(compressed.data());
    auto decompressResult = BrotliDecoderDecompressStream(
        decState.get(),
        &size,
        &compressedPos,
        &decompressedAvailable,
        &decompressedPos,
        nullptr);
    if (decompressResult != BROTLI_DECODER_RESULT_SUCCESS) {
      if (decompressResult == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT && last) {
        // We expect this result for all but the last.
        FAIL() << "Decoder needs more input but input is complete";
      } else if (decompressResult == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
        FAIL() << "Decoder needs more output space";
      } else if (decompressResult == BROTLI_DECODER_RESULT_ERROR) {
        auto errCode = BrotliDecoderGetErrorCode(decState.get());
        FAIL() << "Brotli error: " << BrotliDecoderErrorString(errCode);
      }
    }
    EXPECT_EQ(size, 0);
    EXPECT_EQ(chunk,
              StringPiece(reinterpret_cast<char*>(decompressedBuffer.get()),
                          decompressedPos - decompressedBuffer.get()));
  }
}

}
