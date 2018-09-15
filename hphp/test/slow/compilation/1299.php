<?php

function foo($a) {
  $r = '';
  if ($a) {
    $r ->error->line = 1;
  }
  return $r;
}

<<__EntryPoint>>
function main_1299() {
var_dump(foo(true));
var_dump(foo(false));
}
