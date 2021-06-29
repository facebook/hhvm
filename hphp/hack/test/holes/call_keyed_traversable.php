<?hh

class TestKeyedTraversable{
  public function o(KeyedTraversable<int, int> $o): void {}
}

function o(KeyedTraversable<int, int> $o): void {}

function call_keyed_traversable(
  KeyedTraversable<string, int> $o1,
  KeyedTraversable<int, string> $o2,
): void {
  /* HH_FIXME[4110] */
  o($o1);
  /* HH_FIXME[4110] */
  o($o2);

  $foo = new TestKeyedTraversable();

  /* HH_FIXME[4110] */
  $foo->o($o1);
  /* HH_FIXME[4110] */
  $foo->o($o2);
}

function call_keyed_traversable_cast(
  KeyedTraversable<string, int> $o1,
  KeyedTraversable<int, string> $o2,
): void {
  /* HH_FIXME[4417] */
  o(\HH\FIXME\UNSAFE_CAST<KeyedTraversable<string,int>,KeyedTraversable<int,int>>($o1));
  /* HH_FIXME[4417] */
  o(\HH\FIXME\UNSAFE_CAST<KeyedTraversable<int,string>,KeyedTraversable<int,int>>($o2));

  $foo = new TestKeyedTraversable();

  /* HH_FIXME[4417] */
  $foo->o(\HH\FIXME\UNSAFE_CAST<KeyedTraversable<string,int>,KeyedTraversable<int,int>>($o1));
  /* HH_FIXME[4417] */
  $foo->o(\HH\FIXME\UNSAFE_CAST<KeyedTraversable<int,string>,KeyedTraversable<int,int>>($o2));
}
