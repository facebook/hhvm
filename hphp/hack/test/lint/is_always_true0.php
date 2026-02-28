<?hh

class A {
  public function f(): void {}
}

function test(A $a): void {
  if ($a is A) {
    $a->f();
  }
}
