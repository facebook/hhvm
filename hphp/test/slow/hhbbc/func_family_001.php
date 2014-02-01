<?php

class Heh {
  public function say() { echo "hi\n"; }
}

abstract class Base {
  abstract public function foo();
}

class D1 extends Base { public function foo() { return new Heh(); } }
class D2 extends Base { public function foo() { return new Heh(); } }

function main(Base $b) {
  $x = $b->foo();
  $x->say();
}

main(new D1);
main(new D2);

