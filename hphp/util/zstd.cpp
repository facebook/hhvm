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

#include <folly/Format.h>
#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

ZstdCompressor::ZstdCompressor(int compression_level)
    : compression_level_(compression_level) {}

void ZstdCompressor::zstd_cctx_deleter(ZSTD_CCtx* ctx) {
  size_t err = ZSTD_freeCCtx(ctx);
  if (ZSTD_isError(err)) {
    throw std::runtime_error(folly::sformat(
        "Error freeing ZSTD_CCtx: {}", ZSTD_getErrorName(err)));
  }
}

ZstdCompressor::ZSTD_CCtx_Ptr ZstdCompressor::make_zstd_cctx() {
  auto ptr = ZSTD_CCtx_Ptr(ZSTD_createCCtx());
  if (!ptr) {
    throw std::runtime_error("Error allocating ZSTD_CCtx");
  }
  return ptr;
}

const char* ZstdCompressor::compress(const void* data,
                                     size_t& len,
                                     bool last) {
  auto outSize = ZSTD_compressBound(len);
  auto out = std::make_unique<char[]>(outSize);

  if (!ctx_) {
    if (last) {
      // optimize single segment (avoid copying into intermediate buffers)
      auto ret = ZSTD_compress(out.get(), outSize, data, len, compression_level_);
      if (ZSTD_isError(ret)) return nullptr;
      len = ret;
      return out.release();
    } else {
      ctx_ = make_zstd_cctx();
      ZSTD_initCStream(ctx_.get(), compression_level_);
    }
  }

  ZSTD_inBuffer inBuf = { data, len, 0 };
  ZSTD_outBuffer outBuf = { out.get(), outSize, 0 };

  size_t ret;
  do {
    ret = ZSTD_compressStream(ctx_.get(), &outBuf, &inBuf);
    if (ZSTD_isError(ret)) {
      throw std::runtime_error(folly::sformat(
          "Failed to compress into zstd stream!: {}", ZSTD_getErrorName(ret)));
    }
  } while (inBuf.pos != inBuf.size);

  if (last) {
    ret = ZSTD_endStream(ctx_.get(), &outBuf);
    ctx_.reset();
  } else {
    ret = ZSTD_flushStream(ctx_.get(), &outBuf);
  }

  if (ret != 0) {
    throw std::runtime_error("Failed to flush zstd stream!");
  }

  len = outBuf.pos;

  return out.release();
}

///////////////////////////////////////////////////////////////////////////////
}
