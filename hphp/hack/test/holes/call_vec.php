<?hh

class TestVec{
  public function e(vec<int> $e): void {}
}

function e(vec<int> $e): void {}

function call_vec(
  vec<float> $e,
): void {
  /* HH_FIXME[4110] */
  e($e);

  $foo = new TestVec();

  /* HH_FIXME[4110] */
  $foo->e($e);
}

function call_vec_cast(
  vec<float> $e,
): void {
  /* HH_FIXME[4417] */
  e(\HH\FIXME\UNSAFE_CAST<vec<float>,vec<int>>($e));

  $foo = new TestVec();

  /* HH_FIXME[4417] */
  $foo->e(\HH\FIXME\UNSAFE_CAST<vec<float>,vec<int>>($e));
}
