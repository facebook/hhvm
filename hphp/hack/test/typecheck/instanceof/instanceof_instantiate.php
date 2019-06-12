<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C1 {}
class C2 {}
class C3 {}
class C4 implements I<C4> {}
interface I<+T> {}
class C<+TCpos, TCinv, -TCneg, +TCi as I<TCi>> {}
class D<+TDpos, TDinv, -TDneg, +TDi as I<TDi>>
  extends C<TDpos, TDinv, TDneg, TDi> {}
function testit(C<C1, C2, C3, C4> $c): void {
  if ($c is D<_, _, _, _>) {
    // Can't observe simplification of type of $c unless we show it!
    hh_show($c);
  }
}
class G<T1, T2, T3, T4 as I<T4>> {
  public function testit2(C<T1, T2, T3, T4> $c): void {
    if ($c is D<_, _, _, _>) {
      // Can't observe simplification of type of $c unless we show it!
      hh_show($c);
    }
  }
  public function testit3(?C<T1, T2, T3, T4> $c): void {
    if ($c is D<_, _, _, _>) {
      hh_show($c);
    }
  }
}
