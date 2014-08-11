<?php

var_dump(gmp_strval(gmp_or("111111", "2222222")));
var_dump(gmp_strval(gmp_or(123123, 435234)));
var_dump(gmp_strval(gmp_or(555, "2342341123")));
var_dump(gmp_strval(gmp_or(-1, 3333)));
var_dump(gmp_strval(gmp_or(4545, -20)));
var_dump(gmp_strval(gmp_or("test", "no test")));

$n = gmp_init("987657876543456");
var_dump(gmp_strval(gmp_or($n, "34332")));
$n1 = gmp_init("987657878765436543456");
var_dump(gmp_strval(gmp_or($n, $n1)));

var_dump(gmp_or($n, $n1, 1));
var_dump(gmp_or(1));
var_dump(gmp_or(array(), 1));
var_dump(gmp_or(1, array()));
var_dump(gmp_or(array(), array()));

echo "Done\n";
?>
