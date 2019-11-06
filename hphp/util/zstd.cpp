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

#include <folly/Conv.h>
#include <folly/Format.h>
#include <folly/Memory.h>

#include "hphp/util/alloc.h"
#include "hphp/util/compression-ctx-pool.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
size_t throwIfZstdError(size_t code, const std::string& msg) {
  if (!ZSTD_isError(code)) {
    return code;
  }
  throw std::runtime_error(
      folly::to<std::string>(msg, ZSTD_getErrorName(code)));
}
} // namespace

ZstdCompressor::ContextPool ZstdCompressor::streaming_cctx_pool{};

ZstdCompressor::ContextPool ZstdCompressor::single_shot_cctx_pool{};

bool ZstdCompressor::s_useLocalArena = false;

void ZstdCompressor::zstd_cctx_deleter(ZSTD_CCtx* ctx) {
  size_t error = ZSTD_freeCCtx(ctx);
  throwIfZstdError(error, "Error freeing ZSTD_CCtx! ");
}

ZstdCompressor::ZstdCompressor(int compression_level, bool should_checksum)
    : compression_level_(compression_level), should_checksum_(should_checksum) {
}

void ZstdCompressor::setChecksum(bool should_checksum) {
  should_checksum_ = should_checksum;
}

ZstdCompressor::ContextPool::Ref ZstdCompressor::make_zstd_cctx(bool last) {
  auto ptr = (last ? single_shot_cctx_pool : streaming_cctx_pool).get();
  if (!ptr) {
    throw std::runtime_error("Error allocating ZSTD_CCtx");
  }
  return ptr;
}

StringHolder ZstdCompressor::compress(const void* data,
                                      size_t& len,
                                      bool last) {
  auto const outSize = ZSTD_compressBound(len);
  char* out;
  StringHolder holder;
  if (s_useLocalArena) {
    out = (char*)local_malloc(outSize);
    holder = StringHolder(out, outSize, FreeType::LocalFree);
  } else {
    out = (char*)malloc(outSize);
    holder = StringHolder(out, outSize, FreeType::Free);
  }

  if (!ctx_) {
    ctx_ = make_zstd_cctx(last);
    throwIfZstdError(
        ZSTD_CCtx_reset(ctx_.get(), ZSTD_reset_session_and_parameters),
        "ZSTD_CCtx_reset() Compression context reset failed! ");
    throwIfZstdError(
        ZSTD_CCtx_setParameter(ctx_.get(), ZSTD_c_compressionLevel,
                               compression_level_),
        "ZSTD_CCtx_setParameter() Setting compression level failed! ");
    throwIfZstdError(ZSTD_CCtx_setParameter(
        ctx_.get(), ZSTD_c_checksumFlag, should_checksum_),
        "ZSTD_CCtx_setParameter() Setting checksum failed! ");
  }

  ZSTD_inBuffer inBuf = {data, len, 0};
  ZSTD_outBuffer outBuf = {out, outSize, 0};
  const auto endMode = last ? ZSTD_e_end : ZSTD_e_flush;
  size_t ret;

  do {
    ret = ZSTD_compressStream2(ctx_.get(), &outBuf, &inBuf, endMode);
    throwIfZstdError(ret, "Failed to compress into zstd stream! ");
  } while (inBuf.pos != inBuf.size);

  if (last) {
    ctx_.reset();
  }

  len = outBuf.pos;
  holder.shrinkTo(len);
  return holder;
}
///////////////////////////////////////////////////////////////////////////////
}
