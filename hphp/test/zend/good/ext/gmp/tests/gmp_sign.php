<?php

var_dump(gmp_sign(-1));
var_dump(gmp_sign(1));
var_dump(gmp_sign(0));
var_dump(gmp_sign("123718235123123"));
var_dump(gmp_sign("-34535345345"));
var_dump(gmp_sign("+34534573457345"));
$n = gmp_init("098909878976786545");
var_dump(gmp_sign($n));
var_dump(gmp_sign($n, $n));
var_dump(gmp_sign(array()));
var_dump(gmp_sign());

echo "Done\n";
?>
