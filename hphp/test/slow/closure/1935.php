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

<<__EntryPoint>>
function main_1935() {
$x = f();
$x(2);
$x(2);
}
