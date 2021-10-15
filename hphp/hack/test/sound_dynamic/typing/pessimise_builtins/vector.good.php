<?hh

class C {}

function test_Vector(~C $lc, dynamic $d, Vector<C> $v) : Vector<C> {
  hh_show($v[0]);
  $v[0] = new C();
  $v[0] = $d;
  $v[] = new C();
  $v[] = $d;
  $w1 = Vector<C>{$d};
  hh_show($w1);
  $w2 = Vector{$d};
  hh_show($w2);
  $w3 = Vector{$lc};
  hh_show($w3);
  return Vector{$d};
}
