<?php

var_dump(gmp_strval(gmp_xor("111111", "2222222")));
var_dump(gmp_strval(gmp_xor(123123, 435234)));
var_dump(gmp_strval(gmp_xor(555, "2342341123")));
var_dump(gmp_strval(gmp_xor(-1, 3333)));
var_dump(gmp_strval(gmp_xor(4545, -20)));
var_dump(gmp_strval(gmp_xor("test", "no test")));

$n = gmp_init("987657876543456");
var_dump(gmp_strval(gmp_xor($n, "34332")));
$n1 = gmp_init("987657878765436543456");
var_dump(gmp_strval(gmp_xor($n, $n1)));

var_dump(gmp_xor($n, $n1, 1));
var_dump(gmp_xor(1));
var_dump(gmp_xor(array(), 1));
var_dump(gmp_xor(1, array()));
var_dump(gmp_xor(array(), array()));

echo "Done\n";
?>
