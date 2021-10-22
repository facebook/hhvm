<?hh

class C<T> {}
class D<T> extends C<T> {}

function expect_d_int (D<int> $i) : void {}

function f(dynamic $d, C<int> $c, bool $b) : AnyArray<arraykey, mixed> {
  if ($b) {
    $x = $d;
    $y = $d;
  } else {
    $x = $c;
    $y = $c;
  }

  if ($x is AnyArray<_,_>) {
   return $x;
  }
  if ($y is D<_>) {
    expect_d_int($y);
  }
  return dict[];
}
