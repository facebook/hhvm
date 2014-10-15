<?php

var_dump(gmp_strval(gmp_abs("")));
var_dump(gmp_strval(gmp_abs("0")));
var_dump(gmp_strval(gmp_abs(0)));
var_dump(gmp_strval(gmp_abs(-111111111111111111111)));
var_dump(gmp_strval(gmp_abs("111111111111111111111")));
var_dump(gmp_strval(gmp_abs("-111111111111111111111")));
var_dump(gmp_strval(gmp_abs("0000")));
var_dump(gmp_strval(gmp_abs("09876543")));
var_dump(gmp_strval(gmp_abs("-099987654")));

var_dump(gmp_abs());
var_dump(gmp_abs(1,2));
var_dump(gmp_abs(array()));

echo "Done\n";
?>
