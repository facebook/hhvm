<?hh // partial

class A {
  <<__Rx, __Mutable>>
  public function m(): void {
  }
}

<<__Rx>>
function f(Rx<(function(OwnedMutable<A>): void)> $a): void {
  $z = \HH\Rx\mutable(new A());
  // OK
  $a(\HH\Rx\move($z));
}

<<__Rx>>
function g():void {
  // OK
  f(<<__Rx>>(<<__OwnedMutable>> A $a) ==> {
    $a->m();
  });
  // OK
  f((<<__OwnedMutable>> A $a) ==> {
    $a->m();
  });
  // OK
  f((<<__MaybeMutable>> A $a) ==> {
  });
}
