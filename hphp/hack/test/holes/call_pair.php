<?hh

class TestPair{
  public function b(Pair<int, int> $b): void {}
}

function b(Pair<int, int> $b): void {}

function call_pair(
  Pair<bool, int> $b1,
  Pair<int, bool> $b2,
): void {
  /* HH_FIXME[4110] */
  b($b1);
  /* HH_FIXME[4110] */
  b($b2);

  $foo = new TestPair();

  /* HH_FIXME[4110] */
  $foo->b($b1);
  /* HH_FIXME[4110] */
  $foo->b($b2);
}

function call_pair_cast(
  Pair<bool, int> $b1,
  Pair<int, bool> $b2,
): void {
  /* HH_FIXME[4417] */
  b(\HH\FIXME\UNSAFE_CAST<Pair<bool,int>,Pair<int,int>>($b1));
  /* HH_FIXME[4417] */
  b(\HH\FIXME\UNSAFE_CAST<Pair<int,bool>,Pair<int,int>>($b2));

  $foo = new TestPair();

  /* HH_FIXME[4417] */
  $foo->b(\HH\FIXME\UNSAFE_CAST<Pair<bool,int>,Pair<int,int>>($b1));
  /* HH_FIXME[4417] */
  $foo->b(\HH\FIXME\UNSAFE_CAST<Pair<int,bool>,Pair<int,int>>($b2));
}
