<?hh

class C<+T1, T2, -T3> {
  public function foo(T3 $_, nothing $n): T1 { return $n; }
}
class D<+T1, -T3> extends C<T1, int, T3> {
  public function bar(T3 $_, nothing $n): T1 { return $n; }
}
class E extends D<bool, string> {}
class F<T1, reify T2> {}

function redundant<<<__Explicit>> T1, T2, <<__Explicit>> T3>(
  C<int, string, bool> $c1,
  C<T1, T2, T3> $c2,
  E $e,
  F<int, string> $f,
): void {
  $c1 as C<_,_,_>;
  $c2 as C<_,_,_>;
  new C() as C<_,_,_>;
  new D() as C<_,_,_>;
  new E() as C<_,_,_>;
  $e as C<_,_,_>;
  $f as F<_, _>;
}

function not_redundant(
  F<int, string> $f1,
  F<int, string> $f2,
  C<int, string, mixed> $c1,
  C<int, string, mixed> $c2
): void {
  $f1 as F<_, bool>;
  $f2 as F<float, bool>;
  new C() as E;
  new C() as D<_,_>;
  new $c1 as E;
  new $c2 as D<_,_>;
}
