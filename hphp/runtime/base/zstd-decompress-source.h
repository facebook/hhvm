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

#include <cstddef>
#include <cstdint>
#include <string>

#include <folly/Range.h>

#include <zstd.h>

#include "hphp/runtime/base/contiguous-source.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Streaming zstd decompression source for VariableUnserializer. It decompresses
 * a single zstd frame incrementally into a bounded sliding window, so the peak
 * uncompressed footprint is the window (plus one transient string body) rather
 * than the full uncompressed serialized string.
 *
 * All byte access the unserializer performs against the uncompressed stream is
 * routed through these methods, which decompress lazily via ensure(). The
 * compressed input bytes are non-owning (the caller keeps them alive for the
 * source's lifetime, exactly as ContiguousSource does). It is the streaming
 * counterpart of ContiguousSource, exposing the same primitives.
 */
struct ZStdDecompressSource {
  // Selects the contiguous-only fast paths at compile time in the unserializer;
  // false here disables them (the streaming window has no random back-seek).
  static constexpr bool contiguous = false;

  ZStdDecompressSource(const char* str, size_t len);
  ~ZStdDecompressSource();

  ZStdDecompressSource(const ZStdDecompressSource&) = delete;
  ZStdDecompressSource& operator=(const ZStdDecompressSource&) = delete;

  // True once all unconsumed bytes are drained and the frame is fully decoded.
  bool endOfBuffer();

  // Return the byte at the cursor without consuming it; throws at EOF.
  char peek();

  // Return the byte at the cursor and advance past it; throws at EOF.
  char readChar();

  // Return the byte immediately before the cursor (kept-back history).
  char peekBack();

  // Parse a base-10 int64 at the cursor and advance past its digits.
  int64_t readInt();

  // Parse a double at the cursor and advance past it.
  double readDouble();

  // Return a StringPiece of up to n bytes at the cursor and advance past them.
  // The piece points into the sliding window; the caller must copy it before
  // the next call that decompresses (the window may be overwritten/moved).
  folly::StringPiece readStr(unsigned n);

  // Read a byte and throw if it differs from expected.
  void expectChar(char expected);

 private:
  // Guarantee at least k contiguous unconsumed bytes are available at the
  // cursor, compacting the window and/or decompressing more of the frame as
  // needed. Grows the window when k exceeds its capacity. This is the ONLY
  // method that decompresses.
  void ensure(size_t k);

  // int64 text is <= 21 chars; 64 leaves ample room for the trailing separator.
  static constexpr size_t kNumberLookahead = 64;
  // Doubles can be longer; readDouble() grows this on demand if needed.
  static constexpr size_t kDoubleLookahead = 512;
  // History bytes kept behind the cursor so peekBack() stays valid.
  static constexpr size_t kKeepBack = 1;
  // Extra window headroom beyond a requested ensure(k) so the trailing
  // separator(s) after a value are fetched in the same decompress step; this
  // keeps a readStr() piece valid across the immediately following delimiter
  // read (which then does not need to decompress/compact).
  static constexpr size_t kTrailingSlack = 16;
  static constexpr size_t kInitialWindow = 128 * 1024;

  ZSTD_DCtx* m_dctx{nullptr};      // zstd streaming decompression context
  ZSTD_inBuffer m_in;              // cursor over the compressed input
  std::string m_window;            // decompressed sliding window (size = capacity)
  size_t m_pos{0};                 // cursor within the window
  size_t m_len{0};                 // valid decompressed bytes in the window
  bool m_frameComplete{false};     // frame fully decoded (nothing more to pull)
};

///////////////////////////////////////////////////////////////////////////////
}
