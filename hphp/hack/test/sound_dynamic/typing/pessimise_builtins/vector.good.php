<?hh

class C {}

function f() : ~string {
  return "1";
}

function test_Vector(vec<~C> $vlc, dynamic $d, Vector<C> $v, vec<~int> $vi) : Vector<C> {
  $lc = $vlc[0];
  $i = $vi[0];
  hh_expect_equivalent<~C>($v[0]);

  $v[0] = new C();
  $v[0] = $d;
  $v[0] = $lc;

  $v[] = new C();
  $v[] = $d;
  $v[] = $lc;

  Vector<string>{ f() };

  $w1 = Vector<C>{$d};
  hh_expect<Vector<C>>($w1);
  $w2 = Vector{$d};
  hh_expect<Vector<C>>($w2);
  $w3 = Vector{$lc};
  hh_expect<Vector<C>>($w3);
  $w4 = Vector{new C()};
  hh_expect<Vector<C>>($w4);
  $w5 = Vector{$d, $i, $lc};
  hh_expect<Vector<(C|int)>>($w5);
  return Vector{$d};
}
