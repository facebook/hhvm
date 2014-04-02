<?php

class B {
  function test() {
    D::meth();
  }
  function test2() {
    E::meth();
  }
}
class D extends B {
  function meth() {
    var_dump(isset($this));
    if (isset($this)) var_dump($this);
  }
}
class E extends B {
  function meth() {
    var_dump(isset($this));
    if (isset($this)) var_dump($this);
  }
}
function main() {
  $d = new D;
  $d->test();
  $d->test2();
  $e = new E;
  $e->test();
  $e->test2();
}
main();
