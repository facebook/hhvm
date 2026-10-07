<?hh

class Cov<+T as arraykey> { }

function test1(mixed $m):void {
  $m as Cov<_>;
  hh_show($m);
}

function test2(mixed $m):void {
  $m as shape('a' => Cov<_>);
  hh_show($m);
}

function test3(mixed $m):void {
  $m as (Cov<_>, int);
  hh_show($m);
}

function test4(mixed $m):void {
  $m as ?Cov<_>;
  hh_show($m);
}

function test5(mixed $m):void {
  $m as ?(Cov<_>, int);
  hh_show($m);
}
