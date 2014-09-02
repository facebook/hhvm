<?php

var_dump(gmp_scan0("434234", -10));
var_dump(gmp_scan0("434234", 1));
var_dump(gmp_scan0(4096, 0));
var_dump(gmp_scan0("1000000000", 5));
var_dump(gmp_scan0("1000000000", 200));

$n = gmp_init("24234527465274");
var_dump(gmp_scan0($n, 10));

var_dump(gmp_scan0(array(), 200));
var_dump(gmp_scan0(array()));
var_dump(gmp_scan0());

echo "Done\n";
?>
