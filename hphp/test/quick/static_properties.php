<?php

// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

print "Test begin\n";

class A {
  const CD = "A::CD";
  const CE = B::CE;
  const CF = C::CG;
  static $a = "A::a";
  static protected $b = "A::b";
  static private $c = "A::c";
  static $d = A::CD;
  static protected $e = A::CE;
  static private $f = A::CF;

  function aFunc() {
    print "In A::aFunc():\n";
    print "  A::a: " . A::$a . "\n";
    print "  A::b: " . A::$b . "\n";
    print "  A::c: " . A::$c . "\n";
    print "  A::d: " . A::$d . "\n";
    print "  A::e: " . A::$e . "\n";
    print "  A::f: " . A::$f . "\n";
    print "\n";
    print "  B::a: " . B::$a . "\n";
    print "  B::b: " . B::$b . "\n";
#    print "  B::c: " . B::$c . "\n";
    print "  B::d: " . B::$d . "\n";
    print "  B::e: " . B::$e . "\n";
#    print "  B::f: " . B::$f . "\n";
    print "  B::g: " . B::$g . "\n";
    print "\n";
    print "  C::a: " . C::$a . "\n";
    print "  C::b: " . C::$b . "\n";
#    print "  C::c: " . C::$c . "\n";
    print "  C::d: " . C::$d . "\n";
    print "  C::e: " . C::$e . "\n";
#    print "  C::f: " . C::$f . "\n";
    print "  C::g: " . C::$g . "\n";
    print "  C::h: " . C::$h . "\n";
  }
}

class B extends A {
  const CD = "B::CD";
  const CE = "B::CE";
  static protected $b = "B::b";
  static private $c = "B::c";
  static $d = B::CD;
  static protected $g = "B::g";
  function bFunc() {
    print "In B::bFunc():\n";
    print "  A::a: " . A::$a . "\n";
    print "  A::b: " . A::$b . "\n";
#    print "  A::c: " . A::$c . "\n";
    print "  A::d: " . A::$d . "\n";
    print "  A::e: " . A::$e . "\n";
#    print "  A::f: " . A::$f . "\n";
    print "\n";
    print "  B::a: " . B::$a . "\n";
    print "  B::b: " . B::$b . "\n";
    print "  B::c: " . B::$c . "\n";
    print "  B::d: " . B::$d . "\n";
    print "  B::e: " . B::$e . "\n";
#    print "  B::f: " . B::$f . "\n";
    print "  B::g: " . B::$g . "\n";
    print "\n";
    print "  C::a: " . C::$a . "\n";
    print "  C::b: " . C::$b . "\n";
#    print "  C::c: " . C::$c . "\n";
    print "  C::d: " . C::$d . "\n";
    print "  C::e: " . C::$e . "\n";
#    print "  C::f: " . C::$f . "\n";
    print "  C::g: " . C::$g . "\n";
    print "  C::h: " . C::$h . "\n";
  }
}

class C extends B {
  const CG = "C::CG";
  static protected $b = "C::b";
  static $h = "C::h";
  public $i = C::CG;
  function cFunc() {
    print "In C::cFunc():\n";
    print "  A::a: " . A::$a . "\n";
    print "  A::b: " . A::$b . "\n";
#    print "  A::c: " . A::$c . "\n";
    print "  A::d: " . A::$d . "\n";
    print "  A::e: " . A::$e . "\n";
#    print "  A::f: " . A::$f . "\n";
    print "\n";
    print "  B::a: " . B::$a . "\n";
    print "  B::b: " . B::$b . "\n";
#    print "  B::c: " . B::$c . "\n";
    print "  B::d: " . B::$d . "\n";
    print "  B::e: " . B::$e . "\n";
#    print "  B::f: " . B::$f . "\n";
    print "  B::g: " . B::$g . "\n";
    print "\n";
    print "  C::a: " . C::$a . "\n";
    print "  C::b: " . C::$b . "\n";
#    print "  C::c: " . C::$c . "\n";
    print "  C::d: " . C::$d . "\n";
    print "  C::e: " . C::$e . "\n";
#    print "  C::f: " . C::$f . "\n";
    print "  C::g: " . C::$g . "\n";
    print "  C::h: " . C::$h . "\n";
  }
}

function main() {
  $a = new A;
  $a->aFunc();

  $b = new B;
  $b->aFunc();
  $b->bFunc();

  $c = new C;
  $c->aFunc();
  $c->bFunc();
  $c->cFunc();

  print "isset(C::\$h): ".(isset(C::$h)?"true":"false")."\n";
  print "empty(C::\$h): ".(empty(C::$h)?"true":"false")."\n";

  print "isset(C::\$i): ".(isset(C::$i)?"true":"false")."\n";
  print "empty(C::\$i): ".(empty(C::$i)?"true":"false")."\n";

  print "C::\$h: ".C::$h."\n";

  C::$h = 42;
  print "C::\$h: ".C::$h."\n";

  C::$h += 42;
  print "C::\$h: ".C::$h."\n";

  print "C::\$h: ".++C::$h."\n";
  print "C::\$h: ".C::$h++."\n";
  print "C::\$h: ".C::$h--."\n";
  print "C::\$h: ".--C::$h."\n";

  $x = 1234;
  C::$h =& $x;
  print "C::\$h: ".C::$h."\n";
  $x++;
  print "C::\$h: ".C::$h."\n";
  C::$h = 5678;
  print "x: ".$x."\n";

  C::$h = array(0, 1, 2);
  $y = C::$h[1];
  print "\$y: $y\n";
  C::$h[2] = 42;

  $y = C::$h[2];
  print "\$y: $y\n";

  print "Test end\n";
}

class D {
  static function main() {
    $a = new A;
    $a->aFunc();

    $b = new B;
    $b->aFunc();
    $b->bFunc();

    $c = new C;
    $c->aFunc();
    $c->bFunc();
    $c->cFunc();

    print "isset(C::\$h): ".(isset(C::$h)?"true":"false")."\n";
    print "empty(C::\$h): ".(empty(C::$h)?"true":"false")."\n";

    print "isset(C::\$i): ".(isset(C::$i)?"true":"false")."\n";
    print "empty(C::\$i): ".(empty(C::$i)?"true":"false")."\n";

    print "C::\$h: ".C::$h."\n";

    C::$h = 42;
    print "C::\$h: ".C::$h."\n";

    C::$h += 42;
    print "C::\$h: ".C::$h."\n";

    print "C::\$h: ".++C::$h."\n";
    print "C::\$h: ".C::$h++."\n";
    print "C::\$h: ".C::$h--."\n";
    print "C::\$h: ".--C::$h."\n";

    $x = 1234;
    C::$h =& $x;
    print "C::\$h: ".C::$h."\n";
    $x++;
    print "C::\$h: ".C::$h."\n";
    C::$h = 5678;
    print "x: ".$x."\n";

    C::$h = array(0, 1, 2);
    $y = C::$h[1];
    print "\$y: $y\n";
    C::$h[2] = 42;
    $y = C::$h[2];
    print "\$y: $y\n";

    C::$h = 20;
    $w =& C::$h;
    $w = 5;
    print "C::\$h: ".C::$h."\n";

    print "Test end\n";
  }
}

main();
D::main();
