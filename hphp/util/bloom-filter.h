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
#ifndef HPHP_BLOOM_FILTER_H
#define HPHP_BLOOM_FILTER_H

#include <boost/dynamic_bitset.hpp>

#include "hphp/util/hash.h"

namespace HPHP {

template<size_t NBITS> struct BloomFilter {
  BloomFilter() : bits_{NBITS} {}
  using T = const void*;
  static size_t h1(size_t h) { return h % NBITS; }
  static size_t h2(size_t h) { return (h / NBITS) % NBITS; }
  void insert(T x) {
    auto h = hash_int64(intptr_t(x));
    bits_.set(h1(h)).set(h2(h));
  }
  bool test(T x) const {
    auto h = hash_int64(intptr_t(x));
    return bits_.test(h1(h)) & bits_.test(h2(h));
  }
  void clear() {
    bits_.reset();
    static_assert(NBITS < (1LL << 32), "");
  }
  private:
  boost::dynamic_bitset<> bits_;
};
}

#endif // HPHP_BLOOM_FILTER_H
