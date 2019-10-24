<?hh // strict

function testObjGet(dynamic $x): void {
  $z = 5;
  hh_show($z);
  $y = $x(inout $z);
  hh_show($y);
  hh_show($z);
  $w = "5";
  hh_show($w);
  $y = $x->method(inout $w);
  hh_show($y);
  hh_show($w);
}
