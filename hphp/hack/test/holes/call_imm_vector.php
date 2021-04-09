<?hh

class TestImmVector{
  public function k(ImmVector<int> $k): void {}
}

function k(ImmVector<int> $k): void {}

function call_immvector(
  ImmVector<float> $k,
): void {
  /* HH_FIXME[4110] */
  k($k);

  $foo = new TestImmVector();

  /* HH_FIXME[4110] */
  $foo->k($k);
}

function call_immvector_cast(
  ImmVector<float> $k,
): void {
  /* HH_FIXME[4417] */
  k(unsafe_cast<ImmVector<float>,ImmVector<int>>($k));

  $foo = new TestImmVector();

  /* HH_FIXME[4417] */
  $foo->k(unsafe_cast<ImmVector<float>,ImmVector<int>>($k));
}
