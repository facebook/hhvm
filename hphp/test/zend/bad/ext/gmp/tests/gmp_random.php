<?php

var_dump(gmp_strval(gmp_random()));
var_dump(gmp_strval(gmp_random(-1)));
var_dump(gmp_strval(gmp_random(0)));
var_dump(gmp_strval(gmp_random(10)));
var_dump(gmp_strval(gmp_random("-10")));
var_dump(gmp_strval(gmp_random(-10)));

var_dump(gmp_random(array()));
var_dump(gmp_random(array(),1));
var_dump(gmp_random(""));
var_dump(gmp_random("test"));

echo "Done\n";
?>
