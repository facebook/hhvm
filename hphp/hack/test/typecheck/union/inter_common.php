<?hh

class C {}
class C2 {}
class D extends C2{}
interface I1 {}
interface I2 {}
interface I3 {}

function test(C $c, D $d, int $x) : void {
  if ($c is I1 && $c is I2) {
    hh_show($c);
    $x = $c;
  } else if ($d is I1 && $d is I2) {
    hh_show($d);
    $x = $d;
  }
  hh_show($x);
}

function test2(C2 $c, D $d, int $x) : void {
  if ($c is I1 && $c is I2) {
    hh_show($c);
    $x = $c;
  } else if ($d is I1 && $d is I2) {
    hh_show($d);
    $x = $d;
  }
  hh_show($x);
}

function test3(C2 $c, D $d, int $x) : void {
  if ($c is I1 && $c is I2) {
    hh_show($c);
    $x = $c;
  } else if ($d is I1 && $d is I3) {
    hh_show($d);
    $x = $d;
  }
  hh_show($x);
}
