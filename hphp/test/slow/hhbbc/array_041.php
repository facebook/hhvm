<?php

class C { function heh() { echo "heh\n"; } }
function foo() { return array(); }
function some_int() { return mt_rand() ? 1 : 2; }
function bar() {
  $x = foo();
  $val = some_int();
  $x[$val] = new C;
  global $g;
  $g = $val;
  return $x;
}
function main() {
  global $g;
  $y = bar();
  $l = (int)$g;
  $y = $y[$l];
  $y->heh();
}
main();
