<?php

function heh() { return 4; }
function bar() { return array('foo' => heh()); }
function foo() {
  $x = bar();
  $x['foo'] = 2;
  return $x;
}
function main() {
  $x = foo();
  echo $x['foo'] . "\n";
}
main();
