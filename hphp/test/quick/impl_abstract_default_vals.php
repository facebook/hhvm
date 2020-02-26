<?hh
// check default values and type hints, implementating abstract methods: pass
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
  abstract public function a(?varray $a1 = null, varray $a2 = varray[]);
  abstract public function b(bool $b1 = null);
  abstract public function c(A $c1 = null);
  abstract public function d(float $d1 = null, float $d2 = 2.0);
  abstract public function f(float  $f1 = null, float  $f2 = 2.0);
  abstract public function i(int $i1 = A::i0, int  $i2 = 2);
  abstract public function s(string $s1 = null, string $s2 = A::sabc);
}

class B extends A {
  const btrue = true;
  const d3 = 3.0;
  const ibig = 9123123123123;
  public function a(?varray $a1 = null, varray $a2 = varray[], varray $a3 = varray[1, 2, 3], varray $a4 = varray["abc"]) {}
  public function b(bool $b1 = null, bool $b2 = false, bool  $b3 = DTRUE, bool $b4 = B::btrue) {}
  public function c(A       $c1 = null) {}
  public function d(float  $d1 = null, float  $d2 = 1.0) {} // , double $d3 = DD2, double $d4 = B::d3) {}
  public function f(float   $f1 = null, float   $f2 = 1.0, float  $f3 = DD2, float  $f4 = A::f3) {}
  public function i(int     $i1 = null, int $i2 = B::ibig,   int $i3 = D2, int $i4 = A::i0) {}
  public function s(string  $s1 = null, string  $s2 = "S", string $s3 = DABC, string $s4 = A::sabc) {}
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
