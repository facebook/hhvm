<?php

error_reporting(-1);

function lol() { return STDIN; }
function foo() {
  $x = lol();
  unset($x[0]['id']);
  return $x;
}

var_dump(foo());
