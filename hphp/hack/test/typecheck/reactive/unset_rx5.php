<?hh // partial

class A {
  public ?int $v;
}

<<__Rx>>
function f(dict<int, dict<int, A>> $a): void {
  // OK
  unset($a[0][1]);
}
