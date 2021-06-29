<?hh

class TestImmSet{
  public function j(ImmSet<int> $j): void {}
}

function j(ImmSet<int> $j): void {}

function call_imm_set(
  ImmSet<string> $j,
): void {
  /* HH_FIXME[4110] */
  j($j);

  $foo = new TestImmSet();

  /* HH_FIXME[4110] */
  $foo->j($j);
}

function call_imm_set_cast(
  ImmSet<string> $j,
): void {
  /* HH_FIXME[4417] */
  j(\HH\FIXME\UNSAFE_CAST<ImmSet<string>,ImmSet<int>>($j));

  $foo = new TestImmSet();

  /* HH_FIXME[4417] */
  $foo->j(\HH\FIXME\UNSAFE_CAST<ImmSet<string>,ImmSet<int>>($j));
}
