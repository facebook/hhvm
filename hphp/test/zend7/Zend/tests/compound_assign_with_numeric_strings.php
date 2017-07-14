<?php

$n = "65";
$n <<= $n;
var_dump($n);

$n = "-1";
try {
    $n <<= $n;
    var_dump($n);
} catch (ArithmeticError $e) {
	echo "\nException: " . $e->getMessage() . "\n";
}

$n = "65";
$n >>= $n;
var_dump($n);

$n = "-1";
try {
  $n >>= $n;
  var_dump($n);
} catch (ArithmeticError $e) {
	echo "\nException: " . $e->getMessage() . "\n";
}

$n = "0";
try{
  $n %= $n;
  var_dump($n);
} catch (DivisionByZeroError $e) {
	echo "\nException: " . $e->getMessage() . "\n";
}

$n = "-1";
$n %= $n;
var_dump($n);
