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
#include <folly/String.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Struct to hold encoding : param pairs
 */
struct CompressionEncodingPair {
  std::string encoding;
  std::string params;
};

/*
 * Parse a request into a compressionEncodingHeader struct
 */
std::vector<CompressionEncodingPair> parseEncoding(folly::StringPiece header);

/*
 * Struct holding a param and it's value w/ string (e.g. param="q", value="1.0")
 */
 struct CompressionEncodingParam {
   std::string param;
   std::string value;
 };

/*
 * Function to parse params into a vector of CompressionEncodingParam.
 * Consumer should do determine type based on their encoding param specs.
 */
std::vector<CompressionEncodingParam> parseParams(const std::string& allParams);

/*
 * Test whether a request indicates it accepts a certain encoding.
 */
bool acceptsEncoding(ITransportHeaders *headers, const char *encoding);

/**
 * The ResponseCompressor interface is an abstraction that different compression
 * schemes can implement to interface with the transport layer.
 *
 * A ResponseCompressor object is only valid for a single compression stream
 * (whether single-shot or chunked).
 */
struct ResponseCompressor {
  explicit ResponseCompressor(ITransportHeaders *headers);

  virtual ~ResponseCompressor() = default;

  /**
   * These functions are callable only before the first call to
   * `compressResponse`. After that point we're committed, and calling these
   * will blow up in your face to make sure you know it.
   */
  virtual void enable() = 0;
  virtual void disable() = 0;

  /**
   * Tests whether any of the implemented compression mechanisms are enabled,
   * irrespective of whether the client can accept them.
   */
  virtual bool isEnabled() const = 0;

  /**
   * Tests whether the client accepts this compression scheme.
   *
   * Note isEnabled() && isAccepted() does not imply isCompressed().
   */
  virtual bool isAccepted();

  /**
   * To support experimentation and safe roll-out, compressors can indicate
   * that they would like to receive shadowed traffic, even if they aren't
   * selected to be used.
   */
  virtual bool shouldShadow() const { return false; }

  /**
   * Only callable after the first `compressResponse` call.
   *
   * Returns whether the response is compressed.
   */
  virtual bool isCompressed() const = 0;

  /**
   * The name of the encoding this compressor will use.
   */
  virtual const char* encodingName() const = 0;

  /**
   * Returns compressed response. If compression failed or would be
   * ineffective, returns StringHolder(nullptr, 0).
   */
  virtual StringHolder compressResponse(const char *data, int len, bool last) = 0;

  /**
   * When the manager knows it will use this compressor, it will call this
   * method so that the compressor has an opportunity to set any additional
   * response headers needed (the manager sets "Vary" and "Content-Encoding").
   */
  virtual void setResponseHeaders() {}

 protected:
  ITransportHeaders * const m_headers;
  int8_t m_accepted{-1}; // cached value: -1 unknown, 0 false, 1 true.
};

struct GzipResponseCompressor : ResponseCompressor {
  explicit GzipResponseCompressor(ITransportHeaders *headers);
  void enable() override;
  void disable() override;
  bool isEnabled() const override { return m_enabled; }
  bool isCompressed() const override { return m_compressor != nullptr; }
  const char* encodingName() const override { return "gzip"; }
  StringHolder compressResponse(const char *data, int len, bool last) override;

 private:
  GzipCompressor* getCompressor();

  int8_t m_enabled;
  std::unique_ptr<GzipCompressor> m_compressor;
};

struct BrotliResponseCompressor : ResponseCompressor {
  explicit BrotliResponseCompressor(ITransportHeaders *headers);
  void enable() override;
  void disable() override;
  bool isEnabled() const override { return m_enabled || m_chunkedEnabled; }
  bool isCompressed() const override { return m_compressor != nullptr; }
  const char* encodingName() const override { return "br"; }
  StringHolder compressResponse(const char *data, int len, bool last) override;

 private:
  BrotliCompressor* getCompressor(int size, bool last);

  int8_t m_enabled;
  int8_t m_chunkedEnabled;
  std::unique_ptr<BrotliCompressor> m_compressor;
};

struct ZstdResponseCompressor : ResponseCompressor {
  explicit ZstdResponseCompressor(ITransportHeaders *headers);
  void enable() override;
  void disable() override;
  bool isEnabled() const override { return m_enabled; }
  bool isCompressed() const override { return m_compressor != nullptr; }
  const char* encodingName() const override { return "zstd"; }
  StringHolder compressResponse(const char *data, int len, bool last) override;

 private:
  ZstdCompressor* getCompressor();

  int8_t m_enabled;
  std::unique_ptr<ZstdCompressor> m_compressor;
};

struct ResponseCompressorManager {
  explicit ResponseCompressorManager(ITransportHeaders *headers);

  // for testing
  explicit ResponseCompressorManager(
      ITransportHeaders *headers,
      std::vector<std::unique_ptr<ResponseCompressor>> impls);

  using Factory = std::unique_ptr<ResponseCompressor>(*)(ITransportHeaders *headers);

  /**
   * Adds another response compressor implementation that will be considered
   * for use in compressing responses. The DispatchedResponseCompressor will
   * use the first response compressor (ordered by priority high to low) that
   * is enabled, accepted, and successfully compresses the response.
   *
   * The above implementations are included by default with these priorities:
   * - Zstd, 30
   * - Brotli, 20
   * - Gzip, 10
   *
   * Not thread safe, call only at static-init time.
   */
  static void addImpl(Factory implFactory, int priority);

  void enable();
  void disable();
  bool isEnabled() const;
  bool isAccepted();
  bool isCompressed() const;
  const char* encodingName() const;
  StringHolder compressResponse(const char *data, int len, bool last);

 private:
  enum class CompressionDecision {
    NotDecidedYet,
    Decided,
  };

  void setResponseHeaders();

  ITransportHeaders * const m_headers;
  CompressionDecision m_compressionDecision;
  // stored in preference order, more preferred first.
  std::vector<std::unique_ptr<ResponseCompressor>> m_impls;
  ResponseCompressor* m_selectedImpl;
  std::vector<ResponseCompressor*> m_shadowImpls;
  bool m_chunkedEncoding;

  static std::vector<std::unique_ptr<ResponseCompressor>> makeImpls(
      ITransportHeaders *headers);

  // unique_ptrs are correctly empty-init'ed in the first pass of static
  // initialization, whereas vectors are not. Using a unique_ptr makes this
  // safe to access from other static initialization contexts.
  static std::unique_ptr<std::vector<std::pair<Factory, int>>> s_implFactories;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif  // incl_HPHP_COMPRESSION_H_
