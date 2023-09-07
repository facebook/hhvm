<?hh // partial
<<__Native>>
function gmp_abs(mixed $a): mixed;


<<__Native>>
function gmp_add(mixed $a,
                 mixed $b): mixed;


<<__Native>>
function gmp_and(mixed $a,
                 mixed $b): mixed;


<<__Native>>
function gmp_clrbit(inout mixed $a,
                    int $index): void;


<<__Native>>
function gmp_cmp(mixed $a,
                 mixed $b): mixed;


<<__Native>>
function gmp_com(mixed $a): mixed;


<<__Native>>
function gmp_div_q(mixed $a,
                   mixed $b,
                   int $round = GMP_ROUND_ZERO): mixed;


<<__Native>>
function gmp_div_qr(mixed $a,
                    mixed $b,
                    int $round = GMP_ROUND_ZERO): mixed;


<<__Native>>
function gmp_div_r(mixed $a,
                   mixed $b,
                   int $round = GMP_ROUND_ZERO): mixed;


<<__Native>>
function gmp_div(mixed $a,
                 mixed $b,
                 int $round = GMP_ROUND_ZERO): mixed;


<<__Native>>
function gmp_divexact(mixed $a,
                      mixed $b): mixed;


<<__Native>>
function gmp_fact(mixed $a): mixed;


<<__Native>>
function gmp_gcd(mixed $a,
                 mixed $b): mixed;


<<__Native>>
function gmp_gcdext(mixed $a,
                    mixed $b): mixed;


<<__Native>>
function gmp_hamdist(mixed $a,
                     mixed $b): mixed;


<<__Native>>
function gmp_init(mixed $number,
                  int $base = 0): mixed;


<<__Native>>
function gmp_intval(mixed $gmpnumber): int;


<<__Native>>
function gmp_invert(mixed $a,
                    mixed $b): mixed;


<<__Native>>
function gmp_jacobi(mixed $a,
                    mixed $b): mixed;


<<__Native>>
function gmp_legendre(mixed $a,
                      mixed $p): mixed;


<<__Native>>
function gmp_mod(mixed $n,
                 mixed $d): mixed;


<<__Native>>
function gmp_mul(mixed $a,
                 mixed $b): mixed;


<<__Native>>
function gmp_neg(mixed $a): mixed;


<<__Native>>
function gmp_nextprime(mixed $a): mixed;


<<__Native>>
function gmp_or(mixed $a,
                mixed $b): mixed;


<<__Native>>
function gmp_perfect_square(mixed $a): bool;



<<__Native>>
function gmp_popcount(mixed $a): mixed;


<<__Native>>
function gmp_pow(mixed $base,
                 int $exp): mixed;


<<__Native>>
function gmp_powm(mixed $base,
                  mixed $exp,
                  mixed $mod): mixed;


<<__Native>>
function gmp_prob_prime(mixed $a,
                        int $reps = 10): mixed;


<<__Native>>
function gmp_random(int $limiter = 20): void;


<<__Native>>
function gmp_root(mixed $a, int $root): mixed;


<<__Native>>
function gmp_rootrem(mixed $a, int $root): mixed;


<<__Native>>
function gmp_scan0(mixed $a,
                   int $start): mixed;


<<__Native>>
function gmp_scan1(mixed $a,
                   int $start): mixed;


<<__Native>>
function gmp_setbit(inout mixed $a,
                    int $index,
                    bool $bit_on = true): void;


<<__Native>>
function gmp_sign(mixed $a): mixed;


<<__Native>>
function gmp_sqrt(mixed $a): mixed;


<<__Native>>
function gmp_sqrtrem(mixed $a): mixed;


<<__Native>>
function gmp_strval(mixed $a,
                    int $base = 10): mixed;


<<__Native>>
function gmp_sub(mixed $a,
                 mixed $b): mixed;


<<__Native>>
function gmp_testbit(mixed $a,
                     int $index): bool;


<<__Native>>
function gmp_xor(mixed $a,
                 mixed $b): mixed;


<<__NativeData>>
class GMP implements Serializable {
  <<__Native>>
  public function serialize() : string;


  <<__Native>>
  public function unserialize(mixed $data) : void;


  <<__Native>>
  public function __debugInfo() : darray;

}
