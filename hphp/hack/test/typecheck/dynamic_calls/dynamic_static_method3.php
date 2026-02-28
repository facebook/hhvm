<?hh

class Foo {}

function test(dynamic $x, string $y): void {
  $w = null;

  $r1 = $x::$x();
  $r2 = $x::$y();
  $r3 = $x::$z(); // $z undefined
  $r4 = $x::$w();

  hh_show(tuple($r1, $r2, $r3, $r4));
}
