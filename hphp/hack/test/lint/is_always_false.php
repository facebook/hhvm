<?hh // strict

class A {}
abstract class B {}
class C extends B {
  public function f(): void {}
}

function test(A $a): void {
  if ($a is C) {
    $a->f();
  }
}
