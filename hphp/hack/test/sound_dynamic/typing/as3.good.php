<?hh

class C<T> {}
<<__SupportDynamicType>> class D<T> extends C<T> {}

function expect_like_d_int(~D<int> $d) : void {}

function test(~C<int> $x) : void {
  if ($x is D<_>) {
    expect_like_d_int($x);
  }
  $x as D<_>;
  expect_like_d_int($x);
}
