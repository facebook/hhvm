<?hh // strict
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
  if ($x instanceof J) {
    hh_show($x);
    hh_show($xold);
  }
  if ($g instanceof K) {
    hh_show($g);
    hh_show($gold);
  }
  if ($b instanceof C) {
    hh_show($b);
    hh_show($bold);
  }
  if ($z instanceof H) {
    hh_show($z);
  }
}
