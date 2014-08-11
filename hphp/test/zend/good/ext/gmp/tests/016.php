<?php

var_dump(gmp_strval(gmp_powm(0,1,10)));
var_dump(gmp_strval(gmp_powm(5,1,10)));
var_dump(gmp_strval(gmp_powm(-5,1,-10)));
var_dump(gmp_strval(gmp_powm(-5,1,10)));
var_dump(gmp_strval(gmp_powm(-5,11,10)));
var_dump(gmp_strval(gmp_powm("77",3,1000)));

$n = gmp_init(11);
var_dump(gmp_strval(gmp_powm($n,3,1000)));
$e = gmp_init(7);
var_dump(gmp_strval(gmp_powm($n,$e,1000)));
$m = gmp_init(900);
var_dump(gmp_strval(gmp_powm($n,$e,$m)));

var_dump(gmp_powm(array(),$e,$m));
var_dump(gmp_powm($n,array(),$m));
var_dump(gmp_powm($n,$e,array()));
var_dump(gmp_powm(array(),array(),array()));
var_dump(gmp_powm(array(),array()));
var_dump(gmp_powm(array()));
var_dump(gmp_powm());

$n = gmp_init("-5");
var_dump(gmp_powm(10, $n, 10));

$n = gmp_init("0");
var_dump(gmp_powm(10, $n, 10));

echo "Done\n";
?>
