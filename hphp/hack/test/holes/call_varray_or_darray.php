<?hh

class TestVarrayOrDarray{
  public function d(varray_or_darray<int> $d): void {}
}

function d(varray_or_darray<int> $d): void {}

function call_varray_or_darray(
  varray_or_darray<float> $d,
): void {
  /* HH_FIXME[4110] */
  d($d);

  $foo = new TestVarrayOrDarray();

  /* HH_FIXME[4110] */
  $foo->d($d);
}

function call_varray_or_darray_cast(
  varray_or_darray<float> $d,
): void {
  /* HH_FIXME[4417] */
  d(unsafe_cast<varray_or_darray<float>,varray_or_darray<int>>($d));

  $foo = new TestVarrayOrDarray();

  /* HH_FIXME[4417] */
  $foo->d(unsafe_cast<varray_or_darray<float>,varray_or_darray<int>>($d));
}
