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

class Boo {
  function say() { echo "hi\n"; }
}

class D1 implements I3 {
  public function foo() {
    return new Heh();
  }
}

class D2 implements I {
  public function foo() {
    return new Boo();
  }
}

class D3 implements I4 {
  function foo() { return new Heh(); }
}

function main(I1 $b) {
  if ($b instanceof D1) { echo "D1\n"; }
  if ($b instanceof D2) { echo "D2\n"; }
  if ($b instanceof D3) { echo "D3\n"; }
  $x = $b->foo();
  if ($x instanceof Heh) { echo "Heh\n"; }
  $x->say();
}

main(new D1);
main(new D2);
