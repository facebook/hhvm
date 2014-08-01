<?php
//will work
var_dump(gmp_intval(""));
var_dump(gmp_intval(1.0001));
var_dump(gmp_intval("-1"));
var_dump(gmp_intval(-1));
var_dump(gmp_intval(-2349828));
var_dump(gmp_intval(2342344));
$g = gmp_init("12345678");
var_dump(gmp_intval($g));

//will fail
var_dump(gmp_intval("1.0001"));
var_dump(gmp_intval(1,1));
var_dump(gmp_intval(new stdclass));
var_dump(gmp_intval(array())); //will be 0 in php but that is stupid
$fp = fopen(__FILE__, 'r');
var_dump(gmp_intval($fp));
