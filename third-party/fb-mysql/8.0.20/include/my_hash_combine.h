/* Copyright (c) 2017, 2019, Oracle and/or its affiliates.

The code in this file is copied from Boost 1.63.0
boost/functional/hash/hash.hpp, which contains the following copyright notice:

Copyright 2005-2014 Daniel James.
Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

 Based on Peter Dimov's proposal
 http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2005/n1756.pdf
 issue 6.18._

 This also contains public domain code from MurmurHash. From the
 MurmurHash header:

MurmurHash3 was written by Austin Appleby, and is placed in the public
domain. The author hereby disclaims copyright to this source code. */

/** @file include/my_hash_combine.h
A convenient way to combine two hash values.

It was decided to copy (parts of) the boost::hash_combine() implementation
instead of using it directly (by including <boost/functional/hash.hpp>)
because of the following reasons, raised by Steinar and Tor:

Pros:

 - It solves a real problem (how to hash std::pair).
 - Few dependencies (just type_traits and enable_if).
 - Seems like a reasonable implementation.

Cons:

 - It's more Boost.
 - Doesn't seem to be accepted into C++17, so it's something we'd have to
   drag around for a long time without an easy migration path off it.
 - It solves the problem in a suboptimal way; combining values after
   hash-finalization is going to both hash worse and slower than before it.
   The real way requires an interface change to how std::hash works
   (exposing more internal hasher state). I know people have been working on
   this, but evidently it didn't reach C++17 either.
 - Uses boost::hash instead of std::hash. This is probably the biggest killer
   for me.
 - Can easily be implemented by ourselves by lifting the core parts of the
   Boost implementation. (It's about 20 lines.)

I could go either way, but my immediate thought is probably that we should
copy the Boost implementation into some header, and then prefix it with a
warning saying that you shouldn't use this if you need optimal performance or
hash distribution.

Steinar */

#ifndef MY_HASH_COMBINE_INCLUDED
#define MY_HASH_COMBINE_INCLUDED

#if defined(_MSC_VER)
#define MY_FUNCTIONAL_HASH_ROTL32(x, r) _rotl(x, r)
#else
#define MY_FUNCTIONAL_HASH_ROTL32(x, r) (x << r) | (x >> (32 - r))
#endif /* _MSC_VER */

#include <stdint.h>

template <typename SizeT>
inline void my_hash_combine(SizeT &seed, SizeT value) {
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline void my_hash_combine(uint32_t &h1, uint32_t k1) {
  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  k1 *= c1;
  k1 = MY_FUNCTIONAL_HASH_ROTL32(k1, 15);
  k1 *= c2;

  h1 ^= k1;
  h1 = MY_FUNCTIONAL_HASH_ROTL32(h1, 13);
  h1 = h1 * 5 + 0xe6546b64;
}

inline void my_hash_combine(uint64_t &h, uint64_t k) {
  const uint64_t m = 0xc6a4a7935bd1e995ull;
  const int r = 47;

  k *= m;
  k ^= k >> r;
  k *= m;

  h ^= k;
  h *= m;

  // Completely arbitrary number, to prevent 0's
  // from hashing to 0.
  h += 0xe6546b64;
}

#endif /* MY_HASH_COMBINE_INCLUDED */
