<?php

var_dump(gmp_hamdist(1231231, 0));
var_dump(gmp_hamdist(1231231, -1));
var_dump(gmp_hamdist(1231231, "8765434567897654333334567"));
var_dump(gmp_hamdist(-364264234, "8333765434567897654333334567"));

$n = gmp_init("8765434567");
$n1 = gmp_init("987654445678");
var_dump(gmp_hamdist($n, "8333765434567897654333334567"));
var_dump(gmp_hamdist($n, $n));
var_dump(gmp_hamdist($n, $n1));

var_dump(gmp_hamdist($n, $n1, 1));
var_dump(gmp_hamdist($n, array()));
var_dump(gmp_hamdist(array(), $n));
var_dump(gmp_hamdist(array(), array()));
var_dump(gmp_hamdist());

echo "Done\n";
?>
