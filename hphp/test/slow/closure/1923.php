<?php

function f() {
  $test = false;
  $f = function ($p) use (&$test) {
    if ($p) $test = true;
  }
;
  $f(true);
  var_dump($test);
}

<<__EntryPoint>>
function main_1923() {
f();
}
