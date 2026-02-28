<?hh

class C {}
type T1 = shape(nameof C => float);
type T2 = shape(C::class => float);

function expect_float(float $f): void {}
function expect_int(int $f): void {}

function f(T1 $t1, T2 $t2): void {
  hh_show($t1);

  expect_float($t1[nameof C]);
  expect_float($t1[C::class]);
  expect_int($t1[nameof C]);
  expect_int($t1[C::class]);

  expect_float($t2[nameof C]);
  expect_float($t2[C::class]);
  expect_int($t2[nameof C]);
  expect_int($t2[C::class]);
}
