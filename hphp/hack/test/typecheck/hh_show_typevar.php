<?hh // strict
class Foo {}
function test1<Ta super Tb, Tb as Foo>(Ta $x, Tb $y): void {
  hh_show($x);
  hh_show($y);
  return;
}
function test2<Ta as Tb, Tb super Foo>(Ta $x, Tb $y): void {
  hh_show($x);
  hh_show($y);
  return;
}
