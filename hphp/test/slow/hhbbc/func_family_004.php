<?php

abstract class Base {
  abstract public function foo();
}

class D1 extends Base {
  public function foo() { throw new Exception('heh'); }
}
class D2 extends Base {
  public function foo() { return "this is a string\n"; }
}

function main(Base $b) {
  $x = $b->foo();
  var_dump($x);
}

try {
  main(new D2);
  main(new D1);
} catch (exception $x) {}

