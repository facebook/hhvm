<?php

var_dump(gmp_mod());
var_dump(gmp_mod(""));
var_dump(gmp_mod("",""));
var_dump(gmp_mod(0,1));
var_dump(gmp_mod(0,-1));
var_dump(gmp_mod(-1,0));

var_dump(gmp_mod(array(), array()));

$a = gmp_init("-100000000");
$b = gmp_init("353467");

var_dump(gmp_mod($a, $b));

echo "Done\n";
?>
