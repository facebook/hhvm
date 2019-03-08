<?php

$x = 0;

abstract final class FStatics {
  public static $y = 0;
}
function f() {
  global $x;
  yield $x++;
  yield FStatics::$y++;
}
for ($i = 0;
 $i < 5;
 $i++) {
  foreach (f() as $value) {
    var_dump($value);
  }
}
var_dump($x);
