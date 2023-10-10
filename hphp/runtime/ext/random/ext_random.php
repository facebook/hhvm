<?hh

/**
 * Generates cryptographically secure pseudo-random bytes that are suitable for
 *   use in cryptography when generating salts, keys and initialization vectors.
 *
 * @param int $length - The length of the random string in bytes.
 *
 * @return string - The crypto-secure random bytes in binary format.
 *
 * @throws Exception - If generating sufficiently random data fails.
 *
 */
<<__Native>>
function random_bytes(int $length)[leak_safe]: string;

/**
 * Generates cryptographic random integers that are suitable for use where
 *   unbiased results are critical (e.g. shuffling a Poker deck).
 *
 * @param int $min - The lowest value to be returned down to PHP_INT_MIN.
 * @param int $max - The highest value to be returned up to PHP_INT_MAX.
 *
 * @return int - The crypto-secure random integer.
 *
 * @throws Exception - If generating sufficiently random data fails.
 * @throws Error - If $min > $max.
 *
 */
<<__Native>>
function random_int(int $min, int $max)[leak_safe]: int;
