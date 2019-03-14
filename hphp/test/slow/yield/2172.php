<?php


abstract final class FStatics {
  public static $y = 0;
}

function f() {

  yield Yield2172::$x++;
  yield FStatics::$y++;
}
for ($i = 0;
 $i < 5;
 $i++) {
  foreach (f() as $value) {
    var_dump($value);
  }
}
var_dump(Yield2172::$x);

abstract final class Yield2172 {
  public static $x = 0;
}
