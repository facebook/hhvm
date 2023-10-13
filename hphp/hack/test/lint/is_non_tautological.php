<?hh

abstract class A {
  public function a(): void {}
}
abstract class B extends A {}
class C extends B {
  public function c(): void {}
}
interface I {
  public function i(): void {}
}
interface J {
  public function j(): void {}
}

function test1(B $b): void {
  if ($b is C) {
    $b->c();
  }
}

function test2(C $c): void {
  if ($c is I) {
    $c->i();
  }
}

function test3(I $i): void {
  if ($i is C) {
    $i->c();
  }
}

function test4(I $i): void {
  if ($i is J) {
    $i->j();
  }
}
