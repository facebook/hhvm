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

#include "hphp/runtime/base/zstd-string-buffer.h"

#include "hphp/util/exception.h"
#include "hphp/util/safe-cast.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
// zstd's maximum block size.
constexpr uint32_t kFlushThreshold = 128 * 1024;
}

ZStdStringBuffer::CCtxPool ZStdStringBuffer::s_cctxPool{};

void ZStdStringBuffer::freeCCtx(ZSTD_CCtx* ctx) {
  ZSTD_freeCCtx(ctx);
}

ZStdStringBuffer::ZStdStringBuffer(int compressionLevel)
  : m_compressionLevel(compressionLevel) {}

ZStdStringBuffer::~ZStdStringBuffer() = default;

void ZStdStringBuffer::setOutputLimit(int maxBytes) {
  m_out.setOutputLimit(maxBytes);
}

void ZStdStringBuffer::compressStaging(bool last) {
  if (!last && m_staging.size() < kFlushThreshold) return;

  if (!m_ctxConfigured) {
    m_cctx = s_cctxPool.get();
    auto const resetRc =
      ZSTD_CCtx_reset(m_cctx.get(), ZSTD_reset_session_and_parameters);
    if (ZSTD_isError(resetRc)) {
      throw Exception("zstd context reset failed: %s",
                      ZSTD_getErrorName(resetRc));
    }
    auto const levelRc = ZSTD_CCtx_setParameter(
      m_cctx.get(), ZSTD_c_compressionLevel, m_compressionLevel);
    if (ZSTD_isError(levelRc)) {
      throw Exception("zstd setParameter failed: %s",
                      ZSTD_getErrorName(levelRc));
    }
    m_zbuf.resize(ZSTD_CStreamOutSize());
    m_ctxConfigured = true;
  }

  ZSTD_inBuffer in{m_staging.data(), m_staging.size(), 0};
  auto const mode = last ? ZSTD_e_end : ZSTD_e_continue;
  size_t ret;
  do {
    ZSTD_outBuffer out{m_zbuf.data(), m_zbuf.size(), 0};
    ret = ZSTD_compressStream2(m_cctx.get(), &out, &in, mode);
    if (ZSTD_isError(ret)) {
      throw Exception("zstd compression failed: %s", ZSTD_getErrorName(ret));
    }
    if (out.pos > 0) {
      m_out.append(m_zbuf.data(), safe_cast<int>(out.pos));
    }
  } while (in.pos < in.size || (last && ret != 0));

  m_staging.clear();
}

OptString ZStdStringBuffer::detach() {
  compressStaging(/*last=*/true);
  return m_out.detach();
}

///////////////////////////////////////////////////////////////////////////////
}
