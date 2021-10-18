<?hh

class C {}

function expect_vector_c(Vector<C> $v) : void {}
function expect_vector_c_or_int(Vector<(C | int)> $v) : void {}

function test_Vector(~C $lc, dynamic $d, Vector<C> $v, ~int $i) : Vector<C> {
  hh_show($v[0]);

  $v[0] = new C();
  $v[0] = $d;
  $v[0] = $lc;

  $v[] = new C();
  $v[] = $d;
  $v[] = $lc;

  $w1 = Vector<C>{$d};
  expect_vector_c($w1);
  $w2 = Vector{$d};
  expect_vector_c($w2);
  $w3 = Vector{$lc};
  expect_vector_c($w3);
  $w4 = Vector{new C()};
  expect_vector_c($w4);
  $w5 = Vector{$d, $i, $lc};
  expect_vector_c_or_int($w5);
  return Vector{$d};
}
