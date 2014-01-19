<?php

function f(array $a, $e) {
  $a[$e][$e] = 30;
  $x = new stdClass();
  $x = $a[$e];
  var_dump($a, $x);
}
function g(string $x) {
  var_dump($x[0]);
  var_dump($x[1]);
  var_dump($x[0]);
}
f(array(), 0);
g('bar');
g('');
g('b');
