<?php

print "Test begin\n";

class C {
  function __construct() {
    print "In C::__construct()\n";
    C::sMeth();
  }
  static function sMeth() {
    print "In C::sMeth(): \$this:" . (isset($this) ? $this : null) . ":\n";
  }
}
class D {
  function D() {
    print "In D::D()\n";
  }
}
class E {
  function E() {
    print "In E::E()\n";
  }
  function __construct() {
    print "In E::__construct()\n";
  }
}

class F extends C {}
class G extends D {}
class H extends E {}

class I extends F {
  function __construct() {
    print "In I::__construct()\n";
  }
}
class J extends G {
  function J() {
    print "In J::J()\n";
  }
}
class K extends H {
  function K() {
    print "In K::K()\n";
  }
  function __construct() {
    print "In K::__construct()\n";
  }
}

C::__construct();
D::D();
E::E();
E::__construct();
F::__construct();
G::D();
H::E();
H::__construct();
I::__construct();
J::D();
J::J();
K::E();
K::K();
K::__construct();

$X = "C";
  $m = "__construct"; C::$m(); $X::$m();
$X = "D";
  $m = "D"; D::$m(); $X::$m(); $X::$m();
$X = "E";
  $m = "E"; E::$m(); $X::$m();
  $m = "__construct"; E::$m(); $X::$m();
$X = "F";
  $m = "__construct"; F::$m(); $X::$m();
$X = "G";
  $m = "D"; G::$m(); $X::$m();
$X = "H";
  $m = "E"; H::$m(); $X::$m();
  $m = "__construct"; H::$m(); $X::$m();
$X = "I";
  $m = "__construct"; I::$m(); $X::$m();
$X = "J";
  $m = "D"; J::$m(); $X::$m();
  $m = "J"; J::$m(); $X::$m();
$X = "K";
  $m = "E"; K::$m(); $X::$m();
  $m = "K"; K::$m(); $X::$m();
  $m = "__construct"; K::$m(); $X::$m();

print "Test end\n";
