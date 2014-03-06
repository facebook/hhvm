<?php

class C { public function heh() { echo "hey\n"; } }
function foo() {
  $x = array();
  for ($i = 0; $i < 10; ++$i) {
    $x[] = new C;
  }
  return $x;
}
function main() {
  $x = foo();
  $x[0]->heh();
}
main();
