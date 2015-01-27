<?hh // decl     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

const int GMP_ROUND_ZERO = 0;
const int GMP_ROUND_PLUSINF = 0;
const int GMP_ROUND_MINUSINF = 0;
const int GMP_MSW_FIRST = 0;
const int GMP_LSW_FIRST = 0;
const int GMP_LITTLE_ENDIAN = 0;
const int GMP_BIG_ENDIAN = 0;
const int GMP_NATIVE_ENDIAN = 0;
const string GMP_VERSION = '5.0.1';

function gmp_abs(mixed $a): mixed;

function gmp_add(mixed $a,
                 mixed $b): mixed;

function gmp_and(mixed $a,
                 mixed $b): mixed;

function gmp_clrbit(mixed &$a,
                    int $index): void;

function gmp_cmp(mixed $a,
                 mixed $b): mixed;

function gmp_com(mixed $a): mixed;

function gmp_div_q(
  mixed $a,
  mixed $b,
  int $round = GMP_ROUND_ZERO,
): mixed;

function gmp_div_qr(mixed $a,
                    mixed $b,
                    int $round = GMP_ROUND_ZERO): mixed;

function gmp_div_r(mixed $a,
                   mixed $b,
                   int $round = GMP_ROUND_ZERO): mixed;

function gmp_div(mixed $a,
                 mixed $b,
                 int $round = GMP_ROUND_ZERO): mixed;

function gmp_divexact(mixed $a,
                      mixed $b): mixed;

function gmp_fact(mixed $a): mixed;

function gmp_gcd(mixed $a,
                 mixed $b): mixed;

function gmp_gcdext(mixed $a,
                    mixed $b): mixed;

function gmp_hamdist(mixed $a,
                     mixed $b): mixed;

function gmp_init(mixed $number,
                  int $base = 0): mixed;

function gmp_intval(mixed $gmpnumber): int;

function gmp_invert(mixed $a,
                    mixed $b): mixed;

function gmp_jacobi(mixed $a,
                    mixed $b): mixed;

function gmp_legendre(mixed $a,
                      mixed $p): mixed;

function gmp_mod(mixed $n,
                 mixed $d): mixed;

function gmp_mul(mixed $a,
                 mixed $b): mixed;

function gmp_neg(mixed $a): mixed;

function gmp_nextprime(mixed $a): mixed;

function gmp_or(mixed $a,
                mixed $b): mixed;

function gmp_perfect_square(mixed $a): bool;


function gmp_popcount(mixed $a): mixed;

function gmp_pow(mixed $base,
                 int $exp): mixed;

function gmp_powm(mixed $base,
                  mixed $exp,
                  mixed $mod): mixed;

function gmp_prob_prime(mixed $a,
                        int $reps = 10): mixed;

function gmp_random(int $limiter = 20): void;

function gmp_root(mixed $a, int $root): mixed;

function gmp_rootrem(mixed $a, int $root): mixed;

function gmp_scan0(mixed $a,
                   int $start): mixed;

function gmp_scan1(mixed $a,
                   int $start): mixed;

function gmp_setbit(mixed $a,
                    int $index,
                    bool $bit_on = true): void;

function gmp_sign(mixed $a): mixed;

function gmp_sqrt(mixed $a): mixed;

function gmp_sqrtrem(mixed $a): mixed;

function gmp_strval(mixed $a,
                    int $base = 10): mixed;

function gmp_sub(mixed $a,
                 mixed $b): mixed;

function gmp_testbit(mixed $a,
                     int $index): bool;

function gmp_xor(mixed $a,
                 mixed $b): mixed;
