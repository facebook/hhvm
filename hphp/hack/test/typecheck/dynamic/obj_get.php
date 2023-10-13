<?hh

function testObjGet(dynamic $x): void {
  $y = $x::staticProp; // $y : dynamic
  hh_show($y);
  $y = $x->property; // $y : dynamic
  hh_show($y);
  $x->property = 7;
}
