<?php

$x = 0;
class C { public function heh() { echo "heh\n"; } }
function foo() {
  switch ($GLOBALS['x']) {
  case 0:
    return new C;
  }
}

function bar() {
  $x = foo();
  $x->heh();
  $x->heh();
  return $x;
}

function main() {
  $x = bar();
  var_dump($x);
}

main();
