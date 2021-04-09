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
  i(unsafe_cast<Traversable<float>,Traversable<int>>($i));

  $foo = new TestTraversable();

  /* HH_FIXME[4417] */
  $foo->i(unsafe_cast<Traversable<float>,Traversable<int>>($i));
}
