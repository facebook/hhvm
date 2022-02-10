<?hh

abstract class A {
  public function f(): void {}
}
class B extends A {}

function test(B $b): void {
  $b as A;
}
