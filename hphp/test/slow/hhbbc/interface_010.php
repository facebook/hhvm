<?php

class Heh {
  public function say() { echo "hi\n"; }
}

interface I {
  public function foo();
}

class Base implements I { function foo() { return 1; }}

class D1 extends Base { public function foo() { return new Heh(); } }
class D2 extends Base { public function foo() { return new Heh(); } }
class D3 { public function foo() { return new Heh(); } }

function gen() {
  return rand() ? new D1 : new D2;
}

function main() {
  $b = gen();
  if ($b instanceof D1) { echo "D\n"; }
  if ($b instanceof D2) { echo "D\n"; }
  if ($b instanceof I) { echo "I\n"; }
  if ($b instanceof Base) { echo "Base\n"; }
  if ($b instanceof D3) { echo "D3\n"; }
  $x = $b->foo();
  if ($x instanceof Heh) { echo "Heh\n"; }
  $x->say();
}

main();
