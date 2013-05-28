<?php

$x = 0;
function f() {
  global $x;
  static $y = 0;
  yield $x++;
  yield $y++;
}
for ($i = 0;
 $i < 5;
 $i++) {
  foreach (f() as $value) {
    var_dump($value);
  }
}
var_dump($x);
