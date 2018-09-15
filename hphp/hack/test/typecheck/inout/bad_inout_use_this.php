<?hh // strict

function f(inout ?C $c): void {
  $c = null;
}

class C {
  public function bar(): void {
    f(inout $this);
  }
}
