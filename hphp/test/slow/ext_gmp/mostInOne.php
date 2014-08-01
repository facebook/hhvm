<?php
require_once __DIR__ . '/gmp.inc';

//gmp_abs
$abs1 = gmp_abs("274982683358");
$abs2 = gmp_abs("-274982683358");
echo gmp_strval($abs1) . "\n";
echo gmp_strval($abs2) . "\n";

//gmp_add
$sum = gmp_add("123456789012345", "76543210987655");
echo gmp_strval($sum) . "\n";

//gmp_and
$and1 = gmp_and("0xfffffffff4", "0x4");
$and2 = gmp_and("0xfffffffff4", "0x8");
echo gmp_strval($and1) . "\n";
echo gmp_strval($and2) . "\n";

//gmp_clrbit
$clrbit = gmp_init("0xff");
gmp_clrbit($clrbit, 0);
echo gmp_strval($clrbit) . "\n";

//gmp_cmp
$cmp1 = gmp_cmp("1234", "1000"); // greater than
$cmp2 = gmp_cmp("1000", "1234"); // less than
$cmp3 = gmp_cmp("1234", "1234"); // equal to
echo "$cmp1 $cmp2 $cmp3" . "\n";

//gmp_com
$com = gmp_com("1234");
echo gmp_strval($com) . "\n";

//gmp_div_q
$div1 = gmp_div_q("100", "5");
echo gmp_strval($div1) . "\n";
$div2 = gmp_div_q("1", "3");
echo gmp_strval($div2) . "\n";
$div3 = gmp_div_q("1", "3", GMP_ROUND_PLUSINF);
echo gmp_strval($div3) . "\n";
$div4 = gmp_div_q("-1", "4", GMP_ROUND_PLUSINF);
echo gmp_strval($div4) . "\n";
$div5 = gmp_div_q("-1", "4", GMP_ROUND_MINUSINF);
echo gmp_strval($div5) . "\n";

//gmp_div_qr
$a = gmp_init("0x41682179fbf5");
$res = gmp_div_qr($a, "0xDEFE75");
var_dump($res);
printf("Result is: q - %s, r - %s" . PHP_EOL,
        gmp_strval($res[0]),
        gmp_strval($res[1]));

//gmp_div_r
$div = gmp_div_r("105", "20");
echo gmp_strval($div) . "\n";

//gmp_div
$div1 = gmp_div("100", "5");
echo gmp_strval($div1) . "\n";

//gmp_divexact
$div1 = gmp_divexact("10", "2");
echo gmp_strval($div1) . "\n";
$div2 = gmp_divexact("10", "3"); // bogus result
echo gmp_strval($div2) . "\n";

//gmp_fact
$fact1 = gmp_fact(5); // 5 * 4 * 3 * 2 * 1
echo gmp_strval($fact1) . "\n";
$fact2 = gmp_fact(50); // 50 * 49 * 48, ... etc
echo gmp_strval($fact2) . "\n";

//gmp_gcd
$gcd = gmp_gcd("12", "21");
echo gmp_strval($gcd) . "\n";

//gmp_gcdext
$a = gmp_init(12);
$b = gmp_init(21);
$g = gmp_gcd($a, $b);
$r = gmp_gcdext($a, $b);

$check_gcd = (gmp_strval($g) == gmp_strval($r['g']));
$eq_res = gmp_add(gmp_mul($a, $r['s']), gmp_mul($b, $r['t']));
$check_res = (gmp_strval($g) == gmp_strval($eq_res));

if ($check_gcd && $check_res) {
  $fmt = "Solution: %d*%d + %d*%d = %d\n";
  printf($fmt,
         gmp_strval($a),
         gmp_strval($r['s']),
         gmp_strval($b),
         gmp_strval($r['t']),
         gmp_strval($r['g']));
} else {
  echo "Error while solving the equation\n";
}

//gmp_hamdist
$ham1 = gmp_init("1001010011", 2);
$ham2 = gmp_init("1011111100", 2);
echo gmp_hamdist($ham1, $ham2) . "\n";
echo gmp_popcount(gmp_xor($ham1, $ham2)) . "\n";

//gmp_init (althought that's probably tested well by now)
var_dump($a = gmp_init(123456));
var_dump($b = gmp_init("0xFFFFDEBACDFEDF7200"));

//gmp_intval
echo gmp_intval(PHP_INT_MAX) . "\n";
echo gmp_intval(gmp_add(PHP_INT_MAX, 1)) . "\n";
echo gmp_intval(gmp_sub(PHP_INT_MAX, 1)) + 1 . "\n";
echo gmp_strval(gmp_add(PHP_INT_MAX, 1)) . "\n";

//gmp_invert
echo gmp_invert("5", "10"); // no inverse, outputs nothing, result is FALSE
$invert = gmp_invert("5", "11");
echo gmp_strval($invert) . "\n";

//gmp_jacobi
echo gmp_jacobi("1", "3") . "\n";
echo gmp_jacobi("2", "3") . "\n";

//gmp_legendre
echo gmp_legendre("1", "3") . "\n";
echo gmp_legendre("2", "3") . "\n";

//gmp_mod
$mod = gmp_mod("8", "3");
echo gmp_strval($mod) . "\n";

//gmp_mul
$mul = gmp_mul("12345678", "2000");
echo gmp_strval($mul) . "\n";

//gmp_neg
$neg1 = gmp_neg("1");
echo gmp_strval($neg1) . "\n";
$neg2 = gmp_neg("-1");
echo gmp_strval($neg2) . "\n";

//gmp_nextprime
$prime1 = gmp_nextprime(10); // next prime number greater than 10
$prime2 = gmp_nextprime(-1000); // next prime number greater than -1000
echo gmp_strval($prime1) . "\n";
echo gmp_strval($prime2) . "\n";

//gmp_or
$or1 = gmp_or("0xfffffff2", "4");
echo gmp_strval($or1, 16) . "\n";
$or2 = gmp_or("0xfffffff2", "2");
echo gmp_strval($or2, 16) . "\n";

//gmp_perfect_square
var_dump(gmp_perfect_square("9")); // 3 * 3, perfect square
var_dump(gmp_perfect_square("7")); // not a perfect square
// 1234567890 * 1234567890, perfect square
var_dump(gmp_perfect_square("1524157875019052100"));

//gmp_popcount
$pop1 = gmp_init("10000101", 2); // 3 1's
echo gmp_popcount($pop1) . "\n";
$pop2 = gmp_init("11111110", 2); // 7 1's
echo gmp_popcount($pop2) . "\n";


//gmp_pow
$pow1 = gmp_pow("2", 31);
echo gmp_strval($pow1) . "\n";
$pow2 = gmp_pow("0", 0);
echo gmp_strval($pow2) . "\n";
$pow3 = gmp_pow("2", -1); // Negative exp, generates warning
echo gmp_strval($pow3) . "\n";

//gmp_powm
$pow1 = gmp_powm("2", "31", "2147483649");
echo gmp_strval($pow1) . "\n";

//gmp_prob_prime
echo gmp_prob_prime("6") . "\n"; // definitely not a prime
echo gmp_prob_prime("1111111111111111111") . "\n"; // probably a prime
echo gmp_prob_prime("11") . "\n"; // definitely a prime

//gmp_random -- not implemented

//gmp_scan0
// "0" bit is found at position 3. index starts at 0
$s1 = gmp_init("10111", 2);
echo gmp_scan0($s1, 0) . "\n";
// "0" bit is found at position 7. index starts at 5
$s2 = gmp_init("101110000", 2);
echo gmp_scan0($s2, 5) . "\n";

//gmp_scan1
// "0" bit is found at position 3. index starts at 0
$s1 = gmp_init("01000", 2);
echo gmp_scan1($s1, 0) . "\n";
// "0" bit is found at position 7. index starts at 5
$s2 = gmp_init("01000001111", 2);
echo gmp_scan1($s2, 5) . "\n";

//gmp_setbit
$a = gmp_init("2"); //
echo gmp_strval($a), ' -> 0b', gmp_strval($a, 2), "\n";
gmp_setbit($a, 0); // 0b10 now becomes 0b11
echo gmp_strval($a), ' -> 0b', gmp_strval($a, 2), "\n";
$a = gmp_init("0xfd");
echo gmp_strval($a), ' -> 0b', gmp_strval($a, 2), "\n";
gmp_setbit($a, 1); // index starts at 0
echo gmp_strval($a), ' -> 0b', gmp_strval($a, 2), "\n";
$a = gmp_init("0xff");
echo gmp_strval($a), ' -> 0b', gmp_strval($a, 2), "\n";
gmp_setbit($a, 0, false); // clear bit at index 0
echo gmp_strval($a), ' -> 0b', gmp_strval($a, 2), "\n";

//gmp_sign
echo gmp_sign("500") . "\n"; //positive
echo gmp_sign("-500") . "\n"; // negative
echo gmp_sign("0") . "\n"; // zero

//gmp_sqrt
$sqrt1 = gmp_sqrt("9");
$sqrt2 = gmp_sqrt("7");
$sqrt3 = gmp_sqrt("1524157875019052100");
echo gmp_strval($sqrt1) . "\n";
echo gmp_strval($sqrt2) . "\n";
echo gmp_strval($sqrt3) . "\n";

//gmp_sqrtrem
list($sqrt1, $sqrt1rem) = gmp_sqrtrem("9");
list($sqrt2, $sqrt2rem) = gmp_sqrtrem("7");
list($sqrt3, $sqrt3rem) = gmp_sqrtrem("1048576");
echo gmp_strval($sqrt1) . ", " . gmp_strval($sqrt1rem) . "\n";
echo gmp_strval($sqrt2) . ", " . gmp_strval($sqrt2rem) . "\n";
echo gmp_strval($sqrt3) . ", " . gmp_strval($sqrt3rem) . "\n";

//gmp_strval
$a = gmp_init("0x41682179fbf5");
printf("Decimal: %s, 36-based: %s" . PHP_EOL,
       gmp_strval($a),
       gmp_strval($a, 36));

//gmp_sub
$sub = gmp_sub("281474976710656", "4294967296"); // 2^48 - 2^32
echo gmp_strval($sub) . "\n";

//gmp_testbit
$n = gmp_init("1000000");
var_dump(gmp_testbit($n, 1));
gmp_setbit($n, 1);
var_dump(gmp_testbit($n, 1));

//gmp_xor
$xor1 = gmp_init("1101101110011101", 2);
$xor2 = gmp_init("0110011001011001", 2);
$xor3 = gmp_xor($xor1, $xor2);
echo gmp_strval($xor3, 2) . "\n";


//misc


$someBigNumber =
    '5213841469519415116094330572703657595919530921861173819326117'
  . '9310511854807446237996274956735188575272489122793818301194912'
  . '9833673362440656643086021394946395224737190702179860943702772'
  . '9317675238467481846766940513200056812714526356082778577134275'
  . '7789609173637178721468440901265495853710507922796892589235420'
  . '1995611212902196086403441815981362977477130996051870721134978'
  . '0499510597317328160963185950244594553469083026425223082533446'
  . '8503526193118817101000313783533208381420617177669147303598253'
  . '4904287554687311595628638823537875937519577818577805321712';
$a = gmp_init($someBigNumber, 10);
var_dump(gmp_strval($a) == $someBigNumber);
var_dump(gmp_strval($a, 32));


$b = gmp_init(0xFFFFFF, 16);
var_dump(gmp_strval($b));
$c = gmp_init($b);
var_dump(gmp_strval($c, 16));
