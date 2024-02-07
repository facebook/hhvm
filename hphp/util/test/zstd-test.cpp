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
#include <folly/compression/Compression.h>

#include <gtest/gtest.h>

namespace HPHP {

namespace {
const std::string kTestInput =
    R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur nec rhoncus augue. Proin ut diam tempus, egestas odio nec, placerat metus. Mauris fringilla neque vel ultricies malesuada. Maecenas tempor egestas leo, et imperdiet eros interdum quis. Nunc fringilla nisl a venenatis malesuada. Integer aliquet dolor et pharetra rutrum. Nullam sed elit congue, efficitur lorem a, convallis ligula. Etiam accumsan ligula tempus lacinia ultricies. Aliquam ipsum urna, ultricies non purus a, tempus rhoncus tortor. Quisque a quam est.

Nulla in ornare magna, quis auctor est. Nulla eget lectus a eros laoreet ornare. Mauris et arcu sed elit faucibus elementum sed non odio. In interdum a lacus vel iaculis. Sed feugiat sem at risus dignissim accumsan. Aliquam in est at erat finibus pharetra. Duis volutpat ullamcorper enim eu rhoncus.

Nulla neque mauris, maximus id massa a, interdum tempor urna. Fusce velit tortor, auctor ut ante ac, accumsan condimentum ex. Fusce nec convallis justo. Vestibulum maximus placerat pellentesque. Maecenas diam eros, aliquam quis vestibulum at, interdum et nulla. Morbi euismod ipsum vel hendrerit eleifend. Nunc gravida hendrerit felis ac faucibus. Nunc eget nunc sed purus placerat hendrerit in quis nulla. Nunc ipsum nisi, tempor scelerisque scelerisque nec, faucibus a nisl. Quisque ac nunc nunc. Nullam scelerisque ultricies est, sed elementum ex molestie ut.

Pellentesque aliquam tortor in velit porttitor, eu ullamcorper quam dictum. Morbi et orci ac libero vehicula facilisis. Etiam viverra in risus molestie ornare. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque vulputate ex sit amet urna imperdiet, et dapibus mauris malesuada. Sed sollicitudin sem felis, id gravida libero auctor eu. Nullam sit amet hendrerit nibh. Aenean sed turpis finibus, accumsan massa sit amet, ornare urna. Phasellus consectetur odio enim, quis hendrerit ligula tempus laoreet. Praesent nec sollicitudin massa. Ut nec elit a quam tempor pharetra. Mauris dignissim justo vel ante volutpat rutrum.

Donec pharetra pharetra justo, sit amet consequat velit. Vestibulum volutpat sapien ullamcorper lacinia iaculis. Suspendisse potenti. Etiam mattis tincidunt rutrum. Vivamus ac aliquet neque. Vestibulum vulputate vulputate ullamcorper. Nullam nec dui at elit pharetra vestibulum sed iaculis risus. Curabitur et tortor mollis, eleifend eros eget, lacinia urna. Integer vitae nibh tempus, congue dolor eu, bibendum nunc. Maecenas a sapien ultricies, placerat eros et, luctus quam. Nulla vel efficitur urna, ac viverra mauris. Donec id ultrices mauris, eu consectetur sapien. Integer in tempus augue. Fusce scelerisque tellus libero, tempor consectetur justo lobortis a. Ut urna sem, gravida nec nulla non, dignissim porta arcu.)";
}

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
    EXPECT_TRUE(compressed.data() != nullptr);

    size_t decompressedBufferSize = chunk.size();
    auto decompressedBuffer = std::make_unique<char[]>(decompressedBufferSize);

    ZSTD_inBuffer inBuf = { compressed.data(), size, 0 };
    ZSTD_outBuffer outBuf = {
        decompressedBuffer.get(), decompressedBufferSize, 0 };

    size_t ret = ZSTD_decompressStream(dctx, &outBuf, &inBuf);
    EXPECT_FALSE(ZSTD_isError(ret)) << ZSTD_getErrorName(ret);

    EXPECT_EQ(outBuf.pos, chunk.size());
    EXPECT_EQ(chunk, std::string((char*)outBuf.dst, outBuf.pos));
  }
}

TEST(ZstdTest, ContextReuseSingleShot) {
  auto codec = folly::io::getCodec(folly::io::CodecType::ZSTD);

  size_t len1;
  size_t len2;

  {
    ZstdCompressor compressor(1);
    auto len = kTestInput.size();
    auto out = compressor.compress(kTestInput.data(), len, true);

    EXPECT_NE(out.data(), nullptr);
    EXPECT_NE(len, 0);

    auto compressed = std::string{out.data(), len};
    auto uncompressed = codec->uncompress(compressed);

    EXPECT_EQ(uncompressed, kTestInput);

    len1 = len;
  }

  {
    ZstdCompressor compressor(19);
    auto len = kTestInput.size();
    auto out = compressor.compress(kTestInput.data(), len, true);

    EXPECT_NE(out.data(), nullptr);
    EXPECT_NE(len, 0);

    auto compressed = std::string{out.data(), len};
    auto uncompressed = codec->uncompress(compressed);

    EXPECT_EQ(uncompressed, kTestInput);

    len2 = len;
  }

  EXPECT_LT(len2, len1);
}

TEST(ZstdTest, ContextReuseStreamed) {
  auto codec = folly::io::getCodec(folly::io::CodecType::ZSTD);

  size_t full_len1;
  size_t full_len2;

  {
    ZstdCompressor compressor(1);
    auto data1 = kTestInput.data();
    auto len1 = kTestInput.size() / 2;
    auto data2 = data1 + len1;
    auto len2 = kTestInput.size() - len1;

    auto out1 = compressor.compress(data1, len1, false);

    EXPECT_NE(out1.data(), nullptr);
    EXPECT_NE(len1, 0);
    auto compressed = std::string{out1.data(), len1};

    auto out2 = compressor.compress(data2, len2, true);
    EXPECT_NE(out2.data(), nullptr);
    EXPECT_NE(len2, 0);
    compressed += std::string{out2.data(), len2};

    auto uncompressed = codec->uncompress(compressed);

    EXPECT_EQ(uncompressed, kTestInput);

    full_len1 = compressed.size();
  }

  {
    ZstdCompressor compressor(19);
    auto data1 = kTestInput.data();
    auto len1 = kTestInput.size() / 2;
    auto data2 = data1 + len1;
    auto len2 = kTestInput.size() - len1;

    auto out1 = compressor.compress(data1, len1, false);

    EXPECT_NE(out1.data(), nullptr);
    EXPECT_NE(len1, 0);
    auto compressed = std::string{out1.data(), len1};

    auto out2 = compressor.compress(data2, len2, true);
    EXPECT_NE(out2.data(), nullptr);
    EXPECT_NE(len2, 0);
    compressed += std::string{out2.data(), len2};

    auto uncompressed = codec->uncompress(compressed);

    EXPECT_EQ(uncompressed, kTestInput);

    full_len2 = compressed.size();
  }

  EXPECT_LT(full_len2, full_len1);
}

TEST(ZstdTest, Checksumming) {
  auto codec = folly::io::getCodec(folly::io::CodecType::ZSTD);
  size_t len1;
  size_t len2;

  {
    ZstdCompressor compressor(1);
    auto len = kTestInput.size();
    auto out = compressor.compress(kTestInput.data(), len, true);

    EXPECT_NE(out.data(), nullptr);
    EXPECT_NE(len, 0);

    auto compressed = std::string{out.data(), len};
    auto uncompressed = codec->uncompress(compressed);

    EXPECT_EQ(uncompressed, kTestInput);

    len1 = len;
  }

  {
    ZstdCompressor compressor(1, true);
    auto len = kTestInput.size();
    auto out = compressor.compress(kTestInput.data(), len, true);

    EXPECT_NE(out.data(), nullptr);
    EXPECT_NE(len, 0);

    auto compressed = std::string{out.data(), len};
    auto uncompressed = codec->uncompress(compressed);

    EXPECT_EQ(uncompressed, kTestInput);

    len2 = len;
  }

  EXPECT_LT(len1, len2);
}

TEST(ZstdTest, Superblock) {
  auto codec = folly::io::getCodec(folly::io::CodecType::ZSTD);
  size_t len1;
  size_t len2;

  {
    ZstdCompressor compressor(1, false, 0, kTestInput.size() / 4);
    auto len = kTestInput.size();
    auto out = compressor.compress(kTestInput.data(), len, true);

    EXPECT_NE(out.data(), nullptr);
    EXPECT_NE(len, 0);

    auto compressed = std::string{out.data(), len};
    auto uncompressed = codec->uncompress(compressed);

    EXPECT_EQ(uncompressed, kTestInput);

    len1 = len;
  }

  {
    ZstdCompressor compressor(1, true);
    auto len = kTestInput.size();
    auto out = compressor.compress(kTestInput.data(), len, true);

    EXPECT_NE(out.data(), nullptr);
    EXPECT_NE(len, 0);

    auto compressed = std::string{out.data(), len};
    auto uncompressed = codec->uncompress(compressed);

    EXPECT_EQ(uncompressed, kTestInput);

    len2 = len;
  }

  EXPECT_LT(len1, len2);
}

} // namespace HPHP
