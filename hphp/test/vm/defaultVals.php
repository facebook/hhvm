<?php
// check a variety of acceptable default values and type hints: pass
define('D0',0);
define('D1',1);
define('D2',2);
define('DD2',2.0);
define('DABC',  "abc");
define('DTRUE', true);

class A {
  const foo = 'ABC';
  public function a(array  $a0 = null, array  $a1 = array(), Array  $a2 = array(1, 2, 3)) {}
  public function b(bool   $b0 = NULL, boolean $b1 = true, bool $b2 = false, Boolean $b3 = DTRUE) {}
  public function d(double $d0 = null, double $d1 = 1.0, double $d2 = DD2, Double $d3 = 3.0) {}
  public function f(float  $f0 = null, float  $f1 = 1.0, float  $f2 = DD2, Float  $f3 = 3.0) {}
  public function i(int    $i0 = null, integer $i1 = 1, Integer $i2 = 2, Int $i3 = D1) {}
  public function s(string $s0 = null, string $s1 = " ", String $s2 = DABC, String $s3 = A::foo) {}
  public function c(A      $c0 = null) {}
  public function z($z0 = null, $z1 = 1, $z2 = 2.0, $z3 = "abc", $z4 = DABC, $z5 = true) {}
}

interface I {
  public function a(int   $a1 = 0);
  public function b(array $b1 = null);
}

class B implements I {
  public function a(int   $a1 = null, int $z1 = 1, int $z2 = D2, int $z3 = 3, int $z4 = null) {}
  public function b(array $b1 = null, array $y = array(), int $z1 = 1, int $z2 = 2, double $z3 = 3.0, string $z4 = "abc") {}
}

$a = new A();
$a->a();
$a->b();
$a->d();
$a->f();
$a->i();
$a->s();
$a->c();
$a->c($a);
$a->c(null);
$a->z();

$b = new B();
$b->a();
$b->b();

print "Pass\n";

