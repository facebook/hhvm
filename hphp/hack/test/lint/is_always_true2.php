<?hh

interface I {
  public function f(): void;
}
class A implements I {
  <<__Override>>
  public function f(): void {}
}

function test(A $a): void {
  if ($a is I) {
    $a->f();
  }
}
