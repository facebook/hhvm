<?php

function foo($p) {

  for ($i = 0;
 $i < 5;
 $i++) {
    if ($i > $p) {
      $a = 10;
    }
 else {
      $a = &UselessAssignment1743::$b;
    }
  }
}
<<__EntryPoint>>
function main_1743() {
  $a = foo(2);
  var_dump(UselessAssignment1743::$b);
}

abstract final class UselessAssignment1743 {
  public static $b;
}
