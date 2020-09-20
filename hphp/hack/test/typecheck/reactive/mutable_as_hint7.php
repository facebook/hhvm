<?hh // partial

class A {
  <<__Rx, __MaybeMutable>>
  public function m(): void {
  }
}

<<__Rx>>
function f(Rx<(function(MaybeMutable<A>): void)> $a, <<__Mutable>> A $b): void {
  $z = \HH\Rx\mutable(new A());
  // OK (owned)
  $a(\HH\Rx\move($z));
  // OK (borrowed)
  $a($b);
  // OK (immutable)
  $a(new A());
}

<<__Rx>>
function g():void {
  $v = \HH\Rx\mutable(new A());
  // OK
  f(<<__Rx>>(<<__MaybeMutable>> A $a) ==> {
    $a->m();
  }, $v);
  // OK
  f((<<__MaybeMutable>> A $a) ==> {
    $a->m();
  }, $v);
  // OK
  f((<<__MaybeMutable>> A $a) ==> {
  }, $v);
}
