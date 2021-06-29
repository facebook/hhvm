<?hh

class TestTuple {
  public function a((int, int) $a): void {}
}

function a((int, int) $a): void {}

function call_tuple(
  (int, bool) $a1,
  (bool, int) $a2,
): void {
  /* HH_FIXME[4110] */
  a($a1);
  /* HH_FIXME[4110] */
  a($a2);

  $foo = new TestTuple();

  /* HH_FIXME[4110] */
  $foo->a($a1);
  /* HH_FIXME[4110] */
  $foo->a($a2);
}

function call_tuple_cast(
  (int, bool) $a1,
  (bool, int) $a2,
): void {
  /* HH_FIXME[4417] */
  a(\HH\FIXME\UNSAFE_CAST<(int,bool),(int,int)>($a1));
  /* HH_FIXME[4417] */
  a(\HH\FIXME\UNSAFE_CAST<(bool,int),(int,int)>($a2));

  $foo = new TestTuple();

  /* HH_FIXME[4417] */
  $foo->a(\HH\FIXME\UNSAFE_CAST<(int,bool),(int,int)>($a1));
  /* HH_FIXME[4417] */
  $foo->a(\HH\FIXME\UNSAFE_CAST<(bool,int),(int,int)>($a2));
}
