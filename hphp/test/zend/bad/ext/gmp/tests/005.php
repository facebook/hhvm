<?php

var_dump(gmp_strval());
var_dump(gmp_strval(""));
var_dump(gmp_strval("", ""));
var_dump(gmp_strval("", -1));
var_dump(gmp_strval(-1, ""));

$fp = fopen(__FILE__, "r");
var_dump(gmp_strval($fp));

$g = gmp_init("9765456");
var_dump(gmp_strval($g));
var_dump(gmp_strval($g, -1));
var_dump(gmp_strval($g, 100000));
var_dump(gmp_strval($g, 10));

$g = gmp_init("-3373333");
var_dump(gmp_strval($g));
var_dump(gmp_strval($g, -1));
var_dump(gmp_strval($g, 100000));
var_dump(gmp_strval($g, 10));

var_dump(gmp_strval(array(1,2), array(1,2)));
var_dump(gmp_strval(new stdclass, new stdclass));
var_dump(gmp_strval(array(1,2)));
var_dump(gmp_strval(new stdclass));

echo "Done\n";
?>
