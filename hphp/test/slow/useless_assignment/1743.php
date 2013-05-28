<?php

function foo($p) {
  global $b;
  for ($i = 0;
 $i < 5;
 $i++) {
    if ($i > $p) {
      $a = 10;
    }
 else {
      $a = &$b;
    }
  }
}
function bar() {
  $a = foo(2);
  var_dump($GLOBALS['b']);
}
bar();
