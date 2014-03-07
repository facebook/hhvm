<?php

class C { function heh() { echo "heh\n"; } }
function foo() { return array(); }
function bar() {
  $x = foo();
  $x['a'] = new C;
  return $x;
}
function main() {
  $y = bar()['a'];
  $y->heh();
}
main();
