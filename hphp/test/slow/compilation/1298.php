<?php

function foo($a) {
  $r = '';
  if ($a) {
    $r ->error = '';
  }
  return $r;
}

<<__EntryPoint>>
function main_1298() {
var_dump(foo(true));
var_dump(foo(false));
}
