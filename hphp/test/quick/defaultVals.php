<?hh
// check a variety of acceptable default values and type hints: pass
const D0 = 0;
const D1 = 1;
const D2 = 2;
const DD2 = 2.0;
const DABC =  "abc";
const DTRUE = true;

class A {
  const foo = 'ABC';
  public function a(?varray  $a0 = null, varray  $a1 = vec[], varray  $a2 = vec[1, 2, 3]) :mixed{}
  public function b(?bool   $b0 = null, bool $b1 = true, bool $b2 = false, bool $b3 = DTRUE) :mixed{}
  public function d(?float $d0 = null, float $d1 = 1.0, float $d2 = DD2, float $d3 = 3.0) :mixed{}
  public function f(?float  $f0 = null, float  $f1 = 1.0, float  $f2 = DD2, float  $f3 = 3.0) :mixed{}
  public function i(?int    $i0 = null, int $i1 = 1, int $i2 = 2, int $i3 = D1) :mixed{}
  public function s(?string $s0 = null, string $s1 = " ", string $s2 = DABC, string $s3 = A::foo) :mixed{}
  public function c(?A      $c0 = null) :mixed{}
  public function z($z0 = null, $z1 = 1, $z2 = 2.0, $z3 = "abc", $z4 = DABC, $z5 = true) :mixed{}
}

interface I {
  public function a(int   $a1 = 0):mixed;
  public function b(?AnyArray $b1 = null):mixed;
}

class B implements I {
  public function a(?int   $a1 = null, int $z1 = 1, int $z2 = D2, int $z3 = 3, ?int $z4 = null) :mixed{}
  public function b(?varray $b1 = null, varray $y = vec[], int $z1 = 1, int $z2 = 2, float $z3 = 3.0, string $z4 = "abc") :mixed{}
}
<<__EntryPoint>> function main(): void {
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
}
