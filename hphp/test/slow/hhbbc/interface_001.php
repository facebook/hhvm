<?php

class Heh {
  public function say() { echo "hi\n"; }
}

interface I {
  public function foo();
}

class A implements I { public function foo() { return new Heh(); } }
class B implements I { public function foo() { return new Heh(); } }

function main(I $a) {
  $x = $a->foo();
  if ($x instanceof Heh) { echo "Heh\n"; }
  $x->say();
}

main(new A);
main(new B);
