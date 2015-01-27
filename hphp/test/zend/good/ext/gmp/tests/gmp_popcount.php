<?php

var_dump(gmp_popcount(-1));
var_dump(gmp_popcount(0));
var_dump(gmp_popcount(12123));
var_dump(gmp_popcount("52638927634234"));
var_dump(gmp_popcount("-23476123423433"));
$n = gmp_init("9876546789222");
var_dump(gmp_popcount($n));
var_dump(gmp_popcount(array()));
var_dump(gmp_popcount());

echo "Done\n";
?>
