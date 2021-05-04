<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\_Private;

use namespace HH\Lib\{Math, Str};

function random_string(
  (function (int)[_]: string) $random_bytes,
  int $length,
  ?string $alphabet = null,
)[ctx $random_bytes]: string {
  invariant($length >= 0, 'Expected positive length, got %d', $length);
  if ($length === 0) {
    return '';
  }
  if ($alphabet === null) {
    return $random_bytes($length);
  }
  $alphabet_size = Str\length($alphabet);
  $bits = (int)Math\ceil(Math\log($alphabet_size, 2));
  // I do not expect us to have an alphabet with 2^56 characters. It is still
  // nice to have an upper bound, though, to avoid overflowing $unpacked_data
  invariant(
    $bits >= 1 && $bits <= 56,
    'Expected $alphabet\'s length to be in [2^1, 2^56]',
  );

  $ret = '';
  while ($length > 0) {
    // Generate twice as much data as we technically need. This is like
    // guessing "how many times do I need to flip a coin to get N heads?" I'm
    // guessing probably no more than 2N.
    $urandom_length = (int)Math\ceil(2 * $length * $bits / 8.0);
    $data = $random_bytes($urandom_length);

    $unpacked_data = 0; // The unused, unpacked data so far
    $unpacked_bits = 0; // A count of how many unused, unpacked bits we have
    for ($i = 0; $i < $urandom_length && $length > 0; ++$i) {
      // Unpack 8 bits
      $unpacked_data = ($unpacked_data << 8) | \unpack('C', $data[$i])[1];
      $unpacked_bits += 8;

      // While we have enough bits to select a character from the alphabet, keep
      // consuming the random data
      for (; $unpacked_bits >= $bits && $length > 0; $unpacked_bits -= $bits) {
        $index = ($unpacked_data & ((1 << $bits) - 1));
        $unpacked_data >>= $bits;
        // Unfortunately, the alphabet size is not necessarily a power of two.
        // Worst case, it is 2^k + 1, which means we need (k+1) bits and we
        // have around a 50% chance of missing as k gets larger
        if ($index < $alphabet_size) {
          $ret .= $alphabet[$index];
          --$length;
        }
      }
    }
  }

  return $ret;
}

const string ALPHABET_BASE64 =
  'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=';
const string ALPHABET_BASE64_URL =
  'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_';
