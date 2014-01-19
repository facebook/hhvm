<?php

var_dump(ceil(4.3));
var_dump(ceil(9.999));
var_dump(ceil(-3.14));

var_dump(floor(4.3));
var_dump(floor(9.999));
var_dump(floor(-3.14));

var_dump(round(3.4));
var_dump(round(3.5));
var_dump(round(3.6));
var_dump(round(3.6, 0));
var_dump(round(1.95583, 2));
var_dump(round(1241757, -3));
var_dump(round(5.045, 2));
var_dump(round(5.055, 2));
var_dump(round("3.4"));
var_dump(round("3.5"));
var_dump(round("3.6"));
var_dump(round("3.6", 0));
var_dump(round("1.95583", 2));
var_dump(round("1241757", -3));
var_dump(round("5.045", 2));
var_dump(round("5.055", 2));

// ext/standard/tests/math/round_modes.phpt
var_dump(round(2.5, 0, PHP_ROUND_HALUP));
var_dump(round(2.5, 0, PHP_ROUND_HALDOWN));
var_dump(round(2.5, 0, PHP_ROUND_HALEVEN));
var_dump(round(2.5, 0, PHP_ROUND_HALODD));
var_dump(round(-2.5, 0, PHP_ROUND_HALUP));
var_dump(round(-2.5, 0, PHP_ROUND_HALDOWN));
var_dump(round(-2.5, 0, PHP_ROUND_HALEVEN));
var_dump(round(-2.5, 0, PHP_ROUND_HALODD));
var_dump(round(3.5, 0, PHP_ROUND_HALUP));
var_dump(round(3.5, 0, PHP_ROUND_HALDOWN));
var_dump(round(3.5, 0, PHP_ROUND_HALEVEN));
var_dump(round(3.5, 0, PHP_ROUND_HALODD));
var_dump(round(-3.5, 0, PHP_ROUND_HALUP));
var_dump(round(-3.5, 0, PHP_ROUND_HALDOWN));
var_dump(round(-3.5, 0, PHP_ROUND_HALEVEN));
var_dump(round(-3.5, 0, PHP_ROUND_HALODD));
