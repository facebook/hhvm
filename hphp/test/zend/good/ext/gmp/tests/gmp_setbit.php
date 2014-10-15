<?php

$n = gmp_init(-1);
gmp_setbit($n, 10, -1);
var_dump(gmp_strval($n));

$n = gmp_init(5);
gmp_setbit($n, -20, 0);
var_dump(gmp_strval($n));

$n = gmp_init(5);
gmp_setbit($n, 2, 0);
var_dump(gmp_strval($n));

$n = gmp_init(5);
gmp_setbit($n, 1, 1);
var_dump(gmp_strval($n));

$n = gmp_init("100000000000");
gmp_setbit($n, 23, 1);
var_dump(gmp_strval($n));

gmp_setbit($n, 23, 0);
var_dump(gmp_strval($n));

gmp_setbit($n, 3);
var_dump(gmp_strval($n));

$b = "";
gmp_setbit($b, 23);
gmp_setbit($b);
gmp_setbit($b, 23,1,1);
gmp_setbit($b,array());
$a = array();
gmp_setbit($a,array());

echo "Done\n";
?>
