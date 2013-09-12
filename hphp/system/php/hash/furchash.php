<?php

/**
 * furchash_hphp_ext
 *
 * @param string $key - The key to hash
 * @param int $len    - Number of bytes to use from the hash
 * @param int $npart  - The number of buckets
 *
 * @return int - A number in the range of 0-(nPart-1)
 */
<<__Native>>
function furchash_hphp_ext(string $key, int $len, int $npart): int;

/**
 * furchash_hphp_ext_supported
 *
 * @return bool - True
 */
function furchash_hphp_ext_supported(): bool {
  return true;
}

