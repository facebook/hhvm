<?php

var_dump(gmp_intval(1,1));
var_dump(gmp_intval(""));
var_dump(gmp_intval(1.0001));
var_dump(gmp_intval("1.0001"));
var_dump(gmp_intval("-1"));
var_dump(gmp_intval(-1));
var_dump(gmp_intval(-2349828));
var_dump(gmp_intval(2342344));
var_dump(gmp_intval(new stdclass));
var_dump(gmp_intval(array()));

$fp = fopen(__FILE__, 'r');
var_dump(gmp_intval($fp));

$g = gmp_init("12345678");
var_dump(gmp_intval($g));

echo "Done\n";
?>
