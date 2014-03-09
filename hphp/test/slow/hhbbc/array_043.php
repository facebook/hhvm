<?php

class C { function heh() { echo "heh\n"; } }
function foo() {
  return mt_rand() ? array(new C, new C) : array(new C, new C, new C);
}
function bar() {
  $x = foo();
  $x['a'] = new C;
  return $x[0];
}
function main() {
  $x = bar();
  $x->heh();
}
main();
