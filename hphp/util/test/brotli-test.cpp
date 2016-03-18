/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include <dec/decode.h>
#include <enc/encode.h>
#include <folly/ScopeGuard.h>
#include <folly/Range.h>

#include <gtest/gtest.h>

using namespace brotli;
using namespace std;
using namespace folly;

namespace HPHP {

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

  BrotliState state;
  BrotliStateInit(&state);
  SCOPE_EXIT{ BrotliStateCleanup(&state); };

  size_t total = 0;
  brotli::BrotliCompressor compressor{BrotliParams()};

  // generate a huge chunk
  size_t hugeSize = compressor.input_block_size() + 20;
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

    auto compressed = compressBrotli(&compressor, chunk.data(), size, last);
    EXPECT_TRUE(compressed != nullptr);
    SCOPE_EXIT { free((void*)compressed); };

    size_t decompressedBufferSize = chunk.size();
    auto decompressedBuffer = new uint8_t[decompressedBufferSize];
    SCOPE_EXIT{ delete[] decompressedBuffer; };

    uint8_t* decompressedPos = decompressedBuffer;
    size_t decompressedAvailable = decompressedBufferSize;

    const uint8_t* compressedPos = (const uint8_t*)compressed;
    BrotliDecompressBufferStreaming(&size,
                                    &compressedPos,
                                    0,
                                    &decompressedAvailable,
                                    &decompressedPos,
                                    &total,
                                    &state);
    EXPECT_EQ(size, 0);
    EXPECT_EQ(chunk,
              StringPiece((char*)decompressedBuffer,
                          decompressedPos - decompressedBuffer));
  }
}

}
