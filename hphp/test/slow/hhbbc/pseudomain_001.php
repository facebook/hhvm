<?php

function foo() {
  global $x;
  $x = 'yep';
  return 'hehehe';
}

function al() { return 2; }
unset($x);
$x = al();
var_dump(foo());
var_dump($x);
