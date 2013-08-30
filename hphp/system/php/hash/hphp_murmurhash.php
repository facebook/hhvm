<?php

/**
 * hphp_murmurhash
 *
 * @param string $key - The key to hash
 * @param int $len    - Number of bytes to use from the key
 * @param int $seed   - The seed to use for hashing
 *
 * @return - The Int64 hash of the first len input characters
 */
<<__Native>>
function hphp_murmurhash(string $key, int $len, int $seed): int;
