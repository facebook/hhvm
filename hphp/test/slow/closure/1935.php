<?php

function f() {
  $someVar = 456;
  $closure = function($param) use ($someVar) {
      echo $param . ' ' . $someVar . "\n";
      $param = 7;
      $someVar = 11;
    }
;
  return $closure;
}
$x = f();
$x(2);
$x(2);
