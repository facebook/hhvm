<?hh // strict

abstract class A {
  public function f(): void {}
}
class B extends A {}

function test(B $b): void {
  if ($b is A) {
    $b->f();
  }
}
