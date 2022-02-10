<?hh

class A {
  public function f(): void {}
}

function test(A $a): void {
  $a as A;
}
