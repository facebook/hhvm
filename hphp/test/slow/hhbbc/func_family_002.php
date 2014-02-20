<?php

class Heh {
  public function say() { echo "hi\n"; }
}

abstract class RetBase {
  abstract public function get();
}

class RetD1 extends RetBase {
  public function get() { return new Heh(); }
}

class RetD2 extends RetBase {
  public function get() { return new Heh(); }
}

abstract class Base {
  abstract public function foo();
}

class D1 extends Base { public function foo() { return new RetD1(); } }
class D2 extends Base { public function foo() { return new RetD2(); } }

function main(Base $b) {
  $x = $b->foo();
  $x = $x->get();
  $x->say();
}

main(new D1);
main(new D2);

