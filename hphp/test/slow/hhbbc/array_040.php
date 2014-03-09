<?php

class C { function heh() { echo "heh\n"; } }
function foo() { return array(); }
function bar() {
  $x = foo();
  $x[0] = new C;
  return $x;
}
function main() {
  $y = bar()[0];
  $y->heh();
}
main();
