<?php

function bar($n) {
  static $x = 1;
  return str_repeat("x", $n) . $x++;
}

function foo() {
  apc_store("foo", bar(50));
  $x = apc_fetch("foo");
  $x[5] = 1;
  $x = bar(20);
  var_dump($x);
}

foo();
