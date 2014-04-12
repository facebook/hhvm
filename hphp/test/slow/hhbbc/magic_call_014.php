<?php

class Base {
  function heh(D1 $d) {
    return $d->foo(); // no __call, we have access
  }
}

class D1 extends Base {
  protected function foo() {
    return "a string";
  }
  public function __call($x, $y) { echo "nope"; }
}

function main() {
  $x = new D1;
  var_dump($x->heh($x));
}

main();
