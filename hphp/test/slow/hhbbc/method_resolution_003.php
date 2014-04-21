<?php

class Base {
  private function foo() {
    return 12;
  }
  function heh(D1 $d) {
    return $d->foo(); // calls /our/ private function
  }
}

class D1 extends Base {
  private function foo() {
    return "a string";
  }
}

function gen($x) {
  return $x ? new D1 : null;
}

function main() {
  $x = new D1;
  var_dump($x->heh($x));
  $y = gen(1);
  var_dump($y->heh($y));
}

main();
