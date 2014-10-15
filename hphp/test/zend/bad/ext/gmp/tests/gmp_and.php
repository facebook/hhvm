<?php

var_dump(gmp_strval(gmp_and("111111", "2222222")));
var_dump(gmp_strval(gmp_and(123123, 435234)));
var_dump(gmp_strval(gmp_and(555, "2342341123")));
var_dump(gmp_strval(gmp_and(-1, 3333)));
var_dump(gmp_strval(gmp_and(4545, -20)));
var_dump(gmp_strval(gmp_and("test", "no test")));

$n = gmp_init("987657876543456");
var_dump(gmp_strval(gmp_and($n, "34332")));
$n1 = gmp_init("987657878765436543456");
var_dump(gmp_strval(gmp_and($n, $n1)));

var_dump(gmp_and($n, $n1, 1));
var_dump(gmp_and(1));
var_dump(gmp_and(array(), 1));
var_dump(gmp_and(1, array()));
var_dump(gmp_and(array(), array()));

echo "Done\n";
?>
