<?php

class Heh {
  public function say() { echo "hi\n"; }
}

interface I {
  public function foo();
}

class D1 implements I { public function foo() { return new Heh(); } }
class D2 implements I { public function foo() { return new Heh(); } }
class D3 { public function foo() { return new Heh(); } }

function main(I $b) {
  if ($b instanceof D1) { echo "D1\n"; }
  if ($b instanceof D2) { echo "D2\n"; }
  if ($b instanceof D3) { echo "D3\n"; }
  $x = $b->foo();
  if ($x instanceof Heh) { echo "Heh\n"; }
  $x->say();
}

main(new D1);
main(new D2);
