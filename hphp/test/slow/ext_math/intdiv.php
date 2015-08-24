<?php
var_dump(intdiv(3, 2));
var_dump(intdiv(-3, 2));
var_dump(intdiv(3, -2));
var_dump(intdiv(-3, -2));
var_dump(intdiv(PHP_INT_MAX, PHP_INT_MAX));
var_dump(intdiv(-PHP_INT_MAX - 1, -PHP_INT_MAX - 1));
var_dump(intdiv(-PHP_INT_MAX - 1, -1));
var_dump(intdiv(1, 0));
