<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Base<Tb> {}
class C extends Base<string> {}
interface I<-T> {}
interface K extends I<C> {}
interface J<T> extends I<T> {}
interface H<-Th as K> extends I<Th> {}
function test<Tt>(I<C> $x, I<Tt> $g, Base<Tt> $b, I<Tt> $z): void {
  $xold = $x;
  $gold = $g;
  $bold = $b;
  if ($x is J<_>) {
    hh_show($x);
    hh_show($xold);
  }
  if ($g is K) {
    hh_show($g);
    hh_show($gold);
  }
  if ($b is C) {
    hh_show($b);
    hh_show($bold);
  }
  if ($z is H<_>) {
    hh_show($z);
  }
}
