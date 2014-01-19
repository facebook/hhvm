<?php

print "Test begin\n";

class C {}
class D {
  function __construct() {
    print "In D::__construct()\n";
  }
}
class E {
  function E() {
    print "In E::E()\n";
  }
}
class F {
  function F() {
    print "In F::F()\n";
  }
  function __construct() {
    print "In F::__construct()\n";
  }
}
class G extends C {}
class H extends D {}
class I extends E {}
class J extends F {}
class K extends H {
  function __construct() {
    print "In K::__construct()\n";
  }
}
class L extends I {
  function L() {
    print "In L::L()\n";
  }
}
class M extends J {
  function M() {
    print "In M::M()\n";
  }
  function __construct() {
    print "In M::__construct()\n";
  }
}

$c = new C;
$c = new D;
$c = new E;
$c = new F;
$c = new G;
$c = new H;
$c = new I;
$c = new J;
$c = new K;
$c = new L;
$c = new M;

$X = "C"; $c = new $X;
$X = "D"; $c = new $X;
$X = "E"; $c = new $X;
$X = "F"; $c = new $X;
$X = "G"; $c = new $X;
$X = "H"; $c = new $X;
$X = "I"; $c = new $X;
$X = "J"; $c = new $X;
$X = "K"; $c = new $X;
$X = "L"; $c = new $X;
$X = "M"; $c = new $X;

print "Test end\n";
