<?php

function fib($n) {
  assert($n >= 0);
  if ($n === 0 || $n === 1) {
    return 1;
  }
  return fib($n - 1) + fib($n - 2);
}

for ($i = 0; $i < 20; ++$i) {
  if (array_key_exists('n', $_GET)) {
    var_dump(fib((int) $_GET['n']));
  } else {
    var_dump(fib(20));
  }
}
