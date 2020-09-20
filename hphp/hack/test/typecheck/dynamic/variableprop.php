<?hh // strict

function testDynamicProp(dynamic $x, string $y): void {
  $z = $x->$y();
  hh_show($z);
  $z = $x->$y;
  hh_show($z);
}
