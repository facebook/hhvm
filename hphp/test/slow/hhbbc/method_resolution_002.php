<?php

class Base {
  private function foo() {
    return 12;
  }
  function heh(D1 $d) {
    return $d->foo(); // calls the private function
  }
}

class D1 extends Base {
  public function foo() {
    return "a string";
  }
}

function main() {
  $x = new D1;
  var_dump($x->heh($x));
}

main();
