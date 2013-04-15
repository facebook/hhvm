<?php

function foo() {
  $a = $GLOBALS;
  $x = count($a);
  array_pop($a);
  $y = count($a);
  var_dump($x - $y);
}

function bar() {
  $b = $GLOBALS;
  $x = count($b);
  array_shift($b);
  $y = count($b);
  var_dump($x - $y);
}

foo();

bar();
