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
#include <utility>

#include <folly/Range.h>

#include "hphp/util/portability.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Base-10 int64 scan shared by the two unserialization sources (ContiguousSource
// and ZStdDecompressSource). Reads digits (with an optional leading '-') and
// returns the value plus the first non-digit position. Relies, like the callers
// upstream, on the serialized data containing a terminator after the number.
inline std::pair<int64_t, const char*> hh_strtoll_base10(const char* p) {
  int64_t x = 0;
  bool neg = false;
  if (*p == '-') {
    neg = true;
    ++p;
  }
  while (*p >= '0' && *p <= '9') {
    x = (x * 10) + ('0' - *p);
    ++p;
  }
  if (!neg) {
    x = -x;
  }
  return std::pair<int64_t, const char*>(x, p);
}

namespace detail {
// Cold throw helpers shared by the unserialization sources (ContiguousSource and
// ZStdDecompressSource). Kept out-of-line (defined once in contiguous-source.cpp)
// so the inline read primitives that call them stay small and branch-free on the
// hot path.
[[noreturn]] void throwUnexpectedEOB();
[[noreturn]] void throwUnexpectedSep(char expect, char actual);
}

/*
 * Contiguous in-memory unserialization source for VariableUnserializer: a
 * non-owning cursor over the fully-materialized serialized bytes. It is the
 * default (non-streaming) counterpart of ZStdDecompressSource, exposing the
 * same primitives plus contiguous-only accessors (head/begin/end/set) that the
 * speculative fast paths use. Every primitive is a direct pointer operation, so
 * this source adds no overhead over hand-written buffer walking.
 */
struct ContiguousSource {
  // Selects the contiguous-only fast paths at compile time in the unserializer.
  static constexpr bool contiguous = true;

  ContiguousSource(const char* str, size_t len)
    : m_buf(str), m_end(str + len), m_begin(str) {}

  bool endOfBuffer() const { return m_buf >= m_end; }
  char peekBack() const { return m_buf[-1]; }

  // Trivial pointer-walking primitives kept inline so the hot unserialize loop
  // never pays a cross-TU call for them; only the cold throw helpers are
  // out-of-line. readInt/readDouble/readStr stay out-of-line as they pull in
  // zend_strtod/<algorithm> and are not worth inlining into every consumer.
  char peek() const { check(); return *m_buf; }
  char readChar() { check(); return *(m_buf++); }
  void expectChar(char expected) {
    auto const ch = readChar();
    if (UNLIKELY(ch != expected)) detail::throwUnexpectedSep(expected, ch);
  }

  int64_t readInt();
  double readDouble();
  folly::StringPiece readStr(unsigned n);

  // Contiguous-only raw cursor accessors, used by the speculative match/rewind
  // and str->int map fast paths (which cannot run over a streaming window).
  const char* head() const { return m_buf; }
  const char* begin() const { return m_begin; }
  const char* end() const { return m_end; }
  void set(const char* buf, const char* end) { m_buf = buf; m_end = end; }
  void setHead(const char* buf) { m_buf = buf; }

 private:
  void check() const { if (m_buf >= m_end) detail::throwUnexpectedEOB(); }

  const char* m_buf;
  const char* m_end;
  const char* m_begin;
};

///////////////////////////////////////////////////////////////////////////////
}
