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

#include "hphp/util/assertions.h"
#include "hphp/util/bitops.h"

#include <algorithm>
#include <cstdlib>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

template<size_t N>
multibitset<N>::multibitset() {}

template<size_t N>
multibitset<N>::multibitset(size_t nelms) {
  resize(nelms);
  reset();
}

template<size_t N>
multibitset<N>::~multibitset() {
  free(m_bits);
}

template<size_t N>
void multibitset<N>::resize(size_t nelms) {
  auto const prev_nwords = m_nwords;
  auto const prev_size = m_size;

  auto const nbits = N * nelms;
  auto const whole_words = nbits / 64;
  m_nwords = whole_words + (whole_words * 64 != nbits);
  m_size = nelms;
  m_bits = reinterpret_cast<uint64_t*>(
    realloc(m_bits, m_nwords * sizeof(*m_bits))
  );

  if (prev_nwords < m_nwords) {
    memset(m_bits + prev_nwords, 0,
           (m_nwords - prev_nwords) * sizeof(*m_bits));
  } else if (m_size < prev_size) {
    // f{f,l}s() rely on the out-of-range bits being zeroed.
    if (whole_words != m_nwords) {
      auto const tail_nbits = N * m_size - whole_words * 64;
      auto const tail_mask = (1ull << tail_nbits) - 1;
      m_bits[m_nwords - 1] &= tail_mask;
    }
  }
}

template<size_t N>
void multibitset<N>::reset() {
  memset(m_bits, 0, m_nwords * sizeof(*m_bits));
}

template<size_t N>
multibitset<N>::reference::reference(multibitset<N>& mbs, size_t pos)
  : m_mbs(mbs)
  , m_word(N * pos / 64)
  , m_bit(N * pos - m_word * 64)
{}

template<size_t N>
multibitset<N>::reference::operator uint64_t() const {
  constexpr auto mask = (1ull << N) - 1;

  if (m_bit + N <= 64) {
    return (m_mbs.m_bits[m_word] >> m_bit) & mask;
  }

  auto const head_nbits = 64 - m_bit;
  auto const head_mask = (1ull << head_nbits) - 1;

  auto const head = (m_mbs.m_bits[m_word] >> m_bit) & head_mask;
  auto const tail = m_mbs.m_bits[m_word + 1] << head_nbits;

  return (head | tail) & mask;
}

template<size_t N>
typename multibitset<N>::reference&
multibitset<N>::reference::operator=(uint64_t val) {
  constexpr auto mask = (1ull << N) - 1;
  assertx((val & mask) == val);

  {
    auto& word = m_mbs.m_bits[m_word];
    word = (word & ~(mask << m_bit)) | ((val & mask) << m_bit);
  }

  if (m_bit + N > 64) {
    auto const head_nbits = 64 - m_bit;

    auto& word = m_mbs.m_bits[m_word + 1];
    word = (word & ~(mask >> head_nbits)) | ((val & mask) >> head_nbits);
  }
  return *this;
}

template<size_t N>
uint64_t multibitset<N>::operator[](size_t pos) const {
  assertx(pos < m_size);
  return reference(*const_cast<multibitset<N>*>(this), pos);
}

template<size_t N>
typename multibitset<N>::reference multibitset<N>::operator[](size_t pos) {
  assertx(pos < m_size);
  return reference(*this, pos);
}

template<size_t N>
size_t multibitset<N>::size() const { return m_size; }

template<size_t N>
size_t multibitset<N>::ffs(size_t pos) const {
  assertx(pos < m_size);

  size_t b;
  auto const ref = reference(*const_cast<multibitset<N>*>(this), pos);

  auto const mask = ~((1ull << ref.m_bit) - 1);
  auto const word = m_bits[ref.m_word] & mask;
  if (ffs64(word, b)) return (ref.m_word * 64 + b) / N;

  for (auto d = ref.m_word + 1; d < m_nwords; ++d) {
    if (ffs64(m_bits[d], b)) return (d * 64 + b) / N;
  }
  return npos;
}

template<size_t N>
size_t multibitset<N>::fls(size_t pos) const {
  if (pos == npos) pos = m_size - 1;
  assertx(pos < m_size);

  size_t b;
  // We use a reference to the next position to take care of the case where the
  // N-bit at `pos' straddles a word boundary.
  auto const ref = reference(*const_cast<multibitset<N>*>(this), pos + 1);

  auto const mask = (1ull << ref.m_bit) - 1;
  auto const word = m_bits[ref.m_word] & mask;
  if (fls64(word, b)) return (ref.m_word * 64 + b) / N;

  for (auto d = ref.m_word; d-- > 0; ) {
    if (fls64(m_bits[d], b)) return (d * 64 + b) / N;
  }
  return npos;
}

///////////////////////////////////////////////////////////////////////////////

template<size_t N>
chunked_multibitset<N>::chunked_multibitset(size_t chunk_sz)
  : m_chunk_sz{chunk_sz}
{
  always_assert(m_chunk_sz > 0);
}

template<size_t N>
chunked_multibitset<N>::reference::reference(chunked_multibitset<N>& cmbs,
                                             size_t pos)
  : m_cmbs(cmbs)
  , m_pos(pos)
{}

template<size_t N>
void chunked_multibitset<N>::reset() {
  m_chunks.clear();
  m_highwater = 0;
  m_lowwater = npos;
}

template<size_t N>
multibitset<N>& chunked_multibitset<N>::chunk_for(size_t pos) {
  auto const c = pos / m_chunk_sz;
  if (m_chunks[c].size() == 0) {
    m_chunks[c].resize(m_chunk_sz);
    m_highwater = std::max(c, m_highwater);
    m_lowwater = std::min(c, m_lowwater);
  }
  return m_chunks[c];
}

template<size_t N>
chunked_multibitset<N>::reference::operator uint64_t() const {
  auto const it = m_cmbs.m_chunks.find(m_pos / m_cmbs.m_chunk_sz);
  return it != m_cmbs.m_chunks.end()
    ? it->second[m_pos % m_cmbs.m_chunk_sz]
    : 0;
}

template<size_t N>
typename chunked_multibitset<N>::reference&
chunked_multibitset<N>::reference::operator=(uint64_t val) {
  m_cmbs.chunk_for(m_pos)[m_pos % m_cmbs.m_chunk_sz] = val;
  return *this;
}

template<size_t N>
uint64_t chunked_multibitset<N>::operator[](size_t pos) const {
  return reference(*const_cast<chunked_multibitset<N>*>(this), pos);
}

template<size_t N>
typename chunked_multibitset<N>::reference
chunked_multibitset<N>::operator[](size_t pos) {
  return reference(*this, pos);
}

template<size_t N>
size_t chunked_multibitset<N>::ffs(size_t pos) const {
  {
    auto const c = std::max(pos / m_chunk_sz, m_lowwater);
    auto const it = m_chunks.find(c);
    if (it != m_chunks.end()) {
      auto const res = it->second.ffs(pos % m_chunk_sz);
      if (res != npos) return c * m_chunk_sz + res;
    }
  }

  for (auto c = pos / m_chunk_sz + 1; c <= m_highwater; ++c) {
    auto const it = m_chunks.find(c);
    if (it != m_chunks.end()) {
      auto const res = it->second.ffs();
      if (res != npos) return c * m_chunk_sz + res;
    }
  }
  return npos;
}

template<size_t N>
size_t chunked_multibitset<N>::fls(size_t pos) const {
  {
    auto const c = std::min(pos / m_chunk_sz, m_highwater);
    auto const it = m_chunks.find(c);
    if (it != m_chunks.end()) {
      auto const res = it->second.fls(pos != npos ? pos % m_chunk_sz : npos);
      if (res != npos) return c * m_chunk_sz + res;
    }
  }

  for (auto c = pos / m_chunk_sz; c-- > m_lowwater; ) {
    auto const it = m_chunks.find(c);
    if (it != m_chunks.end()) {
      auto const res = it->second.fls();
      if (res != npos) return c * m_chunk_sz + res;
    }
  }
  return npos;
}

///////////////////////////////////////////////////////////////////////////////

}
