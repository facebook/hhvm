<?hh
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

namespace HH\Lib\_Private\Native {

/* These functions are marked NoDoc for docs.hhvm.com:
 * - they are not intenteded to be used directly, other than by the HSL
 * - they should be moved to HH\Lib\_Private so the API is unstable.
 */

/**
 * Generates pseudorandom integers using the Marsenne Twister algorithm. The
 * result is NOT suitable for cryptographic purposes.
 *
 * @param int $min - The lowest value to be returned down to PHP_INT_MIN.
 * @param int $max - The highest value to be returned up to PHP_INT_MAX.
 *
 * @return int - The pseudo-random integer.
 *
 * @throws InvalidArgumentException - If $min > $max.
 *
 */
<<__Native>>
function pseudorandom_int(int $min = \PHP_INT_MIN, int $max = \PHP_INT_MAX)[leak_safe]: int;

/**
 * Seeds the pseudorandom number generator used by `pseudorandom_int`. This
 * should only be used when reproducibility of the random sequence is required
 * (e.g., for testing).
 *
 * @param int $seed - The value with which to seed the pseudorandom number
 *   generator.
 *
 */
<<__Native>>
function pseudorandom_seed(int $seed): void;

/**
 * Generates cryptographic random integers that are suitable for use where
 * unbiased results are critical (e.g. shuffling a Poker deck).
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
function random_int(int $min = \PHP_INT_MIN, int $max = \PHP_INT_MAX)[leak_safe]: int;

}
