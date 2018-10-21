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

#ifndef incl_HPHP_COMPRESSION_H_
#define incl_HPHP_COMPRESSION_H_

#include <memory>
#include <vector>

#include "hphp/runtime/server/transport.h"
#include "hphp/util/brotli.h"
#include "hphp/util/gzip.h"
#include "hphp/util/zstd.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct GzipCompressor;
struct ZstdCompressor;

struct ResponseCompressor {
  explicit ResponseCompressor(ITransportHeaders* headers);

  /**
   * These functions are callable only before the first call to
   * `compressResponse`. After that point we're committed, and calling these
   * will blow up in your face to make sure you know it.
   */
  void enableCompression();
  void disableCompression();

  /**
   * Tests whether any compression mechanisms are enabled, irrespective of
   * whether the client can accept them.
   */
  bool isCompressionEnabled() const;

  /**
   * Test whether client accepts a certain encoding.
   */
  bool acceptsEncoding(const char *encoding) const;

  /**
   * Only callable after the first `compressResponse` call.
   *
   * Returns whether the response is compressed.
   */
  bool isCompressed() const;

  /**
   * Returns compressed response. If compression failed or would be
   * ineffective, returns StringHolder(nullptr, 0).
   */
  StringHolder compressResponse(const char *data, int len, bool last);

 private:
  enum CompressionType {
    Brotli,
    BrotliChunked,
    Zstd,
    Gzip,
    None,
    Max,
  };
  enum class CompressionDecision {
    NotDecidedYet,
    Decided,
  };

  void decideCompression(int size, bool chunkedEncoding);

  /**
   * Test whether client accepts any encodings.
   */
  bool acceptCompression() const;

  static const char* ENCODING_TYPE_TO_NAME[CompressionType::Max + 1];

  const char* compressionName(CompressionType type);

  StringHolder compressGzip(
      const char *data, int size, bool last);
  StringHolder compressBrotli(
      const char *data, int size, bool last);
  StringHolder compressZstd(
    const char *data, int size, bool last);

  void setResponseHeaders();

  //  0 - disabled
  //  1 - enabled, ini_set("off) allows to disable
  // -1 - disabled, ini_set("on") allows to enable
  int8_t m_compressionEnabled[CompressionType::Max];
  // encoding we decided to use
  CompressionType m_encodingType;
  CompressionDecision m_compressionDecision;

  ITransportHeaders* m_headers;

  bool m_chunkedEncoding;

  std::unique_ptr<GzipCompressor> m_gzipCompressor;
  std::unique_ptr<brotli::BrotliCompressor> m_brotliCompressor;
  std::unique_ptr<ZstdCompressor> m_zstdCompressor;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif  // incl_HPHP_COMPRESSION_H_
