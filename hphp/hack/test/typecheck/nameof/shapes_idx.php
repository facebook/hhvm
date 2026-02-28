<?hh

class C {}
type T1 = shape(?nameof C => float);
type T2 = shape(?C::class => float);

function expect_nfloat(?float $f): void {}

function f(T1 $t1, T2 $t2): void {
  expect_nfloat(Shapes::idx($t1, nameof C));
  expect_nfloat(Shapes::idx($t1, C::class));
  expect_nfloat(Shapes::idx($t2, nameof C));
  expect_nfloat(Shapes::idx($t2, C::class));
}
