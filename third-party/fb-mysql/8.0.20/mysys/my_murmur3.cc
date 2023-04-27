/* Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_murmur3.cc
  Implementation of 32-bit version of MurmurHash3 - fast non-cryptographic
  hash function with good statistical properties, which is based on public
  domain code by Austin Appleby.
*/

#include "my_murmur3.h"

#include "my_byteorder.h"

/*
  Platform-specific implementations of ROTL32 helper.
*/

#if defined(_MSC_VER)

/* Microsoft Visual Studio supports intrinsic for rotate left. */

#include <stdlib.h>

#define ROTL32(x, y) _rotl(x, y)

/*
  Force inlining of intrinsic even though /Oi option is turned off
  in release builds.
*/
#pragma intrinsic(_rotl)

#else  // !defined(_MSC_VER)

/*
  Many other compilers should be able to optimize the below
  function to single instruction.
*/

inline uint32 rotl32(uint32 x, char r) { return (x << r) | (x >> (32 - r)); }

#define ROTL32(x, y) rotl32(x, y)

#endif  // !defined(_MSC_VER)

/**
  Compute 32-bit version of MurmurHash3 hash for the key.

  @param key   Key for which hash value to be computed.
  @param len   Key length.
  @param seed  Seed for hash computation.

  @note WARNING! Since MurmurHash3 is known to be susceptible to "hash DoS"
        attack it should not be used in any situation where attacker has
        control over key being hashed and thus can cause performance problems
        due to degradation of hash lookup to linear list search.

  @returns Hash value for the key.
*/

uint32 murmur3_32(const uchar *key, size_t len, uint32 seed) {
  const uchar *tail = key + (len - len % 4);

  uint32 h1 = seed;

  /* Constants for magic numbers that are used more than once. */
  const uint32 c1 = 0xcc9e2d51;
  const uint32 c2 = 0x1b873593;

  /* Body: process all 32-bit blocks in the key. */

  for (const uchar *data = key; data != tail; data += 4) {
    uint32 k1 = uint4korr(data);

    k1 *= c1;
    k1 = ROTL32(k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = ROTL32(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
  }

  /* Tail: handle remaining len % 4 bytes. */

  uint32 k1 = 0;

  switch (len % 4) {
    case 3:
      k1 ^= static_cast<uint32>(tail[2]) << 16;
      /* Fall through. */
    case 2:
      k1 ^= static_cast<uint32>(tail[1]) << 8;
      /* Fall through. */
    case 1:
      k1 ^= tail[0];
      k1 *= c1;
      k1 = ROTL32(k1, 15);
      k1 *= c2;
      h1 ^= k1;
  };

  /*
    Finalization mix:
    Add length and force all bits of a hash block to avalanche.
  */

  h1 ^= len;

  h1 ^= h1 >> 16;
  h1 *= 0x85ebca6b;
  h1 ^= h1 >> 13;
  h1 *= 0xc2b2ae35;
  h1 ^= h1 >> 16;

  return h1;
}
