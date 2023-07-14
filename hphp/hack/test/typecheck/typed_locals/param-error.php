<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

function f(int $x, int ...$y): void {
  let $x: int = 1;
  let $y: int;
}

class C {
  public function g(int $a, int ...$b): void {
    let $a: int;
    let $b: int = 1;
    let $this: C;
  }
}
