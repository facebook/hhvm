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

#include "hphp/util/zstd.h"
#include <folly/ScopeGuard.h>
#include <folly/Range.h>

#include <gtest/gtest.h>

namespace HPHP {

TEST(ZstdTest, Chunks) {

  std::vector<std::string> chunks = {
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

  ZSTD_DCtx *dctx = ZSTD_createDCtx();
  SCOPE_EXIT{ ZSTD_freeDCtx(dctx); };

  ZstdCompressor compressor(3);

  // generate a huge chunk
  size_t hugeSize = 128 * 1024 + 20;
  std::string hugeChunk;
  hugeChunk.reserve(hugeSize);
  std::string filler = "The quick brown fox jumps over the lazy dog. ";
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
    EXPECT_TRUE(compressed != nullptr);
    SCOPE_EXIT { free((void*)compressed); };

    size_t decompressedBufferSize = chunk.size();
    auto decompressedBuffer = std::make_unique<char[]>(decompressedBufferSize);

    ZSTD_inBuffer inBuf = { compressed, size, 0 };
    ZSTD_outBuffer outBuf = {
        decompressedBuffer.get(), decompressedBufferSize, 0 };

    size_t ret = ZSTD_decompressStream(dctx, &outBuf, &inBuf);
    EXPECT_FALSE(ZSTD_isError(ret)) << ZSTD_getErrorName(ret);

    EXPECT_EQ(outBuf.pos, chunk.size());
    EXPECT_EQ(chunk, std::string((char*)outBuf.dst, outBuf.pos));
  }
}

} // namespace HPHP
