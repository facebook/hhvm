<?hh
// check default/optional values and type hints, implementating abstract methods: pass
// adapted from impl_abstract_default_vals.php
const D0 = 0;
const D1 = 1;
const D2 = 2;
const DD2 = 2.0;
const DNULL = null;
const DABC = 'abc';
const DTRUE = true;

abstract class A {
  const i0 = 0;
  const sabc = 'abc';
  const f3 = 3.0;
  abstract public function a(optional ?varray $a1, optional varray $a2):mixed;
  abstract public function b(optional ?bool $b1):mixed;
  abstract public function c(optional ?A $c1):mixed;
  abstract public function d(optional ?float $d1, optional float $d2):mixed;
  abstract public function f(optional ?float  $f1, optional float  $f2):mixed;
  abstract public function i(optional int $i1, optional int  $i2):mixed;
  abstract public function s(optional ?string $s1, optional string $s2):mixed;
}

class B extends A {
  const btrue = true;
  const d3 = 3.0;
  const ibig = 9123123123123;
  public function a(?varray $a1 = null, varray $a2 = vec[], varray $a3 = vec[1, 2, 3], varray $a4 = vec["abc"]) :mixed{}
  public function b(?bool $b1 = null, bool $b2 = false, bool  $b3 = DTRUE, bool $b4 = B::btrue) :mixed{}
  public function c(?A       $c1 = null) :mixed{}
  public function d(?float  $d1 = null, float  $d2 = 1.0) :mixed{} // , double $d3 = DD2, double $d4 = B::d3) {}
  public function f(?float   $f1 = null, float   $f2 = 1.0, float  $f3 = DD2, float  $f4 = A::f3) :mixed{}
  public function i(?int     $i1 = null, int $i2 = B::ibig,   int $i3 = D2, int $i4 = A::i0) :mixed{}
  public function s(?string  $s1 = null, string  $s2 = "S", string $s3 = DABC, string $s4 = A::sabc) :mixed{}
}
<<__EntryPoint>> function main(): void {
$b = new B();
$b->a();
$b->b();
$b->c();
$b->d();
$b->i();
$b->s();
$b->f();

echo "Pass\n";
}
