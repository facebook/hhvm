<?php

// As far as we know, we're deliberately emulating a zend bug here:

class A {
  final function foo() {
    return static function() {
      var_dump(new static);   // Does *not* do LSB
    };
  }
}

class B extends A {
}

class C extends B {
}

$f = A::foo();
$f();
$f = B::foo();
$f();
$f = C::foo();
$f();

