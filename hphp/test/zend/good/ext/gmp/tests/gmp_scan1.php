<?php

var_dump(gmp_scan1("434234", -10));
var_dump(gmp_scan1("434234", 1));
var_dump(gmp_scan1(4096, 0));
var_dump(gmp_scan1("1000000000", 5));
var_dump(gmp_scan1("1000000000", 200));

$n = gmp_init("24234527465274");
var_dump(gmp_scan1($n, 10));

var_dump(gmp_scan1(array(), 200));
var_dump(gmp_scan1(array()));
var_dump(gmp_scan1());

echo "Done\n";
?>
