<?hh

class TestSet{
  public function h(Set<int> $h): void {}
}

function h(Set<int> $h): void {}

function call_set(
  Set<string> $h,
): void {
  /* HH_FIXME[4110] */
  h($h);

  $foo = new TestSet();

  /* HH_FIXME[4110] */
  $foo->h($h);
}

function call_set_cast(
  Set<string> $h,
): void {
  /* HH_FIXME[4417] */
  h(unsafe_cast<Set<string>,Set<int>>($h));

  $foo = new TestSet();

  /* HH_FIXME[4417] */
  $foo->h(unsafe_cast<Set<string>,Set<int>>($h));
}
