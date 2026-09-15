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

#pragma once

#include <string>
#include <utility>

#include <zstd.h>

#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/util/assertions.h"
#include "hphp/util/compression-ctx-pool.h"
#include "hphp/util/portability.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Output buffer with the same shape as StringBuffer that streams every appended
 * byte through zstd and accumulates only the compressed output. The detached
 * String is a single, self-contained zstd frame that decompresses back to the
 * bytes that were appended. Used only by ZStdVariableSerializer; the
 * uncompressed serialize paths use a plain StringBuffer directly. The
 * streaming variant is selected by serialize()'s zstd option.
 */
struct ZStdStringBuffer {
  static constexpr int kDefaultCompressionLevel = 6;

  explicit ZStdStringBuffer(int compressionLevel = kDefaultCompressionLevel);
  ~ZStdStringBuffer();

  ZStdStringBuffer(const ZStdStringBuffer&) = delete;
  ZStdStringBuffer& operator=(const ZStdStringBuffer&) = delete;

  /*
   * Append bytes: they are staged and a zstd block is flushed once the staging
   * buffer reaches kFlushThreshold. The forwarding template covers every
   * StringBuffer::append() overload (char, int64_t, const char*, StringPiece,
   * String, ...).
   */
  template <typename... Args>
  ALWAYS_INLINE void append(Args&&... args) {
    m_staging.append(std::forward<Args>(args)...);
    compressStaging(/*last=*/false);
  }

  /*
   * size() / resize() exist only so the templated appendJsonEscape() compiles
   * when instantiated for this buffer type. This buffer is only ever used for
   * Type::Serialize, never JSON, so resize() is never actually reached
   * (compressed bytes have already left the staging buffer and cannot be
   * rewound).
   */
  uint32_t size() const { return m_staging.size(); }
  void resize(uint32_t) { not_reached(); }

  /*
   * Bound the size of the compressed frame handed back to the caller. As with
   * StringBuffer, exceeding the limit throws StringBufferLimitException.
   */
  void setOutputLimit(int maxBytes);

  /*
   * Return the compressed output, writing the zstd end-of-frame marker first.
   * Call exactly once.
   */
  OptString detach();

 private:
  static void freeCCtx(ZSTD_CCtx* ctx);
  using CCtxPool =
      CompressionContextPool<ZSTD_CCtx, ZSTD_createCCtx, freeCCtx>;
  static CCtxPool s_cctxPool;

  // Compress whatever is staged. A no-op when last is false and the staging
  // buffer is below kFlushThreshold; otherwise feeds the staged bytes to zstd
  // (ZSTD_e_continue), or finalizes the frame (ZSTD_e_end) when last is true.
  void compressStaging(bool last);

  int m_compressionLevel;
  bool m_ctxConfigured{false};
  StringBuffer m_staging;  // uncompressed bytes awaiting the next flush
  StringBuffer m_out;      // accumulated compressed output
  CCtxPool::Ref m_cctx;    // zstd context, lazily acquired on first flush
  std::string m_zbuf;      // reusable zstd output scratch
};

///////////////////////////////////////////////////////////////////////////////
}
