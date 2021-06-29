<?hh

class TestIterable{
  public function m(Iterable<int> $m): void {}
}

function m(Iterable<int> $m): void {}

function call_iterable(
  Iterable<float> $m,
): void {
  /* HH_FIXME[4110] */
  m($m);

  $foo = new TestIterable();

  /* HH_FIXME[4110] */
  $foo->m($m);
}

function call_iterable_cast(
  Iterable<float> $m,
): void {
  /* HH_FIXME[4417] */
  m(\HH\FIXME\UNSAFE_CAST<Iterable<float>,Iterable<int>>($m));

  $foo = new TestIterable();

  /* HH_FIXME[4417] */
  $foo->m(\HH\FIXME\UNSAFE_CAST<Iterable<float>,Iterable<int>>($m));
}
