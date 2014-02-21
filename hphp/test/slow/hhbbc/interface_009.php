<?php

class Heh {
  public function say() { echo "hi\n"; }
}

interface I {
  public function foo();
}

class Base {}
class A extends Base implements I { public function foo() { return new Heh(); }}
class B extends Base implements I { public function foo() { return new Heh(); }}

function gen() {
  return rand() ? new A : new B;
}

function main() {
  $x = gen();
  if ($x instanceof I) { echo "I\n"; }
  if ($x instanceof Base) { echo "I\n"; }
  $x = $x->foo();
  if ($x instanceof Heh) { echo "Heh\n"; }
  $x->say();
}

main();
