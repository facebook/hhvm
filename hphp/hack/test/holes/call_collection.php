<?hh

class TestCollection{
  public function l(Collection<int> $l): void {}
}

function l(Collection<int> $l): void {}

function call_collection(
  Collection<string> $l,
): void {
  /* HH_FIXME[4110] */
  l($l);

  $foo = new TestCollection();

  /* HH_FIXME[4110] */
  $foo->l($l);
}

function call_collection_cast(
  Collection<string> $l,
): void {
  /* HH_FIXME[4417] */
  l(unsafe_cast<Collection<string>,Collection<int>>($l));

  $foo = new TestCollection();

  /* HH_FIXME[4417] */
  $foo->l(unsafe_cast<Collection<string>,Collection<int>>($l));
}
