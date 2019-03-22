<?php

$a = 100;
function f() {
  foreach ($GLOBALS as $k => $v) {
    if ($k == 'a') {
      $GLOBALS[$k] = -1;
    }
  }

  var_dump($GLOBALS['a']);
  $b = $GLOBALS;
  $b['a'] = 0;
  var_dump($GLOBALS['a']);
}
f();
