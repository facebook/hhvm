<?php
// check default values and type hints, implementating abstract methods: pass
define('D0',0);
define('D1',1);
define('D2',2);
define('DD2',2.0);
define('DNULL',null);
define('DABC','abc');
define('DTRUE',true);

abstract class A {
  const i0 = 0;
  const sabc = 'abc';
  const f3 = 3.0;
  abstract public function a(array $a1 = null, array  $a2 = array());
  abstract public function b(boolean $b1 = null);
  abstract public function c(A $c1 = null);
  abstract public function d(double $d1 = null, double $d2 = 2.0);
  abstract public function f(float  $f1 = null, float  $f2 = 2.0);
  abstract public function i(int $i1 = A::i0, Integer  $i2 = 2);
  abstract public function s(string $s1 = null, string $s2 = A::sabc);
}

class B extends A {
  const btrue = true;
  const d3 = 3.0;
  const ibig = 9123123123123;
  public function a(array   $a1 = null, Array   $a2 = array(), array $a3 = array(1, 2, 3), array $a4 = array("abc")) {}
  public function b(boolean $b1 = null, boolean $b2 = false, boolean  $b3 = DTRUE, Boolean $b4 = B::btrue) {}
  public function c(A       $c1 = null) {}
  public function d(double  $d1 = null, double  $d2 = 1.0) {} // , double $d3 = DD2, double $d4 = B::d3) {}
  public function f(float   $f1 = null, float   $f2 = 1.0, float  $f3 = DD2, Float  $f4 = A::f3) {}
  public function i(int     $i1 = null, Integer $i2 = B::ibig,   int $i3 = D2, Int $i4 = A::i0) {}
  public function s(string  $s1 = null, string  $s2 = "S", String $s3 = DABC, string $s4 = A::sabc) {}
}

$b = new B();
$b->a();
$b->b();
$b->c();
$b->d();
$b->i();
$b->s();
$b->f();

echo "Pass\n";

