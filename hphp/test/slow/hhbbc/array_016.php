<?php

function four() { return 4; }
function heh() { return array('foo' => four()); }
function bar() { return array('other' => heh()); }
function foo() {
  $x = bar();
  $x['other']['foo'] = 2;
  return $x;
}
function main() {
  $x = foo();
  echo $x['other']['foo'] . "\n";
}
main();
