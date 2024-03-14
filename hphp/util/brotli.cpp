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
#include "hphp/util/configs/server.h"
#include "hphp/util/exception.h"
#include "hphp/util/logger.h"

#include <brotli/encode.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
void *brotlialloc(void * /*opaque*/, size_t size) {
  if (Cfg::Server::BrotliUseLocalArena) {
    return local_malloc(size);
  } else {
    return malloc(size);
  }
}

void brotlifree(void * /*opaque*/, void *address) {
  if (Cfg::Server::BrotliUseLocalArena) {
    local_free(address);
  } else {
    free(address);
  }
}
} // namespace

BrotliCompressor::BrotliCompressor(BrotliEncoderMode mode, uint32_t quality, uint32_t lgWin) {
  if (quality < BROTLI_MIN_QUALITY || quality > BROTLI_MAX_QUALITY) {
    Logger::Info("brotli compression quality (%d) must be within %d..%d, clamping...", quality, BROTLI_MIN_QUALITY, BROTLI_MAX_QUALITY);
    if (quality < BROTLI_MIN_QUALITY) {
      quality = BROTLI_MIN_QUALITY;
    } else {
      quality = BROTLI_MAX_QUALITY;
    }
  }

  if (lgWin < BROTLI_MIN_WINDOW_BITS || lgWin > BROTLI_MAX_WINDOW_BITS) {
    Logger::Info("brotli window size (%d) must be within %d..%d, clamping...", lgWin, BROTLI_MIN_WINDOW_BITS, BROTLI_MAX_WINDOW_BITS);
    if (lgWin < BROTLI_MIN_WINDOW_BITS) {
      lgWin = BROTLI_MIN_WINDOW_BITS;
    } else {
      lgWin = BROTLI_MAX_WINDOW_BITS;
    }
  }

  m_encState.reset(BrotliEncoderCreateInstance(brotlialloc, brotlifree, nullptr));
  if (!m_encState) {
    throw Exception("Failed to create brotli encoder instance");
  }

  if (BrotliEncoderSetParameter(m_encState.get(), BROTLI_PARAM_MODE, mode) != BROTLI_TRUE) {
    throw Exception("Failed to set brotli mode(%d)", static_cast<uint32_t>(mode));
  }

  if (BrotliEncoderSetParameter(m_encState.get(), BROTLI_PARAM_QUALITY, quality) != BROTLI_TRUE) {
    throw Exception("Failed to set quality(%d)", quality);
  }

  if (BrotliEncoderSetParameter(m_encState.get(), BROTLI_PARAM_LGWIN, lgWin) != BROTLI_TRUE) {
    throw Exception("Failed to set lgWin(%d)", lgWin);
  }
}

StringHolder BrotliCompressor::compress(const void* data,
                                        size_t& len,
                                        bool last) {
  // Brotli does not have a utility to compute max size of the buffer
  // for the stream-based API. Below link discusses some numbers
  // and formula where 16MB block would use only 6 extra bytes.
  // For all practical usage we should be fine with 20 bytes.
  // https://github.com/google/brotli/issues/274
  // We should also allow 6 extra bytes for an empty meta-block at
  // the end of each chunk to force "flush".
  size_t availableBytes = len + 30;

  StringHolder available;
  void* availablePtr;
  if (Cfg::Server::BrotliUseLocalArena) {
    availablePtr = local_malloc(availableBytes);
    available = StringHolder(static_cast<char*>(availablePtr),
                             availableBytes,
                             FreeType::LocalFree);
  } else {
    availablePtr = malloc(availableBytes);
    available = StringHolder(static_cast<char*>(availablePtr),
                             availableBytes,
                             FreeType::Free);
  }

  auto buf = static_cast<uint8_t*>(availablePtr);
  size_t inLength = len;
  auto inBuf = static_cast<const uint8_t *>(data);
  size_t remainingOut = availableBytes;

  // This ought to complete in one call
  if (BrotliEncoderCompressStream(
        m_encState.get(),
        last ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_FLUSH,
        &inLength,
        &inBuf,
        &remainingOut,
        &buf,
        nullptr) != BROTLI_TRUE) {
    Logger::Error("Compression encountered an error");
    return nullptr;
  }

  // This means the encoder didn't successfully encode + flush all data.
  if (inLength != 0 ||
      BrotliEncoderHasMoreOutput(m_encState.get())) {
    Logger::Error("Compression didn't flush all data");
    return nullptr;
  }

  len = availableBytes - remainingOut;
  available.shrinkTo(len);

  return available;
}

///////////////////////////////////////////////////////////////////////////////
}
