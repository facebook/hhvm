<?php

var_dump(gmp_root());

var_dump(gmp_root(1000, 3));
var_dump(gmp_root(100, 3));
var_dump(gmp_root(-100, 3));

var_dump(gmp_root(1000, 4));
var_dump(gmp_root(100, 4));
var_dump(gmp_root(-100, 4));

var_dump(gmp_root(0, 3));
var_dump(gmp_root(100, 0));
var_dump(gmp_root(100, -3));

?>
