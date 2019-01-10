<?php

$a = 100;
function f() {
  foreach ($GLOBALS as $k => $v) {
    if ($k == 'a') {
      $GLOBALS[$k] = -1;
    }
  }
  global $a;
  var_dump($a);
  $b = $GLOBALS;
  $b['a'] = 0;
  var_dump($GLOBALS['a']);
}
f();
