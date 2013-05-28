<?php

function foo() {
  return '1';
}
function bar() {
  $a = 1;
  $a += foo();
  var_dump($a);
  $b = 1;
  $b -= foo();
  var_dump($b);
}
bar();
