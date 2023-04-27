<?hh

class Foo {}

function test(dynamic $x, string $y): void {
  $w = null;

  $r1 = $x::$x();
  $r2 = $x::$y();
  $r3 = $x::$z(); // $z undefined
  $r4 = $x::$w();

  $r5 = Foo::$x();
  $r6 = Foo::$y();
  $r7 = Foo::$z(); // $z undefined
  $r8 = Foo::$w();

  hh_show(tuple($r1, $r2, $r3, $r4, $r5, $r6, $r7, $r8));
}
