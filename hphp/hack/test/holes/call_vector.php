<?hh

class TestVector{
  public function g(Vector<int> $g): void {}
}

function g(Vector<int> $g): void {}

function call_vector(
  Vector<float> $g,
): void {
  /* HH_FIXME[4110] */
  g($g);

  $foo = new TestVector();

  /* HH_FIXME[4110] */
  $foo->g($g);
}

function call_vector_cast(
  Vector<float> $g,
): void {
  /* HH_FIXME[4417] */
  g(\HH\FIXME\UNSAFE_CAST<Vector<float>,Vector<int>>($g));

  $foo = new TestVector();

  /* HH_FIXME[4417] */
  $foo->g(\HH\FIXME\UNSAFE_CAST<Vector<float>,Vector<int>>($g));
}
