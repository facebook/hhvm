<?php

class C { function heh() { echo "heh\n"; } }
function foo() { return array(new C); }
function bar() {
  $x = foo();
  $x[1] = new C;
  return array($x[0], $x[1]);
}
function main() {
  list($y, $yy) = bar();
  $y->heh();
  $yy->heh();
}
main();
