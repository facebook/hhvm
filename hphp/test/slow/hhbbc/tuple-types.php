<?php


function foo(bool $x, int $n, string $y) {
  if ($x) {
    $y = tuple($n);
  } else {
    $y = tuple($y);
    echo $y[0];
  }
  return $y[0];
}
var_dump(foo(false, 4, 'hi'));
