<?php

function fib_accum($a, $b, $n) {
  if ($n <= 0) {
    return $a;
  } else {
    return fib_accum($b, $a + $b, $n - 1);
  }
}

function fib($n) {
  return fib_accum(1, 1, $n);
}

for ($i = 0; $i < 10; $i++) {
  var_dump(fib($i));
}
