<?hh

class TestTraversable{
  public function i(Traversable<int> $i): void {}
}

function i(Traversable<int> $i): void {}

function call_traversable(
  Traversable<float> $i,
): void {
  /* HH_FIXME[4110] */
  i($i);

  $foo = new TestTraversable();

  /* HH_FIXME[4110] */
  $foo->i($i);
}

function call_traversable_cast(
  Traversable<float> $i,
): void {
  /* HH_FIXME[4417] */
  i(\HH\FIXME\UNSAFE_CAST<Traversable<float>,Traversable<int>>($i));

  $foo = new TestTraversable();

  /* HH_FIXME[4417] */
  $foo->i(\HH\FIXME\UNSAFE_CAST<Traversable<float>,Traversable<int>>($i));
}
