<?hh // partial

class A {
  <<__Rx, __Mutable>>
  public function m(): void {
  }
}

<<__Rx>>
function f(Rx<(function(MaybeMutable<A>): void)> $a): void {
  $a(new A());
}

<<__Rx>>
function g():void {
  f((<<__Mutable>> A $a) ==> {
    $a->m();
  });
}
