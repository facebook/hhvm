<?php

var_dump(gmp_divexact(1, 1, 1));
var_dump(gmp_divexact());

$r = gmp_divexact("233", "23345555555555555555555555");
var_dump(gmp_strval($r));

$r = gmp_divexact("233", "0");
var_dump(gmp_strval($r));

$r = gmp_divexact("100", "10");
var_dump(gmp_strval($r));

$r = gmp_divexact("1024", "2");
var_dump(gmp_strval($r));

$n = gmp_init("10000000000000000000");
$r = gmp_divexact($n, "2");
var_dump(gmp_strval($r));

$r = gmp_divexact($n, "50");
var_dump(gmp_strval($r));

$n1 = gmp_init("-100000000000000000000000000");
$r = gmp_divexact($n1, $n);
var_dump(gmp_strval($r));

echo "Done\n";
?>
