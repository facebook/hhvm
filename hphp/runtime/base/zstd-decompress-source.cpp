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

#include "hphp/runtime/base/zstd-decompress-source.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "hphp/util/assertions.h"
#include "hphp/util/exception.h"
#include "hphp/util/portability.h"

#include "hphp/zend/zend-strtod.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// The EOB/separator throw helpers are shared with ContiguousSource; both source
// implementations use the single definition in contiguous-source.cpp.
using detail::throwUnexpectedEOB;
using detail::throwUnexpectedSep;

ZStdDecompressSource::ZStdDecompressSource(const char* str, size_t len) {
  m_dctx = ZSTD_createDCtx();
  if (!m_dctx) {
    throw Exception("zstd decompression context creation failed");
  }
  m_in = ZSTD_inBuffer{str, len, 0};
  // size() reflects the window capacity; m_len tracks valid bytes within it.
  m_window.resize(kInitialWindow);
}

ZStdDecompressSource::~ZStdDecompressSource() {
  if (m_dctx) {
    ZSTD_freeDCtx(m_dctx);
  }
}

void ZStdDecompressSource::ensure(size_t k) {
  if (m_len - m_pos >= k) return;

  // Compact: preserve up to kKeepBack bytes of already-consumed history (so
  // peekBack() keeps working) and slide the unconsumed tail to the front.
  auto const keep = std::min(m_pos, kKeepBack);
  auto const tailStart = m_pos - keep;
  if (tailStart > 0) {
    auto const tailLen = m_len - tailStart;
    std::memmove(&m_window[0], &m_window[tailStart], tailLen);
    m_len = tailLen;
    m_pos = keep;
  }

  // Ensure the window can hold the kept history + k unconsumed bytes + slack.
  auto const needed = m_pos + k + kTrailingSlack;
  if (m_window.size() < needed) {
    m_window.resize(std::max(needed, m_window.size() * 2));
  }

  // Decompress into the free tail until we have k unconsumed bytes or the frame
  // is fully drained. A single ZSTD_decompressStream call fills the whole free
  // tail, so trailing separators past k are pulled in alongside k. The resize
  // above guarantees window.size() >= m_pos + k + kTrailingSlack, so while the
  // loop condition holds (m_len < m_pos + k) there is always free tail to fill.
  while (m_len - m_pos < k && !m_frameComplete) {
    assertx(m_len < m_window.size());
    auto const inPosBefore = m_in.pos;
    auto const outPosBefore = m_len;
    ZSTD_outBuffer out{&m_window[0], m_window.size(), m_len};
    auto const ret = ZSTD_decompressStream(m_dctx, &out, &m_in);
    if (ZSTD_isError(ret)) {
      throw Exception("zstd decompression failed: %s", ZSTD_getErrorName(ret));
    }
    m_len = out.pos;
    if (ret == 0) {
      // Frame fully decoded and flushed.
      m_frameComplete = true;
      break;
    }
    if (m_in.pos == inPosBefore && m_len == outPosBefore) {
      // No forward progress: compressed input is exhausted but the frame is
      // incomplete (truncated). Stop; callers observe this as EOB.
      break;
    }
  }
}

bool ZStdDecompressSource::endOfBuffer() {
  if (m_pos < m_len) return false;
  ensure(1);
  return m_pos >= m_len;
}

char ZStdDecompressSource::peek() {
  ensure(1);
  if (m_pos >= m_len) throwUnexpectedEOB();
  return m_window[m_pos];
}

char ZStdDecompressSource::readChar() {
  auto const c = peek();
  ++m_pos;
  return c;
}

char ZStdDecompressSource::peekBack() {
  return m_window[m_pos - 1];
}

int64_t ZStdDecompressSource::readInt() {
  ensure(kNumberLookahead);
  if (m_pos >= m_len) throwUnexpectedEOB();
  auto const base = m_window.data() + m_pos;
  auto const r = hh_strtoll_base10(base);
  m_pos += static_cast<size_t>(r.second - base);
  return r.first;
}

double ZStdDecompressSource::readDouble() {
  auto lookahead = kDoubleLookahead;
  while (true) {
    ensure(lookahead);
    if (m_pos >= m_len) throwUnexpectedEOB();
    auto const base = m_window.data() + m_pos;
    auto const avail = m_len - m_pos;
    const char* endptr = nullptr;
    auto const r = zend_strtod(base, &endptr);
    auto const consumed = static_cast<size_t>(endptr - base);
    // If the parse ran to the end of the available bytes there may be more
    // digits still in the frame; widen the window and retry.
    if (consumed >= avail && !m_frameComplete) {
      lookahead *= 2;
      continue;
    }
    m_pos += consumed;
    return r;
  }
}

folly::StringPiece ZStdDecompressSource::readStr(unsigned n) {
  ensure(n);
  auto const avail = std::min(static_cast<size_t>(n), m_len - m_pos);
  auto const piece = folly::StringPiece(m_window.data() + m_pos, avail);
  m_pos += avail;
  return piece;
}

void ZStdDecompressSource::expectChar(char expected) {
  auto const ch = readChar();
  if (UNLIKELY(ch != expected)) {
    throwUnexpectedSep(expected, ch);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
