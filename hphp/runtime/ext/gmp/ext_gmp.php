<?hh

<<__Native>>
function gmp_abs(mixed $a): resource;

<<__Native>>
function gmp_add(mixed $a,
                 mixed $b): resource;

<<__Native>>
function gmp_and(mixed $a,
                 mixed $b): resource;

<<__Native>>
function gmp_clrbit(resource $a,
                    int $index): void;

<<__Native>>
function gmp_cmp(mixed $a,
                 mixed $b): int;

<<__Native>>
function gmp_com(mixed $a): resource;

<<__Native>>
function gmp_div_q(mixed $a,
                   mixed $b,
                   int $round = GMP_ROUND_ZERO): resource;

<<__Native>>
function gmp_div_qr(mixed $a,
                    mixed $b,
                    int $round = GMP_ROUND_ZERO): array;

<<__Native>>
function gmp_div_r(mixed $a,
                   mixed $b,
                   int $round = GMP_ROUND_ZERO): resource;

<<__Native>>
function gmp_divexact(mixed $a,
                      mixed $b): resource;

<<__Native>>
function gmp_fact(mixed $a): resource;

<<__Native>>
function gmp_gcd(mixed $a,
                 mixed $b): resource;

<<__Native>>
function gmp_gcdext(mixed $a,
                    mixed $b): array;

<<__Native>>
function gmp_hamdist(mixed $a,
                     mixed $b): int;

<<__Native>>
function gmp_init(mixed $number,
                  int $base = 0): mixed;

<<__Native>>
function gmp_intval(mixed $gmpnumber): resource;

<<__Native>>
function gmp_invert(mixed $a,
                    mixed $b): resource;

<<__Native>>
function gmp_jacobi(mixed $a,
                    mixed $b): int;

<<__Native>>
function gmp_legendre(mixed $a,
                      mixed $p): int;

<<__Native>>
function gmp_mod(mixed $n,
                 mixed $d): resource;

<<__Native>>
function gmp_mul(mixed $a,
                 mixed $b): resource;

<<__Native>>
function gmp_neg(mixed $a): resource;

<<__Native>>
function gmp_nextprime(mixed $a): resource;

<<__Native>>
function gmp_or(mixed $a,
                mixed $b): resource;

<<__Native>>
function gmp_perfect_square(mixed $a): bool;


<<__Native>>
function gmp_popcount(mixed $a): int;

<<__Native>>
function gmp_pow(mixed $base,
                 int $exp): resource;

<<__Native>>
function gmp_powm(mixed $base,
                  mixed $exp,
                  mixed $mod): resource;

<<__Native>>
function gmp_prob_prime(mixed $a,
                        int $reps = 10): resource;

<<__Native>>
function gmp_scan0(mixed $a,
                   int $start): int;

<<__Native>>
function gmp_scan1(mixed $a,
                   int $start): int;

<<__Native>>
function gmp_setbit(mixed &$a,
                    int $index,
                    bool $bit_on = true): void;

<<__Native>>
function gmp_sign(mixed $a): int;

<<__Native>>
function gmp_sqrt(mixed $a): resource;

//<<__Native>>
//function gmp_sqrtrem(mixed $a): resource;

<<__Native>>
function gmp_strval(mixed $a,
                    int $base = 10): string;

<<__Native>>
function gmp_sub(mixed $a,
                 mixed $b): resource;

//<<__Native>>
//function gmp_testbit(mixed $a,
//                     int $index): bool;

<<__Native>>
function gmp_xor(mixed $a,
                 mixed $b): resource;
