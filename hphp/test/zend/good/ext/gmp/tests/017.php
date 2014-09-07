<?php

var_dump(gmp_strval(gmp_sqrt(-2)));
var_dump(gmp_strval(gmp_sqrt("-2")));
var_dump(gmp_strval(gmp_sqrt("0")));
var_dump(gmp_strval(gmp_sqrt("2")));
var_dump(gmp_strval(gmp_sqrt("144")));

$n = gmp_init(0);
var_dump(gmp_strval(gmp_sqrt($n)));
$n = gmp_init(-144);
var_dump(gmp_strval(gmp_sqrt($n)));
$n = gmp_init(777);
var_dump(gmp_strval(gmp_sqrt($n)));

var_dump(gmp_sqrt($n, 1));
var_dump(gmp_sqrt());
var_dump(gmp_sqrt(array()));

echo "Done\n";
?>
