<?php

var_dump(gmp_strval(gmp_fact(0)));
var_dump(gmp_strval(gmp_fact("")));
var_dump(gmp_strval(gmp_fact("0")));
var_dump(gmp_strval(gmp_fact("-1")));
var_dump(gmp_strval(gmp_fact(-1)));
var_dump(gmp_strval(gmp_fact(1.1)));
var_dump(gmp_strval(gmp_fact(20)));
var_dump(gmp_strval(gmp_fact("50")));
var_dump(gmp_strval(gmp_fact("10")));
var_dump(gmp_strval(gmp_fact("0000")));

$n = gmp_init(12);
var_dump(gmp_strval(gmp_fact($n)));
$n = gmp_init(-10);
var_dump(gmp_strval(gmp_fact($n)));

var_dump(gmp_fact());
var_dump(gmp_fact(1,1));
var_dump(gmp_fact(array()));
var_dump(gmp_strval(gmp_fact(array())));

echo "Done\n";
?>
