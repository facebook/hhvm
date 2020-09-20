<?hh // partial

class A {
  <<__Rx, __Mutable>>
  public function m(): void {
  }
}

<<__Rx>>
function f(Rx<(function(Mutable<A>): void)> $a): void {
  // OK
  $a(\HH\Rx\mutable(new A()));
}

<<__Rx>>
function g():void {
  // ERROR
  f(<<__Rx>>(A $a) ==> {
  });
}
