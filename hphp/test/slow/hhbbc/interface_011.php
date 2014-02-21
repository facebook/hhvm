<?php

interface I1 {
  function foo();
}

interface I2 {
  function foo();
}

interface I3 extends I1 {}

interface I4 extends I2 {}

interface I extends I3, I4 {
}

class Heh {
  function say() { echo "hi\n"; }
}

class Base implements I1 { function foo() { return 1; } }

class D1 extends Base implements I3 {
  public function foo() {
    return new Heh();
  }
}

class D2 extends Base implements I {
  public function foo() {
    return (new D1)->foo();
  }
}

class D3 implements I4 {
  function foo() { return new Heh(); }
}

function gen() {
  return rand() ? new D1 : new D2;
}

function main() {
  $b = gen();
  if ($b instanceof D1) { echo "D\n"; }
  if ($b instanceof D2) { echo "D\n"; }
  if ($b instanceof D3) { echo "D3\n"; }
  if ($b instanceof I3) { echo "I3\n"; }
  if ($b instanceof I1) { echo "I1\n"; }
  if ($b instanceof Base) { echo "Base\n"; }
  $x = $b->foo();
  if ($x instanceof Heh) { echo "Heh\n"; }
  $x->say();
}

main();
