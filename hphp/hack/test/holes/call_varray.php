<?hh

class TestVarray{
  public function c(varray<int> $c): void {}
}

function c(varray<int> $c): void {}

function call_varray(
  varray<float> $c,
): void {
  /* HH_FIXME[4110] */
  c($c);

  $foo = new TestVarray();

  /* HH_FIXME[4110] */
  $foo->c($c);
}

function call_varray_cast(
  varray<float> $c,
): void {
  /* HH_FIXME[4417] */
  c(unsafe_cast<varray<float>,varray<int>>($c));

  $foo = new TestVarray();

  /* HH_FIXME[4417] */
  $foo->c(unsafe_cast<varray<float>,varray<int>>($c));
}
