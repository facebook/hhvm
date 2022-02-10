<?hh

interface I {
  public function bar():void { }
}

final class A {
  public function f(): void {}
}

function test(A $a): void {
  if ($a is I) {
    $a->f();
  }
}
