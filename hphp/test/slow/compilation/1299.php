<?php

function foo($a) {
  $r = '';
  if ($a) {
    $r ->error->line = 1;
  }
  return $r;
}
var_dump(foo(true));
var_dump(foo(false));
