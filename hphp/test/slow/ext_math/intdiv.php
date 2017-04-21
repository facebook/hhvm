<?php
var_dump(intdiv(3, 2));
var_dump(intdiv(-3, 2));
var_dump(intdiv(3, -2));
var_dump(intdiv(-3, -2));
var_dump(intdiv(PHP_INT_MAX, PHP_INT_MAX));
var_dump(intdiv(-PHP_INT_MAX - 1, -PHP_INT_MAX - 1));

try {
  var_dump(intdiv(-PHP_INT_MAX - 1, -1));
} catch (Throwable $e) {
  var_dump($e->getMessage());
}

try {
  var_dump(intdiv(PHP_INT_MIN, -1));
} catch (Throwable $e) {
  var_dump($e->getMessage());
}

try {
  var_dump(intdiv(1, 0));
} catch (Throwable $e) {
  var_dump($e->getMessage());
}
