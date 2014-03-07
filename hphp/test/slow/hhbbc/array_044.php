<?php

class C { function heh() { echo "heh\n"; } }
function foo() {
  return mt_rand() ? array(new C, new C) : array(new C, new C, new C);
}
function bar() {
  $x = foo();
  $x['a'] = new C;
  return $x['nothere'];
}
function main() {
  $x = bar();
  if ($x) $x->heh();
}
main();
