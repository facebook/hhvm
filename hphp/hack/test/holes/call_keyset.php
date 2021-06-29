<?hh

class TestKeyset{
  public function f(keyset<int> $f): void {}
}

function f(keyset<int> $f): void {}

function call_keyset(
  keyset<string> $f,
): void {
  /* HH_FIXME[4110] */
  f($f);

  $foo = new TestKeyset();

  /* HH_FIXME[4110] */
  $foo->f($f);
}

function call_keyset_cast(
  keyset<string> $f,
): void {
  /* HH_FIXME[4417] */
  f(\HH\FIXME\UNSAFE_CAST<keyset<string>,keyset<int>>($f));

  $foo = new TestKeyset();

  /* HH_FIXME[4417] */
  $foo->f(\HH\FIXME\UNSAFE_CAST<keyset<string>,keyset<int>>($f));
}
